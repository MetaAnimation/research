#ifndef __DATA
#define __DATA

#include <stdio.h>
#include <string.h>

typedef unsigned long long bitmask_t;
typedef unsigned char VECTYPE;

//#define NODENUM 100	//it should be consistant with the number of entries in R-Tree
#define NODENUM 40	//Hotel

#define DIMENSION 	((NODENUM)/8 + (NODENUM % 8 == 0 ? 0 : 1))

// key can only type cast to double !

class DATA {
public:	
	VECTYPE *data;                    		// Vector    	
    bitmask_t key;								// tmp key, stored in disk !
	
	void read_from_buffer(char *buffer);	// reads data from buffer
    void write_to_buffer(char *buffer);		// writes data to buffer
    void print();
    virtual DATA & operator = (DATA &_d);

	static const int Size= (DIMENSION)*sizeof(VECTYPE)+sizeof(bitmask_t);
    
    DATA();
    virtual ~DATA();
};

#endif // __DATA
