#ifndef _INVERTEDINDEX_H_
#define _INVERTEDINDEX_H_


#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <hash_map>
#include <vector>
#include "Tools.h"
using namespace std;

extern string invertedFile;
extern string textFile;

class InvertedFileList
{
private:
	map<int, vector<int> *> iList;	
public:	

	~InvertedFileList()
	{
		map<int, vector<int> *>::iterator i=iList.begin();
		for(;i!= iList.end(); ++i)
		{
			vector<int> *p = (*i).second;
			delete p;
		}		
	}

	void CreateInvertedFile();
	void ReadInvertedFile();
	map<int, vector<int> *> * getIndex();

};

#endif