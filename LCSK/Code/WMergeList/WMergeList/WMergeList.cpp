#include "StdAfx.h"
#include "MergeList.h"
#include <time.h>
userQ *userQ_new() {
	userQ *returnValue = (userQ *) malloc(sizeof(userQ));
	returnValue->nOfQryKwd = 0;
	returnValue->postion = (float *) malloc(2 * sizeof(float));
	returnValue->preferenceVector = (float *) malloc((GRADE+1) * sizeof(float));//注意？？？用户偏好向量从下标1开始
	memset(returnValue->preferenceVector, 0.0, (GRADE+1) * sizeof(float));
	returnValue->threshold = -1.0;
	return returnValue;
}

void userQ_free(userQ *uq) {
	free(uq->postion);
	free(uq->preferenceVector);
	for(int i=0; i<uq->nOfQryKwd; i++) {
		free(uq->queryKwd[i]);
	}
	free(uq->queryKwd);//这样释放应该没问题吧？？？释放完它的内容再把它释放掉
	free(uq);
}


resuArray *resuArray_new() {
	resuArray *returnValue = (resuArray *) malloc(sizeof(resuArray));
	returnValue->nOfObj = 0;
	returnValue->objArray = (int *) malloc(MAXOBJ * sizeof(int));
	memset(returnValue->objArray, -1, MAXOBJ * sizeof(int));
	return returnValue;
}

void resuArray_free(resuArray *ra) {
	free(ra->objArray);
	free(ra);
}

reObList *reObList_new() {
	reObList *returnValue = (reObList *) malloc(sizeof(reObList));
	returnValue->length = -1;
	returnValue->nOfRelevant = 0;
	return returnValue;
}

void reObList_free(reObList *rol) {
	free(rol->obCtList);
	free(rol);
}

objSubSet *objSubSet_new() {
	objSubSet *returnValue = (objSubSet *) malloc(sizeof(objSubSet));
	returnValue->nOfObj = 0;
	returnValue->weightCost = 0.0;
	returnValue->isValid = false;
	return returnValue;
}

void objSubSet_free(objSubSet *oss) {
	free(oss->ctbVector);
	free(oss->obCtList);
	free(oss);
}

mgList *mgList_new() {
	mgList *returnValue = (mgList *) malloc(sizeof(mgList));
	returnValue->itrNum = 0;
	returnValue->minCost = MINCOST;
	returnValue->nOfSubSet = 0;
	returnValue->flag = true;
	returnValue->rObjList = (resuArray  *) malloc(sizeof(resuArray));
	memset(returnValue->rObjList, 0, sizeof(resuArray));
	returnValue->rObjList->nOfObj = 0;
	returnValue->rObjList->objArray = (int *)malloc(sizeof(int));
	memset(returnValue->rObjList->objArray, 0, sizeof(int));
	return returnValue;
}

void mgList_free(mgList *ml) {
	free(ml->objSS);
	free(ml->rObjList);
	free(ml);
}

int getGrade(char *kwd, spatialObject *spaObj) {
	for(int i=0; i<spaObj->nOfKeywords; i++) {
		if(!strcmp(kwd,spaObj->keywords[i])) {//如果两个关键字匹配上了
			return spaObj->grade[i];
		}
	}
	return 0;
}

float getWeightDistance(spatialObject *spaObj, userQ *uq) {
	float dis = sqrt((spaObj->position[0]-uq->postion[0]) * (spaObj->position[0]-uq->postion[0]) + (spaObj->position[1]-uq->postion[1]) * (spaObj->position[1]-uq->postion[1]));
	int weight = 0;
	for(int i=0; i<spaObj->nOfKeywords; i++) weight += spaObj->grade[i];
	return weight*dis;
}

float getCtbRation(objectCont obc, userQ *uq) {
	float marginSum = 0.0;
	for(int i=0; i<uq->nOfQryKwd; i++) {
		marginSum += obc.ctbVector[i]; //？？？？？哪里计算了obc.ctbVector[i]
	}
	return marginSum;
}

reObList *constructReObList(userQ *uq){//构建与查询相关的OBJ列表,返回的OBJ列表中信息是全的
	char *kwd;
	keyword *keyStct;
	spatialObject *spaObj;
	int oid = -1;
	int temp = 0;
	//用于临时保存OBJ的信息，后面会更新到列表中
	reObList *rol = reObList_new();
	objectList *ol = getObjList();
	//对每个关键字获取它的ObjectList并计算
	for(int i=0; i<uq->nOfQryKwd; i++) {
		kwd = uq->queryKwd[i]; //？？？？这样赋值应该没问题
		keyStct = search_hash_key(kwd);
		//PriorityQueue priQueue = Initialize( keyStct->nOfOid );
       
		for(int j=0; j<keyStct->nOfOid; j++) {
			oid = keyStct->OIDList[j];
			spaObj = (&(ol->spaObj[oid]));
			//spaObj = objList->spaObj[oid];
			//如果当前OBJ没加入到ROL将其加入
			if(rol->length < oid) {
				temp = rol->length;
				rol->length = oid;
				rol->nOfRelevant++;
				if(rol->nOfRelevant == 1) {
					rol->obCtList = (objectCont *) malloc(sizeof(objectCont) * (rol->length+1));
					//为新建的OBJ分配空间以及进行初始化				
					//rol->obCtList[rol->length].
				} else { 
					rol->obCtList = (objectCont *) realloc(rol->obCtList, sizeof(objectCont)*(rol->length+1));
				}
				//对位于rol->length - oid之间新建的那些rol进行初始化工作
				for(int j=temp+1; j<=oid; j++) {
					rol->obCtList[j].OID = -1;
					rol->obCtList[j].ctbRation = 0.0;
					rol->obCtList[j].ctbVector = (float *) malloc(sizeof(float) * uq->nOfQryKwd);
					memset(rol->obCtList[j].ctbVector, 0.0, sizeof(float) * uq->nOfQryKwd);
					rol->obCtList[j].weightDistance = 0.0;
				}
				//对OID进行赋值
				rol->obCtList[oid].OID = oid;
				//注意？？？？分配空间然后再拷贝内存，rol->obCtList[oid].keyword = kwd;
				rol->obCtList[oid].keyword = (char *) malloc(sizeof(char) * strlen(kwd));
				strcpy(rol->obCtList[oid].keyword,kwd);
				//memcpy(rol->obCtList[oid].keyword, kwd, strlen(kwd));
				
				//spaObj = &(ol->spaObj[oid]);

				rol->obCtList[oid].weightDistance = getWeightDistance(spaObj, uq);
				//rol->obCtList[oid].ctbVector = (float *)malloc(uq->nOfQryKwd * sizeof(float));
				//memset()
				rol->obCtList[oid].ctbVector[i] = uq->preferenceVector[getGrade(kwd, spaObj)];//获取该关键字偏好值,？？？注意这里的Grade是从1-5
				
			} else {
				//spaObj = (&(ol->spaObj[oid]));
				//如果该OBJ已经在List里面了，只要更新ctbVector便可
				if(rol->obCtList[oid].OID > -1) {
					rol->obCtList[oid].ctbVector[i] = uq->preferenceVector[getGrade(kwd, spaObj)];//获取该关键字偏好值
				} else {//该空间已有，但OBJ还没写进去
					rol->nOfRelevant++;
					//首次将该obCtList[oid]写进去
					rol->obCtList[oid].OID = oid;
					//注意分配空间然后再拷贝内存，rol->obCtList[oid].keyword = kwd;
					rol->obCtList[oid].keyword = (char *) malloc(sizeof(char) * strlen(kwd));
					strcpy(rol->obCtList[oid].keyword,kwd);
					//memcpy(rol->obCtList[oid].keyword, kwd, strlen(kwd));
					//spaObj = &(ol->spaObj[oid]);
					rol->obCtList[oid].weightDistance = getWeightDistance(spaObj, uq);
					//rol->obCtList[oid].ctbVector = (float *)malloc(uq->nOfQryKwd * sizeof(float));
					rol->obCtList[oid].ctbVector[i] = uq->preferenceVector[getGrade(kwd, spaObj)];//获取该关键字偏好值
				}
			}
		}

	}
	//到目前为止获得的rol是稀松的，需要压缩
	reObList *returnValue = reObList_new();
	returnValue->length = rol->nOfRelevant;
	returnValue->nOfRelevant = 0;

	returnValue->obCtList = (objectCont *) malloc(rol->nOfRelevant * sizeof(objectCont));
	//对returnValue->obCtList进行初始化工作
	for(int i=0; i<rol->nOfRelevant; i++) {
		returnValue->obCtList[i].OID = -1;
		returnValue->obCtList[i].ctbRation = 0.0;
		returnValue->obCtList[i].ctbVector = (float *) malloc(sizeof(float) * uq->nOfQryKwd);
		memset(returnValue->obCtList[i].ctbVector, 0.0, sizeof(float) * uq->nOfQryKwd);
		returnValue->obCtList[i].weightDistance = 0.0;	
	}
//printf("%f %f %f %f\n",rol->obCtList[0].ctbVector[0],rol->obCtList[0].ctbVector[1],rol->obCtList[0].ctbVector[2],rol->obCtList[0].ctbVector[3]);
	int k;
	int l;
	for(k=0,l=0; k<rol->length; k++) {
		if(rol->obCtList[k].OID > -1) {//表明第K个位置有元素
			//???这样赋值会不会出问题，因为里面有指针类型的数据比如ctbVector
			//returnValue->obCtList[l].ctbRation = rol->obCtList[k].ctbRation;
			returnValue->nOfRelevant++;
			//printf("%f %f %f %f\n",rol->obCtList[k].ctbVector[0],rol->obCtList[k].ctbVector[1],rol->obCtList[k].ctbVector[2],rol->obCtList[k].ctbVector[3]);
			returnValue->obCtList[l].ctbRation = getCtbRation(rol->obCtList[k],uq)/(rol->obCtList[k].weightDistance+INFINITEMIN);
			//returnValue->obCtList[l].ctbVector = rol->obCtList[k].ctbVector;
			memcpy(returnValue->obCtList[l].ctbVector, rol->obCtList[k].ctbVector, sizeof(float)*uq->nOfQryKwd);
			//returnValue->obCtList[l].keyword = rol->obCtList[k].keyword;
			returnValue->obCtList[l].keyword = (char *) malloc(sizeof(char) * strlen(rol->obCtList[k].keyword));
			strcpy(returnValue->obCtList[l].keyword, rol->obCtList[k].keyword);
			returnValue->obCtList[l].OID = rol->obCtList[k].OID;
			returnValue->obCtList[l].weightDistance = rol->obCtList[k].weightDistance;
			l++;
		}
	}

	return returnValue;

}

void weightCostSort(reObList *rol, userQ *uq) {
	int i;
	int j;
	//升序排列
	objectCont temp;
	
	for(i=0;i<rol->nOfRelevant-1;i++)
    {
        for(j=0;j<rol->nOfRelevant-1-i;j++)
        {
			if(rol->obCtList[j].weightDistance > rol->obCtList[j+1].weightDistance)
            {
				temp = rol->obCtList[j];
				rol->obCtList[j] = rol->obCtList[j+1];
				rol->obCtList[j+1] = temp;
            }
        } 
    }
}

bool isGreaterTh(float *ctbArray, float *rolArray, userQ *uq) {
	for(int i=0; i<uq->nOfQryKwd; i++) {
		if(ctbArray[i] + rolArray[i]< uq->threshold) return false;
	}
	return true;
}

mgList *wgtPriMergeList(reObList *rol, userQ *uq) {//按照权重Cost由小到大的顺序处理OBJ
	//FILE *filePtr;
	/*
	filePtr = (FILE *) fopen("D:\\rolPre.txt", "w");
	if (filePtr == NULL)
	{
		printf("File %s does not exist!\n", "D:\\rolPre.txt");
		exit(0);
	}
	for(int i=0; i<rol->nOfRelevant; i++) {
		fprintf(filePtr,"%d %f\n",rol->obCtList[i].OID,rol->obCtList[i].weightDistance);
	}
	fclose(filePtr);
	*/
	weightCostSort(rol, uq);
	/*
	FILE *filePtr2;
	filePtr2 = (FILE *) fopen("D:\\rolPost.txt", "w");
	if (filePtr2 == NULL)
	{
		printf("File %s does not exist!\n", "D:\\rolPost.txt");
		exit(0);
	}
	for(int i=0; i<rol->nOfRelevant; i++) {
		fprintf(filePtr2,"%d %f\n",rol->obCtList[i].OID,rol->obCtList[i].weightDistance);
	}
	fclose(filePtr2);
	*/
	mgList *ml = mgList_new();
//printf("The number of nOfRelevant is:%d\n",rol->nOfRelevant);
	//如果rol->nOfRelevant不为0，为SS建立一个空的子集；
	if(rol->nOfRelevant > 0) {
		ml->nOfSubSet++;

		ml->objSS = (objSubSet *)malloc(sizeof(objSubSet));
		ml->objSS[0].isValid = true;
		ml->objSS[0].nOfObj = 0;
		ml->objSS[0].weightCost = 0.0;
		ml->objSS[0].ctbVector = (float *)malloc(uq->nOfQryKwd * sizeof(float));
		memset(ml->objSS[0].ctbVector, 0.0, uq->nOfQryKwd * sizeof(float));
		ml->objSS[0].obCtList = NULL;
	}

	for(int i=0; i<rol->nOfRelevant; i++) {
		//逐渐将新的OBJ添加到列表中并更新MergeList
//printf("NofS is:%d\n",ml->nOfSubSet);
//printf("The item is:%d\n",i+1);
		ml->itrNum = i+1;
		//如果当前OBJ的cost大于当前最优的结果，那么直接退出
		if(rol->obCtList[i].weightDistance >= ml->minCost) {
			//ml->flag = false;
			break;
		}

		int loopNum = ml->nOfSubSet;
		ml->objSS = (objSubSet *)realloc(ml->objSS,loopNum * 2 * sizeof(objSubSet));//为每一个可能的SUBSET分配空间
		//为每一个新分配的集合进行初始化工作
		for(int t=loopNum; t<loopNum * 2; t++) {
			ml->objSS[t].isValid = false;
			ml->objSS[t].nOfObj = 0;
			ml->objSS[t].weightCost = 0.0;
		}
		ml->flag = false;
		for(int j=0; j<loopNum; j++) {
			
			//如果当前阈值之和大于最优解，那么将该Set置为无效，因为后续任何该子集的父集大于最优解
			if((ml->objSS[j].weightCost+rol->obCtList[i].weightDistance) >= ml->minCost) {
				ml->objSS[j].isValid = false;
				continue;
			}
			//如果第J个SUBSET是有效的，将当前OBJ加入ml->nOfSubSet列表			
			if(ml->objSS[j].isValid == true) {
				//如果当前子集与当前OBJ之和大于阈值（可行解），那么修改最优值
				ml->flag = true;
				if(isGreaterTh(ml->objSS[j].ctbVector,rol->obCtList[i].ctbVector,uq)) {
					//如果大于阈值，不论后面还有没有，（cost递增顺序）都只会大于该Cost，所以对于这个SubSet来说没必要继续访问了。
					ml->objSS[j].isValid = false;
					//if(ml->minCost > (ml->objSS[j].weightCost+rol->obCtList[i].weightDistance)) {
					ml->minCost = ml->objSS[j].weightCost+rol->obCtList[i].weightDistance;
					ml->rObjList->nOfObj = ml->objSS[j].nOfObj+1;
					//这样赋值有问题吗，直接将整个数组赋值????????????
					free(ml->rObjList->objArray);
					ml->rObjList->objArray = NULL;
					ml->rObjList->objArray = (int *) malloc(ml->rObjList->nOfObj*sizeof(int));
					memset(ml->rObjList->objArray, 0, ml->rObjList->nOfObj*sizeof(int));
					int l;
					for(l=0; l<ml->objSS[j].nOfObj; l++) {
						ml->rObjList->objArray[l] = ml->objSS[j].obCtList[l].OID;
					}
					ml->rObjList->objArray[l] = rol->obCtList[i].OID;
					//}
				} else {//如果小于阈值,将它压入G中
					ml->objSS[ml->nOfSubSet].isValid = true;
					ml->objSS[ml->nOfSubSet].nOfObj = ml->objSS[j].nOfObj+1;
					ml->objSS[ml->nOfSubSet].weightCost = ml->objSS[j].weightCost + rol->obCtList[i].weightDistance;
					ml->objSS[ml->nOfSubSet].ctbVector = (float *)malloc(sizeof(float)*uq->nOfQryKwd);
					memset(ml->objSS[ml->nOfSubSet].ctbVector, 0.0, sizeof(float)*uq->nOfQryKwd);
					for(int k=0; k<uq->nOfQryKwd; k++) {
						ml->objSS[ml->nOfSubSet].ctbVector[k] = ml->objSS[j].ctbVector[k] + rol->obCtList[i].ctbVector[k];
					}
					ml->objSS[ml->nOfSubSet].obCtList = (objectCont *)malloc((ml->objSS[j].nOfObj+1)*sizeof(objectCont));
					
					//将SUBSET J赋值给当前节点
					//memcpy(ml->objSS[ml->nOfSubSet].obCtList, ml->objSS[j].obCtList, (ml->objSS[j].nOfObj)*sizeof(objectCont));
					//对obCtList进行初始化			
					for(int l=0; l<=ml->objSS[j].nOfObj; l++) {
						ml->objSS[ml->nOfSubSet].obCtList[l].ctbRation = 0.0;
						ml->objSS[ml->nOfSubSet].obCtList[l].ctbVector = (float *)malloc(sizeof(float) * uq->nOfQryKwd);
						memset(ml->objSS[ml->nOfSubSet].obCtList[l].ctbVector, 0.0, sizeof(float) * uq->nOfQryKwd);
						ml->objSS[ml->nOfSubSet].obCtList[l].OID = -1;
						ml->objSS[ml->nOfSubSet].obCtList[l].weightDistance = 0.0;
					}
					//if(j != 0) { //如果j!=0，那么将objSS[j]所有的数据赋值给ml->objSS[ml->nOfSubSet].obCtList[l]
					for(int l=0; l<ml->objSS[j].nOfObj; l++) {
						ml->objSS[ml->nOfSubSet].obCtList[l].ctbRation = ml->objSS[j].obCtList[l].ctbRation;
						memcpy(ml->objSS[ml->nOfSubSet].obCtList[l].ctbVector, ml->objSS[j].obCtList[l].ctbVector, sizeof(float) * uq->nOfQryKwd);
						ml->objSS[ml->nOfSubSet].obCtList[l].keyword = (char *)malloc(sizeof(char)*strlen(ml->objSS[j].obCtList[l].keyword));
						strcpy(ml->objSS[ml->nOfSubSet].obCtList[l].keyword,ml->objSS[j].obCtList[l].keyword);
						//ml->objSS[ml->nOfSubSet].obCtList[l].keyword = ml->objSS[j].obCtList[l].keyword;
						ml->objSS[ml->nOfSubSet].obCtList[l].OID = ml->objSS[j].obCtList[l].OID;
						ml->objSS[ml->nOfSubSet].obCtList[l].weightDistance = ml->objSS[j].obCtList[l].weightDistance;
					}
				//}
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].ctbRation = rol->obCtList[i].ctbRation;
					memcpy(ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].ctbVector, rol->obCtList[i].ctbVector,sizeof(float)*uq->nOfQryKwd );
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].keyword = (char *)malloc(sizeof(char)*strlen(rol->obCtList[i].keyword));
					strcpy(ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].keyword, rol->obCtList[i].keyword);
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].OID = rol->obCtList[i].OID;
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].weightDistance = rol->obCtList[i].weightDistance;

					ml->nOfSubSet++;
				}//if
			}//if
		}//for
		if(!ml->flag) break;
	}//for
					
//int tme = -1;
	return ml;
}
/*
mgList *wgtPriMergeList(reObList *rol, userQ *uq) {//按照权重Cost由小到大的顺序处理OBJ
	reObList *roll = rol;
	weightCostSort(rol, uq);

	mgList *ml = mgList_new();
printf("The number of nOfRelevant is:%d\n",rol->nOfRelevant);	
	for(int i=0; i<rol->nOfRelevant; i++) {
		//逐渐将新的OBJ添加到列表中并更新MergeList
		printf("The item is:%d\n",i+1);
		ml->itrNum = i+1;
		if(ml->nOfSubSet == 0) {
			ml->nOfSubSet = 2;
			ml->objSS = (objSubSet *)malloc(2*sizeof(objSubSet));

			for(int j=0; j<ml->nOfSubSet; j++) {
				ml->objSS[j].isValid = false;
				ml->objSS[j].nOfObj = 0;
				ml->objSS[j].weightCost = 0.0;
				ml->objSS[j].ctbVector = (float *)malloc(uq->nOfQryKwd * sizeof(float));
				memset(ml->objSS[j].ctbVector, 0.0, uq->nOfQryKwd * sizeof(float));
			}
			ml->objSS[0].isValid = true;
			
			for(int j=0; j<uq->nOfQryKwd; j++) {
				ml->objSS[ml->nOfSubSet-1].ctbVector[j] += rol->obCtList[i].ctbVector[j];
//float test = rol->obCtList[i].ctbVector[j];
			}
			
			ml->objSS[ml->nOfSubSet-1].isValid = true;
			ml->objSS[ml->nOfSubSet-1].nOfObj++;
			ml->objSS[ml->nOfSubSet-1].weightCost += rol->obCtList[i].weightDistance;
			int id =  rol->obCtList[i].OID;
			if(ml->objSS[ml->nOfSubSet-1].nOfObj == 1) {
				ml->objSS[ml->nOfSubSet-1].obCtList = (objectCont *) malloc(sizeof(objectCont));
			} else {
				ml->objSS[ml->nOfSubSet-1].obCtList = (objectCont *) realloc(ml->objSS[ml->nOfSubSet-1].obCtList,sizeof(objectCont) * ml->objSS[ml->nOfSubSet-1].nOfObj);
			}
			int nOfSS = ml->nOfSubSet-1;
			int nofObj = ml->objSS[ml->nOfSubSet-1].nOfObj - 1;

			ml->objSS[nOfSS].obCtList[nofObj].ctbRation = rol->obCtList[i].ctbRation;
			ml->objSS[nOfSS].obCtList[nofObj].ctbVector = (float *) malloc(sizeof(float)*uq->nOfQryKwd);
			memset(ml->objSS[nOfSS].obCtList[nofObj].ctbVector, 0.0, sizeof(float)*uq->nOfQryKwd);
			memcpy(ml->objSS[nOfSS].obCtList[nofObj].ctbVector, rol->obCtList[i].ctbVector, sizeof(float)*uq->nOfQryKwd);

			//ml->objSS[nOfSS].obCtList[nofObj].ctbVector = rol->obCtList[i].ctbVector;
			//ml->objSS[nOfSS].obCtList[nofObj].keyword = rol->obCtList[i].keyword;
			ml->objSS[nOfSS].obCtList[nofObj].keyword = (char *) malloc(sizeof(char) * strlen(rol->obCtList[i].keyword));
			strcpy(ml->objSS[nOfSS].obCtList[nofObj].keyword, rol->obCtList[i].keyword);
			ml->objSS[nOfSS].obCtList[nofObj].OID = rol->obCtList[i].OID;
			ml->objSS[nOfSS].obCtList[nofObj].weightDistance = rol->obCtList[i].weightDistance;
			
			if(isGreaterTh(ml->objSS[nOfSS].ctbVector, uq)) {
				ml->objSS[nOfSS].isValid = false;
				//ml->minCost = ml->objSS[ml->objSS->nOfObj-1].weightCost;
				if(ml->minCost > ml->objSS[nOfSS].weightCost) {
					ml->minCost = ml->objSS[nOfSS].weightCost;
					ml->rObjList->nOfObj = ml->objSS[nOfSS].nOfObj;
					//这样赋值有问题吗，直接将整个数组赋值????????????
					free(ml->rObjList->objArray);
					ml->rObjList->objArray = NULL;
					ml->rObjList->objArray = (int *) malloc(ml->rObjList->nOfObj*sizeof(int));
					memset(ml->rObjList->objArray, 0, ml->rObjList->nOfObj*sizeof(int));
					for(int l=0; l<ml->objSS[nOfSS].nOfObj; l++) {
						ml->rObjList->objArray[l] = ml->objSS[nOfSS].obCtList[l].OID;
					}
				}							
			} else {
				//过滤策略
				if(ml->objSS[nOfSS].weightCost > ml->minCost) {
					ml->objSS[nOfSS].isValid = false;
				}
			}
		} else {
			int loopNum = ml->nOfSubSet;
			ml->objSS = (objSubSet *)realloc(ml->objSS,loopNum * 2 * sizeof(objSubSet));//为每一个可能的SUBSET分配空间
			//为每一个新分配的集合进行初始化工作
			for(int t=loopNum; t<loopNum * 2; t++) {
				ml->objSS[t].isValid = false;
				ml->objSS[t].nOfObj = 0;
				ml->objSS[t].weightCost = 0.0;
			}

			for(int j=0; j<loopNum; j++) {
				//如果第J个SUBSET是有效的，将当前OBJ加入ml->nOfSubSet列表
				if(ml->objSS[j].isValid) {
					ml->flag = true;
					ml->objSS[ml->nOfSubSet].isValid = true;
					ml->objSS[ml->nOfSubSet].nOfObj = ml->objSS[j].nOfObj+1;
					ml->objSS[ml->nOfSubSet].weightCost = ml->objSS[j].weightCost + rol->obCtList[i].weightDistance;
					ml->objSS[ml->nOfSubSet].ctbVector = (float *)malloc(sizeof(float)*uq->nOfQryKwd);
					memset(ml->objSS[ml->nOfSubSet].ctbVector, 0.0, sizeof(float)*uq->nOfQryKwd);
					for(int k=0; k<uq->nOfQryKwd; k++) {
						ml->objSS[ml->nOfSubSet].ctbVector[k] = ml->objSS[j].ctbVector[k] + rol->obCtList[i].ctbVector[k];
//float test = rol->obCtList[i].ctbVector[k];
					}
					ml->objSS[ml->nOfSubSet].obCtList = (objectCont *)malloc((ml->objSS[j].nOfObj+1)*sizeof(objectCont));
					
					//将SUBSET J赋值给当前节点
					//memcpy(ml->objSS[ml->nOfSubSet].obCtList, ml->objSS[j].obCtList, (ml->objSS[j].nOfObj)*sizeof(objectCont));
					//对obCtList进行初始化			
					for(int l=0; l<=ml->objSS[j].nOfObj; l++) {
						ml->objSS[ml->nOfSubSet].obCtList[l].ctbRation = 0.0;
						ml->objSS[ml->nOfSubSet].obCtList[l].ctbVector = (float *)malloc(sizeof(float) * uq->nOfQryKwd);
						memset(ml->objSS[ml->nOfSubSet].obCtList[l].ctbVector, 0.0, sizeof(float) * uq->nOfQryKwd);
						ml->objSS[ml->nOfSubSet].obCtList[l].OID = -1;
						ml->objSS[ml->nOfSubSet].obCtList[l].weightDistance = 0.0;
					}
					if(j != 0) { //如果j!=0，那么将objSS[j]所有的数据赋值给ml->objSS[ml->nOfSubSet].obCtList[l]
						for(int l=0; l<ml->objSS[j].nOfObj; l++) {
							ml->objSS[ml->nOfSubSet].obCtList[l].ctbRation = ml->objSS[j].obCtList[l].ctbRation;
							memcpy(ml->objSS[ml->nOfSubSet].obCtList[l].ctbVector, ml->objSS[j].obCtList[l].ctbVector, sizeof(float) * uq->nOfQryKwd);
							ml->objSS[ml->nOfSubSet].obCtList[l].keyword = (char *)malloc(sizeof(char)*strlen(ml->objSS[j].obCtList[l].keyword));
							strcpy(ml->objSS[ml->nOfSubSet].obCtList[l].keyword,ml->objSS[j].obCtList[l].keyword);
							//ml->objSS[ml->nOfSubSet].obCtList[l].keyword = ml->objSS[j].obCtList[l].keyword;
							ml->objSS[ml->nOfSubSet].obCtList[l].OID = ml->objSS[j].obCtList[l].OID;
							ml->objSS[ml->nOfSubSet].obCtList[l].weightDistance = ml->objSS[j].obCtList[l].weightDistance;
						}
					}
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].ctbRation = rol->obCtList[i].ctbRation;
					memcpy(ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].ctbVector, rol->obCtList[i].ctbVector,sizeof(float)*uq->nOfQryKwd );
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].keyword = (char *)malloc(sizeof(char)*strlen(rol->obCtList[i].keyword));
					strcpy(ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].keyword, rol->obCtList[i].keyword);
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].OID = rol->obCtList[i].OID;
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].weightDistance = rol->obCtList[i].weightDistance;
			
					if(isGreaterTh(ml->objSS[ml->nOfSubSet].ctbVector, uq)) {
						ml->objSS[ml->nOfSubSet].isValid = false;
						//ml->minCost = ml->objSS[ml->objSS->nOfObj-1].weightCost;
						if(ml->minCost > ml->objSS[ml->nOfSubSet].weightCost) {		
							ml->minCost = ml->objSS[ml->nOfSubSet].weightCost;
							ml->rObjList->nOfObj = ml->objSS[ml->nOfSubSet].nOfObj;
							//这样赋值有问题吗，直接将整个数组赋值????????????
							free(ml->rObjList->objArray);
							ml->rObjList->objArray = NULL;
							ml->rObjList->objArray = (int *) malloc(ml->rObjList->nOfObj*sizeof(int));
							memset(ml->rObjList->objArray, 0, ml->rObjList->nOfObj*sizeof(int));
							for(int l=0; l<ml->rObjList->nOfObj; l++) {
								ml->rObjList->objArray[l] = ml->objSS[ml->nOfSubSet].obCtList[l].OID;
							}
						}
					} else {
						//过滤策略
						if(ml->objSS[ml->nOfSubSet].weightCost > ml->minCost) {
							ml->objSS[ml->nOfSubSet].isValid = false;
						}
					}
					ml->nOfSubSet++;
				} 
			}//for 
			if(!ml->flag) break; //如果ml->flag为假说明它的所有的元素都为无效，可以直接退出
		}		
	}
//int tme = -1;
	return ml;
}*/

void ctrRatioSort(reObList *rol, userQ *uq) {
	int i;
	int j;
	//降序排列
	//objectCont *temp = objectCont_new();
	objectCont temp;
	for(i=0;i<rol->nOfRelevant-1;i++)
    {
        for(j=0;j<rol->nOfRelevant-1-i;j++)
        {
			if(rol->obCtList[j].ctbRation < rol->obCtList[j+1].ctbRation)
            {
				//这样赋值会不会有问题？
				temp = rol->obCtList[j];
				rol->obCtList[j] = rol->obCtList[j+1];
				rol->obCtList[j+1] = temp;
            }
        } 
    }
}

float residualCost(objSubSet oss, float ctrRation, userQ *uq) {
	float residual = 0.0;
	for(int i=0; i<uq->nOfQryKwd; i++) {
		if(oss.ctbVector[i] > uq->threshold) ;
		else residual += (uq->threshold - oss.ctbVector[i]);
	}
	return residual / ctrRation;
}

mgList *ctrPriMergeList(reObList *rol, userQ *uq) {//按照贡献率由大到小的顺序处理OBJ
	
	ctrRatioSort(rol, uq);
	mgList *ml = mgList_new();
	/*
	for(int i=0; i<rol->nOfRelevant; i++) {
		//逐渐将新的OBJ添加到列表中并更新MergeList
//printf("The item is:%d\n",i+1);
		ml->itrNum = i+1;
		if(ml->nOfSubSet == 0) {
			ml->nOfSubSet = 2;
			ml->objSS = (objSubSet *)malloc(2*sizeof(objSubSet));

			for(int j=0; j<ml->nOfSubSet; j++) {
				ml->objSS[j].isValid = false;
				ml->objSS[j].nOfObj = 0;
				ml->objSS[j].weightCost = 0.0;
				ml->objSS[j].ctbVector = (float *)malloc(uq->nOfQryKwd * sizeof(float));
				memset(ml->objSS[j].ctbVector, 0.0, uq->nOfQryKwd * sizeof(float));
				//ml->objSS[j].obCtLis
			}
			ml->objSS[0].isValid = true;
			for(int j=0; j<uq->nOfQryKwd; j++) {
				ml->objSS[ml->nOfSubSet-1].ctbVector[j] += rol->obCtList[i].ctbVector[j];
//float test = rol->obCtList[i].ctbVector[j];
			}
			
			ml->objSS[ml->nOfSubSet-1].isValid = true;
			ml->objSS[ml->nOfSubSet-1].nOfObj++;
			ml->objSS[ml->nOfSubSet-1].weightCost += rol->obCtList[i].weightDistance;
			int id =  rol->obCtList[i].OID;
			if(ml->objSS[ml->nOfSubSet-1].nOfObj == 1) {
				ml->objSS[ml->nOfSubSet-1].obCtList = (objectCont *) malloc(sizeof(objectCont));
			} else {
				ml->objSS[ml->nOfSubSet-1].obCtList = (objectCont *) realloc(ml->objSS[ml->nOfSubSet-1].obCtList,sizeof(objectCont) * ml->objSS[ml->nOfSubSet-1].nOfObj);
			}
			int nOfSS = ml->nOfSubSet-1;
			int nofObj = ml->objSS[ml->nOfSubSet-1].nOfObj - 1;

			ml->objSS[nOfSS].obCtList[nofObj].ctbRation = rol->obCtList[i].ctbRation;
			ml->objSS[nOfSS].obCtList[nofObj].ctbVector = (float *) malloc(sizeof(float)*uq->nOfQryKwd);
			memset(ml->objSS[nOfSS].obCtList[nofObj].ctbVector, 0.0, sizeof(float)*uq->nOfQryKwd);
			memcpy(ml->objSS[nOfSS].obCtList[nofObj].ctbVector, rol->obCtList[i].ctbVector, sizeof(float)*uq->nOfQryKwd);
			ml->objSS[nOfSS].obCtList[nofObj].keyword = (char *) malloc(sizeof(char) * strlen(rol->obCtList[i].keyword));
			strcpy(ml->objSS[nOfSS].obCtList[nofObj].keyword, rol->obCtList[i].keyword);
			ml->objSS[nOfSS].obCtList[nofObj].OID = rol->obCtList[i].OID;
			ml->objSS[nOfSS].obCtList[nofObj].weightDistance = rol->obCtList[i].weightDistance;
			
			if(isGreaterTh(ml->objSS[nOfSS].ctbVector, uq)) {
				ml->objSS[nOfSS].isValid = false;
				//ml->minCost = ml->objSS[ml->objSS->nOfObj-1].weightCost;
				if(ml->minCost > ml->objSS[nOfSS].weightCost) {
					ml->minCost = ml->objSS[nOfSS].weightCost;
					ml->rObjList->nOfObj = ml->objSS[nOfSS].nOfObj;
					//这样赋值有问题吗，直接将整个数组赋值????????????
					free(ml->rObjList->objArray);
					ml->rObjList->objArray = NULL;
					ml->rObjList->objArray = (int *) malloc(ml->rObjList->nOfObj*sizeof(int));
					memset(ml->rObjList->objArray, 0, ml->rObjList->nOfObj*sizeof(int));
					for(int l=0; l<ml->objSS[nOfSS].nOfObj; l++) {
						ml->rObjList->objArray[l] = ml->objSS[nOfSS].obCtList[l].OID;
					}
				}
				
				
			} else {
				//过滤策略
				if(ml->objSS[nOfSS].weightCost + residualCost(ml->objSS[nOfSS], rol->obCtList[i].ctbRation, uq) > ml->minCost) {
					ml->objSS[nOfSS].isValid = false;
				}
			}

		} else {
			int loopNum = ml->nOfSubSet;
			ml->objSS = (objSubSet *)realloc(ml->objSS,loopNum * 2 * sizeof(objSubSet));//为每一个可能的SUBSET分配空间
			//为每一个新分配的集合进行初始化工作
			for(int t=loopNum; t<loopNum * 2; t++) {
				ml->objSS[t].isValid = false;
				ml->objSS[t].nOfObj = 0;
				ml->objSS[t].weightCost = 0.0;
			}

			for(int j=0; j<loopNum; j++) {
				//如果第J个SUBSET是有效的，将当前OBJ加入ml->nOfSubSet列表
				if(ml->objSS[j].isValid) {
					ml->flag = true;
					ml->objSS[ml->nOfSubSet].isValid = true;
					ml->objSS[ml->nOfSubSet].nOfObj = ml->objSS[j].nOfObj+1;
					ml->objSS[ml->nOfSubSet].weightCost = ml->objSS[j].weightCost + rol->obCtList[i].weightDistance;
					ml->objSS[ml->nOfSubSet].ctbVector = (float *)malloc(sizeof(float)*uq->nOfQryKwd);
					memset(ml->objSS[ml->nOfSubSet].ctbVector, 0.0, sizeof(float)*uq->nOfQryKwd);
					for(int k=0; k<uq->nOfQryKwd; k++) {
						ml->objSS[ml->nOfSubSet].ctbVector[k] = ml->objSS[j].ctbVector[k] + rol->obCtList[i].ctbVector[k];
//float test = rol->obCtList[i].ctbVector[k];
					}
					ml->objSS[ml->nOfSubSet].obCtList = (objectCont *)malloc((ml->objSS[j].nOfObj+1)*sizeof(objectCont));
					
					//将SUBSET J赋值给当前节点
					//memcpy(ml->objSS[ml->nOfSubSet].obCtList, ml->objSS[j].obCtList, (ml->objSS[j].nOfObj)*sizeof(objectCont));
					//对obCtList进行初始化			
					for(int l=0; l<=ml->objSS[j].nOfObj; l++) {
						ml->objSS[ml->nOfSubSet].obCtList[l].ctbRation = 0.0;
						ml->objSS[ml->nOfSubSet].obCtList[l].ctbVector = (float *)malloc(sizeof(float) * uq->nOfQryKwd);
						memset(ml->objSS[ml->nOfSubSet].obCtList[l].ctbVector, 0.0, sizeof(float) * uq->nOfQryKwd);
						ml->objSS[ml->nOfSubSet].obCtList[l].OID = -1;
						ml->objSS[ml->nOfSubSet].obCtList[l].weightDistance = 0.0;
					}
					if(j != 0) { //如果j!=0，那么将objSS[j]所有的数据赋值给ml->objSS[ml->nOfSubSet].obCtList[l]
						for(int l=0; l<ml->objSS[j].nOfObj; l++) {
							ml->objSS[ml->nOfSubSet].obCtList[l].ctbRation = ml->objSS[j].obCtList[l].ctbRation;
							memcpy(ml->objSS[ml->nOfSubSet].obCtList[l].ctbVector, ml->objSS[j].obCtList[l].ctbVector, sizeof(float) * uq->nOfQryKwd);
							ml->objSS[ml->nOfSubSet].obCtList[l].keyword = (char *)malloc(sizeof(char)*strlen(ml->objSS[j].obCtList[l].keyword));
							strcpy(ml->objSS[ml->nOfSubSet].obCtList[l].keyword,ml->objSS[j].obCtList[l].keyword);
							//ml->objSS[ml->nOfSubSet].obCtList[l].keyword = ml->objSS[j].obCtList[l].keyword;
							ml->objSS[ml->nOfSubSet].obCtList[l].OID = ml->objSS[j].obCtList[l].OID;
							ml->objSS[ml->nOfSubSet].obCtList[l].weightDistance = ml->objSS[j].obCtList[l].weightDistance;
						}
					}
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].ctbRation = rol->obCtList[i].ctbRation;
					//ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].ctbVector = (float *)malloc(sizeof(float)*uq->nOfQryKwd);
					//memset(ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].ctbVector, 0.0, sizeof(float)*uq->nOfQryKwd);
					memcpy(ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].ctbVector, rol->obCtList[i].ctbVector,sizeof(float)*uq->nOfQryKwd );
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].keyword = (char *)malloc(sizeof(char)*strlen(rol->obCtList[i].keyword));
					strcpy(ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].keyword, rol->obCtList[i].keyword);
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].OID = rol->obCtList[i].OID;
					ml->objSS[ml->nOfSubSet].obCtList[ml->objSS[j].nOfObj].weightDistance = rol->obCtList[i].weightDistance;
			
					if(isGreaterTh(ml->objSS[ml->nOfSubSet].ctbVector, uq)) {
						ml->objSS[ml->nOfSubSet].isValid = false;
						//ml->minCost = ml->objSS[ml->objSS->nOfObj-1].weightCost;
						if(ml->minCost > ml->objSS[ml->nOfSubSet].weightCost) {		
							ml->minCost = ml->objSS[ml->nOfSubSet].weightCost;
							ml->rObjList->nOfObj = ml->objSS[ml->nOfSubSet].nOfObj;
							//这样赋值有问题吗，直接将整个数组赋值????????????
							free(ml->rObjList->objArray);
							ml->rObjList->objArray = NULL;
							ml->rObjList->objArray = (int *) malloc(ml->rObjList->nOfObj*sizeof(int));
							memset(ml->rObjList->objArray, 0, ml->rObjList->nOfObj*sizeof(int));
							for(int l=0; l<ml->rObjList->nOfObj; l++) {
								ml->rObjList->objArray[l] = ml->objSS[ml->nOfSubSet].obCtList[l].OID;
							}
						}
					} else {
						//过滤策略
						if(ml->objSS[ml->nOfSubSet].weightCost + residualCost(ml->objSS[ml->nOfSubSet], rol->obCtList[i].ctbRation, uq) > ml->minCost) {
							ml->objSS[ml->nOfSubSet].isValid = false;
						}
					}
					ml->nOfSubSet++;
				} 
			}//for 
			if(!ml->flag) break; //如果ml->flag为假说明它的所有的元素都为无效，可以直接退出
		}		
	}*/
//int tme = -1;
	return ml;
}

void printObjectList(mgList *ml) {
	float sum = 0.0;
	for(int i=0; i<ml->rObjList->nOfObj; i++) {
		printf("%d ", ml->rObjList->objArray[i]);
	}
	sum = ml->minCost;
	printf("\nThe minCost is:%f\n",sum);
}

int _tmain(int argc, _TCHAR* argv[]) {
	//读文件进行预处理
	char tempKwd[256] ;
	clock_t start, end;
	char *dataFileName = "D:\\shiyan\\ds\\z_130.txt";
	//char *dataFileName = "D:\\qk\\r_200.txt";
	PreProcess(dataFileName);
//printf("Finish _tmain - PreProcess\n");
	//读取用户的查询
	//char *queryFileName = "D:\\queryFileName\\ds_100_3.txt";
	char *queryFileName = "D:\\shiyan\\ds\\q_300_3.txt";
	userQ *uq = userQ_new();

	FILE *filePtr;
	filePtr = (FILE *) fopen(queryFileName, "r");
	if (filePtr == NULL)
	{
		printf("File %s does not exist!\n", queryFileName);
		exit(0);
	}
	//读入查询位置以及阈值信息
	fscanf(filePtr, "%f %f %f", &uq->postion[0], &uq->postion[1], &uq->threshold);
	//读入用户PreferenceVector
	int i=1; 
	while(i <= GRADE) {
		fscanf(filePtr, "%f ", &uq->preferenceVector[i]);
		i++;
	}
	//读入关键字信息
	while(!feof(filePtr)) {
		uq->nOfQryKwd++;
		if(uq->nOfQryKwd == 1) {
			uq->queryKwd = (char **)malloc(sizeof(char *));
		} else {
			uq->queryKwd = (char **)realloc(uq->queryKwd,sizeof(char *) * uq->nOfQryKwd);
		}
		fscanf(filePtr, "%s ", tempKwd);
		uq->queryKwd[uq->nOfQryKwd - 1] = (char *) malloc(sizeof(char)*strlen(tempKwd));
		strcpy(uq->queryKwd[uq->nOfQryKwd - 1],tempKwd);
	}
	start = clock();
//printf("Finish _tmain - queryFileName\n");
	reObList *returnValue = constructReObList(uq);
//printf("Finish _tmain - constructReObList\n");
	int choice = 1;
	mgList *ml;
	if(choice == 1) {
		ml = wgtPriMergeList(returnValue, uq);
	} else {
		ml = ctrPriMergeList(returnValue, uq);
	}
	end = clock();
	float duration = end-start;
	printf("The running time of MergeList_ds is:%f\n",duration);
//printf("Finish _tmain - MergeList\n");
	printObjectList(ml);
//printf("Finish _tmain - printObjectList\n");
	return 0;
}

