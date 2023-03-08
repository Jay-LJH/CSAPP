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

#define WORD 4
#define DWORD 8
#define GET(p) (*(unsigned int *)(p))
#define GET_POINTER(p) ((int *)(long)(GET(p)))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define PUT_POINTER(p, val) (*(int **)(p) = (val))
#define GET_SIZE(p) (GET(p) >> 1)
#define VALID(p) (GET(p) & 1)
#define SIZE2ALIGN(size) (ALIGN(size + DWORD))
#define NEXT_P(p) (((char *)p) + GET_SIZE(p))
#define NEXT_BLOCK(p) (*(int **)(((char *)p) + WORD))
#define PREV_BLOCK(p) (*(int **)(((char *)p) + DWORD))
#define FOOT(p) (((char *)p) + GET_SIZE(p) - WORD)

int mm_init(void)
{
    int *p = mem_sbrk(5 * WORD);
    PUT(p, 1);
    PUT_POINTER(p + 1, NULL);
    PUT_POINTER(p + 2, NULL);
    PUT(p + 3, 1);
    PUT(p + 4, 1);
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void bind(int *p, int *prev, int *next)
{
    PUT_POINTER(prev + 1, p);
    PUT_POINTER(p + 2, prev);
    PUT_POINTER(p + 1, next);
    if (next != NULL)
    {
        PUT_POINTER(next + 2, p);
    }
}
void *mm_malloc(size_t size)
{
    int *p = (int *)mem_heap_lo();
    int align_size = SIZE2ALIGN(size);
    int p_size;
    while (p != NULL && (p_size = GET_SIZE(p)) < align_size)
    {
        p = NEXT_BLOCK(p);
    }
    if (p == NULL)
    {
        p = mem_sbrk(align_size);
        int val = (align_size << 1) | 1;
        PUT(p, val);
        PUT(FOOT(p), val);
        return p + 1;
    }
    if (p_size > 2 * align_size)
    {
        int remain = p_size - align_size;
        int val = align_size << 1 | 1;
        PUT(p, val);
        PUT(FOOT(p), val);
        int *next_p = (int*)NEXT_P(p);
        val = remain << 1;
        PUT(next_p, val);
        PUT(FOOT(next_p), val);
        int *next = NEXT_BLOCK(p);
        int *prev = PREV_BLOCK(p);
        PUT_POINTER(next_p+1,p+1);
        PUT_POINTER(next_p+2,p+2);
        bind((int *)next_p,prev,next);
        return p+1;
    }
    PUT(p, GET(p) | 0x1);
    PUT(FOOT(p), GET(p));
    int *next = NEXT_BLOCK(p);
    int *prev = PREV_BLOCK(p);
    PUT_POINTER(prev + 1, next);
    if (next != NULL)
        PUT_POINTER(next + 2, prev);
    return p + 1;
}

void mm_free(void *pt)
{
    int *p = (int *)pt - 1;
    if (VALID(p - 1))
    {
        PUT(p, GET(p) & ~1);
        PUT(FOOT(p), GET(p));
        int *prev = mem_heap_lo();
        int *next = NEXT_BLOCK(prev);
        bind(p, prev, next);
        return;
    }
    int prev_size = GET_SIZE(p - 1);
    int *prev_p = (int *)(((char *)p) - prev_size);
    int size = GET_SIZE(p);
    int val = (prev_size + size) << 1;
    PUT(prev_p, val);
    PUT(FOOT(p), val);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    if (0)
    {
        return NULL;
    }
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
/*
 trace  valid  util     ops      secs  Kops
 0       yes   85%    5694  0.000064 89388
 1       yes   89%    5848  0.000058100481
 2       yes   90%    6648  0.000077 86226
 3       yes   95%    5380  0.000055 97641
 4       yes   66%   14400  0.000088164196
 5       yes   84%    4800  0.000120 39867
 6       yes   83%    4800  0.000137 35011
 7       yes   55%   12000  0.005763  2082
 8       yes   51%   24000  0.020194  1188
 9       yes   55%   14401  0.000097149233
10       yes   40%   14401  0.000050289177
Total          72%  112372  0.026702  4208

Perf index = 43 (util) + 40 (thru) = 83/100
*/