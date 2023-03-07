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
#include <math.h>
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
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define GET_SIZE(p) (GET(p) >> 3)
#define VALID(p) (GET(p) & 1)
#define SIZE2ALIGN(size) (ALIGN(size + DWORD))
#define NEXT_P(p) (((char *)p) + GET_SIZE(p))
#define FOOT(p) (((char *)p) + GET_SIZE(p) - WORD)
int mm_init(void)
{
    int *ptr = mem_sbrk(DWORD);
    *ptr = 0x1;
    *(ptr + 1) = 0x1;
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

void *mm_malloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }
    int align_size = SIZE2ALIGN(size);
    char* p = mem_heap_lo() ;
    p=p+WORD;
    int p_size;
    while ((p_size = GET_SIZE(p)) > 0)
    {
        if (!VALID(p))
        {
            if (p_size >= align_size && p_size < 2 * align_size)
            {
                PUT(p, GET(p) | 0x1);
                PUT(FOOT(p), GET(p));
                return p + WORD;
            }
            else if (p_size >= 2 * align_size && p_size < 4 * align_size)
            {
                int remain = p_size - align_size;
                int val = align_size << 3 | 1;
                PUT(p, val);
                PUT(FOOT(p), val);
                char *next_p = NEXT_P(p);
                val = remain << 3;
                PUT(next_p, val);
                PUT(FOOT(next_p), val);
                return p + WORD;
            }
        }
        p = NEXT_P(p);
    }
    mem_sbrk(align_size);
    int val = align_size << 3 | 1;
    PUT(p, val);
    PUT(FOOT(p), val);
    PUT(NEXT_P(p), 1);
    return p + WORD;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    char *p = ((char *)ptr) - WORD;
    PUT(p, GET(p) & ~1);
    PUT(FOOT(p), GET(p));
    if (!VALID(p - WORD))
    {
        int prev_size = GET_SIZE(p - WORD);
        char *prev_p = p - prev_size;
        int val = (GET_SIZE(p) + prev_size) << 3;
        PUT(prev_p, val);
        PUT(FOOT(p), val);
    }
    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    int origin_size = GET_SIZE(ptr - WORD);
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
