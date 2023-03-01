#include "cachelab.h"
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
typedef struct
{
    bool valid;
    int mark;
    int timeStamp;
} block;

int main(int argc, char *argv[])
{
    int S = 0, E = 1, B = 0, opt, s, b;
    int hit = 0, miss = 0, evictions = 0;
    FILE *pFile;
    while (-1 != (opt = getopt(argc, argv, "s:E:b:t:")))
    {
        switch (opt)
        {
        case 's':
            s = atoi(optarg);
            S = pow(2, s);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            B = pow(2, b);
            break;
        case 't':
            pFile = fopen(optarg, "r");
            break;
        case 'v':
            break;
        default:
            printf("wrong argument\n");
            break;
        }
    }

    int blockSize = sizeof(block);
    block *cache = malloc(S * E * blockSize);
    memset(cache, 0, S * E * blockSize);
    int maskS = (S - 1) << b;
    int maskB = B - 1;

    char identifier;
    int address, size;
    int timeStamp = 0;
    while (fscanf(pFile, " %c %x,%d", &identifier, &address, &size) > 0)
    {
        if (identifier == 'I')
            continue;
        int offset = address & maskB;
        int groupId = (address & maskS) >> b;
        int mark = address >> (b + s);
        printf("%x %d",groupId,offset);
        block *p = cache + groupId * E;
        int mintime = 1e9;
        int eviction = 0;
        for (int i = 0; i < E; i++)
        {
            if (!p->valid)
            {
                p->valid = true;
                p->mark = mark;
                p->timeStamp = timeStamp;
                miss++;
                if (identifier == 'M')
                    hit++;
                break;
            }
            if (p->mark == mark)
            {
                hit++;
                p->timeStamp=timeStamp;
                if (identifier == 'M')
                    hit++;
                break;
            }
            eviction = mintime < p->timeStamp ? eviction : i;
            mintime = mintime < p->timeStamp ? mintime : p->timeStamp;
            p++;
            if (i == E - 1)
            {
                block *evict = cache + groupId * E + eviction;
                evict->valid = true;
                evict->mark = mark;
                evict->timeStamp = timeStamp;
                evictions++;
                miss++;
                if (identifier == 'M')
                    hit++;
            }
        }
        timeStamp++;
    }
    printSummary(hit, miss, evictions);
    free(cache);
    fclose(pFile);
    return 0;
}
