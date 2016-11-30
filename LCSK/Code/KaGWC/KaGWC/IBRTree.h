/*
 *	For building the bitmap version of IR-tree
 *	To make it more understandable, each step is encapsulated in a function, and the intermediate resutls are kept
 *	Btree page size is set to 4k... can be change in the function buildBtree()
 */

// handle, need to modify the structure of this inverted file in this section
#ifndef _IBRTREE_
#define _IBRTREE_

#include <string>
#include <fstream>
#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <direct.h>
#include <sstream>
#include "Tools.h"
#include "RTreeIndex.h"
#include "WriteNodeStrategy.h"
#include "data.h"
#include "btree.h"

using namespace std;

extern string basepath;
extern string textFile;
extern string locFile;
extern string treeFile;

extern string btreeFolder;
extern string leafnodeFile;
extern string indexnodeFile;
extern string subdocFolder;
extern string invertedFolder;
extern string btreeFolder;
extern string objwetFile;

extern int numOfEntry;


struct tagObj{
	int objID;
	int tagID;
};


struct invertStru {
	int attrW[ATTRW];
	vector<tagObj> *objList ;
};

class IBRTree
{
private:
	map<int, vector<int> *> leaves;	
	int *N2SG;
	int nodeNum;
	vector<pair<int, vector<int> *>> indexNodes;
	int rtreePageSize;
	int btreePageSize;
	RTreeIndex *rtree;

	void buildBtree(int nodeID);
	// split the textfile into subdoc according to leafnode id point belongs
	void SplitDoc();	
	// create the inverted file for leafnode from leaves and N2SG
	void CreateLeafInverted();
	// create the inverted file for index node with indexNodes
	void CreateNonLeafInverted();
	// read leaf node information to leaves and N2SG
	int ReadLeafNodes();
	// read the information of nonleafnode to indexNodes
	void ReadIndexNodes();
	// bulid the btree for each node of rtree
	void BuildBTIndex();

public:

	IBRTree(int rtreePageSize=4096, int btreePageSize=4096)
	{
		this->rtreePageSize = rtreePageSize;
		this->btreePageSize = btreePageSize;

		rtree = new RTreeIndex(locFile, treeFile, numOfEntry, rtreePageSize);
	}    

	~IBRTree()
	{
		map<int, vector<int> *>::iterator iter = leaves.begin();
		for(;iter != leaves.end(); ++iter)
		{
			delete iter->second;
		}
		vector<pair<int, vector<int> *>>::iterator iter2 = indexNodes.begin();
		for(;iter2 != indexNodes.end(); ++iter2)
		{
			vector<int> * p = iter2->second;
			delete p;
		}	
		delete rtree;
	}	
	
	void BuildIBRTree();
	// read disk file and tree 
	void ReadTree()
	{
		rtree->ReadIndex();
	}
	// only return the tree 
	ISpatialIndex* GetTree()
	{
		return rtree->getTree();
	}
};

#endif