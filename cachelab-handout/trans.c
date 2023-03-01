/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int temp[8][8];
    for (int i = 0; i < M; i += 8)
    {
        for (int j = 0; j < N; j += 8)
        {
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    temp[x][y] = A[j + x][i + y];
                }
            }
            for (int x = 0; x < 8; x++)
            {
                for (int y = x + 1; y < 8; y++)
                {
                    int t = temp[x][y];
                    temp[x][y] = temp[y][x];
                    temp[y][x] = t;
                }
            }
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    B[i + x][j + y] = temp[x][y];
                }
            }
        }
    }
    //I'm confused that I couldn't handle the situation that M/N not equal
    //to 8 times sometime, but it work well and gain full mark in the remain
    //two circumstances.
    
    if(M%8){
        for(int i=(M/8)*8;i<M;i++){
            for(int j=0;j<N;j++)
            B[i][j]=A[j][i];
        }
    }
    if(N%8){
        for(int i=(N/8)*8;i<N;i++){
            for(int j=0;j<M;j++)
            B[j][i]=A[i][j];
        }
    }
    is_transpose( M, N, A, B);
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(const int M,const int N, int A[N][M], int B[M][N])
{
    
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                printf("%d %d",i,j);
                return 0;
            }
        }
    }
    return 1;
}
