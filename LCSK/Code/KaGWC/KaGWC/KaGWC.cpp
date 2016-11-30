// SGKTEST.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <time.h>
#include <iostream>
#include <functional>
#include "IBRTree.h"
//#include "SearchIBRTree.h"
#include "Problem1.h"
//#include "Problem2.h"
using namespace std;

string basepath = "..\\dataHotel\\";

//string locFile = basepath + "loc";
string locFile = basepath + "doc";
string treeFile = basepath + "rtree";
string textFile = basepath + "doc";
string invertedFile = basepath + "invertedfile";

string btreeFolder = basepath + "btreeindex\\";
string leafnodeFile = basepath + "leafnodes";
string indexnodeFile = basepath + "indexnodes";
string subdocFolder = basepath + "subdoc\\";
string invertedFolder = basepath + "inverted\\";
string objwetFile = basepath + "objectweight";

int numOfEntry = 40;		//For hotel data, the fanout of R-tree is 40. it must be consistent with NODENUM defined in data.h!
double alpha = 0.5;
map<int, int> objWeight;

void readObjWet() {
	ifstream owFile(objwetFile.c_str());
	while (!owFile.eof())
	{
		string line;
		getline(owFile, line);
		if (line == "")
			continue;
		istringstream iss(line);
		int objID, weight;
		char c;
		iss >> objID >> c >> weight;
		objWeight[objID] = weight;
	}
	owFile.close();
}

int main()
{
	IBRTree irtree;
	//irtree.BuildIBRTree();
	
	//--------------------Test Code Start-----------------
	///*
	irtree.ReadTree();
	readObjWet();
	IStatistics *out;
	irtree.GetTree()->getStatistics(&out);
	int nodeNum = out->getNumberOfData();

	//Query *Q = new Query("2,5,100,52", 44, -151);
cout << 1 << endl;
	Query *Q = new Query("835.500000 483.000000 0.340500 0.250000 0.250000 0.250000 0.083333 0.166667 43 99");
	clock_t start, finish;
cout << 2 << endl;
	//the following code shows how to use the algorithms for the TYPE1 query
	start = clock();
	Problem1Appr pb1a(Q, nodeNum);
cout << 3 << endl;
	irtree.GetTree()->queryStrategy(pb1a);
cout << 4 << endl;
	finish = clock();
	cout << pb1a;
	cout << endl << endl;
	cout << (finish - start) / CLOCKS_PER_SEC << endl;
	//*/


	//--------------------Test Code End-----------------
	//the following code shows how to retrieve the top-k nearest neighbor that contains some query keywords	
	/*
	SearchIBRTree ss(Q, nodeNum, 10);
	irtree.GetTree()->queryStrategy(ss);
	ISpatialIndex* tree= irtree.GetTree();
	
	map<double, int, greater<double>>::reverse_iterator iter;
	int num = 0;
	for(iter = ss.topk.rbegin(); iter!= ss.topk.rend(); ++iter)
	{
	cout<<++num<<" Point ID:"<<iter->second<<" Distance:"<<iter->first<<endl;
	}
	*/
	
	

	/*
	start = clock();
	Problem1Baseline pb1e1(Q);
	pb1e1.BaseLine();
	finish = clock();
	cout<<pb1e1;
	cout<< (finish - start) / CLOCKS_PER_SEC<<endl;

	start = clock();
	Problem1IBRTree pb1e2(Q, nodeNum);
	irtree.GetTree()->queryStrategy(pb1e2);
	finish = clock();
	cout<<pb1e2;
	cout<< (finish - start) / CLOCKS_PER_SEC<<endl;
	//*/

	/*
	//the following code shows how to use the algorithms for the TYPE2 query
	start = clock();
	Problem2Appr1 pb2a1(Q, nodeNum);
	irtree.GetTree()->queryStrategy(pb2a1);
	finish = clock();
	cout<<pb2a1;
	cout<< (finish - start) / CLOCKS_PER_SEC<<endl;

	start = clock();
	Problem2Appr2 pb2a2(Q, nodeNum);
	irtree.GetTree()->queryStrategy(pb2a2);
	finish = clock();
	cout<<pb2a2;
	cout<< (finish - start) / CLOCKS_PER_SEC<<endl;

	start = clock();
	Problem2Exact1 pb2e(Q, nodeNum);
	irtree.GetTree()->queryStrategy(pb2e);
	finish = clock();
	cout<<pb2e;
	cout<< (finish - start) / CLOCKS_PER_SEC<<endl;		//*/

	return 0;
}

