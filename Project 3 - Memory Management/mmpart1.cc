#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>


using namespace std;

/***************************************************************************************************
 * Code Taken From: people.umass.edu/tongping/teaching/ece570/analyzesequence.c
 *
 * Grab file name from argv[1] and store into variable (fileName)
 * Create stat object to get file info (st)
 * Get file info via stat(const char *restrict pathname, struct stat *restrict statbuf)
 *  pathname = fileName
 *  statbuf = &st
 * Retrieve file size via st_size
 *  fileSize = st.st_size
 * Open the file with open(const char * pathname, mode_t mode)
 *  pathname = fileName
 *  mode = O_RDONLY
 * Calculate mapSize w/ align(size_t file_size, size_t pageSize) Interpretation from: https://zinascii.com/2014/the-8-byte-two-step.html#green-bit
 *  - adds 128 to make sure the size hits the next alignment boundary or lies past it
 *  - Truncate the lower bits
 * Create new mapping in virtual address space w/ void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
 *  addr = 0                    [Starting address for the new mapping]
 *  length = mapSize            [Length of the new mapping]
 *  prot = PROT_READ            [Memory protection of the mapping]
 *  flags = MAP_PRIVATE         [Determines whether updates to the mapping are visible to other processes mapping the same region]
 *  fd = fd                     [file descriptor]
 *  offset = 0                  [Offset in the file where the mapping starts]
 *  - Returns a pointer to the mapped area
 * Create a new FILE * object and create a new file with write privileges
 * for (int i = 0; i < fileSize/sizeof(unsigned long long); i++)
 *  - Starting from address 0 until fileSize/sizeof(unsigned long long)
 *      - fileSize = 64 (bytes)
 *      - sizeof(unsigned long long) = 8 (bytes)
 *  - temp = index for page table
 *      - bitwise shift to the right of 7 will remove the offset bits in memAccesses[i] and insert 0's at the beginning
 *  - foo = starting address
 *      - access frame number then multiply by frame size
 *  - bar = offset
 *      - 127 (0b0111 1111) & x (any number) will leave only the last 7 digits (ie the offset)
 *  - foobar = physical address
 *  - decToBytes(int n, FILE * output) 
 *      n = foobar
 *      output = fd2
 *      - Prints bytes of the physical address (foobar) into the file (fd2)
 * close(fd)
 * fclose(fd2)
 **************************************************************************************************/


#define page_size 128 // Given that pages are 128 bytes, required to multiply to obtain starting address

inline size_t align(size_t file_Size, size_t pageSize) {
    return ((file_Size + (pageSize - 1)) & ~(pageSize - 1));
}

int page_table[7] = {2, 4, 1, 7, 3, 5, 6};

int decToBytes(unsigned long long n, FILE * output) {
    
    unsigned char y;

    // Iterate through each byte of address
    for(int i = 0; i < 8; i++){

        // Strip out last byte of value to be printed
        y = (char) (n >> (i * 8)) & 0xff;

        // Write to output file
        fwrite(&y, 1, 1, output);
    }
}

int main(int argc, char** argv) {
    // Store input file name in variable
    char * fileName = argv[1];

    int fd;
    struct stat st;
    unsigned long fileSize;
    unsigned long mapSize;
    unsigned long long * memAccesses;
    stat(fileName, &st);
    fileSize = st.st_size;

    fd = open(fileName, O_RDONLY);

    // Compute the aligned size
    mapSize = align(fileSize, page_size);

    memAccesses = (unsigned long long *) mmap(0, mapSize, PROT_READ, MAP_PRIVATE, fd, 0);

    FILE * fd2 = fopen("output-part1", "wb");

    for (int i = 0; i < fileSize/sizeof(unsigned long long); i++) {
        // %lx prints unsigned long int in hex
        unsigned long long temp = memAccesses[i] >> 7;
        unsigned long long foo = page_table[temp] * page_size;
        unsigned long long bar = memAccesses[i] & 127;
        unsigned long long foobar = foo + bar;
        decToBytes(foobar, fd2);
    }
    close(fd);
    fclose(fd2);
}


