#include "data.h"

// DATA
DATA::DATA() {
	attrw = new int[ATTRW];
	data = new VECTYPE[DIMENSION];	
	tag = new VECTYPE[TAGDIM];
	
	memset(attrw, 0, sizeof(int)*ATTRW);
	memset(data, 0, sizeof(VECTYPE)*DIMENSION);
	memset(tag, 0, sizeof(VECTYPE)*TAGDIM);
}

DATA::~DATA() {
	delete[] attrw;
    delete [] data;
	delete[] tag;
}

DATA & DATA::operator = (DATA &_d) {
	key = _d.key;
	memcpy(attrw, _d.attrw, sizeof(int) * ATTRW);
	memcpy(data, _d.data, sizeof(VECTYPE) * DIMENSION);		
	memcpy(tag, _d.tag, sizeof(VECTYPE) * TAGDIM);
    return *this;
}

void DATA::read_from_buffer(char *buffer) {
	int size=0, pos=0;
	// read the key inf
	size = sizeof(bitmask_t);
	memcpy(&key, &buffer[pos], size);
	pos += size;
	// read the attrw inf
	size = sizeof(int) * ATTRW;
	memcpy(attrw, &buffer[pos], size);
	pos += size;
	// read the data inf
	size = sizeof(VECTYPE) * DIMENSION;
    memcpy(data, &buffer[pos], size);
	pos += size;
	// read the tag inf
	size = sizeof(VECTYPE) * TAGDIM;
	memcpy(tag, &buffer[pos], size);
}

void DATA::write_to_buffer(char *buffer) {
	int size = 0, pos = 0;
	// read the key inf
	size = sizeof(bitmask_t);
	memcpy(buffer, &key, size);
	pos += size;
	// read the attrw inf
	size = sizeof(int) * ATTRW;
	memcpy(&buffer[pos], attrw, size);
	pos += size;
	// read the data inf
	size = sizeof(VECTYPE) * DIMENSION;
	memcpy(&buffer[pos], data, size);
	pos += size;
	// read the tag inf
	size = sizeof(VECTYPE) * TAGDIM;
	memcpy(&buffer[pos], tag, size);
}

void DATA::print() {
	printf("(");
	for (int i = 0; i < DIMENSION; i++)
		printf("%d ",data[i]);
	printf(")\n");
}

