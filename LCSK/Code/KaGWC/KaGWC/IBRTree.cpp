#include "IBRTree.h"

// read the leaf nodes point to N2SG <point id, leafnodeID>
// leaves <leafID, points>
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

// split the doc file according to the leafnodes information
void IBRTree::SplitDoc()
{
	ifstream textF(textFile.c_str());
	map<int, ofstream *> sdhandlers; // <leafnodeID, file>
	
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
// leaves <leafID, points> to <kwd, point index>
// --M-- modify in this part to add the summary information for kwd
// --M-- storage the weight inf of object to objweight
void IBRTree::CreateLeafInverted()
{
	// output the objweight inf to owFile
	ofstream owFile(objwetFile.c_str());
	map<int, vector<int> *>::iterator iter = leaves.begin();

	for(;iter != leaves.end(); ++iter)
	{
		int leafID = iter->first;
		vector<int> *p = iter->second;		
		map<int, invertStru> inverted;

		vector<vector<int> *> objectTexts; // record the kwd of obj
		vector<vector<int> *> objectTags; // tags corresponding to kwds
		
		string leafDoc = subdocFolder + MyTool::IntToString(leafID);
		ifstream docF(leafDoc.c_str());
		string line;
		int loop = 0;
		while(!docF.eof())
		{
			getline(docF, line);
//cout << loop++ << endl;
			if(line == "")
				continue;
			int oid, wid; char c;
			// --M-- read new added inf, coor and tags
			int tag, totalweight = 0;
			double coor[2];
			istringstream iss(line);
			// --M-- read the unrelevant coordinates inf
			iss >> oid >> coor[0] >> coor[1];
			//iss>>oid;
			vector<int> *pt = new vector<int>();
			vector<int > *objtag = new vector<int>();
			while(iss>>c>>wid>>tag)
			{			
				pt->push_back(wid);
				objtag->push_back(tag);
				totalweight += tag;
			}
			objectTexts.push_back(pt);
			objectTags.push_back(objtag);
			// --M-- output the weight inf
			owFile << oid << ","<<totalweight<<endl;
		}
		docF.close();

		//------------------------------------------------------------------------
		vector<int> temp(*p);
		sort(temp.begin(), temp.end());				
		// Nodes id read from Rtree is not ordered, but in subdoc they are ordered!													
		map<int, int> mapping;
		for(unsigned int i=0;i<temp.size(); i++)
		{
			mapping[temp[i]] = i;
		}		
		//------------------------------------------------------------------------
// note if there wrong conrresponding relationship
		for(unsigned int i=0;i<p->size();i++)	//for each object in this leaf node
		{
			int nid = (*p)[i];
			int idx = mapping[nid];	//get this object's position in the ordered list
			vector<int> *words = objectTexts[idx];
			vector<int> *tags = objectTags[idx];
			if (words->size() != tags->size()) {
cout << "There is problem in handle inverted file" << endl;
			}
			vector<int>::iterator wi = words->begin();
			vector<int>::iterator tgi = tags->begin();
			// compute the weight value of this obj
			int objWeight = 0;
			for (; tgi != tags->end(); tgi++) {
				//if (*tgi < weight) weight = *tgi;
				objWeight += *tgi;
			}
			int tp = objWeight;
			for(tgi = tags->begin(); wi!=words->end();++wi,++tgi)		//for each word contained in this object
			{
				int wordID = *wi;
				int wordIDd = wordID;
				int tagID = *tgi;
				map<int, invertStru>::iterator ti = inverted.find(wordID);
				if (ti != inverted.end()) { // exists
					tagObj tobj;
					tobj.objID = i;
					tobj.tagID = tagID;

					if (objWeight < inverted[wordID].attrW[0]) {
						inverted[wordID].attrW[0] = objWeight;
					}
					inverted[wordID].attrW[tagID]++;
					inverted[wordID].objList->push_back(tobj);
				}
				else // do not exist
				{
					invertStru is;
					memset(is.attrW, 0, sizeof(int)*ATTRW);
					is.attrW[0] = objWeight;
					is.attrW[tagID]++;

					vector<tagObj> *p = new vector<tagObj>();
					tagObj tobj;
					tobj.objID = i;
					tobj.tagID = tagID;

					p->push_back(tobj);
					is.objList = p;
					inverted[wordID] = is;
				}
			}
		}

		ofstream outF( (invertedFolder + MyTool::IntToString(leafID)).c_str());		
		//write the object's index, instead of its ID£¡
		map<int, invertStru>::iterator fi;
		for(fi = inverted.begin(); fi != inverted.end(); ++fi)
		{
			outF<<fi->first<<"\t";
			invertStru ivs = fi->second;
			// output the attrw inf
			for (int l = 0; l < ATTRW; l++) {
				outF << ivs.attrW[l] <<" ";
			}
			vector<tagObj>::iterator iter = ivs.objList->begin();

			for (; iter != ivs.objList->end(); ++iter) {
				outF << "," << iter->objID << " "<< iter->tagID;
			}		
			outF<<endl;
		}
		outF.close();

		vector<vector<int> *>::iterator iter1;
		for(iter1 = objectTexts.begin(); iter1 != objectTexts.end(); ++iter1)
			delete *iter1;
		vector<vector<int> *>::iterator iter2;
		for (iter2 = objectTags.begin(); iter2 != objectTags.end(); ++iter2)
			delete *iter2;

		map<int, invertStru>::iterator iter3;
		for(iter3 = inverted.begin(); iter3 != inverted.end(); ++iter3)
			delete iter3->second.objList;			
	}
	owFile.close();
}
// indexNodes <indexID, childIDs>
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
		indexNodes.push_back(make_pair(nid, p));
	}

	inodeF.close();
}
// from indexNodes<tid,ctid> to <kwd, tnodeID>
// --------------------M-- modify in this part to add the summary information for kwd
void IBRTree::CreateNonLeafInverted()
{
	for(int idx = indexNodes.size() - 1; idx >= 0; idx--)		//for each non-leaf node. from bottom to up
	{
		int indexID = indexNodes[idx].first;		
		int count = 0;

		vector<int> *p = indexNodes[idx].second;
		vector<int>::iterator iter = p->begin();
		map<int, invertStru> inverted;

		for(; iter != p->end(); ++iter)	//for each its child nodes
		{
			int id = *iter;
			ifstream listF( (invertedFolder + MyTool::IntToString(id)).c_str());
			string line;
			while(getline(listF, line))
			{
				int wordID, sumInf[ATTRW];
				char c;
				istringstream iss(line);
				iss >> wordID;
				// compute the lable of this node 
				for (int l = 0; l < ATTRW; l++) {
					iss >> sumInf[l];
				}			

				map<int, invertStru>::iterator iter = inverted.find(wordID);
				if(iter != inverted.end()) // this kwd exits
				{
					if (sumInf[0] < iter->second.attrW[0]) {
						iter->second.attrW[0] = sumInf[0];
					}
					for (int l = 1; l < ATTRW; l++) {
						iter->second.attrW[l] += sumInf[l];
					}

					tagObj tob;
					tob.objID = count;
					tob.tagID = sumInf[0];
					iter->second.objList->push_back(tob);
				}
				else // do not exists
				{
					invertStru is;
					memset(is.attrW, 0, sizeof(int)*ATTRW);	

					is.attrW[0] = sumInf[0];				
					for (int l = 1; l < ATTRW; l++) {
						is.attrW[l] += sumInf[l];
					}

					tagObj tob;
					tob.objID = count;
					tob.tagID = sumInf[0];
					
					vector<tagObj> *tp = new vector<tagObj>();
					tp->push_back(tob);
					is.objList = tp;

					inverted[wordID] = is;
				}								
			}
			listF.close();			
			count++;
		}

		ofstream outF( (invertedFolder + MyTool::IntToString(indexID)).c_str());
		map<int, invertStru>::iterator fi;
		for (fi = inverted.begin(); fi != inverted.end(); ++fi)
		{
			outF << fi->first << "\t";
			invertStru ivs = fi->second;
			// output the attrw inf
			for (int l = 0; l < ATTRW; l++) {
				outF << ivs.attrW[l] << " ";
			}
			vector<tagObj>::iterator iter = ivs.objList->begin();
			for (; iter != ivs.objList->end(); ++iter) {
				outF << "," << iter->objID << " "<< iter->tagID;
			}
			outF << endl;
		}
		outF.close();

		map<int, invertStru>::iterator iter3;
		for (iter3 = inverted.begin(); iter3 != inverted.end(); ++iter3)
			delete iter3->second.objList;
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
		DATA *p = new DATA();
		// read key
		iss >> wordID;
		p->key = wordID;
		// read the suminf
		for (int l = 0; l < ATTRW; l++) {
			iss >> p->attrw[l];
		}
		// read objList inf
		int indexID; // tree node num
		int tagID;
		int loop = 0;
		while(iss>>c>>indexID>>tagID)
		{
			int bytePos = indexID / 8;
			int bitPos = indexID % 8;
			unsigned char mask = 1 << bitPos;
			//unsigned char tagmask = 1 << tagID;
			unsigned char tagmask = tagID;
			p->data[bytePos] = p->data[bytePos] | mask;
			//p->tag[loop] = p->tag[loop] | tagmask;
			p->tag[loop] = tagmask;
//int tg = p->tag[loop];
			loop++;
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
	// write the leaf and index node rtree information to file respectively
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
		// note that in the inverted file, only the index of object or tnode are stored instead of the id
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
