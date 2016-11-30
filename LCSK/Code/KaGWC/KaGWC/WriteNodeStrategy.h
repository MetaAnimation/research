/*
 *	Write the R-tree structure into a file
 */

#ifndef _WRITENODESTRATEGY_
#define _WRITENODESTRATEGY_

#include <SpatialIndex.h>

extern string leafnodeFile;
extern string indexnodeFile;

class WriteNodeStrategy : public SpatialIndex::IQueryStrategy
{
private:
	queue<id_type> ids;	
	ofstream *nFile;
	ofstream *tFile;	

public:

	WriteNodeStrategy()
	{
		nFile = new ofstream(leafnodeFile.c_str());
		tFile = new ofstream(indexnodeFile.c_str());		
	}

	~WriteNodeStrategy()
	{
		nFile->close();
		tFile->close();
		
		delete tFile;
		delete nFile;
		
	}

	
	void getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext)
	{	
		const INode* n = dynamic_cast<const INode*>(&entry);				
		if (n != 0)
		{
			if(n->isIndex())
			{
				id_type id = n->getIdentifier();
				(*tFile)<<id<<":";				
				for (uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
				{
					id_type childID = n->getChildIdentifier(cChild);
					ids.push(childID);
					(*tFile)<<childID<<',';					
				}
				(*tFile)<<endl;
			}
			if(n->isLeaf())
			{
				IShape *out;
				n->getShape(&out);
				Region *ps = dynamic_cast<Region *> (out);

				id_type id = n->getIdentifier();
				(*nFile)<<id<<":";		
				
				delete ps;

				for (uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
				{
					id_type childID = n->getChildIdentifier(cChild);
					(*nFile)<<childID<<',';					
				}	
				(*nFile)<<endl;
			}
		}

		if (! ids.empty())
		{
			nextEntry = ids.front(); ids.pop();
			hasNext = true;
		}
		else
		{
			hasNext = false;
		}		
	}
};

#endif