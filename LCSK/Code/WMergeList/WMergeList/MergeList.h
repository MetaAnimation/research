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

//************************************数据类型声明****************************************
#define GRADE 5//景点级别等级
#define MAXOBJ 100
#define MINCOST 1e+10
#define INFINITEMIN 1.0e-10

typedef struct userQS {
	float *postion; //用于记录用户查询的位置
	int nOfQryKwd; //用于记录查询关键字的个数
	char **queryKwd; //用于记录查询关键字组
	float *preferenceVector; //用于记录用户的个人偏好向量
	float threshold; //用于记录查询阈值
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
	bool isValid;//用于表示该subSet是否失效
	objectCont *obCtList;
}objSubSet;

typedef struct MergeList {
	int itrNum;//迭代次数，也就是当前添加到底几个元素了
	int nOfSubSet;
	float minCost;//用于记录当前最优方案的最小COST
	bool flag; //用于记录rObjList是否是第一次赋值
	resuArray *rObjList;//用于记录满足条件的最优SBUSET
	objSubSet *objSS;//用于动态维护当前的迭代列表
}mgList;

static resuArray* resuObj;

//**************************************函数声明***************************************
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

int getGrade(char *kwd, spatialObject *spaObj); //获取该关键字在该OBJ中的级别
float getWeightDistance(spatialObject *spaObj, userQ *uq); //获取该OBJ相对于查询点的权重距离
float getCtbRation(objectCont obc, userQ *uq); //获取该OBJ相对于查询的贡献比
reObList *constructReObList(userQ *uq); //构建与查询相关的OBJ列表
void weightCostSort(reObList *rol, userQ *uq); //按照weightCost由小到大对相关的OBJ进行排序
bool isGreaterTh(float *ctbArray, float *rolArray, userQ *uq) ; //判断当前贡献率数组是否超过阈值
mgList *wgtPriMergeList(reObList *rol, userQ *uq); //按照权重Cost由小到大的顺序处理OBJ
void ctrRatioSort(reObList *rol, userQ *uq) ; //按照贡献比由大到小对相关的OBJ进行排序
float residualCost(objSubSet oss, float ctrRation, userQ *uq); //重新计算贡献率
mgList *ctrPriMergeList(reObList *rol, userQ *uq); //按照贡献率由大到小的顺序处理OBJ;
void printObjectList(mgList *ml); //将OBL列表打印出来
int _tmain(int argc, _TCHAR* argv[]); //主函数入口

#endif 

