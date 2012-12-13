#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "mymalloc.h"


// global variables.
block_list_t g_mymem[ALLYES_MEM_BLOCK_COUNT] = {
    {NULL, ALLYES_MEM_MAX_SIZE},
    {NULL, ALLYES_MEM_MAX_SIZE},
    {NULL, ALLYES_MEM_MAX_SIZE},
    {NULL, ALLYES_MEM_MAX_SIZE},
    {NULL, ALLYES_MEM_MAX_SIZE},
    {NULL, ALLYES_MEM_MAX_SIZE},
    {NULL, ALLYES_MEM_MAX_SIZE},
    {NULL, ALLYES_MEM_MAX_SIZE},
    {NULL, ALLYES_MEM_MAX_SIZE},
    {NULL, ALLYES_MEM_MAX_SIZE}};

static void * __mymalloc_r(block_list_t * ls, int size);
static void __myfree_r(block_list_t * ls);

void * mymalloc(int index, int size)
{
    assert (index >= 0 && index < ALLYES_MEM_BLOCK_COUNT);
	return __mymalloc_r(&(g_mymem[index]), size);
}


void myfree(int index)
{
    assert (index >= 0 && index < ALLYES_MEM_BLOCK_COUNT);
	__myfree_r(&(g_mymem[index]));
}


/*********************************************************************
 *
 *    suppose multiple threads operations
 *
 ********************************************************************/
static void * __mymalloc_r(block_list_t * ls, int size)
{
	block_node_t * p;
	int n;

	assert(ls != NULL && size > 0);

	n = ls->block_size;
	p = ls->head;
	if (!p || size > p->avail_len)
	{
		if (size > n)
			n = size;
		p = (block_node_t *) malloc (sizeof(block_node_t) + n);
		if (!p)
			abort();
		//memset(p, 0, sizeof(block_node_t)+n);
		//write_log("malloc : %x", p);
		p->origin = (char *)p + sizeof(block_node_t);
		p->avail_len = n - size;
		p->avail = p->origin + size;
		p->next = ls->head;
		ls->head = p;
		return p->origin;
	}

	p->avail_len -= size;
	p->avail += size;
	return (char *)p->avail-size;
}


static void __myfree_r(block_list_t * ls)
{
	block_node_t * p;

	if (ls)
	{
		p = ls->head;
		while (p)
		{
			ls->head = p->next;
			free(p);                    // free node + block.
			//write_log("free : %x", p);
			p = ls->head;
		}
		ls->head = NULL;
	}
}
