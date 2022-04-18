#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

//Max # of Pages => 32
//Max # of Frames => 8
//Max # of Entries in Page Table => 32

//Offset => 7 bits
//Virtual Address Space => 12 bits
//Physical Address Space => 10 bits

//Ex:
    //VA => 0x44 => 0000 0100 0100
    //Offset => 100 0100
    //PA => 01 0100 0100 => 0x144

    //VA => 0x224 => 0010 0010 0100
    //Offset => 0100100
    //Page Number => 0010 0 => 4
    //Page Table entry for #4  => 3
    //PA => 0x1A4 => 01 1010 0100

    //VA => 0x144 => 0001 0100 0100
    //Offset => 100 0100 => VA & 0x7f OR VA % pageSize
    //Page Num => 0001 0 => VA >> 7
int main(int argc, char* argv[]) {
	int fd_in;
    char* inputFileName = argv[1];
    //char* outputFileName = argv[2];
    unsigned long filesize;
    unsigned long* memAccesses;
    struct stat st;
    int i;
    int pageTable[] = {2, 4, 1, 7, 3, 5, 6};

    if(argc != 3) {
		fprintf(stderr, "Usage: %s SequenceFile\n", argv[0]);
		exit(0); 
	}
	stat(inputFileName, &st);
	filesize = st.st_size;

	//allocate space for all addresses
    //filesize == 64bits == 8 bytes
    //8 entries 
    // 64 / 8 entries = 4 bits / entry
	memAccesses = (unsigned long*) malloc( (size_t) filesize );

	//use open and read
	fd_in = open(inputFileName, O_RDONLY);
	if(fd_in == -1) {
		fprintf(stderr, "fd is invalid, with error %d\n", strerror(errno));
		exit(-1);
	}
	
	//read all bytes from file at once without check errors !!!
	read(fd_in, memAccesses, (int) filesize);
	close(fd_in);

    unsigned long offset;
    unsigned long pageNum;
    unsigned long frameNum;
    unsigned long physicalAddress;
    FILE* out = fopen(argv[2], "wb");

	// Traverse all address
	for(i = 0; i < filesize/(sizeof (unsigned long)); i++) {
        //Virtual Address = memAccesses[i]
        // printf("logical address %d = %#010lx\n", i, memAccesses[i]);
        pageNum = memAccesses[i] >> 7;
        offset = memAccesses[i] & 0x7f;
        frameNum = pageTable[pageNum];
        physicalAddress = frameNum << 7 | offset;
        // printf("physical address %d = %#010lx\n",i, physicalAddress);
        printf("logical address %#010lx => physical address %#010lx\n", memAccesses[i], physicalAddress);
        fprintf(out, "%#010lx\n", physicalAddress);
        // fwrite(&physicalAddress, sizeof physicalAddress, 1, out);
        // printf("%#010lx\n", pageNum);
        // printf("%#010lx\n", offset);
	}


	free (memAccesses); //free dynamically allocated memory
    return 0;
}
