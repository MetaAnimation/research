#include "RTreeIndex.h"


void RTreeIndex::CreateRTree()
{
	diskfile = StorageManager::createNewDiskStorageManager(treeFile, rtreePageSize);		//4k size page
	file = StorageManager::createNewRandomEvictionsBuffer(*diskfile, cacheblock, false);		//cache block number 1000
	tree = RTree::createNewRTree(*file, 0.7, capacity, capacity, 2, SpatialIndex::RTree::RV_RSTAR, indexIdentifier);

	ifstream pFile(pointFile.c_str());
	if (pFile.is_open())
	{		
		int count = 0;
		while (! pFile.eof() )
		{
			string line;
			getline(pFile, line);
			if(line == "")
				continue;
			int id;char c;
			double coor[2];
			istringstream iss(line);
			iss>> id >>c>>coor[0]>>c>>coor[1];
			Point p = Point(coor, 2);			
			tree->insertData(0, 0, p, count);
			count++;
		}
		pFile.close();		
	}
}

void RTreeIndex::ReadIndex()
{
	//diskfile = StorageManager::loadDiskStorageManager(treeFile);
	diskfile = StorageManager::loadDiskStorageManager(treeFile);
	file = StorageManager::createNewRandomEvictionsBuffer(*diskfile, cacheblock, false);
	tree = RTree::loadRTree(*file, 1);

	IStatistics *out;
	tree->getStatistics(&out);
cout << out->getNumberOfData() << endl;
cout << out->getNumberOfNodes() << endl;

}
