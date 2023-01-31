/*

AUTHOR DETAILS :: -->
Declaration of authorship
Author Name:: Kamal Choudhury
Organisation for which project is being made :: Indian Institute of Technology Patna (IIT PATNA)
Roll No. (In IIT PATNA) :: 1801CS27
Subject :: CS321 & CS322 (Computer Architecture --> LAB & THEORY)
Submitted ON :: 20/11/2020
This C file emu.c is part of the assignment of CS321 at the department of Computer Science and Engg, IIT Patna .

FILE DETAILS :: -->
This is the implementation of emulator
The emulator takes a binary file (machine code) with extension .o & gives following functionalities
1) Traces the program 
2) Produces memory dump after execution
3) Produces memory dump before the execution
4) Gives ISA for the assembly 

*/

/* ALL the Header files used are listed below */
#include <stdio.h>  
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>  
#include <stdbool.h>

/******************************************* Global Variables & structures are defined below ***************************************************************/
/* Instruction Table */
char instruction_set_table[][8] = {"ldc","adc","ldl","stl","ldnl","stnl","add","sub","shl","shr","adj","a2sp","sp2a","call","return","brz","brlz","br","HALT","data","SET"};
char emulator_functions[][8] = {"-trace","-before","-after","-isa"};
int instruction_type[] =  {1,1,1,1,1,1,0,0,0,0,1,0,0,1,0,1,1,1,0,1,1};
int instruction_type2[] = {1,1,1,1,1,1,0,0,0,0,1,0,0,2,0,2,2,2,0,1,1};/* tells the type of instruction whther branching or single operand or 0 operand*/
int number_of_instr = 0;/* Number of lines in the program */
int program_counter_lines = 0; /* Number of instructions for the addresses */
int memory[5000000]; /* This is the total memory architecture */
int A_register=0, B_register=0, program_counter=0, stack_pointer=2000; /* These are the four main parts of the architecture & assumed to be initialised by 0 */
/* Stack pointer is initialised with 2000 (in decimal) */ 
int binary[32]; /* Temporary array to store 32-bits of binary form a number */

/************************************************* Various function prototypes are below *****************************************************************/
void file_read_store(char *str);/* Reading from the asm file & storing it in struct data structure */
int CHAR_TO_INDEX(char c);/* Converting alphanumeric character to a number */
void bit24(int arr[],int ); /* Converting an integer into 24 bit (6-digit in hexadecimal) */
void bit32(int arr[],int ); /* Converting an integer into 32 bit (8-digit in hexadecimal) */
char hexadecimal_unit(int);	/* Converting 0-15 number to hexadecimal value */
int opcode_fetch(int); /* Fetches the opcode given the instruction */
int operand_fetch(int); /* Fetches the opcode given the instruction */
void trace_instruction(int); /* Tracing the instructions line by line & showing contents of PC,SP,A & B */ 
void show_memory(); /* Show the memory contents */
void print_hexadecimal(int); /* print the hexadecimal form of a number in 8-digit*/
void print_hexadecimal_24bit(int); /* print the hexadecimal form of a number in 6-digit*/


/****************************************************** MAIN FUNCTION STARTS *********************************************************************************/

/*Taking command line arguments*/
int main(int argc,char **argv)
{
	int i,j,emu_func,length_file;
	emu_func = -1;
	for(i=0;i<4;i++)
	{
		if(strcmp(emulator_functions[i],argv[1])==0)
		{
			emu_func = i+1;
		}
	}
	if(argc!=3)
		emu_func = -1;
	if(emu_func==(-1))
	{
		/* If the command is not properly given then showing the commands */
		printf("You can use this emulator to carry out following functionalities :: \n");
		printf("The Below functionalities are described by their commands & description in bracket \n");
		printf("1) -trace (Show the contents of stack,PC,A & B after each instruction executed.) \n");
		printf("2) -before (Shows Memory Dump before the execution of instructions.) \n");
		printf("3) -after (Shows Memory Dump after the execution of instructions.) \n");
		printf("To use any of the above functionalties, in the command line write --> ./emu -command filename.o \n");
		printf("For example if you want to trace the subtract.o file write on the command line --> ./emu -trace subtract.o \n");
		return 0;
	}	
	file_read_store(argv[2]);	 /* Reading the file & storing the data in memory array */
	if(emu_func==2)
	{
		show_memory();	 		 /* Showing memory before execution */
		return 0;
	}
	
	trace_instruction(emu_func); /* Execute the obejct file line by line */
	if(emu_func==3)
		show_memory();			 /* If after option is chosen then display the  */
	return 0;
}

/*********************************************** READING THE OBJECT FILE & STORING IT IN MEMORY ***************************************************************/
void file_read_store(char *filename)
{
	/*Making the file pointer*/
	FILE *fp;
	int curr_line,temp;
	/*Declaring the variables*/
	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
    	printf("Could not open file %s",filename);
    	return ;
	}
	/*Iterating every line in the file*/
	curr_line = 0;
	while (!feof(fp))
	{
		fread(&temp, sizeof(int), 1, fp);
		memory[curr_line] = temp; /* Storing the data or instruction in the before memory */ 
		curr_line++;
	}
	number_of_instr = curr_line;
	/*Closing the file*/
	fclose(fp);
}
/***************************************************** END OF FUNCTION FOR READING OBJECT FILE & STORING IT ****************************************************/

/********************************************************** FUNCTIONS FOR EXECUTION & MEMORY DUMP **************************************************************/
void trace_instruction(int emu_func)
{
	int curr_opcode,curr_operand,number_of_executions;/* Storing current opcode & current operand for each line */
	bool halt_program;
	program_counter = -1; /* Initialising program counter by 0 */
	number_of_executions = 0;
	while(1)
	{
		program_counter++;
		curr_opcode = opcode_fetch(memory[program_counter]);
		curr_operand = operand_fetch(memory[program_counter]);
		halt_program = false;
		/* Switch statements for the opcode */
		switch(curr_opcode)
		{
			case 0:
				B_register = A_register;
				A_register = curr_operand;
	        	break;
	        case 1:
	        	A_register += curr_operand;
	            break;
	        case 2:
	        	B_register = A_register;
	        	A_register = memory[stack_pointer + curr_operand];
	            break;
	        case 3:
	        	memory[stack_pointer + curr_operand]=A_register;
	            A_register = B_register;
	            break;
	        case 4:
	        	A_register = memory[A_register + curr_operand];
	            break;
	        case 5:
	        	memory[A_register + curr_operand] = B_register;
	            break;
	        case 6:
	        	A_register = B_register + A_register;
	            break;
	        case 7:
	        	A_register = B_register - A_register;
	            break;
	        case 8:
	        	A_register = B_register << A_register;
	            break;
	        case 9:
	        	A_register = B_register >> A_register;
	            break;
	        case 10:
	        	stack_pointer += curr_operand;
	            break;
	        case 11:
	        	stack_pointer = A_register;
	        	A_register = B_register;
	            break;
	        case 12:
	        	B_register = A_register;
	        	A_register = stack_pointer;
	        	break;
	        case 13:
	        	B_register = A_register;
	        	A_register = program_counter;
	        	program_counter += curr_operand;
	        	break;
	        case 14:
	        	program_counter = A_register;
	        	A_register = B_register;
	            break;
	        case 15:
	        	if(A_register==0)
	        		program_counter += curr_operand;
	            break;
	        case 16:
	        	if(A_register<0)
	        		program_counter += curr_operand;
	            break;
	        case 17:
	        	program_counter += curr_operand;
	            break;
	        case 18:
	        	halt_program = true;
	            break;
			case 19:
				break;
			case 20:
				break;					
			default:
			    halt_program = true; 
				printf("No such instruction found \nNow the program is ending!!!!!!\n");	
				return ;			
		}
		if(emu_func==1)
		{
			/* Print the contents only if trace command is given */ 
			if(curr_opcode<21)
			{
				printf("The instruction is %s ",instruction_set_table[curr_opcode]);
				if(instruction_type[curr_opcode]!=0)
				{
					printf("0x");
					print_hexadecimal_24bit(curr_operand);
				}
			}
			printf("\nNow the contents of the architecure are (after instruction executed) :: \n");
			/* Printing the cotents of the architecture */
			printf("PC = ");	
			print_hexadecimal(program_counter+1);	
			printf(" SP = ");	
			print_hexadecimal(stack_pointer);
			printf(" A = ");	
			print_hexadecimal(A_register);
			printf(" B = ");	
			print_hexadecimal(B_register);
			printf("\n");
		}
		/* Halt the execution if HALT encountered or corrupted data is given */
		if(halt_program)
			break;
		number_of_executions++;
	}
	if(emu_func==1)
		printf("The number of executed instructions = %d\n",number_of_executions);
}
/* Function to show the memory dump :: */
void show_memory()
{
	int num_of_addr,i,j;
	printf("The memory dump will have starting address on the leftmost side & then contents of next 16 addresses in one single line\n");
	printf("The memory contents are as below (address on left & data on right) ::\n");
	num_of_addr = number_of_instr/16;
	num_of_addr += 10;
	
	/* The memory dump will have address on the leftmost side & then contents of next 16 addresses in one single line  */
	for(i=0;i<num_of_addr;i++)
	{
		/* Shwing the starting address */
		printf("Memory contents starting from ("); 
		print_hexadecimal(16*i);
		printf(") :: ");
		/* Now showing the memory content at that place */
		for(j=0;j<16;j++)
		{
			print_hexadecimal(memory[16*i+j]);
			printf(" ");
		}
		printf("\n");	
	} 
}
/********************************************************** END OF FUNCTIONS FOR EXECUTION & MEMORY DUMP *******************************************************/


/********************************************************** FUNCTIONS FOR BINARY & HEXADECIMAL CONVERSION *****************************************************/

/* Function to fetch opcode from an instruction if given in a numerical format */
int opcode_fetch(int instruction)
{
	/* opcode will be returned, i is the iterator & pow2 stores powers of 2 */
	int opcode,i,pow2;
	/* If instruction is positive directly take last 2 digits */	
	if(instruction>=0)
		opcode = instruction%256;		
	else
	{
		/* If the instruction is in negative format then convert the number to 2's complement & then extract last 2 digits (In hexadecimal) */
		instruction = -instruction;
		opcode = 0;
		pow2 = 1;
		/* Extracting last 2 digits */
		for(i=0;i<8;i++)
		{
			opcode += pow2*(1-instruction%2);
			instruction/=2;
			pow2*=2;
		}
		opcode++;
		opcode = opcode%256;
	}
	return opcode;
}	

/* Function to fetch operand's offset/value from an instruction if given in a numerical format */
int operand_fetch(int instruction)
{
	/* operand will be returned, i is the iterator & pow2 stores powers of 2 */
	int operand,i,pow2;
	/* If instruction is positive or take first 6 digits (In hexadecimal) by dividing by 256 */	
	if(instruction>=0)
		operand = instruction/256;
	else
	{
		/* If negative then further cases two cases will arise */
		if(instruction%256==0)
			operand = instruction/256;
		else
			operand = instruction/256-1;		
	}			
	return operand;
}

/* Function to store number in a 6 size array (6-digit in hexadecimal) */
void bit24(int arr[],int num)
{
	int i,bit24_max=1;
	int binary[24];
	for(i=0;i<24;i++)
	{
    	binary[i] = 0;
    	bit24_max*=2;
	}
	if(num>0)
	{
    	for(i=0;i<24;i++)
    	{
        	binary[i] = num%2;
        	num = num/2;
    	}
    	binary[23] = 0;
	}
	else
	{
    	num = bit24_max + num;
    	for(i=0;i<24;i++)
    	{
        	binary[i] = num%2;
        	num = num/2;
    	}
	}
	for(i=0;i<24;i+=4)
    	arr[i/4] = binary[i] + binary[i+1]*2 + binary[i+2]*4 + binary[i+3]*8;
}
/* Function to print decimal number in a 8-digit hexadecimal */
void print_hexadecimal(int num)
{
	int i;
	int bit32_max;
	int arr[8];
	for(i=0;i<32;i++)
	{
    	binary[i] = 0;
	}
	if(num>=0)
	{
	    for(i=0;i<32;i++)
	    {
	        	binary[i] = num%2;
	        	num = num/2;
	    }
	    binary[31] = 0;
	}
	else
	{
		num = -num;
		for(i=0;i<32;i++)
	    {
	        	binary[i] = num%2;
	        	num = num/2;
	    }
    	for(i=0;i<32;i++)
    	{
    		binary[i] = 1 - binary[i];
		}
		binary[0] ++;
		for(i=0;i<31;i++)
		{
			if(binary[i]==2)
			{
				binary[i+1]++;
				binary[i] = 0;
			}
		}
	}
	for(i=0;i<32;i+=4)
    	arr[i/4] = binary[i] + binary[i+1]*2 + binary[i+2]*4 + binary[i+3]*8;
    for(i=7;i>=0;i--)	
    	printf("%c",hexadecimal_unit(arr[i]));
}
/* Function to print hexadecimal but for 24-bit numbers */
void print_hexadecimal_24bit(int num)
{
	int i;
	int bit24_max;
	int arr[8];
	for(i=0;i<24;i++)
	{
    	binary[i] = 0;
	}
	if(num>=0)
	{
	    for(i=0;i<24;i++)
	    {
	        	binary[i] = num%2;
	        	num = num/2;
	    }
	    binary[23] = 0;
	}
	else
	{
		num = -num;
		for(i=0;i<24;i++)
	    {
	        	binary[i] = num%2;
	        	num = num/2;
	    }
    	for(i=0;i<24;i++)
    	{
    		binary[i] = 1 - binary[i];
		}
		binary[0] ++;
		for(i=0;i<23;i++)
		{
			if(binary[i]==2)
			{
				binary[i+1]++;
				binary[i] = 0;
			}
		}
	}
	for(i=0;i<24;i+=4)
    	arr[i/4] = binary[i] + binary[i+1]*2 + binary[i+2]*4 + binary[i+3]*8;
    for(i=5;i>=0;i--)	
    	printf("%c",hexadecimal_unit(arr[i]));
}
/* To display number between 0 to 15 in hexadecimal */
char hexadecimal_unit(int num)
{
	char a;
	if(num<=9)
    	a = '0' + num;
	else
    	a = 'A' + (num-10);
	return a;
}
/***************************************************** END OF FUNCTIONS FOR BINARY & HEXADECIMAL CONVERSION ****************************************************/
