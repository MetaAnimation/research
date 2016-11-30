#include "StdAfx.h"
#include "HashTable.h"
//����ؼ��ֵ�keyֵ
PHASH_FUNC PHASH_FUNC_new() {
	PHASH_FUNC returnValue = (PHASH_FUNC) malloc(sizeof(HASH_FUNC));
	returnValue->a = 0;
	returnValue->b = 0;
	returnValue->m = 0;
	return returnValue;
}

void PHASH_FUNC_free(PHASH_FUNC pf) {
	free(pf);
}

unsigned int BKDRHashKey(char *str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
    int len = strlen(str);

	for(int i=0; i<len; i++) {
		hash = hash * seed + (str[i]);
	}

    return (hash & 0x7FFFFFFF);
}
//ͨ�������keyֵ������ؼ��ֵ�һ��hashֵ
unsigned int BKDRHash(char *str, PHASH_FUNC ph)
{
    unsigned int hash = 0;
	hash = BKDRHashKey(str);
	return HASH_MAP_ADDR(ph->a,ph->b,ph->m,hash);
}

//��ʼ��hashTable����Ҫ��Ϊһ��hash�������ʵĶ���hash�ռ䲢��������hash����
void init_hash_table(kwdLs *kl)//��ʼ��
{
	int i = 0;
	int m = 0;
	int key = 0;
	//hfList = (PHASH_FUNC ) malloc(sizeof(HASH_FUNC)*M);
	memset(hfList, 0, sizeof(HASH_FUNC)*M);
	PHASH_FUNC ph = NULL;//������ϣ��������ph
	p_hash_func = random_hash_func(M); //һ����ϣ��������p_hash_func
	memset(countKwd, 0, sizeof(int) * HTL);

	memset(hfList, 0 ,sizeof(hfList));

	for(i=0; i < kl->nOfkwd; i++)//����ӳ�䵽һ����ϣ��ͬһλ�õ���Ŀ
	{
		countKwd[BKDRHash(kl->keywords[i], p_hash_func)]++;
	}
	

	for(i=0; i<M; i++)
	{
		/* ���������ϣ��T[i]ֻ��1��Ԫ�أ���a=b=0,m=1,�����뼴��*/
		if(countKwd[i]==1)
		{		
			m = countKwd[i] * countKwd[i];
			hsTable[i] = (keyword **) malloc(m*sizeof(keyword *));
			for(int j=0; j<m; j++) {
				hsTable[i][j] = keyword_new( );
			}
			//memset(hsTable[i], 0 , sizeof(hsTable[i]));
			hfList[i] = PHASH_FUNC_new();
			hfList[i]->a = 0;
			hfList[i]->b = 0;
			hfList[i]->m = 1;		
		}
		/* ���������ϣ��T[i]Ԫ�ش���1�������������ɵĶ�����ϣ��������������Ӧ����*/
		if(countKwd[i]>1)
		{
			m = countKwd[i] * countKwd[i];
			ph = random_hash_func(m); //������ɶ�����ϣ��������ph
			hsTable[i] = (keyword **)malloc(m * sizeof(keyword *));//Ϊ������ϣ�����ph+m���ռ䣬����m=count[i]*count[i]
			//memset(hsTable[i], 0, sizeof(hsTable[i]));//���ж�����ϣ��ռ�����
			for(int j=0; j<m; j++) {
				hsTable[i][j] = keyword_new( );
			}
			hfList[i] = PHASH_FUNC_new();
			memcpy(hfList, ph, sizeof(HASH_FUNC));//����������ɵ�ph����ռ���ײ���˳��a,b,m��
			free(ph);
			ph = NULL;
		}
	}
}

PHASH_FUNC random_hash_func(int mod)//������ɹ�ϣ�������˳��a,b,m��
{
	PHASH_FUNC pfuc = NULL;
	pfuc = (PHASH_FUNC)malloc(sizeof(HASH_FUNC));
	pfuc->a = rand()%(P-1)+1;//a��1��p-1
	pfuc->b = rand()%P;//b��0��p-1
	pfuc->m = mod;

	return pfuc;
}
/*
void insert_keys_to_hash(keyword* hsTable[], int keys[])//����һ��ؼ���
{
	int i;
	for(i=0; i<N;i++) 
		insert_key_to_hash(T, keys[i]);
}*/

keyword *search_hash_key(char *kwd)//���ҹؼ��֣����Ӷ�ΪO(1)
{
	int i = 0, j = 0;
	int k;

	i = BKDRHash(kwd, p_hash_func);//������һ����ϣ���λ��
	j = HASH_MAP_ADDR(hfList[i]->a, hfList[i]->b, hfList[i]->m, BKDRHashKey(kwd));//�����ڶ�����ϣ���λ��
	if( hfList[i]->m == 0 || hfList[i]->m <= j)//��������ڻ��߶�����С�ڼ�������ڶ�����ϣ���λ�ã���δ�ҵ�
		return NULL;

	if(!strcmp(hsTable[i][j]->keywordName, kwd)) { //�����ǰλ�ùؼ������õ��ڲ�ѯ�ؼ��֣�ֱ�ӷ���
		return hsTable[i][j];
	} 
	//ѭ������(��˵������һ�β��Ҿ����ˣ�����Ϊ�˷�ֹ���⣬����ѭ������)
	for(k=j; ; ) {
		if( hsTable[i][k]->PrimaryIndex == -1 ) {//�����ǰλ�ÿ���Ϊ�գ���ֹͣ
			return NULL;
		} else if(!strcmp(hsTable[i][k]->keywordName, kwd)){//�����ǰλ���еĹؼ��ֺ�Ҫ����Ĺؼ���һ��
			return hsTable[i][k];
		} else {
			k++;
			k = k % hfList[i]->m;
			if(k == j) return NULL;
		}
	}
	return NULL;
}


void insert_key_to_hash(char *kwd, int oid) {
	int i = 0, j = 0;
	int k;
int len = strlen(kwd);    
	i = BKDRHash(kwd, p_hash_func);//������һ����ϣ���λ��
	j = HASH_MAP_ADDR(hfList[i]->a, hfList[i]->b, hfList[i]->m, BKDRHashKey(kwd));//�����ڶ�����ϣ���λ��

	for(k=j; ; ) {
		if( hsTable[i][k]->PrimaryIndex == -1 ) {//�����ǰλ�ÿ���Ϊ�գ��ڵ�ǰλ�ò���(???hsTable û�г�ʼ��)
				hsTable[i][k]->keywordName = kwd;
				hsTable[i][k]->PrimaryIndex = i;
				hsTable[i][k]->SecondaryIndex = j;
				hsTable[i][k]->nOfOid ++;
				//˵���ǵ�һ�β���ؼ��֣����Է���ռ�ʱΪ1
				hsTable[i][k]->OIDList = (int *) malloc(sizeof(int));
				hsTable[i][k]->OIDList[hsTable[i][k]->nOfOid - 1] = oid;
				break;
			} else if(!strcmp(hsTable[i][k]->keywordName,kwd)){//�����ǰλ���еĹؼ��ֺ�Ҫ����Ĺؼ���һ��
				hsTable[i][k]->nOfOid ++;
				//˵�����ǵ�һ�β���ؼ��֣��������·���ռ�
				hsTable[i][k]->OIDList = (int *) realloc(hsTable[i][k]->OIDList,sizeof(int) * hsTable[i][k]->nOfOid);
				hsTable[i][k]->OIDList[hsTable[i][k]->nOfOid - 1] = oid;
				break;
			} else {
				k++;
				k = k % hfList[i]->m;
				if(k == j) break;
			}
	}	
}
