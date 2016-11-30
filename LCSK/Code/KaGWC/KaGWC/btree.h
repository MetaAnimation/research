#ifndef __BTREE
#define __BTREE

#include "blk_file.h"
#include "data.h"

class BTNode;
class BTDirNode;
class BTDataNode;

// set dirty to true in any insert/delete routines
// key of an entry: rightmost key in its subtree !

enum DIMTYPE {PTYPE,VTYPE};


class DirEntry {
    friend class BTDirNode ;
    friend class BTree ;
public:
    BTree  *my_tree;        // pointer to my B-tree
    BTNode  *son_ptr;       // pointer to son if in main mem.
    bool son_is_data;   	// true, if son is a data page
    bitmask_t key;              // key value
    int son;                // block # of son
	
    BTNode * get_son();  	// returns the son, loads son if necessary
    void read_from_buffer(char *buffer);// reads data from buffer
    void write_to_buffer(char *buffer); // writes data to buffer
    
    static const int Size=sizeof(bitmask_t)+sizeof(int);
    						// returns amount of needed buffer space
    
    virtual DirEntry  & operator = (DirEntry  &_d);
    DirEntry(BTree *bt = NULL);
    virtual ~DirEntry();
};

class BTNode {
	friend class BTree ;
public:
	BTree  *my_tree;               		// pointer to B-tree
	int capacity;                       // max. # of entries
	int num_entries;                    // # of used entries
	bool dirty;                         // TRUE, if node has to be written
	int block;                          // disc block
	char level;                         // level of the node in the tree
		
    void read_from_buffer(char *buffer);	// reads data from buffer
    void write_to_buffer(char *buffer); 	// writes data to buffer
    bool is_data_node() { return ((int)level)==0; };
	int get_num() { return num_entries; };
	
	BTNode(BTree  *bt);
    BTNode(BTree  *bt, int _block);
    virtual bool insert(DATA *d, BTNode  **sn) = 0;
					// inserts d recursivly, if there occurs a split, false will be
					// returned and in sn a pointer to the new node
	virtual bool remove(DATA *d) = 0;
	
    virtual ~BTNode();
};

class BTDirNode: public BTNode {
    friend class BTree ;
public:
	bool son_is_data;                		// true, if son is a data page
    DirEntry  *entries;            			// array of entries
    void read_from_buffer(char *buffer);	// reads data from buffer
    void write_to_buffer(char *buffer); 	// writes data to buffer
	
	static const int HEADER_SIZE=sizeof(char)+sizeof(int);
	BTDirNode(int capacity);	// create a dummy node
	BTDirNode(BTree  *bt);
    BTDirNode(BTree  *bt, int _block);
    
    void split(BTDirNode  *sn);    // splits directory page to *this and sn
    void insert_entry(DirEntry  *de);	// inserts an entry
    bool insert(DATA *d, BTNode  **sn);
               	// inserts d recursively, if there occurs a split, false will be
				// returned and in sn a pointer to the new node
	
	void redistribute(int lpos);	// redistribute entries in (L,L+1), merge if necessary
	bool remove_entry(DirEntry  *de);		// removes an entry
	bool remove(DATA *d);
				// removes d recursively, if not full, return false and parent merge entries
	
    virtual ~BTDirNode();
};

class BTDataNode: public BTNode{
    friend class BTree ;
public:
    DATA  *entries;            				// array of entries
    void read_from_buffer(char *buffer);	// reads data from buffer
    void write_to_buffer(char *buffer); 	// writes data to buffer
	
	static const int HEADER_SIZE=sizeof(char) + sizeof(int);
	BTDataNode(int capacity);		// create a dummy node
    BTDataNode(BTree  *bt);
    BTDataNode(BTree  *bt, int _block);
    
    void split(BTDataNode  *sn);   	// splits data page into sn *this
    void insert_entry(DATA *d);		// inserts an entry
    bool insert(DATA *d, BTNode  **sn);
               	// inserts d, if there occurs a split, false will be
				// returned and in sn a pointer to the new node
	bool remove_entry(DATA *d);		// returns true if the entry can be removed
	bool remove(DATA *d);			// returns true if successfully removed
	
	virtual ~BTDataNode();
};

class BTree {	// a hilbert B+ tree
public:
    friend class BTDirNode;
    float pmax,vmax;				// min/max speed in all dimensions !
    int nBits;						// order of the Hilbert curve
	int root;						// block # of root node
	bool root_is_data;     			// true, if root is a data page
    float tref;						// reference time
    
    BTNode  *root_ptr;           	// root-node
    int num_of_inodes;	        	// # of stored directory pages
    int num_of_dnodes;	        	// # of stored data pages
    int num_of_data;
    CachedBlockFile *file;	  		// storage manager for harddisc blocks
    void load_root();            	// loads root_node into memory
    void del_root();
    char *header;
protected:
    char *user_header;
    void read_header(char *buffer);      // reads Rtree header
    void write_header(char *buffer);     // writes Rtree header
public:
    BTree(char *fname,int _b_length, int cache_size);
    BTree(char *fname, int cache_size);
    void insert(DATA *d);                // inserts new data into tree
   	bool remove(DATA *d);
	bool search(bitmask_t key, DATA &pdata);
	BTNode *search_entry(BTNode *btn, bitmask_t key);
	bool search_data(BTNode *btn, bitmask_t key, DATA &pdata);

    
	float RealToDual(float value,DIMTYPE curtype);
	float DualToReal(float value,DIMTYPE curtype);
    virtual ~BTree();
};

#endif // __BTREE
