#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <assert.h>
#include "PreProcess.h"


////*******************************数据类型声明********************************
#define HASH_MAP_ADDR(a,b,m,k) ((((a)*(k)+(b))%P)%(m)) /* 两级公用散列函数h(x)= ((a*x+b)modp)mod m */ 
#define P 0x7FFFFFFF
#define M 13131 /* 定义hash T的长度，根据书上示例一级表长m=9；在完全哈希下，n=m */
#define N M

typedef struct _hash_func
{
	int a;
	int b;
	int m;
} HASH_FUNC, *PHASH_FUNC;

static PHASH_FUNC p_hash_func = NULL; /*定义一级哈希表的哈希函数*/
static PHASH_FUNC hfList[M];
//static keyword **hsTable = {0,}; /* 定义哈希表*/
static int countKwd[M];

/************************************函数声明************************************/
unsigned int BKDRHashKey(char *str); //生成关键字的Key值
unsigned int BKDRHash(char *str, PHASH_FUNC ph); //生成关键字的一级Hash值
void init_hash_table(kwdLs *kls); //初始化hashTable，主要是为一级hash表分配合适的二级hash空间并构建二级hash函数
PHASH_FUNC random_hash_func(int mod); //随机生成哈希表参数（顺序a,b,m）
void insert_key_to_hash(char *kwd, int oid); //插入关键字以及OID，复杂度为O(1),但是为了避免发生冲突采用循环插入方式
keyword *search_hash_key(char *kwd); //查找关键字，复杂度为O(1),但是为了避免发生冲突采用循环查找方式

#endif

