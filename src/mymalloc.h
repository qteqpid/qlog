/*
 * mymalloc.h
 */


#ifndef MYMALLOC_H
#define MYMALLOC_H

// system header files.

#define ALLYES_MEM_MAX_SIZE 1024 * 1024
#define ALLYES_MEM_BLOCK_COUNT 10


typedef struct block_node_t
{
	char *                origin;           // original address.
	char *                avail;            // available address.
	int                   avail_len;           // available size.
	struct block_node_t * next;
}block_node_t;


typedef struct block_list_t
{
	block_node_t *  head;        // header of memory address list.
	int             block_size;             // size of block.
}block_list_t;

// APIs of my malloc.
extern void * mymalloc(int index, int size);
extern void myfree(int index);

#endif
