#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "pagetable.c"
#include "phypages.c"

#define MAX_PAGE_TABLE 32

void initPageTable(PageTableEntry pageTable[], int pageTableLength);
void initFreeFrames(FreeFrames* frameTable);
int findLowestRefCount(PageTableEntry pageTable[], int pageTableLength);

int main(int argc, char* argv[]) {
    int fd_in;
    char* inputFileName = argv[4];
    char* outputFileName = argv[5];

    unsigned long bytesPerPage = atol(argv[1]);
    unsigned long virtualMemory = atol(argv[2]);
    unsigned long physicalMemory = atol(argv[3]);

    printf("NEW SIZES: %d Bytes per Page | %d Bytes in Virtual Memory | %d Bytes in Physical Memory\n", bytesPerPage, virtualMemory, physicalMemory);

    long physicalMemPages = physicalMemory / bytesPerPage;
    long virtualMemPages = virtualMemory / bytesPerPage;

    printf("%d Pages Physically, %d Pages Virtually\n", physicalMemPages, virtualMemPages);

    unsigned long filesize;
    unsigned long* memAccesses;
    struct stat st;
    int i;

    if(argc != 6) {
        fprintf(stderr, "Usage: %s SequenceFile\n", argv[0]);
        exit(0);
    }
    stat(inputFileName, &st);
    filesize = st.st_size;

    memAccesses = (unsigned long*) malloc( (size_t) filesize );

    fd_in = open(inputFileName, O_RDONLY);
    if(fd_in == -1) {
        fprintf(stderr, "fd is invalid, with error %d\n", strerror(errno));
        exit(-1);
    }

    read(fd_in, memAccesses, (int) filesize);
    close(fd_in);

    PageTableEntry pageTable[virtualMemPages];
    FreeFrames framesTable;
    initFreeFrames(&framesTable);
    initPageTable(pageTable, virtualMemPages);
    unsigned long offset;
    unsigned long pageNum;
    unsigned long physicalAddress;
    unsigned int currentRefCount = 0;
    unsigned int pageFaults = 0;
    unsigned int currentLowestRefCount;
    FILE* out = fopen(argv[5], "wb");

    for(i = 0; i < filesize/(sizeof (unsigned long)); i += 2) {
        pageNum = memAccesses[i] >> 7;
        offset = memAccesses[i] & 0x7f;

        //Check Valid Bit
        if(pageTable[pageNum].validBit == 0){
            //Validate Bit
            if(framesTable.currentFrame <= physicalMemPages){
                //If we haven't filled up the Free Frames Table
                pageTable[pageNum].frameNum = framesTable.currentFrame++;
                //printf("Page Table Frame Number: %d && %#010lx\n", pageTable[pageNum].frameNum, pageTable[pageNum].frameNum);
                pageTable[pageNum].refCount = currentRefCount++;
            } else {
                currentLowestRefCount = findLowestRefCount(pageTable, virtualMemPages);
                pageTable[currentLowestRefCount].validBit = 0;

                pageTable[pageNum].validBit = 1;
                pageTable[pageNum].frameNum = pageTable[currentLowestRefCount].frameNum;
                pageTable[pageNum].refCount = currentRefCount++;
            }
            pageTable[pageNum].validBit = 1;
            pageFaults++;
        } else {
            currentLowestRefCount = findLowestRefCount(pageTable, virtualMemPages);
            pageTable[currentLowestRefCount].validBit = 0;
            pageTable[pageNum].validBit = 1;
            pageTable[pageNum].frameNum = pageTable[currentLowestRefCount].frameNum;
            pageTable[pageNum].refCount = currentRefCount++;
            //Valid
            //pageTable[pageNum].validBit = 0;

        }

        physicalAddress = pageTable[pageNum].frameNum << 7 | offset;
        fprintf(out, "%#010lx\n", physicalAddress);
        //printf("Valid Bit: %d, Frame Num: %#010lx, Ref Count, %d\n", pageTable[i].validBit, pageTable[i].frameNum, pageTable[i].refCount);
        //printf("Offset: %#010lx, Page Table Index: %#010lx, Frame Num: %#010lx\n", offset, pageNum, pageTable[i].frameNum);
        //printf("Virtual Address %d => %#010lx | Physical Address %d => %#010lx\n", i, memAccesses[i], i, physicalAddress);
    }

    close(out);
    free (memAccesses); //free dynamically allocated memory

    printf("Page Faults: %d\n", pageFaults);
    return 0;
}

void initPageTable(PageTableEntry pageTable[], int pageTableLength){
    int i;
    for(i = 0; i < pageTableLength; i++){
        pageTable[i].validBit = 0;
        pageTable[i].refCount = 0;
        pageTable[i].frameNum = 0;
        //printf("Current Page Table Entry %d: validBit = %d, frameNum = %d, refCount = %d\n", i, pageTable[i].validBit, pageTable[i].frameNum, pageTable[i].refCount);
    }
}

void initFreeFrames(FreeFrames* frameTable){
    frameTable->currentFrame = 1;
}

int findLowestRefCount(PageTableEntry pageTable[], int pageTableLength){
    //Returns Index of Smallest Ref Count
    int i;
    int lowestRefCount = pageTable[0].refCount;
    int lowestRefCountIdx = 0;
    for(i = 0; i < pageTableLength; i++){
        if(pageTable[i].refCount < lowestRefCount && pageTable[i].validBit == 1) {
            lowestRefCount = pageTable[i].refCount;
            lowestRefCountIdx = i;
        }
    }
    return lowestRefCountIdx;
}
