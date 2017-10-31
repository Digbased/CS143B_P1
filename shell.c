#include <stdio.h>
#include <string.h>
#include "code/file_system.h"

#define LINE_BUF 250
#define CMD_LEN 2

//create
void cr(char* sym_name);
//destroy
void de(char* sym_name);
//open
void op(char* sym_name);
//close
void cl(int oft_index);
//read open file
void rd(int oft_index,int bytes_read);
//write open file
void wr(int oft_index,char b,int bytes_written);
//lseek file
void sk(int oft_index,int pos);
//display directory
void dr();
//initialize disk
void in(char* disk_filename);
//save disk to actual file
int sv(char* disk_filename);

//e.g. ./shell input-file.txt output-file.txt
int main(int argc,char* argv[])
{
	char input[LINE_BUF];
	char cmd[CMD_LEN];

	FILE* the_input = NULL;
	//if input file is specified.. load it in
	if(argc == 1)
	{
		the_input = stdin;
	}
	else if(argc == 2)
	{
		char* input_cmds = argv[1];
		the_input = fopen(input_cmds,"r");
		if(the_input == NULL)
		{
			printf("Error: %s not found\n",input_cmds);
			return -1;
		}
	}
	else if(argc == 3)
	{
		char* input_cmds = argv[1];
		the_input = fopen(input_cmds,"r");
		if(the_input == NULL)
		{
			printf("Error: %s not found\n",input_cmds);
			return -1;
		}
		
		char* output = argv[2];
		freopen(output,"w",stdout);
	}
	else
	{
		printf("Error: Too many arguments passed (%d)\n",argc);
		return -2;
	}

	//pre initialize disk when application first starts
	file_system.init(NULL);
	
	if(the_input == stdin) printf("> ");
	char* status = fgets(input,LINE_BUF,the_input);
	while(status != NULL)//when eof is reached
	{

		//replace newline character with null term
		char* ptr;
		if((ptr=strchr(input,'\n')) != NULL)
			*ptr = '\0';

		int format_status = sscanf(input,"%2s",cmd);

		//debug prints
		if(strcmp("pr",cmd) == 0)
		{
			printf("bitmap: ");
			print_bitmap();
			printf("blocks: \n");
			print_blocks();
			printf("ofts: \n");
			print_ofts();
			printf("fds: \n");
			print_fds();
		}
		//can a file have more than 4 characters?
		else if(strcmp(cmd,"cr") == 0)
		{
			char name[sizeof(int)];
			format_status = sscanf(input+2,"%4s",name);
			if(format_status != 1)
				printf("Error cr: unexpected input\n");
			else
				//printf("cr\n");
				cr(name);
		}
		else if(strcmp(cmd,"de") == 0)
		{
			char name[sizeof(int)];
			format_status = sscanf(input,"%2s %s",cmd,name);
			if(format_status != 2)
				printf("Error de: unexpected input\n");
			else
				//printf("de\n");
				de(name);
		}
		else if(strcmp(cmd,"op") == 0)
		{
			char name[sizeof(int)];
			format_status = sscanf(input,"%2s %s",cmd,name);
			if(format_status != 2)
				printf("Error op: unexpected input\n");
			else
				//printf("op\n");
				op(name);
		}
		else if(strcmp(cmd,"cl") == 0)
		{
			int oft_index;
			format_status = sscanf(input,"%2s %d",cmd,&oft_index);
			if(format_status != 2)
				printf("Error cl: unexpected input\n");
			else
				//printf("cl\n");
				cl(oft_index);
		}
		else if(strcmp(cmd,"rd") == 0)
		{
			int oft_index, count;
			format_status = sscanf(input,"%2s %d %d",cmd,&oft_index,&count);
			if(format_status != 3)
				printf("Error rd: unexpected input\n");
			else
				//printf("rd\n");
				rd(oft_index,count);
		}
		else if(strcmp(cmd,"wr") == 0)
		{
			int oft_index, count;
			char b;
			format_status = sscanf(input,"%2s %d %c %d",cmd,&oft_index,&b,&count);
			if(format_status != 4)
				printf("Error wr: unexpected input\n");
			else
				//printf("wr\n");
				wr(oft_index,b,count);
		}
		else if(strcmp(cmd,"sk") == 0)
		{
			int oft_index, pos;
			format_status = sscanf(input,"%2s %d %d",cmd,&oft_index,&pos);
			if(format_status != 3)
				printf("Error sk: unexpected input\n");
			else
				//printf("sk\n");
				sk(oft_index,pos);
		}
		else if(strcmp(cmd,"dr") == 0)
		{
			if(format_status != 1)
				printf("Error dr: unexpected input\n");
			else
				dr();
		}
		else if(strcmp(cmd,"in") == 0)
		{
			char disk_filename[LINE_BUF];
			format_status = sscanf(input,"%2s %s",cmd,disk_filename);
			if(format_status != 2)
			{
				format_status = sscanf(input,"%2s",cmd);
				if(format_status == 1)
					in(NULL);
				else
					printf("Error in: unexpected input\n");
			}
			else
			{
				//printf("disk restored\n");
				in(disk_filename);
			}
		}
		else if(strcmp(cmd,"sv") == 0)
		{
			char disk_filename[LINE_BUF];
			format_status = sscanf(input,"%2s %s",cmd,disk_filename);
			if(format_status != 2)
			{
				printf("Error: sv cmd requires you to save to file param\n");
			}
			else
			{
				int save_status = sv(disk_filename);
				if(save_status == -1)
					printf("Error: %s could not be saved\n",disk_filename);
				else
					printf("disk saved\n");
			}
		}
		else
		{
			//check for newline character,carriage return, and linefeed
			int valid_characters = (strcmp(input,"\n") == 0) || (strcmp(input,"\r") == 0) || (strcmp(input,"\r\n") == 0);
		//	int strlen_input = strlen(input);
		//	printf("strlen_input: %d\n",strlen_input);
			if(strlen(input) != 0 && valid_characters != 1)
				printf("Error: cmd not found: %s\n",input);
		}

		//reset cmd string
		cmd[0] = '\0';
	
		if(the_input == stdin) printf("> ");
		status = fgets(input,LINE_BUF,the_input);

	}

	if(the_input != stdin)
		fclose(the_input);

	return 0;
}

//TODO: handle error messages here

//create
void cr(char* sym_name)
{
	file_system.create(sym_name);
}
//destroy
void de(char* sym_name)
{
	int status = file_system.destroy(sym_name);
	if(status == 1)
		printf("%s destroyed\n",sym_name);
}
//open
void op(char* sym_name)
{
	int oft_index = file_system.open(sym_name);
	if(oft_index != -1)
		printf("%s opened %d\n",sym_name,oft_index);
}
//close
void cl(int oft_index)
{
	int closed_oft_index = file_system.close(oft_index);
	if(closed_oft_index != -1)
		printf("%d closed\n",closed_oft_index);
}
//read open file ~ print read memory here
void rd(int oft_index,int bytes_read)
{
	char mem_area[bytes_read];
	file_system.read(oft_index,mem_area,bytes_read);
	printf("%s\n",mem_area);
}
//write open file ~ char b is the character to write to oft buffer
void wr(int oft_index,char b,int bytes_written)
{
	char mem_area[bytes_written];
	memset(mem_area,b,bytes_written);
	file_system.write(oft_index,mem_area,bytes_written);
	printf("%d bytes written\n",bytes_written);
}
//lseek file
void sk(int oft_index,int pos)
{
	file_system.lseek(oft_index,pos);
	printf("position is %d\n",pos);
}
//display directory
void dr()
{
	file_system.directory();
}
//initialize disk
void in(char* disk_filename)
{
	int status = file_system.init(disk_filename);
	switch(status)
	{
		case 0:
			printf("disk initialized\n");
			break;
		case 1:
			printf("disk restored\n");
			//printf("disk restored from file: %s\n",disk_filename);
			break;
	}

}
//save disk to actual file and close disk afterwards
int sv(char* disk_filename)
{
	int status = file_system.save(disk_filename);
	return status;
}

