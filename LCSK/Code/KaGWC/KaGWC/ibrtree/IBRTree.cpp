#include "IBRTree.h"
#include <utility>

int IBRTree::ReadLeafNodes()
{
	int total = 0;
	leaves.clear();
	ifstream leafF(leafnodeFile.c_str());
	while( !leafF.eof())
	{
		string line;
		getline(leafF, line);
		if(line == "")
			continue;			
		istringstream iss(line);
		int leafID ;
		char c;
		iss>>leafID>>c;
		vector<int> *p = new vector<int>();
		int nodes;
		int count = 0;
		while(iss>>nodes)
		{
			p->push_back(nodes);
			total ++;			
			iss>>c;
			count ++;
		}		
		leaves[leafID] = p;
	}	

	N2SG = new int[total];
	map<int, vector<int> *>::iterator iter;
	for(iter = leaves.begin(); iter != leaves.end() ; ++ iter)
	{
		vector<int> *p = iter->second;
		vector<int>::iterator iter2;
		for(iter2 = p->begin(); iter2 != p->end(); ++iter2)
		{
			int node = *iter2;
			N2SG[node] = iter->first;
		}
	}
	return total;
}


void IBRTree::SplitDoc()
{
	ifstream textF(textFile.c_str());
	map<int, ofstream *> sdhandlers;
	
	string line;
	int count = 0;
	while(!textF.eof())
	{
		getline(textF, line);
		if(line == "")
			continue;

		int nid = N2SG[count];
		map<int, ofstream *>::iterator si = sdhandlers.find(nid);
		ofstream *op;
		if(si == sdhandlers.end())		//if the corresponding file not exists, then create it
		{	
			if(sdhandlers.size() == 500)	//the number of files opened by a program is limited...
			{
				for(int k=0;k<50;k++)
				{
					map<int, ofstream *>::iterator replace = sdhandlers.begin();
					replace->second->close();
					sdhandlers.erase(replace);
				}
			}
			op = new ofstream((subdocFolder+MyTool::IntToString(nid)).c_str(), ios::app);
			sdhandlers[nid] = op;
		}					
		else
			op = si->second;

		(*op)<<line<<endl;
		count++;
	}
	
	map<int, ofstream *>::iterator iter = sdhandlers.begin();
	for(;iter != sdhandlers.end(); ++iter)
	{
		iter->second->close();
		delete iter->second;
	}
	textF.close();
}

void IBRTree::CreateLeafInverted()
{
	map<int, vector<int> *>::iterator iter = leaves.begin();
	for(;iter != leaves.end(); ++iter)
	{
		int leafID = iter->first;
		vector<int> *p = iter->second;
		
		map<int, vector<int> *> inverted;
		//------------------------------------------------------------------------
		//vector<vector<int> *> objectTexts;	// remove duplicated words!! one word may occur multiple times at a location
		vector<set<int> *> objectTexts;
		//------------------------------------------------------------------------

		string leafDoc = subdocFolder + MyTool::IntToString(leafID);
		ifstream docF(leafDoc.c_str());
		string line;
		while(!docF.eof())
		{
			getline(docF, line);
			if(line == "")
				continue;
			int oid, wid; char c;
			istringstream iss(line);
			iss>>oid;
			set<int> *p = new set<int>();
			while(iss>>c>>wid)
			{
				p->insert(wid);
			}
			objectTexts.push_back(p);
		}
		docF.close();

		//------------------------------------------------------------------------
		vector<int> temp(*p);
		sort(temp.begin(), temp.end());				
			//Nodes id read from Rtree is not ordered, but in subdoc they are ordered!													
		map<int, int> mapping;
		for(unsigned int i=0;i<temp.size(); i++)
		{
			mapping[temp[i]] = i;
		}		
		//------------------------------------------------------------------------

		for(unsigned int i=0;i<p->size();i++)	//for each object in this leaf node
		{
			int nid = (*p)[i];
			int idx = mapping[nid];				//get this object's position in the ordered list
			set<int> *words = objectTexts[idx];
			set<int>::iterator wi = words->begin();
			for(; wi!=words->end();++wi)		//for each word contained in this object
			{
				int wordID = *wi;
				map<int, vector<int> *>::iterator ti = inverted.find(wordID);
				if(ti != inverted.end())
				//------------------------------------------------------------------------
					inverted[wordID]->push_back(i);			//push_back the original position in the unordered list in R-tree!!
				//------------------------------------------------------------------------
				else
				{
					vector<int> *p = new vector<int>();
					p->push_back(i);
					inverted[wordID] = p;
				}
			}
		}

		ofstream outF( (invertedFolder + MyTool::IntToString(leafID)).c_str());		
				//write the object's index, instead of its ID£¡
		map<int, vector<int> *>::iterator fi;		
		for(fi = inverted.begin(); fi != inverted.end(); ++fi)
		{
			outF<<fi->first<<"\t";
			
			vector<int> *p = fi->second;
			vector<int>::iterator iter = p->begin();
			for(; iter != p->end() ; ++iter)
				outF<<*iter<<",";
			outF<<endl;
		}
		outF.close();

		vector<set<int> *>::iterator iter1;
		for(iter1 = objectTexts.begin(); iter1 != objectTexts.end(); ++iter1)
			delete *iter1;
		map<int, vector<int> *>::iterator iter2;
		for(iter2 = inverted.begin(); iter2 != inverted.end(); ++iter2)
			delete iter2->second;			
	}
}

void IBRTree::ReadIndexNodes()
{
	ifstream inodeF(indexnodeFile.c_str());
	string line;
	while( getline( inodeF, line))
	{
		if(line=="")
			continue;
		istringstream iss(line);
		int nid;char c;
		iss>>nid>>c;
		vector<int> *p = new vector<int>();
		int nodes;
		while(iss>>nodes>>c)
		{
			p->push_back(nodes);
		}
		//indexNodes.push_back(make_pair<int, vector<int> *>(nid, p));
		indexNodes.push_back(make_pair(nid, p));
	}

	inodeF.close();
}

void IBRTree::CreateNonLeafInverted()
{
	for(int idx = indexNodes.size() - 1; idx >= 0; idx--)		//for each non-leaf node. from bottom to up
	{
		map<int, vector<int> *> inverted;

		int indexID = indexNodes[idx].first;		

		vector<int> *p = indexNodes[idx].second;
		vector<int>::iterator iter = p->begin();
		int count = 0;
		for(; iter != p->end(); ++iter)							//for each its child nodes
		{
			int id = *iter;
			ifstream listF( (invertedFolder + MyTool::IntToString(id)).c_str());
			string line;
			while(getline(listF, line))
			{
				int wordID;
				istringstream iss(line);
				iss>>wordID;

				map<int, vector<int> *>::iterator iter = inverted.find(wordID);
				if(iter != inverted.end())
				{
					iter->second->push_back(count);
				}
				else
				{
					vector<int> *p = new vector<int>();
					p->push_back(count);
					inverted[wordID] = p;
				}								
			}
			listF.close();			
			count++;
		}

		ofstream outF( (invertedFolder + MyTool::IntToString(indexID)).c_str());
		map<int, vector<int> *>::iterator fi;		
		for(fi = inverted.begin(); fi != inverted.end(); ++fi)
		{
			outF<<fi->first<<"\t";
			
			vector<int> *p = fi->second;
			vector<int>::iterator iter = p->begin();
			for(; iter != p->end() ; ++iter)
				outF<<*iter<<",";
			outF<<endl;
		}
		outF.close();

		map<int, vector<int> *>::iterator iter2;
		for(iter2 = inverted.begin(); iter2 != inverted.end(); ++iter2)
			delete iter2->second;	
	}
}

void IBRTree::buildBtree(int nodeID)
{
	string btFile = btreeFolder + MyTool::IntToString(nodeID);
	ifstream leafIFL( (invertedFolder + MyTool::IntToString(nodeID)).c_str());		
		
	char *btfname = new char[btFile.size()+1];
	memcpy(btfname, btFile.c_str(), btFile.size());
	btfname[btFile.size()] = '\0';
	BTree* bt=new BTree(btfname,btreePageSize, 0);			

	string line;
	while( getline(leafIFL, line) )
	{				
		int wordID; char c;
		istringstream iss(line);
		iss>>wordID;
		DATA *p = new DATA();
		p->key = wordID;
		
		int indexID;
		while(iss>>indexID>>c)
		{
			int bytePos = indexID/8;
			int bitPos = indexID % 8;
			unsigned char mask = 1 << bitPos;
			p->data[bytePos] = p->data[bytePos] | mask;
		}
		bt->insert(p);
		delete p;
	}
	leafIFL.close();
	delete bt;
	delete btfname;
}

void IBRTree::BuildBTIndex()
{
 	map<int, vector<int> *>::iterator iterLeaf = leaves.begin();	

	for(; iterLeaf != leaves.end(); iterLeaf++)
	{
		int leafID = iterLeaf->first;
		buildBtree(leafID);	
	}

	vector<pair<int, vector<int> *>>::iterator iterIndex = indexNodes.begin();
	for(; iterIndex != indexNodes.end(); ++iterIndex)
	{
		int nid = iterIndex->first;
		buildBtree(nid);
	}
}


void IBRTree::BuildIBRTree()
{	
	//RTreeIndex rt(locFile, treeFile, numOfEntry, rtreePageSize);
	if(MyTool::FileExist(treeFile + ".idx"))	//create the R-tree first
	{
		cout<<"Reading R-tree"<<endl;
		rtree->ReadIndex();		
	}
	else
	{
		cout<<"Creating R-tree"<<endl;
		rtree->CreateRTree();
	}

	if(!MyTool::FileExist(leafnodeFile))
	{
		cout<<"Writing R-tree structure to file"<<endl;
		WriteNodeStrategy wns;		
		rtree->getTree()->queryStrategy(wns);		//write the tree structure into a file
	}

	nodeNum = ReadLeafNodes();
	ReadIndexNodes();

	if(_chdir(subdocFolder.c_str()))			//split the document according to leaf nodes
	{
		cout<<"Splitting texts according to R-tree structure"<<endl;
		_mkdir(subdocFolder.c_str());
		SplitDoc();
	}

	if(_chdir(invertedFolder.c_str()))			//create inverted file for each node
	{
		cout<<"Creating inverted file for R-tree nodes"<<endl;
		_mkdir(invertedFolder.c_str());
		CreateLeafInverted();
		CreateNonLeafInverted();		
	}	

	if(_chdir(btreeFolder.c_str()))				//build the B-tree index for the inverted files
	{
		cout<<"Creating B-tree for inverted files"<<endl;
		_mkdir(btreeFolder.c_str());
		BuildBTIndex();
	}
}
