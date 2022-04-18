//pagetable.c will manage the page table for the process.

typedef struct PageTableEntry {
    int validBit;
    unsigned long frameNum;
    unsigned int refCount;
} PageTableEntry;

//typedef struct PageTable {
//} PageTable;
