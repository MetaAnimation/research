#include "InvertedFileList.h"

void InvertedFileList::CreateInvertedFile()		//create and read into memory
{	
	ifstream textF(textFile.c_str());
	ofstream indexF(invertedFile.c_str());

	int count = 0;
	while ( !textF.eof())
	{
		string line;
		getline(textF, line);
		if(line == "")
			continue;

		istringstream iss(line);
		int oid, wid;
		iss>>oid;
		char c;
		while(iss>>c>>wid)
		{ 
			map<int, vector<int> *>::iterator i = iList.find(wid);
			if( i == iList.end())
			{
				vector<int> * p = new vector<int>();
				p->push_back(count);
				iList[wid] = p;
			}
			else					//
			{
				vector<int> *p = (*i).second;
				p->push_back(count);
			}
		}	
		count ++;		
	}
	vector<int> * p = new vector<int>();
	p->push_back(count);
	p->push_back(iList.size());
	iList[-1] = p;
	textF.close();

	map<int, vector<int> *>::iterator iter;
	for(iter = iList.begin(); iter != iList.end();++ iter)
	{
		vector<int> *p = (*iter).second;
		indexF<<(*iter).first;
		for(unsigned int i = 0;i<p->size();i++)
		{
			indexF<<','<<(*p)[i];
		}
		indexF<<endl;
	}
	indexF.close();
}

void InvertedFileList::ReadInvertedFile()
{	
	ifstream indexF(invertedFile.c_str());	
	while ( ! indexF.eof())
	{
		string line;
		getline(indexF, line);
		if(line == "")
			continue;

		istringstream iss(line);
		int wid, oid; char c;
		iss>>wid;
		vector<int> *p = new vector<int>();
		iList[wid] = p;

		while(iss>>c>>oid)		
			p->push_back(oid);
	}
}

map<int, vector<int> *> * InvertedFileList::getIndex()
{
	return &iList;
}