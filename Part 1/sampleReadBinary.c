#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char ** argv) {
	char *fileName;
	int fd_in;
	struct stat st;
	unsigned long filesize;
	unsigned long* memAccesses;
	int i; 

	if(argc != 2) {
		fprintf(stderr, "Usage: %s SequenceFile\n", argv[0]);
		exit(0); 
	}
	fileName = argv[1];
	stat(fileName, &st);
	filesize = st.st_size;


	//allocate space for all addresses
	//filesize == 128 bits
	//pointer to a long that is 128 bits
	memAccesses = (unsigned long*) malloc( (size_t) filesize );

	//use open and read
	fd_in = open(fileName, O_RDONLY);
	if(fd_in == -1) {
		fprintf(stderr, "fd is invalid, with error %d\n", strerror(errno));
		exit(-1);
	}
	
	//read all bytes from file at once without check errors !!!
	read(fd_in, memAccesses, (int) filesize);
	close(fd_in);

	// Traverse all address
	for(i = 0; i < filesize/(sizeof (unsigned long)); i++) {
	  	printf("logical address %d = %#010lx\n", i, memAccesses[i]);
	}
	free (memAccesses); //free dynamically allocated memory
}
