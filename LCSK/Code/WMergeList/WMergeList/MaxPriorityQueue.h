#ifndef _MAXPRIORITYQUEUE_H_
#define _MAXPRIORITYQUEUE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "PreProcess.h"

//*******************************数据类型声明********************************
#define Error( Str )        FatalError( Str )
#define FatalError( Str )   fprintf( stderr, "%s\n", Str ), exit( 1 )
#define MaxData 3.4e+30

typedef objectCont ElementType;
typedef struct HeapStruct
{
	int Capacity;
	int Size;
	ElementType *Elements;
}HeapStruct, *PriorityQueue;

//*******************************函数声明********************************
PriorityQueue Initialize( int MaxElements,int nOfVector); //对优先队列进行初始化，主要是开辟空间
void Destroy( PriorityQueue H ); //销毁优先队列
void MakeEmpty( PriorityQueue H ); //清空优先队列
void Insert( ElementType X, PriorityQueue H ); //将X插入到优先队列中
ElementType DeleteMax( PriorityQueue H ); //删除优先队列顶端元素
ElementType FindMax( PriorityQueue H ); //获取优先队列顶端元素
int IsEmpty( PriorityQueue H ); //判断优先队列是否为空
int IsFull( PriorityQueue H ); //判断优先队列是否满

#endif


