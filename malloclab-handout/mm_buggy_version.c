/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "1",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// use implicit link list, malloc return  immediately if the free block is less
// than twice of the required size, cut the block if between 2-4, continue search
// if bigger.
/*
 * mm_init - initialize the malloc package.
 */
#define WORD 4
#define DWORD 8
#define GET(p) (*(unsigned int *)(p))
#define GET_POINTER(p) ((int *)(long)(GET(p)))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define PUT_POINTER(p,val) (*(int **)(p) = (val))
#define SIZE2ALIGN(size) (ALIGN(size + WORD))
int *low;
int round2log2(int num)
{
    for (int i = 3; i < 14; i++)
    {
        if (num <= (1 << i))
            return i - 3;
    }
    return 11;
}
int mm_init(void)
{
    mem_sbrk(12 * sizeof(void *));
    low = mem_heap_lo();
    memset(low, 0, 12 * sizeof(void *));
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

void *mm_malloc(size_t size)
{
    if (size == 0)
        return NULL;
    int align_size = SIZE2ALIGN(size);
    int index = round2log2(align_size);
    int *p = GET_POINTER(low + index);
    if (p == 0)
    {
        p = mem_sbrk(align_size);
        PUT(p, align_size);
        return p + 1;
    }
    if (GET(p) >= align_size)
    {
        PUT_POINTER(low + index, GET_POINTER(p + 1));
        return p + 1;
    }
    int *prev_ptr = p;
    p = GET_POINTER(p + 1);
    while (p != 0 && GET(p) < align_size)
    {
        prev_ptr = p;
        p = GET_POINTER(p + 1);
    }
    if (p == 0)
    {
        p = mem_sbrk(align_size);
        PUT(p, align_size);
        return p + 1;
    }
    PUT_POINTER(prev_ptr+1,GET_POINTER(p+1));
    return p+1;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    int size=GET(((int *)ptr)-1);
    int index = round2log2(size);
    int* p=GET_POINTER(low + index);
    PUT_POINTER(ptr,p);
    PUT_POINTER(low + index,ptr-4);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    int origin_size = GET(ptr - WORD);
    int align_size = SIZE2ALIGN(size);
    if (origin_size > align_size)
    {
        return ptr;
    }
    char *p = mm_malloc(size);
    memcpy(p, ptr, size);
    mm_free(ptr);
    return p;
}
