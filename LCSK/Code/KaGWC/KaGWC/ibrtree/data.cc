#include "data.h"


// DATA

DATA::DATA() {
	data = new VECTYPE[DIMENSION];	
	memset(data, 0, DIMENSION);
}

DATA::~DATA() {
    delete [] data;
}

DATA & DATA::operator = (DATA &_d) {
	memcpy(data, _d.data, sizeof(VECTYPE) * DIMENSION);		//forget to use VECTYPE£¡£¡£¡
    key=_d.key;
    return *this;
}

void DATA::read_from_buffer(char *buffer) {
    int i = sizeof(VECTYPE) * DIMENSION;
    memcpy(data, buffer, i);    
    memcpy(&key, &buffer[i], sizeof(bitmask_t));
}

void DATA::write_to_buffer(char *buffer) {
    int i = sizeof(VECTYPE) * DIMENSION;
    memcpy(buffer, data, i);
    memcpy(&buffer[i], &key, sizeof(bitmask_t));
}

void DATA::print() {
	printf("(");
	for (int i = 0; i < DIMENSION; i++)
		printf("%d ",data[i]);
	printf(")\n");
}

