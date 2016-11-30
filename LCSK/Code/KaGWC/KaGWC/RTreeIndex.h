/*
 *	Encapsulate the external R-tree index
 *	Notice that we use 4k page size, and thus at most 100 children can be put in one page. 
 *	Capacity should be computed according to the page size.
 *
 *  When deleting "tree" in the debug mode, "Lost Pointers: 0" will be shown, not knowing the reason yet, not affecting the results.
 */

#ifndef _RTREE_H_
#define _RTREE_H_

#include <SpatialIndex.h>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;
using namespace SpatialIndex;


class RTreeIndex
{
private:
	string pointFile;
	string treeFile;
	IStorageManager* diskfile;
	StorageManager::IBuffer* file;
	ISpatialIndex* tree;
	int rtreePageSize;
	unsigned int capacity;
	id_type indexIdentifier;
	unsigned int cacheblock;
	

public:
	RTreeIndex(string pFile, string tFile, unsigned int capacity, int rtreePageSize, unsigned int cacheblock = 1000)
	{
		pointFile = pFile;
		this->capacity = capacity;
		treeFile = tFile;
		this->rtreePageSize = rtreePageSize;
		this->cacheblock = cacheblock;
	}

	~RTreeIndex()
	{
		delete tree;
		delete file;		
		delete diskfile;		
	}
	void CreateRTree();
	void ReadIndex();
	
	ISpatialIndex* getTree()
	{return tree;}
};

#endif