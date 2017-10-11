#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

//create a new file with the specified name sym_name
void create(char* filename);

//destroy the named file
//returns 1 on success, 0 on error
int destroy(char* filename);

//open the named file for reading and writing
//returns an index value which is used by subsequent read, write, lseek, or close operations (OFT index)
int open(char* filename);

//closes the specified file
void close(int index);

// -- operations available after opening the file --//


//returns the number of successfully read bytes from file into main memory
//reading begins at the current position of the file
int read(int index,char* mem_area,int count);

//returns number of bytes written from main memory starting at mem_area into the specified file
//writing begins at the current position of the file
int write(int index,char* mem_area,int count);

//move to the current position of the file to pos, where pos is an integer specifying
//the number of bytes from the beginning of the file. When a file is initially opened,
//the current position is automatically set to zero. After each read or write
//operation, it points to the byte immediately following the one that was accessed last.
void lseek(int index, int pos);

//displays list of files and their lengths
void directory();

#endif
