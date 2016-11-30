#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <assert.h>
#include "PreProcess.h"


////*******************************������������********************************
#define HASH_MAP_ADDR(a,b,m,k) ((((a)*(k)+(b))%P)%(m)) /* ��������ɢ�к���h(x)= ((a*x+b)modp)mod m */ 
#define P 0x7FFFFFFF
#define M 13131 /* ����hash T�ĳ��ȣ���������ʾ��һ����m=9������ȫ��ϣ�£�n=m */
#define N M

typedef struct _hash_func
{
	int a;
	int b;
	int m;
} HASH_FUNC, *PHASH_FUNC;

static PHASH_FUNC p_hash_func = NULL; /*����һ����ϣ��Ĺ�ϣ����*/
static PHASH_FUNC hfList[M];
//static keyword **hsTable = {0,}; /* �����ϣ��*/
static int countKwd[M];

/************************************��������************************************/
unsigned int BKDRHashKey(char *str); //���ɹؼ��ֵ�Keyֵ
unsigned int BKDRHash(char *str, PHASH_FUNC ph); //���ɹؼ��ֵ�һ��Hashֵ
void init_hash_table(kwdLs *kls); //��ʼ��hashTable����Ҫ��Ϊһ��hash�������ʵĶ���hash�ռ䲢��������hash����
PHASH_FUNC random_hash_func(int mod); //������ɹ�ϣ�������˳��a,b,m��
void insert_key_to_hash(char *kwd, int oid); //����ؼ����Լ�OID�����Ӷ�ΪO(1),����Ϊ�˱��ⷢ����ͻ����ѭ�����뷽ʽ
keyword *search_hash_key(char *kwd); //���ҹؼ��֣����Ӷ�ΪO(1),����Ϊ�˱��ⷢ����ͻ����ѭ�����ҷ�ʽ

#endif

