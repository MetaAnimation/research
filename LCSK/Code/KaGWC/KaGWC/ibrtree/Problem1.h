#ifndef _PROBLEM1EXACT_
#define _PROBLEM1EXACT_

#include <iostream>
#include <SpatialIndex.h>
#include <map>
#include <set>
#include <vector>
#include "Tool.h"
#include "data.h"
#include "btree.h"

using namespace std;

extern string textFile;
extern string locFile;
extern string invertedFile;
extern string subdocFolder;
extern string btreeFolder;

void GenerateSubSets(vector<int> &intersect, set<KEYTYPE> &res)
{	
	int L = intersect.size();

	unsigned long length = pow(2.0, L);
	for(unsigned int i=1;i<length;i++)
	{
		unsigned int p = 0;
		unsigned int mask = 0x1;
		for(int j=0;j<L;j++)
		{
			p+= (mask & i) << (intersect[j] - j);
			mask = mask << 1;
		}
		res.insert(p);
	}
}	



bool compareNode(RTreeNode *a, RTreeNode *b)
{
	return a->minValue > b->minValue;
}

///* approximation algorithm
class Problem1Appr : public SpatialIndex::IQueryStrategy
{
private:	
	vector<RTreeNode *> heapNode;	
	Query *Q;
	int nodeNum;
	vector<int> keywords;
	KEYTYPE keywordsBM;			//keywords bitmask
	set<int> group;
	double Cost;
	map<int, double> objectDist;
	int loopt;
public:

	double ComputeCost()
	{
		double res = 0.0;
		set<int>::iterator si;
		for(si = group.begin(); si != group.end(); ++si)
			res += objectDist[*si];
		return res;
	}

	friend ostream& operator<<(ostream& out,Problem1Appr & t)    
	{
		out<<t.ComputeCost()<<":";
		set<int>::iterator iter = t.group.begin();
		for(; iter != t.group.end(); ++iter)
			out<<*iter<<" ";
		out<<endl;
		return out;
	}
	
	Problem1Appr(Query *Q, int nodeNum)
	{
loopt = 0;
		this->Q = Q;
		this->nodeNum = nodeNum;		

		istringstream iss(Q->text);
		int wid; char c;
		while(iss>>wid)
		{
			keywords.push_back(wid);
			iss>>c;
		}

		keywordsBM = pow(2.0, (int)keywords.size())-1;
		Cost = 0;
	}
	
	~Problem1Appr()
	{
		while (!heapNode.empty())
		{
			RTreeNode *p = heapNode.back();
			heapNode.pop_back();
			delete p;
		}

	}

	void getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext)
	{	
		const INode* n = dynamic_cast<const INode*>(&entry);
cout << loopt << endl;
loopt++;

int lev = n->getLevel();
int childn = n->getChildrenCount();
		if (n != 0)
		{
			int pid = n->getIdentifier();
			string btFile = btreeFolder + MyTool::IntToString(pid);
			char *btfname = new char[btFile.size()+1];
			memcpy(btfname, btFile.c_str(), btFile.size());		
			btfname[btFile.size()] = '\0';
			BTree *bt = new BTree(btfname, 0);	
int nn = bt->num_of_inodes;
int nd = bt->num_of_data;
int ndn =  bt->num_of_dnodes;
			map<int, vector<int>*> objectTexts;		//the text description of an object
			set<int> indexID;						//the id of child nodes which contains query keywords				
			map<int, vector<int>*>::iterator iterMap;
			for(unsigned int k=0;k<keywords.size();k++)		
								// Find all the object that contain the query keywords, and created an inverted list								
								// in this list, the position of an object is used, instead of the object's ID
			{
				int wordID = keywords[k];
				unsigned char *data = new unsigned char[DIMENSION];
				bool flag = bt->search(wordID, &data);
				if(flag)
				{					
					for(int i=0;i<DIMENSION;i++)
					{
						if(data[i] > 0)							
						{
							unsigned char mask = 1;
							for(int j=0;j<8;j++)
							{
								if((data[i] & mask) > 0)
								{
									int index = i*8+j;
									indexID.insert(index);
									iterMap = objectTexts.find(index);
									if(iterMap ==  objectTexts.end())
									{
										vector<int> *p = new vector<int>();
										p->push_back(k);		//the index of the word in keywords, not the real word id!
										objectTexts[index] = p;
									}
									else
									{
										vector<int> *p = iterMap->second;
										p->push_back(k);
									}
								}
								mask = mask << 1;
							}
						}
					}
				}
				delete data;
			}
			delete bt;
			delete btfname;

			set<int>::iterator si = indexID.begin();
			for(;si != indexID.end(); ++si)	
			{
				uint32_t cChild = *si;
				int cid = n->getChildIdentifier(cChild);
				
				IShape *out;
				n->getChildShape(cChild, &out);
				Region* pr = dynamic_cast<Region *>(out);	

				double dx = Q->x - pr->m_pHigh[0];
				double dy = Q->y - pr->m_pHigh[1];											

				KEYTYPE key = MyTool::ConvertToInt(objectTexts[cChild]);
				KEYTYPE interS = key & keywordsBM;
				if( interS == 0)		//no uncovered keywords contained
				{
					delete pr;
					continue;
				}

				double dist;
				RTreeNode *rtp = new RTreeNode;
				
				if(n->isLeaf())
				{
					dist = sqrt(dx * dx + dy * dy);
					objectDist[n->getChildIdentifier(cChild)] = dist;
					rtp->isNode = false;
				}
				if(n->isIndex())
				{
					double coor[2];
					coor[0] = Q->x; coor[1] = Q->y;
					Point pt(coor, 2);

					dist = MyTool::ComputeMinPossible(pr, &pt);
					rtp->pr = pr;
					rtp->isNode = true;
				}
				
				rtp->identifier = n->getChildIdentifier(cChild);
				rtp->bitmap = interS;
				int num = MyTool::getNumOf1(interS);
				rtp->minValue = dist/num;
				                                              
				heapNode.push_back(rtp);
				push_heap(heapNode.begin(), heapNode.end(), compareNode);
			}

			map<int, vector<int>*>::iterator oi = objectTexts.begin();
			for(; oi != objectTexts.end(); ++oi)
			{
				vector<int> *p = oi->second;
				delete p;
			}
			
		}

		if (!heapNode.empty())		
		{			
			while (!heapNode.empty())		
				//after each object is processed, the queue will be updated, therefore a node in the queue is always useful
			{
				RTreeNode *p = heapNode.front();
				pop_heap(heapNode.begin(), heapNode.end(), compareNode);	//the first element is moved to the last position				
				heapNode.pop_back();

				if( p->isNode)				//if the node is not an object, we read the next page.
				{	
					nextEntry = p->identifier;		
					hasNext = true;
					delete p;
					break;
				}

				double dist = p->minValue;
				KEYTYPE key = p->bitmap;
				group.insert(p->identifier);
cout << p->identifier << endl;
				Cost += dist;
				keywordsBM = keywordsBM - key;
				
				if(keywordsBM == 0)
				{
					delete p;
					hasNext = false;
					break;
				}

				//update...
				
				int length = heapNode.size();
				for(int i=0; i< length; i++)
				{
					RTreeNode *t = heapNode[i];
					KEYTYPE tkey = t->bitmap;
					if( (tkey & key) == 0)		//the node shares no keywords with the current object
						continue;					

					KEYTYPE interS = tkey & keywordsBM;		//remove the words already covered
					if( interS == 0)			//the node's all keywords are covered
					{						
						delete t;
						heapNode[i] = heapNode[length-1];
						heapNode.pop_back();
						length--;
						i--;	//process the new object exchanged here in next loop!
						continue;
					}

					if(tkey != interS)
					{
						int pSize = MyTool::getNumOf1(tkey);
						t->bitmap = interS;
						int nSize = MyTool::getNumOf1(interS);
						t->minValue = t->minValue * pSize / nSize;
					}
				}				
				
				make_heap(heapNode.begin(), heapNode.end(), compareNode);				
				delete p;
			}		
		}
	
		else 
			hasNext = false;
	}
};//*/



class Problem1Baseline
{
	Query *Q;
	vector<int> keywords;
	double* Cost;
	set<KEYTYPE> **group;
	unsigned int keywordsBM;
	map<unsigned int, double> objectDist;

public:

	Problem1Baseline(Query *Q)
	{
		this->Q = Q;

		istringstream iss(Q->text);
		int wid; char c;
		while(iss>>wid)
		{
			keywords.push_back(wid);
			iss>>c;
		}

		keywordsBM = pow(2.0, (int)keywords.size());

		group = new set<KEYTYPE>*[keywordsBM];
		Cost = new double[keywordsBM];
		for(unsigned int i=0;i<keywordsBM;i++)
		{
			group[i] = NULL;
			Cost[i] = -1;
		}		
	}

	~Problem1Baseline()
	{
		for(unsigned int i=0;i<keywordsBM;i++)
			delete group[i];
		delete[] group;
		delete Cost;
	}

	double getCost()
	{
		return Cost[keywordsBM-1];
	}

	friend ostream& operator<<(ostream& out,Problem1Baseline & t)
	{
		out<<(t.Cost)[t.keywordsBM-1]<<":";
		set<KEYTYPE> * p = (t.group)[t.keywordsBM-1];
		set<KEYTYPE>::iterator iter = p->begin();
		for(; iter != p->end(); ++iter)
			out<<*iter<<" ";
		out<<endl;
		return out;
	}

	void ProcessLine(vector<int> &intersect, double dist, int oid)
	{
		set<unsigned int> subsets;
		GenerateSubSets(intersect, subsets);

		set<unsigned int>::iterator iterS;		
		for(iterS = subsets.begin(); iterS != subsets.end(); ++iterS)
		{
			int bitmap = *iterS;
			double cost = Cost[bitmap];			
			if(cost > dist)
			{
				set<KEYTYPE> *p = group[bitmap];					
				p->clear();
				p->insert(oid);
				group[bitmap] = p;
				Cost[bitmap] = dist;				
			}
			else if(cost < 0)
			{
				set<KEYTYPE> *p = new set<KEYTYPE>();
				p->insert(oid);
				Cost[bitmap] = dist;
				group[bitmap] = p;
			}
		}
	}

	void PostProcess()
	{		
		for(unsigned int i = 1; i < keywordsBM; ++i)
		{			
			if(Cost[i] >= 0)
			{
				int oid = *(group[i]->begin());
				objectDist[oid] = Cost[i];
			}
		}

		for(unsigned int i=1;i<keywordsBM;i++)
		{
			double min = 10000;
			int bestSep = -1;
			for(unsigned int j=1;j<=i/2;j++)
			{
				if( (j & i) != j)		//j must be the subset of i
					continue;
				set<KEYTYPE> *p1 = group[j];
				set<KEYTYPE> *p2 = group[i-j];

				double d = Cost[j];
				set<KEYTYPE>::iterator s1, s2;
				for(s2 = p2->begin(); s2 != p2->end(); ++s2)
				{
					s1 = p1->find(*s2);
					if(s1 == p1->end())
					{
						d += objectDist[*s2];
					}
				}
				if(d == Cost[j])		//two subsets are contributed by same objects, do nothing
					continue;
				
				if(min > d)
				{
					min = d;
					bestSep = j;
				}
			}
			double cost = Cost[i];
			if(cost < 0)		//only if resMap[i] does not exist or it has larger value, we update it
			{
				Cost[i] = min;
				set<KEYTYPE> *p = new set<KEYTYPE>();
				p->insert(group[bestSep]->begin(), group[bestSep]->end());
				p->insert(group[i-bestSep]->begin(), group[i-bestSep]->end());
				group[i] = p;
			}
			else if( min < cost)
			{
				Cost[i] = min;
				set<KEYTYPE> *p = group[i];
				p->clear();
				p->insert(group[bestSep]->begin(), group[bestSep]->end());
				p->insert(group[i-bestSep]->begin(), group[i-bestSep]->end());
			}
		}
	}

	void BaseLine()
	{
		ifstream textF(textFile.c_str());
		ifstream locF(locFile.c_str());

		int count = 0;
		while ( !textF.eof())
		{
			string docLine;
			getline(textF, docLine);
			if(docLine == "")
				continue;
			string locLine;
			getline(locF, locLine);

			istringstream iss(docLine);			
			int oid, wid; char c;
			iss>>oid;
			vector<int> intersect;
			while(iss>>c>>wid)
			{
				unsigned int i;
				vector<int>::iterator ti = intersect.begin();
				for(; ti != intersect.end(); ++ti)
				{
					if(keywords[*ti] == wid)
						break;
				}
				if( ti != intersect.end())
					continue;

				for(i=0;i<keywords.size();i++)
				{
					if(keywords[i] == wid)
						break;
				}
				if(i < keywords.size())
					intersect.push_back(i);
			}
			
			if(!intersect.empty())
			{
				sort(intersect.begin(), intersect.end());
				double x, y;
				istringstream iss2(locLine);
				iss2>>oid>>c>>x>>c>>y;
				double dist = sqrt( (Q->x - x)*(Q->x - x) + (Q->y-y)*(Q->y-y) );
				ProcessLine(intersect, dist, count);
			}
			count++;
		}
		textF.close();
		locF.close();

		PostProcess();
	}
};


/* the dynamic programming method */
class Problem1IBRTree : public SpatialIndex::IQueryStrategy
{
private:	
	priority_queue<RTreeNode *, deque<RTreeNode *>, NodeValueLess<RTreeNode> > queueNode;	
	Query *Q;
	int nodeNum;	
	vector<int> keywords;
	set<KEYTYPE> **group;
	double *Cost;
	KEYTYPE keywordsBM;	
	set<KEYTYPE> markedSet;
	set<KEYTYPE> valuedSet;
	bool *mark;
				
public:
	
	Problem1IBRTree(Query *Q, int nodeNum)
	{
		this->Q = Q;
		this->nodeNum = nodeNum;		

		istringstream iss(Q->text);
		int wid; char c;
		while(iss>>wid)
		{
			keywords.push_back(wid);
			iss>>c;
		}

		keywordsBM = pow(2.0, (int)keywords.size());

		group = new set<KEYTYPE>*[keywordsBM];
		Cost = new double[keywordsBM];		
		mark = new bool[keywordsBM];
		for(KEYTYPE i=0;i<keywordsBM;i++)
		{
			group[i] = NULL;
			Cost[i] = -1;			
			mark[i] = false;
		}	
	}

	~Problem1IBRTree()
	{
		for(KEYTYPE i=0;i<keywordsBM;i++)
			delete group[i];
		delete[] group;
		delete Cost;

		while (!queueNode.empty())		
		{
			RTreeNode *p = queueNode.top();
			queueNode.pop();
			delete p;			
		}
	}

	double getCost()
	{
		return Cost[keywordsBM-1];
	}

	friend ostream& operator<<(ostream& out,Problem1IBRTree & t)
	{
		out<<(t.Cost)[t.keywordsBM-1]<<":";
		set<KEYTYPE> * p = (t.group)[t.keywordsBM-1];
		set<KEYTYPE>::iterator iter = p->begin();
		for(; iter != p->end(); ++iter)
			out<<*iter<<" ";
		out<<endl;
		return out;
	}

	void getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext)
	{	
		const INode* n = dynamic_cast<const INode*>(&entry);

		if (n != 0)
		{
			int pid = n->getIdentifier();
			string btFile = btreeFolder + MyTool::IntToString(pid);
			char *btfname = new char[btFile.size()+1];
			memcpy(btfname, btFile.c_str(), btFile.size());		
			btfname[btFile.size()] = '\0';
			BTree *bt = new BTree(btfname, 0);	
			
			map<int, vector<int>*> objectTexts;		//the text description of an object
			set<int> indexID;						//the id of child nodes which contains query keywords				
			map<int, vector<int>*>::iterator iterMap;
			for(unsigned int k=0;k<keywords.size();k++)
			{
				int wordID = keywords[k];
				int sdfasd = DIMENSION;
				unsigned char *data = new unsigned char[DIMENSION];
				bool flag = bt->search(wordID, &data);
				if(flag)
				{			
					int i;
					for(i=0;i<DIMENSION;i++)
					{
						if(data[i] > 0)							
						{
							unsigned char mask = 1;
							for(int j=0;j<8;j++)
							{
								if((data[i] & mask) > 0)
								{
									int index = i*8+j;
									indexID.insert(index);
									iterMap = objectTexts.find(index);
									if(iterMap ==  objectTexts.end())
									{
										vector<int> *p = new vector<int>();		
										p->push_back(k);		//the index of the word in keywords, not the real word id!
										objectTexts[index] = p;
									}
									else
									{
										vector<int> *p = iterMap->second;
										p->push_back(k);
									}
								}
								mask = mask << 1;
							}
						}
					}
				}
				delete data;
			}
			delete bt;
			delete btfname;

			set<int>::iterator si = indexID.begin();
			for(;si != indexID.end(); ++si)	
			{
				uint32_t cChild = *si;
				int cid = n->getChildIdentifier(cChild);
				
				IShape *out;
				n->getChildShape(cChild, &out);
				Region* pr = dynamic_cast<Region *>(out);	

				double dx = Q->x - pr->m_pHigh[0];
				double dy = Q->y - pr->m_pHigh[1];											

				KEYTYPE keyValue = MyTool::ConvertToInt(objectTexts[cChild]);
				if(mark[keyValue])		//the keywords set of this node has know lowest cost
				{
					delete pr;
					continue;
				}

				double dist;
				RTreeNode *rtp = new RTreeNode;

				if(n->isLeaf())
				{
					dist = sqrt(dx * dx + dy * dy);
					rtp->isNode = false;
				}
				if(n->isIndex())
				{
					double coor[2];
					coor[0] = Q->x; coor[1] = Q->y;
					Point pt(coor, 2);

					dist = MyTool::ComputeMinPossible(pr, &pt);
					rtp->pr = pr;
					rtp->isNode = true;
				}
				
				rtp->identifier = n->getChildIdentifier(cChild);
				rtp->minValue = dist;
				rtp->bitmap = keyValue;
				queueNode.push(rtp);
			}	

			map<int, vector<int>*>::iterator oi = objectTexts.begin();
			for(; oi != objectTexts.end(); ++oi)
			{
				vector<int> *p = oi->second;
				delete p;
			}
		}

		if (!queueNode.empty())		
		{			
			while (!queueNode.empty())
			{
				RTreeNode *p = queueNode.top();
				queueNode.pop();
				if( p->isNode )		//if the node is not an object, we read the next page.
				{
					KEYTYPE keyValue = p->bitmap;
					if(mark[keyValue])
					{
						delete p;						
						continue;
					}
					else
					{
						nextEntry = p->identifier;
						hasNext = true;
						delete p;
						break;
					}
				}

				double dist = p->minValue;

				set<KEYTYPE>::iterator si = valuedSet.begin();	
							// utilize dist to move some subsets to marked set. because the lowest cost is known now
				set<KEYTYPE> moved;
				for(;si != valuedSet.end(); ++si)
				{
					int bitmap = *si;
					if(Cost[bitmap] <= dist)
					{
						moved.insert(bitmap);
						markedSet.insert(bitmap);
						mark[bitmap] = true;
					}
				}
				for(si = moved.begin(); si != moved.end(); ++si)
					valuedSet.erase(*si);
				
				int oid = p->identifier;
				
				KEYTYPE bitmap = p->bitmap;
				if(!mark[bitmap])
				{
					set<KEYTYPE> subsets;
					set<KEYTYPE> tempSet;
					vector<int> *wids = MyTool::ConvertToSet(bitmap);
					GenerateSubSets(*wids, subsets);
					delete wids;
					for(si = subsets.begin(); si != subsets.end(); ++si)
					{
						int bitmap = *si;
						if(mark[bitmap])		//this subset already has lowest cost
							continue;

						if(Cost[bitmap] >= 0)
						{
							valuedSet.erase(bitmap);
							set<KEYTYPE> *g = group[bitmap];
							g->clear(); g->insert(oid);
						}
						else
						{	
							set<KEYTYPE> *g = new set<KEYTYPE>();
							g->insert(oid);
							group[bitmap] = g;
						}
						Cost[bitmap] = dist;
						mark[bitmap] = true;
						tempSet.insert(bitmap);
						markedSet.insert(bitmap);
					}

					for(si = tempSet.begin(); si != tempSet.end(); ++si)
					{
						KEYTYPE i = *si;
						for(KEYTYPE j=1;j<keywordsBM;j++)
						{							
							if(Cost[j] < 0)
								continue;

							KEYTYPE unionS = i | j;
							if( unionS == i || unionS == j)
								continue;

							double D = dist + Cost[j];
							if(Cost[unionS] < 0)			//the new combination does not have a value
							{
								Cost[unionS] = D;
								set<KEYTYPE> *g = new set<KEYTYPE>();
								g->insert(group[j]->begin(), group[j]->end());
								g->insert(oid);
								group[unionS] = g;
								valuedSet.insert(unionS);
							}
							else if( Cost[unionS] > D)		//the previous cost is larger than the current one
							{
								Cost[unionS] = D;
								set<KEYTYPE> *g = group[unionS];
								g->clear();
								g->insert(group[j]->begin(), group[j]->end());
								g->insert(oid);
							}
						}
					}
				}
				delete p;
				if(mark[keywordsBM-1])
				{
					hasNext = false;
					break;
				}
			}			
		}
	
		else 
			hasNext = false;
	}
	
};

#endif