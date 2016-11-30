#ifndef _PREPROCESS_H_
#define _PREPROCESS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//************************************数据类型声明*******************************
#define HTL 13131 //定义hashTable的长度
#define DIM 100
//static int countKeyword[HTL];
typedef struct spatialObjectS {   
	int OID; 
	char *objectName;
	float *position;
	int *grade; //用于记录该Object中各个关键字的级别
	int nOfKeywords;
	char **keywords; //关键字列表
}spatialObject;

typedef struct objectListS {
	int nOfObjects;
	spatialObject *spaObj; //Object列表
} objectList;

typedef struct keywordS {
	char *keywordName;
	int PrimaryIndex;
	int SecondaryIndex;
	int nOfOid;
	int *OIDList;
}keyword;

typedef struct kwdList {
	int nOfkwd;
	char **keywords;
}kwdLs;

typedef struct objectContribution {
	char *keyword;
	int OID;
	float weightDistance;
	float *ctbVector;
	float ctbRation;
} objectCont;

static kwdLs *kl;
static objectList *objList;
static keyword **hsTable[HTL] = {0,}; /* 定义哈希表  ???这样定义是否有问题*/
//************************************函数声明*******************************
spatialObject *spatialObject_new( );
void spatialObject_free(spatialObject *spaObj);
objectList *objectList_new( );
void objectList_free(objectList *objList);
keyword *keyword_new( );
void keyword_free(keyword *kwd);
objectCont *objectCont_new();
void objectCont_free(objectCont * obc);
kwdLs *kwdLs_new();
void kwdLs_free(kwdLs *kls);

objectList *getObjList();
bool notInKwdList(char *kwd); //判断该关键字是否在已有的关键字列表中
void readData(char *filename); //从文件中读取数据，并存放到objList中
void ConstructHashTable(); //从objList中读取数据并构建hashTable并存放在hsTable中
void PreProcess(char *filename); //预处理步骤

#endif


