#include "StdAfx.h"
#include "PreProcess.h"
#include "HashTable.h"

spatialObject *spatialObject_new( ){
	spatialObject *returnValue;
	returnValue = (spatialObject *) malloc(sizeof(spatialObject));
	returnValue->nOfKeywords = 0;
	returnValue->OID = -1;
	//returnValue->position = (float *) malloc(sizeof(float) * 2);
	//memset(returnValue->position, 0.0, sizeof(float) * 2);
	return returnValue;
}

void spatialObject_free(spatialObject *spaObj){
	free(spaObj->grade);	
	free(spaObj->keywords);
	free(spaObj->position);
	//free(spaObj->objectName);
	free(spaObj);
}

objectList *objectList_new( ){
	objectList *returnValue;
	returnValue = (objectList *) malloc(sizeof(objectList));
	returnValue->nOfObjects = 0;
	return returnValue;
}

void objectList_free(objectList *objList){
	free(objList->spaObj);
	free(objList);
}

keyword *keyword_new( ){
	keyword *returnValue;
	returnValue = (keyword *) malloc(sizeof(keyword));
	returnValue->PrimaryIndex = -1;
	returnValue->SecondaryIndex = -1;
	returnValue->nOfOid = 0;
	return returnValue;
}

void keyword_free(keyword *kwd){
	free(kwd->keywordName);
	free(kwd->OIDList);
	free(kwd);
}

objectCont *objectCont_new() {
	objectCont *returnValue = (objectCont *)malloc(sizeof(objectCont));
	returnValue->ctbRation = 0.0;
	returnValue->OID = -1;
	returnValue->weightDistance = 0.0;
	return returnValue;
}

void objectCont_free(objectCont * obc) {
	free(obc->ctbVector);
	free(obc->keyword);
	free(obc);
}

kwdLs *kwdLs_new() {
	kwdLs *returnValue = (kwdLs *) malloc(sizeof(kwdLs));
	returnValue->nOfkwd = 0;
	return returnValue;
}

void kwdLs_free(kwdLs *kls) {
	free(kls->keywords);
	free(kls);
}

bool notInKwdList(char *kwd) { //�����жϸùؼ����Ƿ��ڹؼ����б���
	int length = kl->nOfkwd;
	for(int i=0; i<length; i++) {
		if(!strcmp(kwd, kl->keywords[i])) return false;
	}
	return true;
}

void readData(char *filename) {
	FILE *filePtr;
	filePtr = (FILE *) fopen(filename, "r");
	if (filePtr == NULL)
	{
		printf("File %s does not exist!\n", filename);
		exit(0);
	}

	objList = objectList_new();
	kl = kwdLs_new();
	char objectName[256];
	float position_x;
	float position_y;
	char keyword[256];
	char lineData[1000];
	int keywordGrade;
	int ret = 0;
	int tempObj = 0;
	int tempKwd = 0;
	spatialObject *sb;	
	//int orderOfID = 0;

	//???ÿ��lineData�ᱻ������
	while(fgets(lineData, 1000, filePtr) != NULL) {
		//spatialObject *spaObj = spatialObject_new();
		objList->nOfObjects++;
		tempObj = objList->nOfObjects - 1;
		if(objList->nOfObjects == 1) {
			objList->spaObj = (spatialObject *) malloc(sizeof(spatialObject));
		} else {
			objList->spaObj = (spatialObject *) realloc(objList->spaObj, sizeof(spatialObject) * (objList->nOfObjects));
		}
		ret = sscanf(lineData,"%s %f %f", objectName, &position_x, &position_y);
		char *posOfdh = strchr(lineData,',');
		if(posOfdh == NULL) continue;
		strcpy(lineData,posOfdh+1);
		//lineData = posOfdh;
//printf("%s",objectName);
		//fscanf(filePtr, "%s %f %f", objectName, &position_x, &position_y);//����objectName���Ǳ�ʾ��ַ
		//?????����д�кܴ�����  objList->spaObj[tempObj].objectName = objectName;
		//ע�����ռ�Ȼ���ٿ����ڴ棬rol->obCtList[oid].keyword = kwd;
//objList->spaObj[tempObj] = spatialObject_new( );
		/*
		objList->spaObj[tempObj].objectName = (char *) malloc(sizeof(char) * strlen(objectName));
		memcpy(objList->spaObj[tempObj].objectName, objectName, strlen(objectName));//����objectName���Ǳ�ʾ��ַ

		objList->spaObj[tempObj].position = (float *)malloc(sizeof(float)*2);
		objList->spaObj[tempObj].position[0] = position_x;
		objList->spaObj[tempObj].position[1] = position_y;
		objList->spaObj[tempObj].OID = tempObj; //OID��0��ʼ����
		
		//������Object,???ÿ��keyword�ᱻ������
		while(sscanf(lineData,"%s %d", keyword, &keywordGrade) != -1) {//������ǵ���β,ע������keyword�ͱ�ʾ��ַ��
			objList->spaObj[tempObj].nOfKeywords++;
			tempKwd = objList->nOfObjects-1;
			if(objList->spaObj[tempObj].nOfKeywords == 1) {
				objList->spaObj[tempObj].keywords = (char **) malloc(sizeof(char *));//ע�⵽����ָ�뻹������
				objList->spaObj[tempObj].grade = (int *) malloc(sizeof(int));
			} else {
				objList->spaObj[tempObj].keywords = (char **) realloc(objList->spaObj[tempObj].keywords,sizeof(char *) * (objList->spaObj[tempObj].nOfKeywords));//ע�⵽����ָ�뻹������
				objList->spaObj[tempObj].grade = (int *) realloc(objList->spaObj[tempObj].grade, sizeof(int) * objList->spaObj[tempObj].nOfKeywords);
			}

			//??????�����кܴ����� objList->spaObj[tempObj].keywords[tempKwd] = keyword;
			objList->spaObj[tempObj].keywords[tempKwd] = (char *) malloc(sizeof(char) * strlen(keyword));
			memcpy(objList->spaObj[tempObj].keywords[tempKwd], keyword, strlen(keyword));//������Ҫע��ȡ��ַ����
			objList->spaObj[tempObj].grade[tempKwd] = keywordGrade;

			//�����,�޸���Ҫ��Ϊ��ͨ�����ݹؼ��ֶ����Ǵ��ݹؼ���hashֵ
			if(notInKwdList(keyword)) {
				kl->nOfkwd++;
				if(kl->nOfkwd == 1) {
					kl->keywords = (char **) malloc(sizeof(char *));
				} else {
					kl->keywords = (char **) realloc(kl->keywords,sizeof(char *) * kl->nOfkwd);
				}
				kl->keywords[kl->nOfkwd-1] = keyword;
			}
	
		}*/
		sb = spatialObject_new( );
		sb->objectName = (char *) malloc(sizeof(char) * strlen(objectName));
		//memcpy(sb->objectName, objectName, strlen(objectName));//����objectName���Ǳ�ʾ��ַ
		strcpy(sb->objectName,objectName);
		sb->position = (float *)malloc(sizeof(float)*2);
		sb->position[0] = position_x;
//printf("%f ",sb->position[0]);
		sb->position[1] = position_y;
//printf("%f ",sb->position[1]);
		sb->OID = tempObj; //OID��0��ʼ����
		//������Object,???ÿ��keyword�ᱻ������
		//lineData = posOfdh;
		while(sscanf(lineData,"%s %d", keyword, &keywordGrade) != -1) {//������ǵ���β,ע������keyword�ͱ�ʾ��ַ��
			sb->nOfKeywords++;
			tempKwd = sb->nOfKeywords-1;
			if(sb->nOfKeywords == 1) {
				sb->keywords = (char **) malloc(sizeof(char *));//ע�⵽����ָ�뻹������
				sb->grade = (int *) malloc(sizeof(int));
			} else {
				sb->keywords = (char **) realloc(sb->keywords,sizeof(char *) * (sb->nOfKeywords));//ע�⵽����ָ�뻹������
				sb->grade = (int *) realloc(sb->grade, sizeof(int) * sb->nOfKeywords);
			}

			//??????�����кܴ����� objList->spaObj[tempObj].keywords[tempKwd] = keyword;
			sb->keywords[tempKwd] = (char *) malloc(sizeof(char) * strlen(keyword));
int len = strlen(keyword);
			//memcpy(sb->keywords[tempKwd], keyword, strlen(keyword));//������Ҫע��ȡ��ַ����
            strcpy(sb->keywords[tempKwd],keyword);
len = strlen(sb->keywords[tempKwd]);
			sb->grade[tempKwd] = keywordGrade;
			
			//�����,�޸���Ҫ��Ϊ��ͨ�����ݹؼ��ֶ����Ǵ��ݹؼ���hashֵ
			if(notInKwdList(keyword)) {
				kl->nOfkwd++;
				if(kl->nOfkwd == 1) {
					kl->keywords = (char **) malloc(sizeof(char *));
				} else {
					kl->keywords = (char **) realloc(kl->keywords,sizeof(char *) * kl->nOfkwd);
				}
				//kl->keywords[kl->nOfkwd-1] = keyword;
				kl->keywords[kl->nOfkwd-1] = (char *) malloc(sizeof(char) * strlen(keyword));
				strcpy(kl->keywords[kl->nOfkwd-1], keyword);
				//memcpy(kl->keywords[kl->nOfkwd-1], keyword, strlen(keyword));
len = strlen(kl->keywords[kl->nOfkwd-1]);				
			}
			posOfdh = strchr(lineData,',');
		    if(posOfdh == NULL) break;
		    strcpy(lineData,posOfdh+1);
	
		}

		objList->spaObj[tempObj].nOfKeywords = sb->nOfKeywords;
		objList->spaObj[tempObj].objectName = (char *) malloc(sizeof(char) * strlen(sb->objectName));
		//memcpy(objList->spaObj[tempObj].objectName, sb->objectName, strlen(sb->objectName));//����objectName���Ǳ�ʾ��ַ
		strcpy(objList->spaObj[tempObj].objectName,sb->objectName);
		objList->spaObj[tempObj].OID = sb->OID;
		objList->spaObj[tempObj].grade = (int *)malloc(sizeof(int)*sb->nOfKeywords);
		memcpy(objList->spaObj[tempObj].grade, sb->grade, sizeof(int)*sb->nOfKeywords);
		objList->spaObj[tempObj].position = (float *)malloc(sizeof(float)*2);
		memcpy(objList->spaObj[tempObj].position, sb->position, sizeof(float)*2);
		objList->spaObj[tempObj].keywords = (char **)malloc(sizeof(char *)*sb->nOfKeywords);
		for(int i=0; i<sb->nOfKeywords; i++) {
			objList->spaObj[tempObj].keywords[i] = (char *) malloc(sizeof(char) * strlen(sb->keywords[i]));
			//memcpy(objList->spaObj[tempObj].keywords[i], sb->keywords[i], strlen(sb->keywords[i]));//����objectName���Ǳ�ʾ��ַ
			strcpy(objList->spaObj[tempObj].keywords[i],sb->keywords[i]);
		}
		int t=0;
		spatialObject_free(sb);
	}
int loop = objList->nOfObjects;

	fclose(filePtr);
}

void ConstructHashTable() {	
	init_hash_table(kl);
	for(int i=0; i<objList->nOfObjects; i++) {
		for(int j=0; j<objList->spaObj[i].nOfKeywords; j++) {
			insert_key_to_hash(objList->spaObj[i].keywords[j], objList->spaObj[i].OID);
		}
	}
}

objectList *getObjList() {
	return objList;
}

void PreProcess(char *filename) {

	readData(filename);
//printf("Finish PreProcess - readData\n");
	ConstructHashTable();
//printf("Finish PreProcess - ConstructHashTable\n");
}

