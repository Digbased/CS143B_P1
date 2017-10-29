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
void sv(char* disk_filename);

int main(int argc,char* argv[])
{
	char input[LINE_BUF];
	char cmd[CMD_LEN];

	printf("> ");
	char* status = fgets(input,LINE_BUF,stdin);
	while(status != NULL)//when eof is reached
	{
		//do shell stuff in here!

		//replace newline character with null term
		char* ptr;
		if((ptr=strchr(input,'\n')) != NULL)
			*ptr = '\0';

		int format_status = sscanf(input,"%2s",cmd);

		//can a file have more than 4 characters?
		if(strcmp(cmd,"cr") == 0)
		{
			char name[sizeof(int)];
			format_status = sscanf(input+2,"%4s",name);
			if(format_status != 1)
				printf("Error cr: unexpected input\n");
			else
				printf("cr\n");//cr(name);
		}
		else if(strcmp(cmd,"de") == 0)
		{
			char name[sizeof(int)];
			format_status = sscanf(input,"%2s %s",cmd,name);
			if(format_status != 2)
				printf("Error de: unexpected input\n");
			else
				printf("de\n");//de(name);
		}
		else if(strcmp(cmd,"op") == 0)
		{
			char name[sizeof(int)];
			format_status = sscanf(input,"%2s %s",cmd,name);
			if(format_status != 2)
				printf("Error op: unexpected input\n");
			else
				printf("op\n");//op(name);
		}
		else if(strcmp(cmd,"cl") == 0)
		{
			int oft_index;
			format_status = sscanf(input,"%2s %d",cmd,&oft_index);
			if(format_status != 2)
				printf("Error cl: unexpected input\n");
			else
				printf("cl\n");//cl(oft_index);
		}
		else if(strcmp(cmd,"rd") == 0)
		{
			int oft_index, count;
			format_status = sscanf(input,"%2s %d %d",cmd,&oft_index,&count);
			if(format_status != 3)
				printf("Error rd: unexpected input\n");
			else
				printf("rd\n");//rd(oft_index,count);
		}
		else if(strcmp(cmd,"wr") == 0)
		{
			int oft_index, count;
			char b;
			format_status = sscanf(input,"%2s %d %c %d",cmd,&oft_index,&b,&count);
			if(format_status != 4)
				printf("Error wr: unexpected input\n");
			else
				printf("wr\n");//wr(oft_index,b,count);
		}
		else if(strcmp(cmd,"sk") == 0)
		{
			int oft_index, pos;
			format_status = sscanf(input,"%2s %d %d",cmd,&oft_index,&pos);
			if(format_status != 3)
				printf("Error sk: unexpected input\n");
			else
				printf("sk\n");//sk(oft_index,pos);
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
				{
					printf("disk initialized!!\n");
					//in(NULL);
				}
				else
					printf("Error in: unexpected input\n");
			}
			else
			{
				printf("disk restored\n");
				//in(disk_filename);
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
				printf("disk saved\n");
				//sv(disk_filename);
			}
		}
		else
		{
			if(strlen(input) != 0)
				printf("Error: cmd not found: %s\n",cmd);
		}

		//reset cmd string
		cmd[0] = '\0';
		printf("> ");
		status = fgets(input,LINE_BUF,stdin);
	
	}
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
	file_system.destroy(sym_name);
}
//open
void op(char* sym_name)
{
	file_system.open(sym_name);
}
//close
void cl(int oft_index)
{
	file_system.close(oft_index);
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
	file_system.init(disk_filename);
}
//save disk to actual file and close disk afterwards
void sv(char* disk_filename)
{
	file_system.save(disk_filename);
}

