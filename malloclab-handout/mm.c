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
#define PUT_POINTER(p, val) (*(int **)(p) = (val))
#define SIZE2ALIGN(size) (ALIGN(size + DWORD))
#define GET_SIZE(p) (GET(p) >> 1)
#define VALID(p) (GET(p) & 1)
#define PREV_P(p) (((char *)p) - GET_SIZE((char *)p - WORD))
#define NEXT_P(p) (((char *)p) + GET_SIZE(p))
#define NEXT_BLOCK(p) (*(int **)(((char *)p) + WORD))
#define PREV_BLOCK(p) (*(int **)(((char *)p) + DWORD))
#define FOOT(p) (((char *)p) + GET_SIZE(p) - WORD)
int round2log2(int num)
{
    for (int i = 4; i < 15; i++)
    {
        if (num <= (1 << i))
            return i - 4;
    }
    return 11;
}

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

int mm_init(void)
{
    int *p = mem_sbrk(12 * 4 * WORD + 4);
    for (int i = 0; i < 12; i++)
    {
        PUT(p, 1);
        PUT_POINTER(p + 1, NULL);
        PUT_POINTER(p + 2, NULL);
        PUT(p + 3, 1);
        p = p + 4;
    }
    PUT(p, 1);
    return 0;
}

void *mm_malloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }
    int align_size = SIZE2ALIGN(size);
    int *p = (int *)mem_heap_lo() + round2log2(align_size) * 4;
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
    PUT(p, GET(p) | 0x1);
    PUT(FOOT(p), GET(p));
    int *next = NEXT_BLOCK(p);
    int *prev = PREV_BLOCK(p);
    PUT_POINTER(prev + 1, next);
    if (next != NULL)
        PUT_POINTER(next + 2, prev);
    return p + 1;
}

void mm_free(void *ptr)
{
    int *p = (int *)ptr - 1;
    if (VALID(p - 1))
    {
        PUT(p, GET(p) & ~1);
        PUT(FOOT(p), GET(p));
        int size = GET_SIZE(p);
        int *prev = (int *)mem_heap_lo() + 4 * round2log2(size);
        int *next = NEXT_BLOCK(prev);
        while (next != NULL && GET_SIZE(next) < size)
        {
            prev = next;
            next = NEXT_BLOCK(prev);
        }
        bind(p, prev, next);
        return;
    }
    int prev_size = GET_SIZE(p - 1);
    int *prev_p = (int *)(((char *)p) - prev_size);
    int size = prev_size + GET_SIZE(p);
    int val = (size) << 1;
    PUT(prev_p, val);
    PUT(FOOT(p), val);
    int *next = NEXT_BLOCK(prev_p);
    int *prev = PREV_BLOCK(prev_p);
    PUT_POINTER(prev + 1, next);
    if (next != NULL)
        PUT_POINTER(next + 2, prev);
    prev = (int *)mem_heap_lo() + round2log2(size) * 4;
    next = NEXT_BLOCK(prev);
    while (next != NULL && GET_SIZE(next) < size)
    {
        prev = next;
        next = NEXT_BLOCK(prev);
    }
    bind(prev_p, prev, next);
}

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
void print_heap()
{
    int *p = mem_heap_lo();
    for (int i = 0; i < 12; i++)
    {
        for (int *ptr = p; ptr != NULL; ptr = NEXT_BLOCK(ptr))
            printf("%x\t%d\t%x\t%x\t%d\n", ptr, *ptr, *(ptr + 1), *(ptr + 2), *(int *)FOOT(ptr));
        p += 4;
    }
}
/*
trace  valid  util     ops      secs  Kops
 0       yes   54%    5694  0.000074 76946
 1       yes   47%    5848  0.000084 69619
 2       yes   68%    6648  0.000072 92333
 3       yes   80%    5380  0.000059 91966
 4       yes    0%   14400  0.000138104197
 5       yes   82%    4800  0.000096 50157
 6       yes   80%    4800  0.000097 49587
 7       yes   55%   12000  0.000086139860
 8       yes   51%   24000  0.000158152284
 9       yes   29%   14401  0.000182 79257
10       yes    4%   14401  0.000096150167
Total          50%  112372  0.001140 98555

Perf index = 30 (util) + 40 (thru) = 70/100
*/