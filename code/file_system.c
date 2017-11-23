#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "file_system.h"
#include "io_system.h"

#define BITS 8
#define FILES_COUNT ((OFT_SIZE) - 1)
//free refers to a free fd or unassigned block number in a fd
#define FREE -1

extern char ldisk[L][B];
extern open_file_table oft[OFT_SIZE];

//8 BITS used to set and clear bits in the bitmap of block 0
static int MASK[BITS]; 

void print_fds()
{
	for(int i = 0;i < MAX_DIR_ENTRIES;++i)
	{
		file_descriptor fd = GetFD(i);
		printf("fd[%d] file len: %d\n",i,fd.file_len);
		printf("disk map: ");
		for(int k = 0;k < DISK_BLOCKS_COUNT;++k)
			printf("%d ",fd.block_numbers[k]);
		printf("\n");
	}
}

void print_ofts()
{
	for(int i = 0;i < OFT_SIZE;++i)
	{
		printf("oft[%d] fd_index: %d\n",i,oft[i].fd_index);
		printf("oft file_len: %d\n",oft[i].file_len);
		printf("oft cur_pos: %d\n",oft[i].cur_pos);
		int* tmp_buf = (int*)(oft[i].buffer);
		for(int k = 0;k < 16;++k)
		{
			printf("%d ",tmp_buf[k]);
		}
		printf("\n");
	}
}

//helper function to initialize bit masks
//initialize 8 bit masks diagonally
void init_mask()
{
	//initialize the bit masks starting from most significant bit to least
	//significant order
	MASK[0] = 0x80;
	for(int i = 1;i < BITS;++i)
	{
		MASK[i] = MASK[i - 1] >> 1;
	}
}

void print_bitmap()
{
	char bitmap[B];
	io_system.read_block(0,bitmap);
	
	int n = B / BITS;//is the number of bytes required to store the bitmap in block 0
	for(int k = n;k >= 0;--k)
	{
		for(int i = BITS - 1;i >= 0; --i)
		{
			int isEnabled = (bitmap[k] >> i) & (0x01);
			printf("%d",isEnabled);	
		}
	}
	printf("\n");
}

void print_blocks()
{
	for(int k = 0;k < B;++k)
	{
		char block[B];
		io_system.read_block(k,block);

		printf("logical_index: %d\n", k);
		//print as ints
		for(int i = 0;i < B; i += sizeof(int))
		{
			printf("%d ",*(int*)(&block[i]) );
		}
		printf("\n");
		//print as char strings
		for(int i = 0;i < B;i += sizeof(int) * DIR_ENTRY_CAPACITY)
		{	
			if(*(int*)(&block[i]) == FREE)
				printf("%d ",*(int*)(&block[i]));
			else
				printf("%s ",(char*)&block[i]);
		}
		printf("\n");
	}

}

//create a new file with the specified name sym_name
static void create(char* filename)
{
	//printf("create file: %s\n",filename);

	//search in ldisk if filename is already listed in one of the directory data blocks
	file_descriptor directory_fd = GetFD(0);
	for(int k = 0;k < DISK_BLOCKS_COUNT;++k)
	{
		if(directory_fd.block_numbers[k] == FREE) continue;
		char dirBlock[B];
		io_system.read_block(directory_fd.block_numbers[k],dirBlock);
		for(int m = 0;m < B;m += sizeof(int) * DIR_ENTRY_CAPACITY)
		{
			char* sym_name = (char*)(&dirBlock[m]);
			if(strcmp(sym_name,filename) == 0)
			{
				printf("Error: %s already exists on ldisk\n",filename);
				return;
			}
		}
	}

	int fd_index = -1;
	int target_fd_location = -1;//where the fd meta data is located in the logical blockk
	int target_logical_index = -1;//which logical block on ldisk the fd was found
	for(int logical_index = 1;logical_index < RESERVED_BLOCKS; ++logical_index)
	{
	//find a free file descriptor in ldisk for the file
		char block[B];
		io_system.read_block(logical_index,block);
		
		for(int k = 0;k < B; k += FDS_PER_BLOCK * FD_CAPACITY)
		{
			int fdValue = *(int*)(block + k);
			if(fdValue == FREE)
			{
				//mark as taken
				fd_index = (k + (BYTES_PER_BLOCK * (logical_index - 1)) ) / INTS_PER_BLOCK;
			//	printf("create fd_index: %d\n",fd_index);
				target_logical_index = logical_index;
				target_fd_location  = k;
				logical_index = RESERVED_BLOCKS;
				break;
			}
		}
		
	}

	if(fd_index == -1)
	{
		printf("Error not enough space to store %s on ldisk\n",filename);
		return;
	}

	//search for a free directory entry slot to store filename and file_length in the directory data blocks
	int target_block_index = -1;
	int target_entry_location = -1;
	for(int k = 0;k < DISK_BLOCKS_COUNT;++k)
	{
		//grow the directory as needed when creating new files..
		if(directory_fd.block_numbers[k] == FREE)
		{
			int expected_block = -1;
			for(int i = RESERVED_BLOCKS;i < B;++i)
			{
				if(file_system.isBitEnabled(i) == 0)
				{
					expected_block = i;
					file_system.enableBit(i);
					directory_fd.block_numbers[k] = i;
					WriteFDToLDisk(0,directory_fd);//write back to directory fd
					break;
				}
			}

			if(expected_block == -1)//if no available blocks can be found
			{
				printf("Error: create cannot find available blocks on ldisk\n");
				return;
			}

		}
		
		char tmpBuf[B];
		io_system.read_block(directory_fd.block_numbers[k],tmpBuf);
		for(int m = 0;m < B;m += sizeof(int) * DIR_ENTRY_CAPACITY)
		{
			//check if available directory entry slot
			int status = *(int*)(&tmpBuf[m]);
			if(status == FREE)
			{
				target_entry_location = m;
				target_block_index = k;
				k = DISK_BLOCKS_COUNT;
				break;
			}
		}

	}

	if(target_block_index == -1)
	{
		printf("Error: unable to find free directory entry slot\n");
		return;
	}

	//update directory file size by 8 bytes
	directory_fd.file_len += sizeof(int) * DIR_ENTRY_CAPACITY;
	WriteFDToLDisk(0,directory_fd);

	//fill free file descriptor info and directory entry info
	char fdBlock[B];
	io_system.read_block(target_logical_index,fdBlock);
	int empty_filelen = 0;
	memcpy(fdBlock + target_fd_location,&empty_filelen,sizeof(int));
	io_system.write_block(target_logical_index,fdBlock);

	//fill directory entry info
	char dirEntryBlock[B];
	io_system.read_block(directory_fd.block_numbers[target_block_index],dirEntryBlock);
	//dir entry sym_name
	memcpy(dirEntryBlock + target_entry_location,filename,sizeof(char) * 4);
	//dir_entry fd_index
	memcpy(dirEntryBlock + target_entry_location + sizeof(int),&fd_index,sizeof(int));
	io_system.write_block(directory_fd.block_numbers[target_block_index],dirEntryBlock);

	//TODO: add changes to oft buffer directory
	memcpy(oft[0].buffer,dirEntryBlock,B);
	oft[0].file_len = directory_fd.file_len;

	//debugging ~ print fd index
	//printf("create: fd_index = %d\n",fd_index);

	//valid output
	printf("%s created\n",filename);
}

//destroy the named file (assume that a file will not be open when the destroy call is made)
//returns 1 on success, 0 on error
static int destroy(char* filename)
{
	//printf("destroy file: %s\n",filename);
	
	//find the file descriptor by searching the directory (look for the directory entry filename)
	
	int t_blocknumber = -1;
	int t_blocklocation = -1;
	int t_fd_index = -1;
	file_descriptor dir_fd = GetFD(0);
	for(int k = 0;k < DISK_BLOCKS_COUNT;++k)
	{
		//not found in this fd_index
		if(dir_fd.block_numbers[k] == FREE) continue;
	
		char directoryData[B];
		io_system.read_block(dir_fd.block_numbers[k],directoryData);
		int dir_entry_len = sizeof(int) * DIR_ENTRY_CAPACITY;//8 bytes
		for(int m = 0;m < B;m += dir_entry_len)
		{
			char* sym_name = (char*)(&directoryData[m]);
			if(strcmp(sym_name,filename) == 0)
			{
				t_blocknumber = dir_fd.block_numbers[k];
				t_blocklocation = m;
				k = DISK_BLOCKS_COUNT;
				break;
			}
		}
			

	}

	if(t_blocknumber == -1)
	{
		printf("Error: could not find %s to destroy\n",filename);
		return 0;
	}


	//remove directory entry
	char temp_buf[B];
	io_system.read_block(t_blocknumber,temp_buf);	
	t_fd_index = *(int*)(&temp_buf[t_blocklocation + sizeof(int)]);
	
	//if the file is open in the oft.. also remove it from there
	for(int k = 0;k < DISK_BLOCKS_COUNT;++k)
	{
		if(oft[k].fd_index == t_fd_index)
		{
			oft[k].cur_pos = -1;
			oft[k].file_len = -1;
			oft[k].fd_index = -1;
			int freeBlock[INTS_PER_BLOCK];
			for(int m = 0;m < INTS_PER_BLOCK;++m)
				freeBlock[m] = -1;
			memcpy(oft[k].buffer,freeBlock,sizeof(int) * INTS_PER_BLOCK);
		}
	}
	
	int reset = -1;
	memcpy(&temp_buf[t_blocklocation],&reset,sizeof(int));//sym_name
	memcpy(&temp_buf[t_blocklocation + sizeof(int)],&reset,sizeof(int));//fd_index
	io_system.write_block(t_blocknumber,temp_buf);

	//copy over to directory oft buffer if oft buffer is referencing from t_blocknumber
	int dir_oft_index = oft[0].cur_pos / BYTES_PER_BLOCK;
	if(dir_fd.block_numbers[dir_oft_index] == t_blocknumber)
	{
		//printf("Note: destroy updating dir oft to reflect changes made in file_system\n");
		memcpy(oft[0].buffer,temp_buf,B);
	}

	//update the bitmap to reflect the freed blocks
	//printf("Destroy: t_fd_index: %d\n",t_fd_index);
	file_descriptor dir_contents_fd = GetFD(t_fd_index);
	for(int i = 0;i < DISK_BLOCKS_COUNT;++i)
	{
		if(dir_contents_fd.block_numbers[i] == FREE) continue;
		
		//printf("Destroy: disable data block %d\n",dir_contents_fd.block_numbers[i]);
		file_system.disableBit(dir_contents_fd.block_numbers[i]);
		int numBuf[INTS_PER_BLOCK];
		io_system.read_block(dir_contents_fd.block_numbers[i],(char*)numBuf);
		for(int k = 0;k < INTS_PER_BLOCK;++k)
			numBuf[k] = -1;
		io_system.write_block(dir_contents_fd.block_numbers[i],(char*)numBuf);
		dir_contents_fd.block_numbers[i] = FREE;
	}

	//free the file descriptor
	dir_contents_fd.file_len = -1;
	WriteFDToLDisk(t_fd_index,dir_contents_fd);	

	//reduce the number of bytes held in directory fd?
	dir_fd.file_len -= sizeof(int) * DIR_ENTRY_CAPACITY;
	WriteFDToLDisk(0,dir_fd);
	oft[0].file_len = dir_fd.file_len;//have directory oft file len match directory fd file len
	//return status	
	return 1;
}

//helper function for open to retrieve the fd of the associated filename
//returns the fd_index of the filename, otherwise return -1 if not found
int open_fd(char* filename)
{
	int fd_index = -1;	
	file_descriptor dir_fd = GetFD(0);
	int dir_entry_size = sizeof(int) * DIR_ENTRY_CAPACITY; //size of each dir entry in bytes
	for(int i = 0;i < DISK_BLOCKS_COUNT;++i)
	{
		if(dir_fd.block_numbers[i] == FREE) continue;
		char dirContents[B];
		io_system.read_block(dir_fd.block_numbers[i],dirContents);
		for(int k = 0;k < B;k += dir_entry_size)
		{
			char* sym_name = (char*)(&dirContents[k]);
			if(strcmp(sym_name,filename) == 0)
			{
				fd_index = *(int*)(&dirContents[k + sizeof(int)]);
				break;
			}	
		}

	}

	return fd_index;
}

//open the named file for reading and writing
//returns an index value which is used by subsequent read, write, lseek, or close operations (OFT index)
static int open(char* filename)
{
	//search if filename is already opened in the oft and return error
	for(int i = 1;i < OFT_SIZE;++i)
	{
		if(oft[i].fd_index == FREE) continue;
		//printf("oft[%d].fd_index = %d\n",i,oft[i].fd_index);

		//i should try to go through the entire directory list and match by fd_index
		file_descriptor dir_fd = GetFD(0);
		for(int k = 0;k < DISK_BLOCKS_COUNT;++k)
		{
			if(dir_fd.block_numbers[k] == FREE) continue;
			char dirEntryBlock[B];
			io_system.read_block(dir_fd.block_numbers[k],dirEntryBlock);
			for(int m = 0;m < B;m += sizeof(int) * DIR_ENTRY_CAPACITY)
			{
				int status = *(int*)(&dirEntryBlock[m]);
				if(status == FREE) continue;
				char* sym_name = (char*)(&dirEntryBlock[m]);
				if(strcmp(sym_name,filename) == 0)
				{
					int target_fd_index = *(int*)(&dirEntryBlock[m + sizeof(int)]);
					if(target_fd_index == oft[i].fd_index)
					{
						printf("Error: %s fd: %d already opened at oft_index: %d\n",filename,oft[i].fd_index,i);
						return -1;
					}
				}
			}
		}

	}

	//search directory to find index of file descriptor	
	int fd_index = open_fd(filename);
	if(fd_index == -1)
	{
		printf("error: could not open %s\n",filename);
		return -1;
	}
	
	//allocate a free OFT entry(reuse deleted entries)
	int oft_index = -1;
	for(int i = 1;i < OFT_SIZE;++i)
	{
		if(oft[i].fd_index == FREE)
		{
			oft_index = i;
			oft[i].fd_index = fd_index;
			oft[i].cur_pos = 0; 
			
			file_descriptor data_fd = GetFD(fd_index);
			oft[i].file_len = data_fd.file_len;
			if(data_fd.file_len == FREE)
				oft[i].file_len = 0;

			//allocate a datablock to the file be opened
			if(data_fd.block_numbers[0] == FREE)
			{
			//	printf("Warning: open... %s is an empty file\n",filename);
				//find a free datablock to allocate with on ldisk
				for(int k = RESERVED_BLOCKS;k < B;++k)
				{
					if(file_system.isBitEnabled(k) == 0)
					{
						file_system.enableBit(k);
						data_fd.block_numbers[0] = k;
						WriteFDToLDisk(fd_index,data_fd);
						break;
					}
				}

				if(data_fd.block_numbers[0] == FREE) // no data blocks found..
				{
					printf("Error: open no more disk space available\n");
					return -1;
				}
			}
		
			
			//read the first data block into the oft buffer
			io_system.read_block(data_fd.block_numbers[0],oft[i].buffer);
			break;
		}
	}

	if(oft_index == -1)
	{
		printf("Error: all entries in OFT are in use\n");
	}

	return oft_index;
}

//closes the specified file
//returns the oft_index closed on success
//returns -1 on error
static int close(int oft_index)
{
	if(oft_index == 0)
	{
		printf("Error: close cannot explicitly close directory oft\n");
		return -1;
	}

	if(oft_index < 0 || oft_index >= OFT_SIZE)
	{
		printf("Error: close invalid oft_index: %d\n",oft_index);
		return -1;
	}
	
	//error.. can't close a oft index that's already closed
	if(oft[oft_index].fd_index == FREE)
	{
		printf("Error: %d already closed\n",oft_index);
		return -1;
	}

	//write buffer to disk ~ how do I know which block_number to write buffer to?
	file_descriptor data_fd = GetFD(oft[oft_index].fd_index);

	//get the current block number oft buffer is supposed to write back to
	int blockIndex = oft[oft_index].cur_pos / BYTES_PER_BLOCK;
	if(blockIndex  < DISK_BLOCKS_COUNT)
	{
		int blockNumber = data_fd.block_numbers[blockIndex];
		io_system.write_block(blockNumber,oft[oft_index].buffer);
	}
	//free OFT entry
	oft[oft_index].fd_index = FREE;	
	oft[oft_index].file_len = FREE;
	oft[oft_index].cur_pos = FREE;

	int freeBlock[INTS_PER_BLOCK];
	for(int i = 0;i < INTS_PER_BLOCK;++i)
	{
		freeBlock[i] = 0;
	}
	memcpy(oft[oft_index].buffer,freeBlock,INTS_PER_BLOCK * sizeof(int));

	//printf("close file at index: %d\n",oft_index);
	//return status
	return oft_index;
}

// -- operations available after opening the file --//



//returns the number of successfully read bytes from file into main memory
//reading begins at the current position of the file
static int read(int index,char* mem_area,int count)
{
	if(index == 0)
	{
		printf("Error: cannot explicitly read from directory oft\n");
		return -1;
	}
	if(index < 0 || index >= OFT_SIZE)
	{
		printf("Error: read invalid oft_index: %d\n",index);
		return -1;
	}
	
	//compute the position within read/write buffer that corresponds to cur_pos within file
	open_file_table* cur_file = &oft[index];	

	if(oft[index].fd_index == FREE)
	{
		printf("Error: read invalid oft_index %d is free\n",oft[index].fd_index);
		return -1;
	}
	

	//if reading attempts to exceed file_len, throw an error
	if(cur_file->cur_pos + count >  oft[index].file_len)
	{
		printf("Error: read cmd cannot read past file's file_len: %d\n",oft[index].file_len);
		return -1;
	}

	//if max eof exceeds
	if(cur_file->cur_pos + count > DIR_BLOCKS * BYTES_PER_BLOCK)
	{
		int max_len = DIR_BLOCKS * BYTES_PER_BLOCK;
		printf("Error: read cmd cannot exceed %d bytes for reading\n",max_len);
		return -1;
	}


	int buf_pos = cur_file->cur_pos % BYTES_PER_BLOCK; 	
	int bytes_read = 0;
	while(1)
	{
		//if the desired count or eof is reached; update current position and return status
		if(bytes_read == count)
		{
		//	printf("Normal read: %d\n",bytes_read);
			cur_file->cur_pos += bytes_read;
			mem_area[bytes_read] = '\0';
			return bytes_read;
		}
		//if eof is reached
		if(cur_file->cur_pos + bytes_read == cur_file->file_len)
		{
			//printf("Warning:read eof reached: %d\n",bytes_read);
			cur_file->cur_pos += bytes_read;
			mem_area[bytes_read] = '\0';
			return bytes_read;
		}
		//if max eof is reached
		if(cur_file->cur_pos + bytes_read == DIR_BLOCKS * BYTES_PER_BLOCK)//192 bytes max
		{
			//printf("Warning: read max eof reached %d\n",bytes_read);
			cur_file->cur_pos += bytes_read;
			mem_area[bytes_read] = '\0';
			return bytes_read;
		}


		//if end of buffer is reached
		//1. write the buffer into the appropriate block on disk (if modified)
		//2. read the next sequential block from disk into buffer;
		//3. continue copying bytes from buffer into mem_area
		if(buf_pos == BYTES_PER_BLOCK)
		{
			//describes which block index to reference for data when looking at the corresponding data's fd
			int block_index = (cur_file->cur_pos + bytes_read) / BYTES_PER_BLOCK;
			int prevIndex = (cur_file->cur_pos + bytes_read - 1) / BYTES_PER_BLOCK;


			file_descriptor data_fd = GetFD(cur_file->fd_index);
			//edge case ~ so if cur_pos is at 190.. and you are attempting to read 50 bytes.. only read the first 2 bytes..
			if(block_index >= DISK_BLOCKS_COUNT || data_fd.block_numbers[block_index] == FREE)
			{
			//	printf("Warning: reading only the  first  %d bytes\n",bytes_read);
				cur_file->cur_pos += bytes_read;
				return bytes_read;
			}

			//I should write back the oft buffer to ldisk but I don't see the point as I'm not modifying the oft buffer in any way here...
			io_system.write_block(data_fd.block_numbers[prevIndex],cur_file->buffer);

			//read next block
			io_system.read_block(data_fd.block_numbers[block_index],cur_file->buffer);
			//reset buf pos relative to new data block (range 0 to 64)
			buf_pos = 0;
		}

		//copy oft buffer into mem area
		mem_area[bytes_read++] = cur_file->buffer[buf_pos++];
	}

	return -1;
}

//returns number of bytes written from main memory starting at mem_area into the specified file
//writing begins at the current position of the file
static int write(int index,char* mem_area,int count)
{
	if(index == 0)
	{
		printf("Error: write cannot explicitly write to oft directory\n");
		return -1;
	}

	if(index < 0 || index >= OFT_SIZE)
	{
		printf("Error: write invalid oft_index: %d\n",index);
		return -1;
	}

	open_file_table* cur_file = &oft[index];
	
	if(cur_file->fd_index == FREE)
	{
		printf("Error: write.. file not opened for reading or writing\n");
		return -1;
	}

	//if the projected number of bytes in the file attempt tp exceed 192 bytes ..output 0 bytes written
	if(cur_file->cur_pos + count > DIR_BLOCKS * BYTES_PER_BLOCK)
	{
		printf("Error: 0 bytes written | attempted to write %d bytes at cur_pos %d\n",count,cur_file->cur_pos);
		return -1;
	}

	//compute the position within read/write buffer that corresponds to current position within file
	int buf_pos = cur_file->cur_pos % BYTES_PER_BLOCK;
	int bytes_written = 0;

	while(1)
	{

		if(buf_pos == BYTES_PER_BLOCK)
		{
			//block index can range between 0 to 3
			
			//edge case
			int prevIndex = (cur_file->cur_pos + bytes_written - 1) / BYTES_PER_BLOCK;
			int block_index = (cur_file->cur_pos + bytes_written) / BYTES_PER_BLOCK;
		//	printf("cur_file cur_pos: %d | bytes_written: %d\n",cur_file->cur_pos,bytes_written);
		//	printf("cur_file block_index: %d\n",block_index);
			if(block_index >= DISK_BLOCKS_COUNT)
			{
				printf("ERROR: eof reached\n");
				return -1;
			}
			
			file_descriptor data_fd = GetFD(cur_file->fd_index);
			if(data_fd.block_numbers[block_index] == FREE) //if next data block is unallocated, find a new data block to assign it to
			{
				//search for a free block
				int free_block_index = -1;
				for(int l = RESERVED_BLOCKS;l < B;++l)
				{
					if(file_system.isBitEnabled(l) != 1)
					{
						free_block_index = l;
						break;
					}	
				}
				
				//if no free blocks are found return with error
				if(free_block_index == -1)
				{
					printf("Error: no more free blocks found to write to..\n");
					return -1;
				}
				else
				{
					//allocate free block
					file_system.enableBit(free_block_index);
					data_fd.block_numbers[block_index] = free_block_index;
					WriteFDToLDisk(cur_file->fd_index,data_fd);

					//update file descriptor contents to include newly allocated block number
					int logical_index = GetBlockNumber(cur_file->fd_index);
					char fd_contents[B];
				        io_system.read_block(logical_index,fd_contents);
					int offset = (cur_file->fd_index % sizeof(int)) * FD_CAPACITY; 
					((int*)fd_contents)[offset + block_index + 1] = free_block_index;
					io_system.write_block(logical_index,fd_contents);

				}
						
			}
			
			//write oft buffer to previous logical block number of ldisk
	//		printf("prev datablock: %d | next datablock: %d\n",data_fd.block_numbers[prevIndex],data_fd.block_numbers[block_index]);
			
	//		printf("prevIndex: %d | block_index: %d\n",prevIndex,block_index);

			io_system.write_block(data_fd.block_numbers[prevIndex],cur_file->buffer);
			//retrieve new data block and have it read into the oft buffer
			io_system.read_block(data_fd.block_numbers[block_index],cur_file->buffer);	
			//reset buf pos to 0 after a new data block has been retrieved or allocated
			buf_pos = 0;
						
		}	
		
		if(bytes_written == count)
		{
			cur_file->cur_pos += bytes_written;
			//update file length in descriptor and oft
			cur_file->file_len += bytes_written;
			char data_fd_contents[B];
			int target_blocknumber = GetBlockNumber(cur_file->fd_index);
			io_system.read_block(target_blocknumber,data_fd_contents);
			((int*)data_fd_contents)[cur_file->fd_index * sizeof(int)] = cur_file->file_len;
			io_system.write_block(target_blocknumber,data_fd_contents);

			//write most recent block index buffer back to ldisk
			int block_index = cur_file->cur_pos / BYTES_PER_BLOCK;
			file_descriptor file_fd = GetFD(cur_file->fd_index);
			if(block_index < DISK_BLOCKS_COUNT && block_index >= 0)
				io_system.write_block(file_fd.block_numbers[block_index],cur_file->buffer);

			return bytes_written;
		}

		//write mem_area into oft buffer
		cur_file->buffer[buf_pos++] = mem_area[bytes_written++];
	}
	
	return -1;
}


//move to the current position of the file to pos, where pos is an integer specifying
//the number of bytes from the beginning of the file. When a file is initially opened,
//the current position is automatically set to zero. After each read or write
//operation, it points to the byte immediately following the one that was accessed last.
static int lseek(int index, int pos)
{
	if(index == 0)
	{
		printf("Error: lseek cannot explicitly seek through directory oft\n");
		return -1;
	}

	//there are only 4 valid oft entries
	//assert(index >= 0 && index < OFT_SIZE);
	if(index < 0 || index >= OFT_SIZE)
	{
		printf("Error: lseek cannot open oft_index: %d\n",index);
		return -1;
	}
	
	open_file_table* cur_file = &oft[index];
	if(cur_file->fd_index == FREE)
	{
		printf("Error: lseek.. oft entry is unused: %d\n",cur_file->fd_index);
		return -1;
	}

	//file can only contain 192 bytes at max
	//assert(pos >= 0 && pos < DIR_BLOCKS * BYTES_PER_BLOCK);
	//note: lseek shouldn't be able to seek past what wasn't written to file
	if(pos < 0 || pos > DIR_BLOCKS * BYTES_PER_BLOCK || pos > cur_file->file_len)
	{
		printf("Error: lseek cannot seek to pos %d\n",pos);
		return -1;
	}

	//printf("lseek to pos: %d at file index: %d\n",pos,index);

	//if new position is not within the current data block:
	// write the buffer into the appropriate block on disk
	// read the new data block from disk into the buffer
	int target_blocknumber = pos / BYTES_PER_BLOCK;
	int current_blocknumber = cur_file->cur_pos / BYTES_PER_BLOCK;
	
	if(target_blocknumber == DISK_BLOCKS_COUNT)
		target_blocknumber = DISK_BLOCKS_COUNT - 1;
	if(current_blocknumber == DISK_BLOCKS_COUNT)
		current_blocknumber = DISK_BLOCKS_COUNT - 1;

	int in_range1 = target_blocknumber >= 0 && target_blocknumber < DISK_BLOCKS_COUNT;
	int in_range2 = current_blocknumber >= 0 && current_blocknumber < DISK_BLOCKS_COUNT;
	
	if(in_range1 == 1 && in_range2 == 1 && target_blocknumber != current_blocknumber)
	{
		file_descriptor file_fd = GetFD(cur_file->fd_index);
		io_system.write_block(file_fd.block_numbers[current_blocknumber],cur_file->buffer);
		io_system.read_block(file_fd.block_numbers[target_blocknumber],cur_file->buffer);
	}

	//Set the current position to new position
	cur_file->cur_pos = pos;
	//Return Status
	return pos;
	//printf("position is %d\n",pos);
}

//displays list of files and their lengths
static void directory()
{
	file_descriptor dir_fd = GetFD(0);
	for(int i = 0;i < DISK_BLOCKS_COUNT; ++i)
	{
		//skip reading block numbers that don't have data
		if(dir_fd.block_numbers[i] == FREE) continue;

		char dirContents[B];
		io_system.read_block(dir_fd.block_numbers[i],dirContents);
		int dir_entry_size = sizeof(int) * DIR_ENTRY_CAPACITY;
		for(int k = 0;k < B;k += dir_entry_size)
		{
			int status = *(int*)(&dirContents[k]);
			//skip free directory entries
			if(status == FREE) continue;
			
			char* sym_name = (char*)(&dirContents[k]);
			int fd_index = *(int*)(&dirContents[k + sizeof(int)]);
			file_descriptor file_fd = GetFD(fd_index);
			
			//use for turning in
			printf("%s %d\n",sym_name,file_fd.file_len);
			//use for debugging
			//printf("filename: %s| fd_index: %d\n",sym_name,fd_index);

		}	
	}
}


void init_bitmap()
{
	//first 7 bits of the bitmap are set to 1 since each bit represents a block
	//0th bit represents bitmap
	//1th to 6th bits represents data blocks reserved for file descriptors
	char bitmap[B] = {0};
	for(int i = 0;i < BITS - 1;++i)
		bitmap[0] = bitmap[0] | (0x01 << i);
	io_system.write_block(0,bitmap);
	
}

//helper function to initialize the directory when ldisk first boots up
void init_dir()
{
	int dirmap[INTS_PER_BLOCK];
	memset((char*)dirmap,FREE,B);
	//fd0, block 1 is reserved as directory file descriptor with file len initialized to 0 assuming its a new disk
	dirmap[0] = 0;
	io_system.write_block(1,(char*)dirmap);

	open_file_table* dir_table = &oft[0];
	dir_table->fd_index = 0;
	dir_table->cur_pos = 0;
	file_descriptor dir_fd = GetFD(dir_table->fd_index);
	dir_table->file_len = dir_fd.file_len;

	int free_space[INTS_PER_BLOCK];
	for(int i = 0;i < INTS_PER_BLOCK;++i)
		free_space[i] = FREE;
	memcpy(dir_table->buffer,free_space,sizeof(int) * INTS_PER_BLOCK);
}

void init_disk()
{
	//easiest way to initialize ldisk assuming no filename was specified is to set all blocks in the disk to be free = -1
	for(int logical_index = 0;logical_index < B; ++logical_index)
	{
		int free = -1;
		int block[INTS_PER_BLOCK];//should be 16
		for(int i = 0;i < INTS_PER_BLOCK; ++i)
		{
			memcpy(&block[i],&free,sizeof(int));
		}

		io_system.write_block(logical_index,(char*)block);			
	}

}

void init_oft()
{
	//directory oft
	oft[0].cur_pos = 0;
	oft[0].file_len = 0;
	oft[0].fd_index = 0;
	memset(oft[0].buffer,0,sizeof(B));

	//a file is free in oft if its fd_index = -1
	//3 other files
	for(int i = 1;i < OFT_SIZE;++i)
	{
		oft[i].cur_pos = -1;
		oft[i].file_len = -1;
		oft[i].fd_index = -1;
	}
}

//restores ldisk from file.txt or create a new one if no file exists
//returns 0 if disk is initialized with no filename specified or file does not exist and will be created..
//returns 1 if file does exist
static int init(char* filename)
{
	init_mask();
	init_oft();

	FILE* ldisk_file = fopen(filename,"r");

	int status = -1;
	//if filename not specified or does not exist
	if(filename == NULL || ldisk_file == NULL)
	{
		status = 0;
	//	puts("disk initialized");
		init_disk();
		init_bitmap();
		init_dir();
		
	}
	else // if filename exists and successfully loads
	{
		status = 1;
	//	printf("disk restored from file: %s\n",filename);
		//load contents of file (ldisk.txt) into ldisk
		for(int l = 0;l < L;++l)
		{
			char data[B];
			fread(data,sizeof(char),B,ldisk_file);
			io_system.write_block(l,data);	
		}

		//restore directory file contents:
			open_file_table* dir_table = &oft[0];
			file_descriptor dir_fd = GetFD(0);
			dir_table->file_len = dir_fd.file_len;
			dir_table->cur_pos = 0;
			dir_table->fd_index = 0;

			//read the first set of dir entries from ldisk to oft buffer
			//if(dir_fd.block_numbers[0] != FREE)
				io_system.read_block(dir_fd.block_numbers[0],dir_table->buffer);
			
		fclose(ldisk_file);

	}

	return status;
}

//closes ldisk and saves all files opened (including directory file) to some file.txt 
static int save(char* filename)
{

	//save all contents stored in ldisk to a file
	FILE* ldisk_file = fopen(filename,"w");
	if(ldisk_file == NULL)
	{
	//	printf("Error: %s could not be saved\n",filename);
		return -1;
	}
	
	//write back all information that might have been modified in oft
	//since some files could be not explicitly closed using file_system.close()
	for(int i = 0;i < OFT_SIZE;++i)
	{
		if(oft[i].fd_index == FREE) continue;
		file_descriptor file_fd = GetFD(oft[i].fd_index);
		int target_block_index = oft[i].cur_pos / BYTES_PER_BLOCK;
		if(file_fd.block_numbers[target_block_index] != FREE)
			io_system.write_block(file_fd.block_numbers[target_block_index],oft[i].buffer);
	}

	//close oft
	for(int i = 0;i < OFT_SIZE;++i)
	{
		oft[i].fd_index = -1;
		oft[i].cur_pos = -1;
	}

	//save all blocks on ldisk to txt f
	//printf("saving:\n");
	for(int l = 0;l < L;++l)
	{
		char data[B];
		io_system.read_block(l,data);

	//	if(l == 0)
	//	{
	//		printf("saving:\n");
	//		print_bitmap();
	//	}

	//	if(l == 7)
	//	{
	//		int numdata[INTS_PER_BLOCK];
	//		io_system.read_block(l,(char*)numdata);
	//		for(int k = 0;k < INTS_PER_BLOCK;++k)
	//		{
	//			printf("%d ",numdata[k]);
	//		}
	//		printf("\n");
	//	}

		fwrite(data,sizeof(char),B,ldisk_file);	
	}

	fclose(ldisk_file);
	
	return 0;
}


//returns 0 if block is free, otherwise it returns 1 if block is taken
static int isBitEnabled(int logical_index)
{
	assert(logical_index >= 0 && logical_index < L);

	char bitmap[B] = {0};
	io_system.read_block(0,bitmap);

	//since each byte is 8 bits and ldisk is a 2d array of characters... the first 8 character bytes in block 0 of the array refer to the disk's bitmap
	//we have take the modulus here to ensure it grabs the correct bit of the correct index
	
	int bucket_index = logical_index / BITS; // describes which byte in the bitmap array we are referencing	
	int bit_index = logical_index % BITS; // describes which bit in the bitmap array indexed by bucket_index  we are referencing
	int bitEnabled = (bitmap[bucket_index] >> bit_index) & 1;
	//printf("is block [%d] taken ? %d\n",logical_index,bitEnabled);	

	return bitEnabled;
}


static void enableBit(int logical_index)
{
	assert(logical_index >= 0 && logical_index < L);
	
	char bitmap[B];
	io_system.read_block(0,bitmap);

	int bucket_index = logical_index / BITS;
	int bit_index = logical_index % BITS;

	//reminder: the bit mask is initialized from  msb to lsb order
	bitmap[bucket_index] = bitmap[bucket_index] | MASK[BITS - 1 - bit_index];
	io_system.write_block(0,bitmap);	
}

static void disableBit(int logical_index)
{
	assert(logical_index >= 0 && logical_index < L);

	char bitmap[B];
	io_system.read_block(0,bitmap);

	int bucket_index = logical_index / BITS;
	int bit_index = logical_index % BITS;
	bitmap[bucket_index] = bitmap[bucket_index] & (~MASK[BITS - 1 - bit_index]);
	io_system.write_block(0,bitmap);
}                        

filespace_struct const file_system = 
{
	create,
	destroy,
	open,
	close,
	read,
	write,
	lseek,
	directory,
	init,
	save,
	isBitEnabled,
	enableBit,
	disableBit
};
