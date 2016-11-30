#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <map>
#include <cmath>
#include <vector>
#include <string>
#include <SpatialIndex.h>
using namespace std;
using namespace SpatialIndex;

typedef unsigned int KEYTYPE; //if more keywords, should use long or long long

typedef union{
	struct{
	public:
		int mbrID, wordsID;
	};
	long long key;
} KeyID;

class Query
{
public:
	Query() {x=0.0; y=0.0;text = "";}
	
	Query(string text, double x , double y)
	{
		this->text = text;
		this->x = x;
		this->y = y;
	}
	string text;
	double x, y;
};


class RTreeNode{
public:
	RTreeNode()
	{
		pr = NULL;
	}
	~RTreeNode()
	{
		delete pr;
	}

	   id_type identifier;
	   double maxCtri;		//the minimum possible score of this node
	   Region * pr;			//mbr region
	   KEYTYPE bitmap;		//keywords bitmap
	   bool isNode;
       
       static bool CompareValueLess(const RTreeNode * a,const  RTreeNode * b)
       {
		   return a->maxCtri < b->maxCtri;
       }
};

template<class _Ty>
struct NodeValueLess : std::binary_function<_Ty, _Ty, bool> {
       bool operator()(_Ty *_X, _Ty *_Y) const
	   {return _Ty::CompareValueLess(_X, _Y); }
};


class MyTool
{

public:

	static vector<int>* ConvertToSet(KEYTYPE key)		//delete in function who calls it
	{
		vector<int> *p = new vector<int>();
		unsigned long mask = 1;
		for(unsigned int i=0;i<sizeof(key) * 8; i++)
		{
			KEYTYPE temp = key & mask;
			if( temp > 0)
				p->push_back(i);
			mask = mask << 1;
		}
		return p;
	}


	static int getNumOf1(KEYTYPE key)
	{
		int num = 0;
		unsigned long mask = 1;
		for(unsigned int i=0;i<sizeof(key) * 8; i++)
		{
			KEYTYPE temp = key & mask;
			if( temp > 0)
				num++;
			mask = mask << 1;
		}
		return num;
	}

	static KEYTYPE ConvertToInt(vector<int> *p)
	{
		KEYTYPE res = 0;
		unsigned long mask = 1;
		for(unsigned int i=0;i<p->size(); i++)
		{
			int index = (*p)[i];
			res += mask << index;
		}
		return res;
	}


	static double ComputeMinPossible(Region* pr, Point *pt)
	{
		return pr->getMinimumDistance(*pt);
	}

	static double ComputeMaxPossible(Region* pr, Point *pt)	
	{
		double maxD = 0.0;
		double x = pt->m_pCoords[0], y = pt->m_pCoords[1];
		double x1 = pr->m_pLow[0], x2 = pr->m_pHigh[0], y1 = pr->m_pLow[1], y2 = pr->m_pHigh[1];
		double centerX = (x1 + x2)/2, centerY = (y1 + y2)/2;

		if( x >= x1 && x <= x2 && y >= y1 && y <= y2)
			return 0;
		else
		{
			if( x >= centerX)
			{
				if( y >= centerY)
					maxD = sqrt( (x-x1) * (x-x1) + (y-y1) * (y-y1));
				else
					maxD = sqrt( (x-x1) * (x-x1) + (y-y2) * (y-y2));
			}
			else
			{
				if( y >= centerY)
					maxD = sqrt( (x-x2) * (x-x2) + (y-y1) * (y-y1));
				else
					maxD = sqrt( (x-x2) * (x-x2) + (y-y2) * (y-y2));
			}
			return maxD;
		}
	}


	static double ComputeMBRDist(Region* pr1, Region* pr2)
	{
		return pr1->getMinimumDistance(*pr2);
	}

	static string IntToString(int d)
	{
		ostringstream oss;
		oss<<d;
		return oss.str();
	}

	static string DoubleToString(double d)
	{
		ostringstream oss;
		oss<<d;
		return oss.str();
	}

	static bool FileExist(string &filename)
	{
		fstream _file;
		_file.open(filename.c_str(),ios::in);
		if(!_file)
			return false;
		else
			return true;
		_file.close();
	}

};

#endif