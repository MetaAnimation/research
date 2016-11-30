#include "StdAfx.h"
#include "MaxPriorityQueue.h"
//最大的数据存放在下表为1的数组里
PriorityQueue Initialize( int MaxElements,int nOfVector)
{
	PriorityQueue H;
	H = (PriorityQueue) malloc( sizeof( struct HeapStruct ) );

	if ( H == NULL )
		FatalError( "Out of space!!!" );

	/* Allocate the array plus one extra for sentinel */
	H->Elements = (ElementType *)malloc( ( MaxElements + 1 ) * sizeof( ElementType ) );
	if ( H->Elements == NULL )
		FatalError( "Out of space!!!" );
	for(int i=0; i<(MaxElements + 1); i++) {
		H->Elements[i].ctbRation = 0.0;
		H->Elements[i].OID = -1;
		H->Elements[i].weightDistance = 0.0;
		H->Elements[i].ctbVector = (float *) malloc(sizeof(float) * nOfVector);
		memset(H->Elements[i].ctbVector, 0.0, sizeof(float) * nOfVector);
	}
	H->Capacity = MaxElements+1;
	H->Size = 0;
	//优先队列从下表为1开始的
	H->Elements[ 0 ].ctbRation = MaxData;
	H->Elements[0].OID = -1;
	H->Elements[0].weightDistance = 0.0;
	H->Elements[0].ctbVector = (float *) malloc(sizeof(float) * nOfVector);
	memset(H->Elements[0].ctbVector, 0.0, sizeof(float) * nOfVector);
	char *kwd = "null";
	H->Elements[ 0 ].keyword = (char *) malloc(sizeof(char) * strlen(kwd));
	strcpy(H->Elements[ 0 ].keyword, kwd);

	return H;
}

/* H->Elements[ 0 ] is a sentinel */
void  Insert( ElementType X, PriorityQueue H )
{
	int i;

	if ( IsFull( H ) )
	{
		Error( "Priority queue is full" );
		return ;
	}
//死循环
	for (i = ++H->Size; H->Elements[ i / 2 ].ctbRation < X.ctbRation; i /= 2) /* The new element is percolated up the heap  */  
        H->Elements[ i ] = H->Elements[ i / 2 ];           /* until the correct location is found */  
	if(H->Elements[ i ].OID == -1) {
		H->Elements[ i ].OID = X.OID;
		H->Elements[ i ].ctbRation = X.ctbRation;
		H->Elements[ i ].weightDistance = X.weightDistance;
		H->Elements[ i ].ctbVector = X.ctbVector;
		H->Elements[ i ].keyword = (char *) malloc(sizeof(char) * strlen(X.keyword));
		strcpy(H->Elements[ i ].keyword, X.keyword);
int f = -1;
	} else {
		H->Elements[ i ] = X; 
	}
}

ElementType  DeleteMax( PriorityQueue H )
{
	int i, Child;
	ElementType MaxElement, LastElement;

	if ( IsEmpty( H ) )
	{
		Error( "Delete:Priority queue is empty!" );
		return H->Elements[ 0 ];
	}

	MaxElement = H->Elements[ 1 ];
	LastElement = H->Elements[ H->Size-- ];

	for ( i = 1; i * 2 <= H->Size; i = Child )
	{
		/* Find smaller child */
		Child = i * 2;
		if ( Child != H->Size && H->Elements[ Child + 1 ].ctbRation > H->Elements[ Child ].ctbRation )
			Child++;

		/* Percolate one level */
		if ( LastElement.ctbRation < H->Elements[ Child ].ctbRation )
			H->Elements[ i ] = H->Elements[ Child ];
		else
			break;
	}
	H->Elements[ i ] = LastElement;
	return MaxElement;
}

void MakeEmpty( PriorityQueue H )
{
	H->Size = 0;
}

ElementType FindMax( PriorityQueue H )
{
	if( !IsEmpty( H ) )
		return H->Elements[ 1 ];
	Error( "FindMax:Priority Queue is Empty" );
	return H->Elements[ 0 ];
}

int IsEmpty( PriorityQueue H )
{
	return H->Size == 0;
}

int IsFull( PriorityQueue H )
{
	return H->Size == H->Capacity-1;//下标为0的是哨兵
}

void Destroy( PriorityQueue H )
{
	free( H->Elements );
	free( H );
}