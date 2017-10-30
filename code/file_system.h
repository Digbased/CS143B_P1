#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H


void init_bitmap();
void print_bitmap();
void print_blocks();
void init_dir();
void print_ofts();

//https://stackoverflow.com/questions/389827/namespaces-in-c
typedef struct
{

	//create a new file with the specified name sym_name
	void (* const create)(char* filename);
	
	//destroy the named file
	//returns 1 on success, 0 on error
	int (* const destroy)(char* filename);
	
	//open the named file for reading and writing
	//returns an index value which is used by subsequent read, write, lseek, or close operations (OFT index)
	int (* const open)(char* filename);
	
	//closes the specified file
	void (* const close)(int index);
	
	// -- operations available after opening the file --//
	
	
	//returns the number of successfully read bytes from file into main memory
	//reading begins at the current position of the file
	int (* const read)(int index,char* mem_area,int count);
	
	//returns number of bytes written from main memory starting at mem_area into the specified file
	//writing begins at the current position of the file
	int (* const write)(int index,char* mem_area,int count);
	
	//move to the current position of the file to pos, where pos is an integer specifying
	//the number of bytes from the beginning of the file. When a file is initially opened,
	//the current position is automatically set to zero. After each read or write
	//operation, it points to the byte immediately following the one that was accessed last.
	void (* const lseek)(int index, int pos);
	
	//displays list of files and their lengths
	void (* const directory)();

	//restores ldisk from file.txt or create a new one if no file exists
	int (* const init)(char* filename);
	
	//save ldisk to file.txt
	int (* const save)(char* filename);
		
	//returns 0 if block is free, otherwise it returns 1 if block is taken
	int (* const isBitEnabled)(int logical_index);

	void (* const enableBit)(int logical_index);
	void (* const disableBit)(int logical_index);


} filespace_struct;
extern filespace_struct const file_system;

#endif
