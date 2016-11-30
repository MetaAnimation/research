#ifndef __BLK_FILE
#define __BLK_FILE

#define BFHEAD_LENGTH (sizeof(int)*2) // interner Teil des Headerblocks

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char Block[];
#define TRUE 1
#define FALSE 0

/******* Added these 3 lines ***********/
#define SEEK_CUR 1
#define SEEK_SET 0
#define SEEK_END 2
/******* end of addition ********/


//-------------------------------------------------------------------------
class BlockFile
{
	FILE* fp;			// Pointer auf das File
	char* filename;		// fuer evtl. truncate noetig
	int blocklength;	        // Laenge eines Blocks in Byte
	int act_block; 	        // interne Nummer des Blocks auf den
							// SEEK_CUR zeigt
	int number;		        // Gesamtzahl der Bloecke (exklusive Header)
	bool new_flag;		// in dieser Sitzung erzeugtes File?
	
	void put_bytes(const char* bytes,int num)  // schreibt num Bytes ins File
	{ fwrite(bytes,num,1,fp);}
	void get_bytes(char* bytes,int num)	      // liest num Bytes nach bytes
	{ fread(bytes,num,1,fp); }
	void fwrite_number(int num);	// schreibt Nummer
	int fread_number();		// liest Nummer
	void seek_block(int bnum)    // setzt SEEK_CUR unkontrolliert
	{ 
		fseek(fp,(bnum-act_block)*blocklength,SEEK_CUR);		//?????why I must add 1? and result is correct? but file size is not correct?
	}
public:
	bool isReadOnly;	// default: false, set to true for read only !
	
	BlockFile(char* name, int b_length);
		        // Filename und Blocklaenge (fuer neue Files)
	~BlockFile();
	
	// Blockfile-internen Header
	void read_header(char * header);	// liest Headerblock ohne
	void set_header(char* header);	// setzt Blockfileheader  ohne
	
	bool read_block(Block b,int i);	        // liest
	bool write_block(const Block b,int i);	// schreibt Block
	int append_block(const Block b);	// liefert Blocknummer
	
	bool delete_last_blocks(int num);	// loescht die letzten num Bloecke
	
	bool file_new()			// TRUE nur fuer ein in dieser Sitzung
	{ return new_flag; }			// erzeugtes File
	int get_blocklength()	// liefert Blocklaenge
	{ return blocklength; }
	int get_num_of_blocks()	// liefert Gesamtzahl (ohne Header)
	{ return number; }
};

//-------------------------------------------------------------------------
class CachedBlockFile : public BlockFile
{
   enum uses {free,used,fixed};	// fuer fuf_cont
   int ptr;		        //current position in cache
   int cachesize;		// Dynamische Cachegroesse (>=0)
   int *cache_cont;	    // array of the indices of blocks that are in cache
   uses *fuf_cont; 		//indicator array that shows whether one cache block is free, used or fixed
   int  *LRU_indicator; //indicator that shows how old (unused) is a page in the cache
   char **cache;   		// Cache

   // interne Funktionen:
   int next();		// liefert freies Feld, oder, wenn C. voll,
   int in_cache(int index);	// liefert Pos. im Cache oder -1

public:
   int page_faults;	// count number of page faults (only for pages from disk to memory ! )
   
   CachedBlockFile(char* name,int blength, int csize);	// Filename,Blocklaenge
   ~CachedBlockFile();

   bool read_block(Block b,int i);
   bool write_block(const Block b,int i);

   bool fix_block(int i);	// Block immer im Cache halten
   bool unfix_block(int i);	// fix_block aufheben
   void unfix_all();			// fix_block insg. aufheben
   void set_cachesize(int s);
   void flush();			// speichert den gesamten Cache
};


#endif // __BLK_FILE
