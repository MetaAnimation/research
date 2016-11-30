#ifndef _MERGELIST_H_
#define _MERGELIST_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "HashTable.h"
#include "PreProcess.h"
#include "MaxPriorityQueue.h"

//************************************������������****************************************
#define GRADE 5//���㼶��ȼ�
#define MAXOBJ 100
#define MINCOST 1e+10
#define INFINITEMIN 1.0e-10

typedef struct userQS {
	float *postion; //���ڼ�¼�û���ѯ��λ��
	int nOfQryKwd; //���ڼ�¼��ѯ�ؼ��ֵĸ���
	char **queryKwd; //���ڼ�¼��ѯ�ؼ�����
	float *preferenceVector; //���ڼ�¼�û��ĸ���ƫ������
	float threshold; //���ڼ�¼��ѯ��ֵ
}userQ;

typedef struct resultObj {
	int nOfObj;
	int *objArray;
} resuArray; 

typedef struct relevantObjectList {
	int nOfRelevant;
	int length;
	objectCont *obCtList;
}reObList; 

typedef struct objectSubSet {
	int nOfObj;
	float weightCost;
	float *ctbVector;
	bool isValid;//���ڱ�ʾ��subSet�Ƿ�ʧЧ
	objectCont *obCtList;
}objSubSet;

typedef struct MergeList {
	int itrNum;//����������Ҳ���ǵ�ǰ��ӵ��׼���Ԫ����
	int nOfSubSet;
	float minCost;//���ڼ�¼��ǰ���ŷ�������СCOST
	bool flag; //���ڼ�¼rObjList�Ƿ��ǵ�һ�θ�ֵ
	resuArray *rObjList;//���ڼ�¼��������������SBUSET
	objSubSet *objSS;//���ڶ�̬ά����ǰ�ĵ����б�
}mgList;

static resuArray* resuObj;

//**************************************��������***************************************
userQ *userQ_new(); 
void userQ_free(userQ *uq);
resuArray *resuArray_new();
void resuArray_free(resuArray *ra);
reObList *reObList_new();
void reObList_free(reObList *rol);
objSubSet *objSubSet_new();
void objSubSet_free(objSubSet *oss);
mgList *mgList_new();
void mgList_free(mgList *ml) ;

int getGrade(char *kwd, spatialObject *spaObj); //��ȡ�ùؼ����ڸ�OBJ�еļ���
float getWeightDistance(spatialObject *spaObj, userQ *uq); //��ȡ��OBJ����ڲ�ѯ���Ȩ�ؾ���
float getCtbRation(objectCont obc, userQ *uq); //��ȡ��OBJ����ڲ�ѯ�Ĺ��ױ�
reObList *constructReObList(userQ *uq); //�������ѯ��ص�OBJ�б�
void weightCostSort(reObList *rol, userQ *uq); //����weightCost��С�������ص�OBJ��������
bool isGreaterTh(float *ctbArray, float *rolArray, userQ *uq) ; //�жϵ�ǰ�����������Ƿ񳬹���ֵ
mgList *wgtPriMergeList(reObList *rol, userQ *uq); //����Ȩ��Cost��С�����˳����OBJ
void ctrRatioSort(reObList *rol, userQ *uq) ; //���չ��ױ��ɴ�С����ص�OBJ��������
float residualCost(objSubSet oss, float ctrRation, userQ *uq); //���¼��㹱����
mgList *ctrPriMergeList(reObList *rol, userQ *uq); //���չ������ɴ�С��˳����OBJ;
void printObjectList(mgList *ml); //��OBL�б��ӡ����
int _tmain(int argc, _TCHAR* argv[]); //���������

#endif 

