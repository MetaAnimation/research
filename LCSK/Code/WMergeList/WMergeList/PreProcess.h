#ifndef _PREPROCESS_H_
#define _PREPROCESS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//************************************������������*******************************
#define HTL 13131 //����hashTable�ĳ���
#define DIM 100
//static int countKeyword[HTL];
typedef struct spatialObjectS {   
	int OID; 
	char *objectName;
	float *position;
	int *grade; //���ڼ�¼��Object�и����ؼ��ֵļ���
	int nOfKeywords;
	char **keywords; //�ؼ����б�
}spatialObject;

typedef struct objectListS {
	int nOfObjects;
	spatialObject *spaObj; //Object�б�
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
static keyword **hsTable[HTL] = {0,}; /* �����ϣ��  ???���������Ƿ�������*/
//************************************��������*******************************
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
bool notInKwdList(char *kwd); //�жϸùؼ����Ƿ������еĹؼ����б���
void readData(char *filename); //���ļ��ж�ȡ���ݣ�����ŵ�objList��
void ConstructHashTable(); //��objList�ж�ȡ���ݲ�����hashTable�������hsTable��
void PreProcess(char *filename); //Ԥ������

#endif


