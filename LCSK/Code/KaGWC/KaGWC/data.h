#ifndef __DATA
#define __DATA

#include <stdio.h>
#include <string.h>
// handle -- record the basic information of data, note the nodenum
typedef unsigned long long bitmask_t;
typedef unsigned char VECTYPE;

//#define NODENUM 100	//it should be consistant with the number of entries in R-Tree
#define NODENUM 40	//Hotel
#define DIMENSION 	((NODENUM)/8 + (NODENUM % 8 == 0 ? 0 : 1))
#define TAGDIM 	NODENUM // M--maxKwd, use 8bit to record a label of one kwd
#define ATTRW 7  // record the attribute inf and weight inf [0]: weight 1-5 Attr

class DATA {
public:	
	bitmask_t key;	// tmp key, stored in disk !
	int *attrw; // record the summary inf
	VECTYPE *data;  // Vector  record the obj contains the key  
	VECTYPE *tag;  // record the corresponding label of object, tag for leafnode and minimum weight for inter node
	
	void read_from_buffer(char *buffer);	// reads data from buffer
    void write_to_buffer(char *buffer);		// writes data to buffer
    void print();
    virtual DATA & operator = (DATA &_d);

	static const int Size= sizeof(bitmask_t) + ATTRW*sizeof(int) + (DIMENSION+TAGDIM)*sizeof(VECTYPE);
    
    DATA();
    virtual ~DATA();
};

#endif // __DATA
