#ifndef _MAXPRIORITYQUEUE_H_
#define _MAXPRIORITYQUEUE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "PreProcess.h"

//*******************************������������********************************
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

//*******************************��������********************************
PriorityQueue Initialize( int MaxElements,int nOfVector); //�����ȶ��н��г�ʼ������Ҫ�ǿ��ٿռ�
void Destroy( PriorityQueue H ); //�������ȶ���
void MakeEmpty( PriorityQueue H ); //������ȶ���
void Insert( ElementType X, PriorityQueue H ); //��X���뵽���ȶ�����
ElementType DeleteMax( PriorityQueue H ); //ɾ�����ȶ��ж���Ԫ��
ElementType FindMax( PriorityQueue H ); //��ȡ���ȶ��ж���Ԫ��
int IsEmpty( PriorityQueue H ); //�ж����ȶ����Ƿ�Ϊ��
int IsFull( PriorityQueue H ); //�ж����ȶ����Ƿ���

#endif


