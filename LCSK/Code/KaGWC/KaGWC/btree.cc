#include "btree.h"
#include "assert.h"


// BTNode

BTNode::BTNode(BTree *bt) {
}

BTNode::BTNode(BTree *bt, int _block) {
}

BTNode::~BTNode() {
}

// DirEntry

DirEntry ::DirEntry(BTree  *bt) {
	my_tree = bt;
	son_ptr = NULL;
}

DirEntry ::~DirEntry() {
	if (son_ptr != NULL) delete son_ptr;
}

DirEntry & DirEntry ::operator = (DirEntry &_d) {
	my_tree = _d.my_tree;
	son = _d.son;
	son_ptr = _d.son_ptr;
	son_is_data = _d.son_is_data;
	key = _d.key;
	return *this;
}

void DirEntry ::read_from_buffer(char *buffer) {
	memcpy(&key, buffer, sizeof(bitmask_t));
	memcpy(&son, buffer+sizeof(bitmask_t), sizeof(int));
}

void DirEntry ::write_to_buffer(char *buffer) {
    memcpy(buffer, &key, sizeof(bitmask_t));
    memcpy(buffer+sizeof(bitmask_t), &son, sizeof(int));
}

BTNode *DirEntry ::get_son() {

    if (son_ptr==NULL) {
		if (son_is_data) 	// data page
		    son_ptr=new BTDataNode(my_tree,son);
		else 				// dir page
		    son_ptr=new BTDirNode(my_tree,son);
    }
    return son_ptr;
}

// BTDirNode

BTDirNode ::BTDirNode(int capacity)
	: BTNode(NULL)
{	
	// from parent class
	my_tree = NULL;	num_entries = 0;	block = -1;
	entries = new DirEntry[capacity];//(my_tree);		// Initialize entries
	for(int i=0;i<capacity;i++)
		entries[i].my_tree = my_tree;
	dirty = false;		// not dirty yet
}

// creates a brand new BT directory node
BTDirNode ::BTDirNode(BTree  *bt) 
	: BTNode(bt)
{ 
	// from parent class
	my_tree = bt;	num_entries = 0;	block = -1;
		
	// header of page keeps node info. 
	// level + num_entries
	int header_size = sizeof(char) + sizeof(int);
	capacity = (bt->file->get_blocklength() - header_size) / (DirEntry::Size);
	
	// Initialize entries
	entries = new DirEntry[capacity];//(my_tree);
	for(int i=0;i<capacity;i++)
		entries[i].my_tree = my_tree;
	
	// create new block for the node
	char* b = new char[bt->file->get_blocklength()];
	block = bt->file->append_block(b);
	delete [] b;
	
	bt->num_of_inodes++;
	dirty = true;	// must be written to disk before destruction
}

// reads an existing BT directory node
BTDirNode ::BTDirNode(BTree  *bt, int _block) 
	: BTNode(bt)
{	
	// from parent class
	my_tree = bt;	num_entries = 0;	block = -1;
	
	// header of page keeps node info. 
	// level + num_entries
	int header_size = sizeof(char) + sizeof(int);
	capacity = (bt->file->get_blocklength() - header_size) / (DirEntry::Size);
	entries = new DirEntry[capacity];//(my_tree);		// Initialize entries
	for(int i=0;i<capacity;i++)
		entries[i].my_tree = my_tree;
	
	// now load block and read BTDirNode data from it.
	block = _block;
	char* b = new char[bt->file->get_blocklength()];
	bt->file->read_block(b, block);    
	read_from_buffer(b);
	delete [] b;
	
	dirty = false;		// not dirty yet
}

BTDirNode ::~BTDirNode() {
	if (dirty){
		// Update changes on disk
		char* b = new char[my_tree->file->get_blocklength()];
		write_to_buffer(b);
		my_tree->file->write_block(b, block);
        delete [] b;
    }
    delete [] entries;
}

void BTDirNode ::read_from_buffer(char *buffer) {
    int j=0;

    // first read header info
    memcpy(&level, &buffer[j], sizeof(char));
    j += sizeof(char);
    son_is_data=(level == 1);	// level 0 for data pages ; level > 1 for index pages
    
    memcpy(&num_entries, &buffer[j], sizeof(int));
    j += sizeof(int);

    // then read entries
    int s = DirEntry::Size;
    for (int i = 0; i < num_entries; i++) {
		entries[i].read_from_buffer(&buffer[j]);
		entries[i].son_is_data = son_is_data;
		j += s;
    }
}

void BTDirNode ::write_to_buffer(char *buffer) {
    int j=0;

    // first, write header info
    memcpy(&buffer[j], &level, sizeof(char));
    j += sizeof(char);
    memcpy(&buffer[j], &num_entries, sizeof(int));
    j += sizeof(int);

    // then, write entries
    int s = DirEntry::Size;
    for (int i = 0; i < num_entries; i++) {
		entries[i].write_to_buffer(&buffer[j]);
		j += s;
    }
}

void BTDirNode::split(BTDirNode  *sn) { 
	int half_num=num_entries/2;
	
	// [0,half_num) cur-node, [half_num,half_num)
	for (int i=half_num;i<num_entries;i++) {
		sn->insert_entry(&entries[i]);
		entries[i].son=-1;		// reset invalid items
		entries[i].son_ptr=NULL;
		//entries[i].key=-1;
	}

	/*printf("splitdir: %d %d\n",half_num,num_entries-half_num);
	if (this==my_tree->root_ptr)
		printf("splitroot (dir)\n");*/
	
	num_entries=half_num;	
}

bool BTDirNode::remove_entry(DirEntry  *de) {	// removes an entry
	int pos=-1;
	for (int i=0;i<num_entries;i++)
		if (entries[i].son==de->son)	// son is unique (for a tree) !
			pos=i;
	
	if (pos>=0) {
		for (int i=pos+1;i<num_entries;i++)
			entries[i-1]=entries[i];
		num_entries--;
		
		// invalidate last entry
		entries[num_entries].son=-1;		// reset invalid items
		entries[num_entries].son_ptr=NULL;
		//entries[num_entries].key=-1;
		return true;
	} else
		return false;
}

void BTDirNode::redistribute(int lpos) {
	int rpos=lpos+1;
	int child_cap=entries[lpos].get_son()->capacity;
	int half_child_cap=child_cap/2;
	
	if (son_is_data) { // handle data nodes
		BTDataNode* lnode=(BTDataNode*)entries[lpos].get_son();
		BTDataNode* rnode=(BTDataNode*)entries[rpos].get_son();
		lnode->dirty=true;	rnode->dirty=true;	// so that both child get updated
		
		if (lnode->num_entries+rnode->num_entries<=child_cap) {
			//printf("mergedata: %d %d\n",lnode->num_entries,rnode->num_entries);
			// move all entries from lnode to rnode
			while (lnode->num_entries>0) {
				int idx=lnode->num_entries - 1;
				rnode->insert_entry(&(lnode->entries[idx]));
				lnode->remove_entry(&(lnode->entries[idx]));
			}
			
			// update keys in current node entries
			remove_entry(&(entries[lpos]));
			delete lnode;	// destroy lnode (as it is not in the tree)
			my_tree->num_of_dnodes--;
		} else {
			// balance the number of entries
			int old_lnum=lnode->num_entries,old_rnum=rnode->num_entries;
			if (lnode->num_entries<half_child_cap) {
				while (lnode->num_entries<rnode->num_entries-1) {	// lnode too small
					int idx=0;	// first item of rnode
					lnode->insert_entry(&(rnode->entries[idx]));
					rnode->remove_entry(&(rnode->entries[idx]));
				}
			} else if (rnode->num_entries<half_child_cap) {
				while (rnode->num_entries<lnode->num_entries-1) {	// rnode too small
					int idx=lnode->num_entries - 1;	// last item of lnode
					rnode->insert_entry(&(lnode->entries[idx]));
					lnode->remove_entry(&(lnode->entries[idx]));
				}
			}
			// update keys in current node entries
			entries[lpos].key=lnode->entries[lnode->num_entries - 1].key;
			//printf("redis-data: (%d %d)->(%d %d)\n",old_lnum,old_rnum,lnode->num_entries,rnode->num_entries);
		}
	} else { // handle dir nodes
		BTDirNode* lnode=(BTDirNode*)entries[lpos].get_son();
		BTDirNode* rnode=(BTDirNode*)entries[rpos].get_son();
		lnode->dirty=true;	rnode->dirty=true;	// so that both child get updated
		
		if (lnode->num_entries+rnode->num_entries<=child_cap) {
			//printf("mergedir: %d %d\n",lnode->num_entries,rnode->num_entries);
			// move all entries from lnode to rnode
			while (lnode->num_entries>0) {
				int idx=lnode->num_entries - 1;
				rnode->insert_entry(&(lnode->entries[idx]));
				lnode->remove_entry(&(lnode->entries[idx]));
			}
			
			// update keys in current node entries
			remove_entry(&(entries[lpos]));
			delete lnode;	// destroy lnode (as it is not in the tree)
			my_tree->num_of_inodes--;
		} else {
			// balance the number of entries
			int old_lnum=lnode->num_entries,old_rnum=rnode->num_entries;
			if (lnode->num_entries<half_child_cap) {
				while (lnode->num_entries<rnode->num_entries-1) {	// lnode too small
					int idx=0;	// first item of rnode
					lnode->insert_entry(&(rnode->entries[idx]));
					rnode->remove_entry(&(rnode->entries[idx]));
				}
			} else if (rnode->num_entries<half_child_cap) {
				while (rnode->num_entries<lnode->num_entries-1) {	// rnode too small
					int idx=lnode->num_entries - 1;	// last item of lnode
					rnode->insert_entry(&(lnode->entries[idx]));
					lnode->remove_entry(&(lnode->entries[idx]));
				}
			}
			
			// update keys in current node entries
			entries[lpos].key=lnode->entries[lnode->num_entries - 1].key;
			//printf("redis-dir: (%d %d)->(%d %d)\n",old_lnum,old_rnum,lnode->num_entries,rnode->num_entries);
		}
	}
}

bool BTDirNode::remove(DATA *d) {	// handles overlapping keys (in dir node ??)
	int sub_id=-1;
	
	for (int i=0;i<get_num();i++) {
		bitmask_t lb_key=(i>0)?(entries[i-1].key):(0);	// set lb_key to -1 to pass lb_test
		bitmask_t ub_key=entries[i].key;
		if (lb_key<=d->key&&d->key<=ub_key) {	// ok to set lb included, 7/2/2005
			BTNode *cnode=entries[i].get_son();
			bool rslt=cnode->remove(d);
			if (rslt) {
				sub_id=i;
				break;
			}
		}
	}
	
	if (sub_id<0)
		return false;
	else {
		dirty=true;
		BTNode *cnode=entries[sub_id].get_son();
		
		// update key if ...
		if (son_is_data) {
	    	BTDataNode* tnode=(BTDataNode*)cnode;
	    	entries[sub_id].key=tnode->entries[tnode->get_num()-1].key;
		} else {
			BTDirNode* tnode=(BTDirNode*)cnode;
	    	entries[sub_id].key=tnode->entries[tnode->get_num()-1].key;
		}
		
		int half_child_cap=(cnode->capacity)/2;
		if (cnode->num_entries<half_child_cap) {
			dirty=true;
			if (sub_id==0) 
				redistribute(0);
			else
				redistribute(sub_id-1);
		}
		return true;
	}
}
	
void BTDirNode::insert_entry(DirEntry  *de) {
	if (num_entries==0) {
		entries[num_entries]=(*de);
	} else {
		// insert in ascending order of key
		int pos=0;
		while (entries[pos].key<=de->key && pos+1<=num_entries)
			pos++;
		
		for (int i=num_entries-1;i>=pos;i--)
			entries[i+1]=entries[i];
		entries[pos]=(*de);
	}
	num_entries++;
}

bool BTDirNode ::insert(DATA *d, BTNode  **sn) {
	dirty=true;
	
	// choose subtree to insert
	int sub_id=-1;
	for (int i=get_num()-1;i>=0;i--)
		if (d->key<=entries[i].key)
			sub_id=i;
	if (sub_id<0)  // > rightmost value !
		sub_id=get_num()-1;
	assert(sub_id>=0&&sub_id<get_num());
	
	BTNode *sub_sn=NULL;
	BTNode *cnode=entries[sub_id].get_son();
	bool rslt=cnode->insert(d,&sub_sn);	// sub_sn is the new node from child
	
	// adjust key of cnode
	if (son_is_data) {
    	BTDataNode* tnode=(BTDataNode*)cnode;
    	entries[sub_id].key=tnode->entries[tnode->get_num()-1].key;
	} else {
		BTDirNode* tnode=(BTDirNode*)cnode;
    	entries[sub_id].key=tnode->entries[tnode->get_num()-1].key;
	}
	
	if (!rslt) {
		DirEntry* de=new DirEntry(my_tree);	// for sub_sn
	    if (son_is_data) {	// curlevel is 1 => nextlevel=0
	    	BTDataNode* tnode=(BTDataNode*)sub_sn;
	    	de->key=tnode->entries[tnode->get_num()-1].key;
		} else {
			BTDirNode* tnode=(BTDirNode*)sub_sn;
	    	de->key=tnode->entries[tnode->get_num()-1].key;
		}
		de->son=sub_sn->block;
	    de->son_ptr=sub_sn;
	    de->son_is_data=son_is_data;
	    
		if (get_num()==capacity) {
			// split current node to sn
			*sn= new BTDirNode(my_tree);
		    BTDirNode* dir_sn=(BTDirNode *)*sn;	// as an alias pointer
		    dir_sn->level=level;
		    dir_sn->son_is_data=son_is_data;
		    split(dir_sn);
		    
			if ( de->key < dir_sn->entries[0].key )
				insert_entry(de);	// put d in current node
			else
				dir_sn->insert_entry(de);	// put d in new node	
			return false;
		} else {
			insert_entry(de);	// insert de
			return true;
		}
	}
	return true;
}


// BTDataNode

BTDataNode ::BTDataNode(int capacity) 
	: BTNode(NULL)
{
	my_tree = NULL;	num_entries = 0;	block = -1;
	entries = new DATA[capacity];		// Initialize entries
	dirty = false;		// not dirty yet
}
    
// creates a brand new BT data node
BTDataNode ::BTDataNode(BTree  *bt) 
	: BTNode(bt)
{ 
	// from parent class
	my_tree = bt;	num_entries = 0;	block = -1;
		
	// header of page keeps node info. 
	// level + num_entries
	int header_size = sizeof(char) + sizeof(int);
	capacity = (bt->file->get_blocklength() - header_size) / (DATA::Size);
	
	// Initialize entries
	entries = new DATA[capacity];
	
	// create new block for the node
	char* b = new char[bt->file->get_blocklength()];
	block = bt->file->append_block(b);
	delete [] b;
	
	bt->num_of_dnodes++;
	dirty = true;	// must be written to disk before destruction
}

// reads an existing BT data node
BTDataNode ::BTDataNode(BTree  *bt, int _block) 
	: BTNode(bt)
{	
	// from parent class
	my_tree = bt;	num_entries = 0;	block = -1;
	
	// header of page keeps node info. 
	// level + num_entries
	int header_size = sizeof(char) + sizeof(int);
	capacity = (bt->file->get_blocklength() - header_size) / (DATA::Size);
	entries = new DATA[capacity];		// Initialize entries
	
	// now load block and read BTDirNode data from it.
	block = _block;
	char* b = new char[bt->file->get_blocklength()];
	bt->file->read_block(b, block);    
	read_from_buffer(b);
	delete [] b;
	
	dirty = false;		// not dirty yet
}

BTDataNode ::~BTDataNode() {
    if (dirty){
		// Update changes on disk
		char* b = new char[my_tree->file->get_blocklength()];
		write_to_buffer(b);
		my_tree->file->write_block(b, block);
        delete [] b;
    }
    delete [] entries;
}

void BTDataNode ::read_from_buffer(char *buffer) {
    int j=0;

    // first read header info
	
    memcpy(&level, &buffer[j], sizeof(char));
    j += sizeof(char);
    
    memcpy(&num_entries, &buffer[j], sizeof(int));
    j += sizeof(int);

    // then read entries
    int s = DATA::Size;
    for (int i = 0; i < num_entries; i++) {
		entries[i].read_from_buffer(&buffer[j]);
		j += s;
    }
}

void BTDataNode ::write_to_buffer(char *buffer) {
    int j=0;

    // first, write header info
    memcpy(&buffer[j], &level, sizeof(char));
    j += sizeof(char);
    memcpy(&buffer[j], &num_entries, sizeof(int));
    j += sizeof(int);

    // then, write entries
    int s = DATA::Size;
    for (int i = 0; i < num_entries; i++) {
		entries[i].write_to_buffer(&buffer[j]);
		j += s;
    }
}

void BTDataNode ::split(BTDataNode  *sn) {
	int half_num=num_entries/2;
	
	// [0,half_num) cur-node, [half_num,half_num)
	for (int i=half_num;i<num_entries;i++) {
		sn->insert_entry(&entries[i]);
		//entries[i].id=-1;		// reset invalid items
		//entries[i].key=-1;
	}

	//printf("splitdata: %d %d\n",half_num,num_entries-half_num);
	num_entries=half_num;
}

bool BTDataNode ::remove_entry(DATA *d) { // returns true if the entry can be removed
	int pos=-1;
	for (int i=0;i<num_entries;i++)
//		if (entries[i].id==d->id)
			pos=i;
	
	if (pos>=0) {	
		for (int i=pos+1;i<num_entries;i++)
			entries[i-1]=entries[i];
		num_entries--;
	
		// invalidate last entry
//		entries[num_entries].id=-1;	
		//entries[num_entries].key=-1;
		
		return true; 
	} else
		return false;
}

bool BTDataNode ::remove(DATA *d) {
	if (remove_entry(d)) {
		dirty=true;
		return true;
	} else 
		return false;
}

void BTDataNode ::insert_entry(DATA *d) {
	if (num_entries==0) {
		entries[num_entries]=(*d);
	} else {
		// insert in ascending order of key
		int pos=0;
		while (entries[pos].key<=d->key && pos+1<=num_entries)
			pos++;
		
		for (int i=num_entries-1;i>=pos;i--)
			entries[i+1]=entries[i];
		entries[pos]=(*d);
	}
	num_entries++;
}

bool BTDataNode ::insert(DATA *d, BTNode  **sn) {
	dirty=true;
	
	// parent may need to update keys (in any case)
	if (get_num()==capacity) {
		// split current node to sn
		*sn= new BTDataNode(my_tree);
	    BTDataNode* data_sn=(BTDataNode *)*sn;	// as an alias pointer
	    data_sn->level=level;
	    split(data_sn);
		
		if ( d->key < data_sn->entries[0].key )
			insert_entry(d);	// put d in current node
		else
			data_sn->insert_entry(d);	// put d in new node
		return false;
	} else {
		insert_entry(d);	// insert d
		return true;
	}
	return true;
}

// BTree

// Construction of a new BTree
BTree ::BTree(char *fname, int _b_length, int cache_size) {	
    file = new CachedBlockFile(fname, _b_length, cache_size);

    // first block is header
    header = new char [file->get_blocklength()];
    
	nBits=0; vmax=0; pmax=0; tref=0;
    root=0;
    root_ptr = NULL;
    num_of_inodes=0;
    num_of_dnodes=0;
    num_of_data=0;
    root_is_data = true;

    //create an (empty) root
    root_ptr = new BTDataNode (this);
	root_ptr->level = 0;
    root = root_ptr->block;
}

// load an existing BTree
BTree ::BTree(char *fname, int cache_size) { 
    file = new CachedBlockFile(fname, 0, cache_size);	// load file
	int len = file->get_blocklength();
    // read header
    header = new char [file->get_blocklength()];
    file->read_header(header);
    read_header(header);
    root_ptr = NULL;
    load_root();
	int l = root_ptr->num_entries;
	l++;
}

BTree ::~BTree() {
    write_header(header);
    file->set_header(header);
    delete [] header;

    del_root();
    delete file;
    //printf("saved B-Tree containing %d internal nodes, %d data nodes, %d data\n",num_of_inodes,num_of_dnodes,num_of_data);
}

void BTree ::read_header(char *buffer) {
    int i = 0;

    memcpy(&num_of_inodes, &buffer[i], sizeof(num_of_inodes));
    i += sizeof(num_of_inodes);
	memcpy(&num_of_dnodes, &buffer[i], sizeof(num_of_dnodes));
    i += sizeof(num_of_dnodes);
    memcpy(&num_of_data, &buffer[i], sizeof(num_of_data));
    i += sizeof(num_of_data);
	memcpy(&root_is_data, &buffer[i], sizeof(root_is_data));
    i += sizeof(root_is_data);
    memcpy(&root, &buffer[i], sizeof(root));
    i += sizeof(root);
	memcpy(&nBits, &buffer[i], sizeof(nBits));
    i += sizeof(nBits);
    memcpy(&pmax, &buffer[i], sizeof(pmax));
    i += sizeof(pmax);
	memcpy(&vmax, &buffer[i], sizeof(vmax));
    i += sizeof(vmax);
	memcpy(&tref, &buffer[i], sizeof(tref));
    i += sizeof(tref);

	user_header = &buffer[i];
}

void BTree ::write_header(char *buffer) {
    int i = 0;

    memcpy(&buffer[i], &num_of_inodes, sizeof(num_of_inodes));
    i += sizeof(num_of_inodes);
	memcpy(&buffer[i], &num_of_dnodes, sizeof(num_of_dnodes));
    i += sizeof(num_of_dnodes);
    memcpy(&buffer[i], &num_of_data, sizeof(num_of_data));
    i += sizeof(num_of_data);
    memcpy(&buffer[i], &root_is_data, sizeof(root_is_data));
    i += sizeof(root_is_data);
    memcpy(&buffer[i], &root, sizeof(root));
    i += sizeof(root);
	memcpy(&buffer[i], &nBits, sizeof(nBits));
    i += sizeof(nBits);
    memcpy(&buffer[i], &pmax, sizeof(pmax));
    i += sizeof(pmax);
	memcpy(&buffer[i], &vmax, sizeof(vmax));
    i += sizeof(vmax);
    memcpy(&buffer[i], &tref, sizeof(tref));
    i += sizeof(tref);
    
    user_header = &buffer[i];
}

bool BTree ::remove(DATA *d) {
	assert(d->key>=0);	// hilbert key within range
	
	load_root();
	
	bool rslt=root_ptr->remove(d);
	if (rslt) {
		num_of_data--;
		if (root_ptr->num_entries==1&&root_is_data==false) {
			// if root is non-leaf and has only 1 entry, decrease the tree by 1 level
			//printf("root down\n");
			BTNode* nroot_ptr=((BTDirNode*)root_ptr)->entries[0].get_son();
			
			root_ptr->dirty=true;
			BTDirNode* iroot_node=(BTDirNode*)root_ptr;
			iroot_node->remove_entry(&(iroot_node->entries[0]));	// avoid repeated deletion
			delete root_ptr;
			num_of_inodes--;
			
			root_ptr=nroot_ptr;
		    root=root_ptr->block;
			root_is_data=(root_ptr->level==0);
		}
	}
	
	del_root();
	
	return rslt;
}

BTNode *BTree::search_entry(BTNode *btn, bitmask_t key){
	int numOfentry = btn->num_entries;
	BTDirNode *inode = (BTDirNode *)btn;


	if(key > inode->entries[numOfentry-1].key || key < 0)
	{
		return NULL;
	}
	else if(key <= inode->entries[0].key)
	{
		return inode->entries[0].get_son();
	}
	else
	{
		for(int i=1;i<inode->num_entries;i++)
		{
			if(key <= inode->entries[i].key && key > inode->entries[i-1].key)
				return inode->entries[i].get_son();
		}
	}
	
}

bool BTree::search_data(BTNode *btn, bitmask_t key, DATA &pdata){
	//??????????????ΪʲônumOfentry Ϊ0��
	int numOfentry = btn->num_entries;
	BTDataNode *dnode = (BTDataNode *)btn;
	int start = 0, end = numOfentry, mid = (start + end) / 2;
	while( dnode->entries[mid].key != key)
	{
		if(start == end && key != dnode->entries[start].key)
			return false;
		if( dnode->entries[mid].key > key)
		{
			end = mid;
			mid = (start + end) /2;
		}
		else
		{
			start = mid + 1;
			mid = (start + end) /2;
		}
	}
// 
	pdata = dnode->entries[mid];
	//memcpy(*pdata, dnode->entries[mid].data, DIMENSION * sizeof(VECTYPE));
	return true;
}

bool BTree::search(bitmask_t key, DATA &pdata){
	bool flag = true;
	load_root();
	
	if(this->root_is_data)
	{
		BTDataNode *dnode=(BTDataNode*)root_ptr;
		flag = search_data(dnode, key, pdata);
	}
	else
	{
		BTNode *inode=root_ptr;
		while(inode->is_data_node() == false)
		{
			inode = search_entry(inode, key);
			if(inode == NULL)
				return false;
		}
		flag = search_data(inode, key, pdata);
	}
	del_root();
	return flag;
}

void BTree ::insert(DATA *d) {
	assert(d->key>=0);	// hilbert key within range
	
	DirEntry* de=NULL;
	BTNode* sn=NULL;

	load_root();
	
	bool rslt=root_ptr->insert(d,&sn);
	if (!rslt) {
		// split root
		BTDirNode* nroot_ptr = new BTDirNode(this);
	    nroot_ptr->son_is_data = root_is_data;
	    nroot_ptr->level = root_ptr->level+1;
	    int nroot=nroot_ptr->block;
	    
	    de=new DirEntry(this);
	    if (root_ptr->is_data_node()) {
	    	BTDataNode* tnode=(BTDataNode*)root_ptr;
	    	de->key=tnode->entries[tnode->get_num()-1].key;
		} else {
			BTDirNode* tnode=(BTDirNode*)root_ptr;
	    	de->key=tnode->entries[tnode->get_num()-1].key;
		}
	    de->son=root_ptr->block;
	    de->son_ptr=root_ptr;
	    de->son_is_data=root_is_data;
	    nroot_ptr->insert_entry(de);

	    de=new DirEntry(this);
	    if (sn->is_data_node()) {
	    	BTDataNode* tnode=(BTDataNode*)sn;
	    	de->key=tnode->entries[tnode->get_num()-1].key;
		} else {
			BTDirNode* tnode=(BTDirNode*)sn;
	    	de->key=tnode->entries[tnode->get_num()-1].key;
		}
		de->son=sn->block;
	    de->son_ptr=sn;
	    de->son_is_data=root_is_data;
	    nroot_ptr->insert_entry(de);

	    root=nroot;
		root_ptr=nroot_ptr;
		root_is_data=false;
	}
	num_of_data++;
	
	del_root();
}

float BTree ::RealToDual(float value,DIMTYPE curtype) {	// precond: gBits, vmax, pmax
	unsigned cDOM=1<<nBits;
	
	if (curtype==VTYPE) {
		float vel=value;
		float d_vel=((float) cDOM)*(vel+vmax)/(2*vmax);
		return d_vel;
	} else if (curtype==PTYPE) {
		float pos=value;
		float d_pos=(((float) cDOM)*pos)/pmax;
		return d_pos;
	} else
		return 0;
}

float BTree ::DualToReal(float value,DIMTYPE curtype) {	// precond: gBits, vmax, pmax
	unsigned cDOM=1<<nBits;
	
	if (curtype==VTYPE) {
		float d_vel=value;
		float vel=-1.0f + 2*d_vel /((float) cDOM);
		vel=vel*vmax;
		return vel;
	} else if (curtype==PTYPE) {
		float d_pos=value;
		float pos=pmax*d_pos/((float) cDOM);
		return pos;
	} else
		return 0;
}

void BTree ::load_root() {
    if (root_ptr==NULL) {
		if (root_is_data)
	    	root_ptr=new BTDataNode(this,root);
        else
	    	root_ptr=new BTDirNode(this,root);

		int n= root_ptr->num_entries;
		int t = 0;
    }
}

void BTree ::del_root() {
    if (root_ptr!=NULL) {
		delete root_ptr;
		root_ptr=NULL;
    }
}

