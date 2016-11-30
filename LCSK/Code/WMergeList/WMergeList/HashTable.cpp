#include "StdAfx.h"
#include "HashTable.h"
//计算关键字的key值
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
//通过计算的key值来计算关键字的一级hash值
unsigned int BKDRHash(char *str, PHASH_FUNC ph)
{
    unsigned int hash = 0;
	hash = BKDRHashKey(str);
	return HASH_MAP_ADDR(ph->a,ph->b,ph->m,hash);
}

//初始化hashTable，主要是为一级hash表分配合适的二级hash空间并构建二级hash函数
void init_hash_table(kwdLs *kl)//初始化
{
	int i = 0;
	int m = 0;
	int key = 0;
	//hfList = (PHASH_FUNC ) malloc(sizeof(HASH_FUNC)*M);
	memset(hfList, 0, sizeof(HASH_FUNC)*M);
	PHASH_FUNC ph = NULL;//二级哈希函数参数ph
	p_hash_func = random_hash_func(M); //一级哈希函数参数p_hash_func
	memset(countKwd, 0, sizeof(int) * HTL);

	memset(hfList, 0 ,sizeof(hfList));

	for(i=0; i < kl->nOfkwd; i++)//计算映射到一级哈希表同一位置的数目
	{
		countKwd[BKDRHash(kl->keywords[i], p_hash_func)]++;
	}
	

	for(i=0; i<M; i++)
	{
		/* 如果二级哈希表T[i]只有1个元素，则a=b=0,m=1,立存入即可*/
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
		/* 如果二级哈希表T[i]元素大于1则需根据随机生成的二级哈希函数参数进行相应操作*/
		if(countKwd[i]>1)
		{
			m = countKwd[i] * countKwd[i];
			ph = random_hash_func(m); //随机生成二级哈希函数参数ph
			hsTable[i] = (keyword **)malloc(m * sizeof(keyword *));//为二级哈希表分配ph+m个空间，其中m=count[i]*count[i]
			//memset(hsTable[i], 0, sizeof(hsTable[i]));//所有二级哈希表空间清零
			for(int j=0; j<m; j++) {
				hsTable[i][j] = keyword_new( );
			}
			hfList[i] = PHASH_FUNC_new();
			memcpy(hfList, ph, sizeof(HASH_FUNC));//将上面刚生成的ph放入空间的首部（顺序a,b,m）
			free(ph);
			ph = NULL;
		}
	}
}

PHASH_FUNC random_hash_func(int mod)//随机生成哈希表参数（顺序a,b,m）
{
	PHASH_FUNC pfuc = NULL;
	pfuc = (PHASH_FUNC)malloc(sizeof(HASH_FUNC));
	pfuc->a = rand()%(P-1)+1;//a在1至p-1
	pfuc->b = rand()%P;//b在0至p-1
	pfuc->m = mod;

	return pfuc;
}
/*
void insert_keys_to_hash(keyword* hsTable[], int keys[])//插入一组关键字
{
	int i;
	for(i=0; i<N;i++) 
		insert_key_to_hash(T, keys[i]);
}*/

keyword *search_hash_key(char *kwd)//查找关键字，复杂度为O(1)
{
	int i = 0, j = 0;
	int k;

	i = BKDRHash(kwd, p_hash_func);//计算在一级哈希表的位置
	j = HASH_MAP_ADDR(hfList[i]->a, hfList[i]->b, hfList[i]->m, BKDRHashKey(kwd));//计算在二级哈希表的位置
	if( hfList[i]->m == 0 || hfList[i]->m <= j)//如果不存在或者二级表长小于计算出的在二级哈希表的位置，则未找到
		return NULL;

	if(!strcmp(hsTable[i][j]->keywordName, kwd)) { //如果当前位置关键字正好等于查询关键字，直接返回
		return hsTable[i][j];
	} 
	//循环查找(虽说理论上一次查找就行了，但是为了防止意外，还是循环查找)
	for(k=j; ; ) {
		if( hsTable[i][k]->PrimaryIndex == -1 ) {//如果当前位置可以为空，则停止
			return NULL;
		} else if(!strcmp(hsTable[i][k]->keywordName, kwd)){//如果当前位置中的关键字和要导入的关键字一样
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
	i = BKDRHash(kwd, p_hash_func);//计算在一级哈希表的位置
	j = HASH_MAP_ADDR(hfList[i]->a, hfList[i]->b, hfList[i]->m, BKDRHashKey(kwd));//计算在二级哈希表的位置

	for(k=j; ; ) {
		if( hsTable[i][k]->PrimaryIndex == -1 ) {//如果当前位置可以为空，在当前位置插入(???hsTable 没有初始化)
				hsTable[i][k]->keywordName = kwd;
				hsTable[i][k]->PrimaryIndex = i;
				hsTable[i][k]->SecondaryIndex = j;
				hsTable[i][k]->nOfOid ++;
				//说明是第一次插入关键字，所以分配空间时为1
				hsTable[i][k]->OIDList = (int *) malloc(sizeof(int));
				hsTable[i][k]->OIDList[hsTable[i][k]->nOfOid - 1] = oid;
				break;
			} else if(!strcmp(hsTable[i][k]->keywordName,kwd)){//如果当前位置中的关键字和要导入的关键字一样
				hsTable[i][k]->nOfOid ++;
				//说明不是第一次插入关键字，所以重新分配空间
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
