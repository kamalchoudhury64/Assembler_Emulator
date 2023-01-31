/*
AUTHOR DETAILS :: -->
Declaration of authorship
Author Name:: Kamal Choudhury
Organisation for which project is being made :: Indian Institute of Technology Patna (IIT PATNA)
Roll No. (In IIT PATNA) :: 1801CS27
Subject :: CS321 & CS322 (Computer Architecture --> LAB & THEORY)
Submitted ON :: 20/11/2020
This , Claims_File.txt, is part of the assignment of CS321 at the department of Computer Science and 
Engg, IIT Patna .

FILE DETAILS :: -->
This is the implementation of two-pass-assembler

*/

/* ALL the Header files used are listed below */
#include <stdio.h>  
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>  
#include <stdbool.h>

#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])
/* Alphanumeric size (# of symbols) */
#define ALPHANUMERIC_SIZE (63)

/******************************************* Global Variables & structures are defined below ***************************************************************/
/* Instruction Table */
char instructionset_1_table[][8] = {"ldc","adc","ldl","stl","ldnl","stnl","add","sub","shl","shr","adj","a2sp","sp2a","call","return","brz","brlz","br","HALT","data","SET"};
int instruction_type[] =  {1,1,1,1,1,1,0,0,0,0,1,0,0,1,0,1,1,1,0,1,1};
int instruction_type2[] = {1,1,1,1,1,1,0,0,0,0,1,0,0,2,0,2,2,2,0,1,1};/* tells the type of instruction whther branching or single operand or 0 operand*/
int number_of_lines = 0;/* Number of lines in the program */
int program_counter_lines = 0; /* Number of instructions for the addresses */
struct Opcode_Trie
{
	/* It is a node of the tree
	This structure contains three components namely the next character, whether that node is the end of the instruction & if the node is the end of instruction store the opcode*/
	struct Opcode_Trie *next_character[ALPHANUMERIC_SIZE];
	bool opcode_complete;
	int j;
};
struct asm_line
{
	/* A structure to store the data from the file line by line*/
	char *before_colon; /* Label part */
	char *mid_part;    	/* Instruction & operator part */
	char *comments;    	/* Comments part */
	char *operand;    	/* Operand of the line */
	int opcode;        	/* Opcode of the instruction */
	int program_counter;/* Program counter for that line */
	int is_colon;    	/* If there is a label in the line or not */
	int label_used;    	/* Storing if the label has been used in the program or not */
	int operand_numeric;/* If the operand is numeric storing its numerical value */
	bool is_operand_numeric; /* If the operand is label or numeric */
	int machine_code_hexadecimal[8];/* Storing the hexadecimal value of machine */
};
struct Opcode_Trie *opcode_root ; /* The TRIE DATA STRUCTURE root for opcodes */
struct Opcode_Trie *label_root ;  /* The TRIE DATA STRUCTURE root for labels */
struct asm_line asm_data[1000];   /* Structure of each line */
char ** errors; /* This will store an array of strings which contains errors on different lines */
char ** errors2;/* Contains 2nd types of errors */
int *is_error; /* It stores whether the line contains error or or warning or nothing */
int *is_error2; /* In case any line has 2 errors this will be used */


/************************************************* Various function prototypes are below *****************************************************************/
struct Opcode_Trie *getNode() ; /* making new space for TRIE data structure */
int search(struct Opcode_Trie *, char *); /* Searching in the TRIE data structure */
void initialize_opcodes();/* Making efficient data structure for opcodes */
void file_read_store(char *str);/* Reading from the asm file & storing it in struct data structure */
int CHAR_TO_INDEX(char c);/* Converting alphanumeric character to a number */
void check_errors(char*);/* Checking errors & producing log file */
int number_of_words(char *);/* Finding number of words in a string */       	 
char* find_nth_word(char *, int);/* Finding nth word in a string */
int string_to_num(char*,bool*);	/* Converting a string to integer & if it is not an integer then making a variable passed by referance as false */
void insert(struct Opcode_Trie *, char *,int ); /* Inserting string in a TRIE data structure */
void listing_file();/* Producing listing file & object file */
void bit24(int arr[],int ); /* Converting an integer into 24 bit (6-digit in hexadecimal) */
void bit32(int arr[],int ); /* Converting an integer into 32 bit (8-digit in hexadecimal) */
char hexadecimal_unit(int);	/* Converting 0-15 number to hexadecimal value */



/****************************************************** MAIN FUNCTION STARTS *********************************************************************************/


/*Taking command line arguments*/
int main(int argc,char **argv)
{
	int f = strlen(argv[1]);
	initialize_opcodes(); /* Storing the operands in a TRIE data structure*/
	(void)argc;    
	file_read_store(argv[1]);	/* Reading the file & storing the data in a mixture of TRIE DATA STRUCTURE & STRUCT */
	check_errors(argv[1]);    	/* Making one pass one the data structure stored & reporting errors in the log file */
	listing_file(argv[1]);    	/* Making the listing file & object file which is done in the second pass */
	argv[1][f-4] = '\0';
	printf("Now 3 files have been created :: \n");
	printf("1) %s.log :: (Telling all the errors) \n",argv[1]);
	printf("2) %s.lst :: (Listing file) \n",argv[1]);
	printf("3) %s.o :: (Object file) \n",argv[1]);
	return 0;
}
/*************************************** READING THE FILE ONCE & STORING IT IN A MIXTURE OF STRUCT & TRIE DATA STRUCTURE ***********************************/
/*The labels are stored in a trie data structure while lines are broken into 3 parts & stored in a struct*/
/*The three parts are that */
void file_read_store(char *filename)
{
	/*Making the file pointer*/
	FILE *fp;
	char str[1000];
	int len,i,colon_index,semi_colon_index,temp;
	/*Declaring the variables*/
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
    	printf("Could not open file %s",filename);
    	return ;
	}
	/*Iterating every line in the file*/
	while (fgets(str, 1000, fp) != NULL)
	{
    	/*Finding length of each line*/
    	len = strlen(str);
   	 
    	/*Decreasing len by 1 to avoid newline*/
    	len--;
    	/*Initialising the separators (Indexes of Colon && Semi-Colon) */
    	colon_index = -1;
    	semi_colon_index = len;
    	/*Finding the indexes at which first colon & semi-colon are found*/
    	asm_data[number_of_lines].is_colon = 0;
    	for(i=0;i<len;i++)
    	{
        	if(str[i]==';')
        	{
            	semi_colon_index = i;
            	break;
        	}
    	}
    	for(i=0;i<semi_colon_index;i++)
    	{
        	if(str[i]==':')
        	{
            	colon_index = i;
            	asm_data[number_of_lines].is_colon = 1;
        	}
    	}
    	temp = colon_index;
    	if(temp<0)
        	temp++;
    	/*Now I am storing the part before colon & between colon & semi-colon in my struct & by using malloc to do that*/
    	asm_data[number_of_lines].before_colon = (char *)malloc((sizeof(char))*(temp+4));
    	asm_data[number_of_lines].mid_part = (char *)malloc((sizeof(char))*(semi_colon_index-colon_index+4));
    	asm_data[number_of_lines].comments = (char *)malloc((sizeof(char))*(len-semi_colon_index+4));
   	 
    	/*Storing label name if any*/
    	for(i=0;i<colon_index;i++)
        	asm_data[number_of_lines].before_colon[i] = str[i];
    	temp = i;
    	if(temp<0)    
        	temp = 0;    
    	/*Adding Null character to make it a string*/    
    	asm_data[number_of_lines].before_colon[temp] = '\0';
   	 
    	/*Storing instructions if any*/
    	for(i=colon_index+1;i<semi_colon_index;i++)
        	asm_data[number_of_lines].mid_part[i-colon_index-1] = str[i];    
    	/*Adding Null character to make it a string*/
    	asm_data[number_of_lines].mid_part[i-colon_index-1] = '\0';
   	 
    	/*Storing comments if any*/
    	for(i=semi_colon_index;i<len;i++)
        	asm_data[number_of_lines].comments[i-semi_colon_index] = str[i];
    	asm_data[number_of_lines].comments[len-semi_colon_index] = '\0';
   	 
    	/*printf("Line number is :: %d\n",number_of_lines);
    	printf("%d %d %d\n",colon_index,semi_colon_index,len);
    	printf("%ld %d %s\n",strlen(asm_data[number_of_lines].before_colon),asm_data[number_of_lines].is_colon,asm_data[number_of_lines].before_colon);
    	printf("%ld %s\n",strlen(asm_data[number_of_lines].mid_part),asm_data[number_of_lines].mid_part);
    	printf("%ld %s\n",strlen(asm_data[number_of_lines].comments),asm_data[number_of_lines].comments);*/
      	 
    	/*Incrementing the number of lines*/
    	number_of_lines++;
	}
	/*Closing the file*/
	fclose(fp);
	/* Creating memory space for the errors table */
	errors = (char **)malloc((sizeof(char*))*(number_of_lines));    
	is_error = (int *)malloc((sizeof(int))*(number_of_lines));
	errors2 = (char **)malloc((sizeof(char*))*(number_of_lines));    
	is_error2 = (int *)malloc((sizeof(int))*(number_of_lines));    
}
/***************************************************** END OF READING FILE & STORING FUNCTION ******************************************************************/


/************************************************** INSTRUCTIONS-HOLDING DATA STRUCTURE ************************************************************************/
/* This is the implementation of a data structure(TRIE) to store the instructions and their opcodes
 By using this we don't need to linearly scan all the instructions to find the opcode
 And this will save a lot of time
 First Implement the structure for opcode holding data structure
 This returns a fresh node which is used to get new nodes*/
struct Opcode_Trie *getNode()
{
	struct Opcode_Trie *pNode = NULL;
	pNode = (struct Opcode_Trie *)malloc(sizeof(struct Opcode_Trie));
	if (pNode)
	{
    	int i;

    	pNode->opcode_complete = false;

    	for (i = 0; i < ALPHANUMERIC_SIZE; i++)
        	pNode->next_character[i] = NULL;
	}

	return pNode;
}

/* If not present, inserts opcode into trie data structure
 If the key is prefix of trie node, then it just marks the leaf code*/
void insert(struct Opcode_Trie *root, char *key,int i)
{
	int level;
	int length = strlen(key);
	int index;
	struct Opcode_Trie *pCrawl = root;
	for (level = 0; level < length; level++)
	{
    	index = CHAR_TO_INDEX(key[level]);
    	if (!pCrawl->next_character[index])
        	pCrawl->next_character[index] = getNode();

    	pCrawl = pCrawl->next_character[index];
	}
	/* By this we are marking that node which has the end of opcode*/
	pCrawl->opcode_complete = true;
	pCrawl->j = i;
}

/* This finds whether the instruction is valid or not
 If the instruction is valid this function returns the opcode value else return -1*/
int search(struct Opcode_Trie *root, char *key)
{
	int level;
	int length = strlen(key);
	int index;
	bool dd;
	struct Opcode_Trie *pCrawl = root;

	for (level = 0; level < length; level++)
	{
    	index = CHAR_TO_INDEX(key[level]);
    	if(index==-1)
        	return -1;
    	if (!pCrawl->next_character[index])
        	return -1;

    	pCrawl = pCrawl->next_character[index];
	}

	dd = (pCrawl != NULL && pCrawl->opcode_complete);
	if(dd==true)
    	return pCrawl->j;
	else
    	return -1;   	 
}
void initialize_opcodes()
{
	int i;
	/* make the opcodes  */
	opcode_root = getNode();
	for (i = 0; i < 21; i++)
	{
    	insert(opcode_root, instructionset_1_table[i],i);
	}
	/* To search any instruction use --> search(root, instruction string)*/
}
/* Converts character to integer to use in TRIE */
int CHAR_TO_INDEX(char c)
{
	int yy = -1;
	if(c>='a' && c<='z')
	{
    	yy = c-'a';
	}
	else if (c>='A' && c<='Z')
	{    
    	yy = c - 'A';
    	yy += 26;
	}
	else if(c>='0' && c<='9')
	{
    	yy = c - '0';
    	yy += 52;
	}
	else if(c=='_')
	{
		yy = 62;
	}
	return yy;
}
/***************************************************** END OF INSTRUCTION_SET HANDLING DATA STRUCTURE **********************************************************/

/****************************************************** COLLECTION OF FUNCTIONS TO CHECK VARIOUS ERRORS ********************************************************/
void check_errors(char* logfile)
{
	/* Writing file pointer */
	FILE* fp;
	/* Declaring the variable names */
	/* i and j are iterators */
	/* word_count stores the number of words in the string whenever required */
	/* flag and temp_size are temporary variables to store numerical values to check various conditions */
	/* Label count stores the current count of label while checking the errors*/
	/* errors_count & warnings_count stores the current number of errors & warnings*/
	/* current_program_counter stores the current count of program counter */
	int i,j,size,word_count,flag,temp_size,number_decimal,label_count,errors_count=0,warnings_count=0,current_program_counter = 0;
	/* check_alphanumeric is a boolean value used to hold the truthfullnes about a character is alphanumeric or not  
	& check_condition is used for checking temporary conditio ns */
	bool check_alphanumeric,check_condition;
	/* these are variable names to store the first word & second word  of a line */
	char *slice,*first_word,*second_word;
	/* labeL_root is the root of the TRIE data structure to store the labels & their line number in the data structure */
	label_root = getNode();
	/* Intially assigning some space to the pointer slice */
	slice = (char *)malloc(sizeof(char)*100);   	 
	/* Scanning all the lines stored in struct data structure & also storing the labels in trie data structure */
	/*In this loop all the duplicate labels OR labels with invalid names will be identified*/
    
	/* Making the log file & opening it */
	size = strlen(logfile);
	logfile[size-3]='l';
	logfile[size-2]='o';
	logfile[size-1]='g';
	fp = fopen(logfile, "w");
    
	for(i=0;i<number_of_lines;i++)
	{
    	/* Initially there is no error*/
    	is_error[i] = 0;
    	size = strlen(asm_data[i].before_colon);
    	slice = (char *)realloc(slice,size+1);
    	asm_data[i].is_operand_numeric = false;
    	asm_data[i].label_used = 0;
    	asm_data[i].opcode = -5;
    	/* If the line contains a label then checking various conditions */
    	if(asm_data[i].is_colon==1)
    	{
        	asm_data[i].label_used = 0;
        	/* storing number of words before the colon in the string */
        	word_count = number_of_words(asm_data[i].before_colon);    
        	slice = find_nth_word(asm_data[i].before_colon,1);
        	temp_size = strlen(slice);
        	/* If the label is properly named & to check if it is duplicate label or not */
        	if(word_count == 1 && search(label_root,slice)!=-1)
        	{
            	if(asm_data[i].before_colon[size-1]!=' ' && asm_data[i].before_colon[size-1]!='\t')
            	{
                	/* Storing the error in the errors table */
                	errors[i] = (char *)malloc((sizeof(char))*(strlen(asm_data[i].before_colon)+60));
                	errors[i][0] = '\0';
                	strcat(errors[i],"There is a ");
                	for(j=0;j<temp_size;j++)
                    	errors[i][j+11] = slice[j];
                	errors[i][j+11] = '\0';    
                	strcat(errors[i]," named label which is a Duplicate Label");
                	is_error[i] = 1;
            	}
        	}
        	else
        	{
            	errors[i] = (char *)malloc((sizeof(char))*(strlen(asm_data[i].before_colon)+40));    
            	errors[i][0] = '\0';    
            	if(word_count == 0)
            	{
                	/* If the Label is empty then report it */
                	/* Storing the error in the errors table */
                	strcat(errors[i],"Label Name cannot be empty");
                	is_error[i] = 1;
            	}
            	else if (word_count > 1 || asm_data[i].before_colon[size-1] == ' ' || asm_data[i].before_colon[size-1]=='\t')
            	{
               	 
                	/* To report that operand cannot be declared before label */
                	if(search(opcode_root,slice)!=-1)
                	{
                    	/* Storing the error in the errors table */
                    	strcat(errors[i],"Mnemonic cannot be declared before a label");
                    	is_error[i] = 1;
                	}
                	else
                	{
                    	/* Storing the error in the errors table */
                    	for(j=0;j<temp_size;j++)
                        	errors[i][j] = slice[j];
                    	errors[i][j] = '\0';  
                    	strcat(errors[i]," is not a valid mnemonic");
                    	is_error[i] = 1;   	 
                	}
            	}
            	else
            	{
                	/* If there is only one word before colon then checking whether the labelname is right */
                	flag = 1;
                	for(j=0;j<temp_size;j++)
                	{
                    	check_alphanumeric = false;
                    	if(slice[j]>='a' && slice[j]<='z')
                        	check_alphanumeric = true;
                    	else if (slice[j]>='A' && slice[j]<='Z')
                        	check_alphanumeric = true;
                    	else if(slice[j]>='0' && slice[j]<='9')
                        	check_alphanumeric = true;
                        else if(slice[j]=='_')	
                            check_alphanumeric = true;
                    	if(!check_alphanumeric)
                        	flag = 0;    
                	}
                	if(flag==0)
                	{
                    	/* Reporting that label must be alphanumeric */
                    	/* Storing the error in the errors table */
                    	strcat(errors[i],"Label must be alphanumeric");
                    	is_error[i] = 1;
                	}
                	else if (slice[0]>='0' && slice[0]<='9')
                	{
                    	/* Reporting that label start with alphabet */                   	 
                    	/* Storing the error in the errors table */
                    	strcat(errors[i],"Label must be starting with an alphabet");
                    	is_error[i] = 1;
                	}
                	else if (search(opcode_root,slice)!=-1)
                	{
                    	/* Reporting that operand cannot be a label */
                    	/* Storing the error in the errors table */
                    	strcat(errors[i],"Label must not be a mnemonic");
                    	is_error[i] = 1;
                	}
                	else
                	{
                    	/* Finally if the everything is fine inserting the label name in the TRIE STRUCTURE*/
                    	insert(label_root,slice,i);                   	 
                	}
            	}
        	}
    	}
    	/* Checking for errors which are other than label naming errors */
    	is_error2[i] = 0;
   	 
    	/* calculating the number of words between label part & comments part */
    	word_count = number_of_words(asm_data[i].mid_part);
    	if(word_count!=0)
    	{
        	/* Retrieving the first word in the mid part */
        	first_word = find_nth_word(asm_data[i].mid_part,1);
        	/* Making space for the second type of error */
        	errors2[i] = (char *)malloc((sizeof(char))*(strlen(asm_data[i].mid_part)+60));
        	errors2[i][0] = '\0';
        	temp_size = strlen(first_word);   	 
       	 
        	/* The first word is searched in the TRIE data structure for instruction set */
        	flag = search(opcode_root,first_word);
        	if(flag == -1)
        	{
            	/* If -1 comes it means that it is not a valid operand */
            	for(j=0;j<temp_size;j++)
                	errors2[i][j] = first_word[j];
            	errors2[i][j] = '\0';  
            	strcat(errors2[i]," is not a valid mnemonic");
            	is_error2[i] = 1;
        	}
        	else if(word_count>2)
        	{
            	/* Even after if the opcode of first word is a instruction we need to check if there are more
            	than two words or instruction in a line */
            	check_condition = true;
            	for(j=1;j<word_count;j++)
            	{
                	if(search(opcode_root,find_nth_word(asm_data[i].mid_part,j+1))!=-1)
                	{
                    	strcat(errors2[i],"There can be only one instruction per line");
                    	check_condition = false;
                    	break;
                	}
            	}
            	if(check_condition == true && instruction_type[flag]==1)
                	strcat(errors2[i],"Only one numerical value or label was expected ");
            	else if(check_condition == true)
                	strcat(errors2[i],"No numerical value or label was expected ");    
            	is_error2[i] = 1;
        	}
        	else
        	{
            	/* Divide the cases into two where the instruction either expects an operand or not*/
            	asm_data[i].opcode = flag;
            	if(instruction_type[flag]==1)
            	{
                	/* If an instruction needs an operand but does not have one we need to report error */
                	if(word_count==1)
                	{
                    	strcat(errors2[i],"A Numerical Value or a label was expected");
                    	is_error2[i] = 1;
                	}
                	else
                	{
                    	/* If there is an operand we need to check whether the operand is correct or not */
                    	second_word = find_nth_word(asm_data[i].mid_part,2);
                    	check_condition = true;
                    	number_decimal = string_to_num(second_word,&check_condition);
                    	if(check_condition == true)
                    	{
                        	asm_data[i].opcode = flag;
                        	asm_data[i].is_operand_numeric = true;
                        	asm_data[i].operand_numeric = number_decimal;
                    	}
                    	if(asm_data[i].opcode==20 && check_condition==false)
                    	{
                        	strcat(errors2[i],"For SET instruction operand can be only numerical");
                        	is_error2[i] = 1;
                    	}
                    	if(asm_data[i].opcode==19 && check_condition==false)
                    	{
                        	strcat(errors2[i],"For data instruction operand can be only numerical");
                        	is_error2[i] = 1;
                    	}
                	}
            	}
            	else
            	{
                	/* If there is no operand required then report error */
                	if(word_count>1)
                	{
                    	strcat(errors2[i],"No numerical Value or label was expected");
                    	is_error2[i] = 1;
                	}
            	}
        	}
    	}    
	}
	/* Checking for the missing labels */
	for(i=0;i<number_of_lines;i++)
	{
    	/* Checking if the line already has errors */
    	if(is_error2[i]==0)
    	{
        	word_count = number_of_words(asm_data[i].mid_part);
        	if(word_count==2)
        	{
            	/* In instructions where operand is required I am extracting out the operand */
            	second_word = find_nth_word(asm_data[i].mid_part,2);
            	temp_size = strlen(second_word);
            	first_word = find_nth_word(asm_data[i].mid_part,1);
            	flag = search(opcode_root,first_word);
            	/* If the operand is numeric it has already been handled So checking if it is a label */
            	if(asm_data[i].is_operand_numeric==false)
            	{
                	/* If it is not a label then reporting errors for it */
                	if(search(label_root,second_word)==-1)
                	{
                    	check_condition = true;
                    	for(j=0;j<temp_size;j++)
                    	{
                        	if(second_word[j]<'0'||second_word[j]>'9')
                            	check_condition = false;
                    	}
                    	/* If the numeric is too long reporting it */
                    	if(check_condition)
                        	strcat(errors2[i],"The number is very long to take input");
                    	else
                    	{
                        	/* Checking the type of operand & seeing if that label exists or not */
                        	if(instruction_type2[flag]==2)
                            	strcat(errors2[i],"The label used as the operand does not exist");
                        	else if(second_word[0]>='0' && second_word[0]<='9')
                            	strcat(errors2[i],"The operand is non-numerical");
                        	else
                            	strcat(errors2[i],"The operand is neither an existing label nor a numerical");       	 
                    	}
                    	is_error2[i] = 1;    
                	}
                	else
                	{
                    	/* If the label exists then marking that the label is used */
                    	asm_data[search(label_root,second_word)].label_used = 1;
                	}
            	}
        	}    
    	}
	}
	label_count=0;
	errors_count=0;
	warnings_count=0;
	/* Putting all errors & warnings in the log file & counting the number of errors & warnings */
	/* Also Defining the program counter for each line side-by-side */
	for(i=0;i<number_of_lines;i++)
	{
    	asm_data[i].program_counter = -1;
    	/* Putting all the errors in the log file */
    	if(is_error[i] == 1)
    	{
        	fprintf(fp,"ERROR on Line No. %d :: %s\n",i+1,errors[i]);
        	errors_count++;
    	}
    	if(is_error2[i] == 1)
    	{
        	fprintf(fp,"ERROR on Line No. %d :: %s\n",i+1,errors2[i]);
        	errors_count++;
    	}
    	/* Putting all the warnings in the log file */
    	if(is_error[i]==0 && asm_data[i].is_colon==1)
    	{
        	/* Checking whether the label has been used or not */
        	if(asm_data[i].label_used==0)
        	{
            	fprintf(fp,"WARNING on Line No. %d :: Label is not used\n",i+1);
            	warnings_count++;
        	}
        	label_count ++;
    	}
    	if(is_error[i]==0 && is_error2[i]==0 && asm_data[i].opcode>=13 && asm_data[i].opcode<=17)
    	{
        	/* Checking whether there is any label before return or not */
        	if(asm_data[i].opcode==14 && label_count==0)
        	{
            	fprintf(fp,"WARNING on Line No. %d :: There should be atleast one label before a return statement \n",i+1);
            	warnings_count++;
        	}
        	else
        	{
            	/* Warnings for infinite loop or if their is a branch just for the next instruction */
            	second_word = find_nth_word(asm_data[i].mid_part,2);
            	flag = search(label_root,second_word);
            	if(flag==i)
            	{
                	fprintf(fp,"WARNING on Line No. %d :: Infinite loop \n",i+1);
                	warnings_count++;
            	}
            	else if(flag==(i+1))
            	{
                	fprintf(fp,"WARNING on Line No. %d :: Branching to next instruction\n",i+1);
                	warnings_count++;
            	}
        	}
    	}
    	/* If there are no errors till now then change the program counter else the listing file & log files break after this point*/
    	if(is_error[i]==0 && is_error2[i]==0)
    	{
        	size = number_of_words(asm_data[i].before_colon);
        	temp_size = number_of_words(asm_data[i].mid_part);
        	if(size>0)
        	{
            	first_word = find_nth_word(asm_data[i].before_colon,1);
            	insert(label_root,first_word,current_program_counter);
        	}
        	/* If the line is just blank lines or just comments then assign program counter as -1
           	else assign it as the current program counter */   
        	if(size>0 || temp_size>0)
            	asm_data[i].program_counter = current_program_counter;
        	else
            	asm_data[i].program_counter = -1;
           	 
        	/* If there is valid operand & instruction then increase the program counter*/    
        	if(temp_size>0)
            	current_program_counter++;
    	}
	}
	program_counter_lines = current_program_counter;
	/* Reporting the number of errors & warnings in the command line */
	if(errors_count==0 && warnings_count==0)
    	printf("There is no warnings or errors.\n");
	else    
    	printf("There are %d warnings and there are %d errors (mentioned in the log file).\n",warnings_count,errors_count);
	if(errors_count>0)
	{
    	printf("Since there are errors in the asm file the listing & object files are made only till the first error is encountered.\n");    
	}    
	fclose(fp);    
}
/* This is a function which calculates the number of words in a string */
int number_of_words(char *str)
{
	/* Declaration of variables */
	/* i is the iterator, len is the length of the string passed as the parameter, word_count is the number of words in the string */
	/* trail is an integer taking values either 0 or 1 & signifies while iterating whether I am in a string or whitespaces */
	int i,len,word_count,trail;
	/* Setting all the intitial values */
	len = strlen(str);
	word_count = 0;
	trail = 1;
	/* Looping the string */
	for(i=0;i<len;i++)
	{
    	/* If the current character in the string is a tab or whitespace Making trail=1 */
    	if(str[i]=='\t'||str[i]==' ')
    	{
        	trail = 1;
    	}
    	else
    	{
        	/* Now if the trail was previously 1 & right now I am at any
        	non-whitespace character I need to increase the number of words */
        	if(trail==1)
        	{
            	word_count++;
        	}       	 
        	/* Setting trail to 0 */
        	trail = 0;
    	}
	}    
	/* returning the number of words */
	return word_count;
}
/* This function returns the nth word in the passed string Here N is 1-indexed */
char* find_nth_word(char *str, int n)
{
	/* Declaring the nth_word as a character pointer which is to be returned */
	char *nth_word;
	/* i is the iterator, len is the length of the string passed as the parameter, word_count is the number of words in the string */
	/* trail is an integer taking values either 0 or 1 & signifies while iterating whether I am in a string or whitespaces */
	int i,len,trail,word_number = 0,curr_index;
	/* Setting the initial values of the variable */
	len = strlen(str);
	word_number = 0; /* word_number keeps the track of the number of word at which we are right now */
	trail = 1;
	curr_index = 0;
	/* Making space for the nth_word It may go maximum the whole length of the string */
	nth_word = (char*)malloc(sizeof(char)*(len+1));
	/* Looping the string */
	for(i=0;i<len;i++)
	{
    	/* If the current character in the string is a tab or whitespace Making trail=1 */
    	if(str[i]=='\t'||str[i]==' ')
    	{
        	trail = 1;
    	}
    	else
    	{   	 
        	/* Now if the trail was previously 1 & right now I am at any
        	non-whitespace character I need to increase the number of words */
        	if(trail==1)
        	{
            	word_number++;
        	}
        	trail = 0;    
        	/* If the current word number is n then we need to store it in the pointer nth_word */
        	if(word_number == n)
        	{
            	/* Storing the character in curr_index of the pointer nth_word & then incrementing the variable */
            	nth_word[curr_index++] = str[i];
           	 
            	/* Now if we are at the end of the string & thus Placing null character at the end */
            	if(i==(len-1) || str[i+1]==' ' || str[i+1]=='\t' )
            	{
                	nth_word[curr_index++]='\0';
            	}
        	}
    	}
	}
	/* Returning the string nth_word */
	return nth_word;
}

/* Checks whether the string given is a valid numberic or not
	& if it is valid it checks whether the string is octal,hexadecimal or decimal & returns the decimal value
*/
int string_to_num(char* str,bool *p)
{
	/* Variables declared*/
	int i,len,num,d;
	char *eptr;
	d = 1;
	/* If - is in front then the string is negative */
	if(str[0]=='-')
	{
    	d = -1;
    	str++;
	}
	else if (str[0]=='+')
	{
    	d = 1;
    	str++;
	}
	len = strlen(str);
	/* Assuming that if the length of the string is more than 13 letters then it is error */
	if(len>13)
	{
    	*p = false;
    	return -1;
	}
	/* Checking conditions for if the string length is 1 */
	if(len==1)
	{
    	if(str[0]>='0' && str[0]<='9')
        	num = str[0]-'0';
    	else
    	{
        	num = -1;
        	*p = false;
    	}
    	return d*num;
	}
	/* Checking about validity of the string (if it is numeric) */
	if(str[0]=='0' && str[1]=='x')
	{
    	if(len>11 || len==2)
        	*p=false;
    	for(i=2;i<len;i++)
        	if(!(str[i]<='9'&&str[i]>='0'))
            	*p = false;
	}
	else
	{
    	for(i=0;i<len;i++)
            	if(!(str[i]<='9'&&str[i]>='0'))
                	*p = false;
	}
	/* If the number format is not valid then return -1 */
	if((*p)==false)
    	return -1;
	/* Converting the number according to its format (either decimal,hexadecimal or octal) */    
	num = strtol(str,&eptr,8);
	if((!(*eptr)) && str[0]=='0')
    	return d*num;
	num = strtol(str,&eptr,10);
	if((!(*eptr)))
    	return d*num;
	num = strtol(str,&eptr,16);
	if((!(*eptr)))
    	return d*num;
	p = false;
	return -1;       	 
}
/********************************************************* END OF ERRORS CHECKING FUNCTIONS ********************************************************************/


/********************************************************* FUNCTIONS FOR PRODUCING LISTING FILE ****************************************************************/
void listing_file(char *listing_file)
{
	/* Declaring the varirables */
	FILE *listing_file_pointer,*object_file_pointer;
	int i,j,str_size,word_count,size,temp_counter,machine_code_decimal,pow_16;
	char *second_word;
	char *object_file;
	int num[6],counter[8];
	/* Making the Listing file & opening it*/
	size = strlen(listing_file);
	listing_file[size-3]='l';
	listing_file[size-2]='s';
	listing_file[size-1]='t';
	listing_file_pointer = fopen(listing_file, "w");
	object_file = (char *)malloc(sizeof(char)*size);
	for(i=0;i<size;i++)
    	object_file[i] = listing_file[i];
	object_file[size-3]='o';
	object_file[size-2]='\0';
	object_file_pointer = fopen(object_file, "wb");    
	for(i=0;i<number_of_lines;i++)
	{
    	/* If the asm file has any error my listing file would be valid till ther only & then it will be stopped */
    	if(is_error[i]==1||is_error2[i]==1)
    	{
        	break;
    	}
    	str_size = strlen(asm_data[i].mid_part);
    	word_count = number_of_words(asm_data[i].mid_part);
    	/* Checking if there is any instruction on that line*/
    	if(word_count>0)
    	{
        	/* Putting the instruction opcode in least 2 significant bits */
        	asm_data[i].machine_code_hexadecimal[0] = asm_data[i].opcode%16;
        	asm_data[i].machine_code_hexadecimal[1] = asm_data[i].opcode/16;
        	for(j=7;j>=2;j--)
            	asm_data[i].machine_code_hexadecimal[j] = 0;
        	/* Filling the remaining 6 bits dependent on the type of instruction*/    
        	if(word_count>1)
        	{
            	second_word = find_nth_word(asm_data[i].mid_part,2);
            	if(instruction_type2[asm_data[i].opcode]==2)
            	{
                	/* Calculating offset for branch types instructions */
                	if(asm_data[i].is_operand_numeric==true)
                    	bit24(num,asm_data[i].operand_numeric);
                	else    
                    	bit24(num,-asm_data[i].program_counter-1+search(label_root,second_word));
            	}
            	else
            	{
                	/* For non-branching instructions */
                	if(asm_data[i].is_operand_numeric==true)
                    	bit24(num,asm_data[i].operand_numeric);
                	else    
                    	bit24(num,search(label_root,second_word));
            	}
            	/* Doing manipulations for the SET instruction */
            	if(asm_data[i].opcode==20)
            	{
                	if(asm_data[i].is_colon==1)
                	{
                    	second_word = find_nth_word(asm_data[i].before_colon,1);
                    	insert(label_root,second_word,asm_data[i].operand_numeric);
                	}
            	}
            	for(j=5;j>=0;j--)
                	asm_data[i].machine_code_hexadecimal[j+2] = num[j];  
                /* Doing manipulations for data instructions */
				if(asm_data[i].opcode==19)
            	{
                    bit32(asm_data[i].machine_code_hexadecimal,asm_data[i].operand_numeric);
            	}	
					  
        	}    
    	}
	}
	/* Doing changes for the SET instruction */
	for(i=0;i<number_of_lines;i++)
	{
    	if(is_error[i]==1 || is_error2[i]==1)
        	break;
    	str_size = strlen(asm_data[i].mid_part);
    	word_count = number_of_words(asm_data[i].mid_part);
    	if(word_count==2)
    	{
        	second_word = find_nth_word(asm_data[i].mid_part,2);
        	if(instruction_type2[asm_data[i].opcode]==2)
        	{
            	/* Calculating offset for branch types instructions */
            	if(asm_data[i].is_operand_numeric==false)
            	{    
                	bit24(num,-asm_data[i].program_counter-1+search(label_root,second_word));
                	for(j=5;j>=0;j--)
                    	asm_data[i].machine_code_hexadecimal[j+2] = num[j];    
            	}
        	}
        	else
        	{
            	/* For non-branching instructions */
            	if(asm_data[i].is_operand_numeric==false)
            	{
                	bit24(num,search(label_root,second_word));
                	for(j=5;j>=0;j--)
                    	asm_data[i].machine_code_hexadecimal[j+2] = num[j];
            	}
        	}
    	}    
	}
	/* Writing in the listing files */
	for(i=0;i<number_of_lines;i++)
	{
   	 
    	/* If there is any error I will stop making the listing filess */
    	if(is_error[i]==1||is_error2[i]==1)
    	{
        	break;
    	}
    	size = number_of_words(asm_data[i].mid_part);
    	str_size = number_of_words(asm_data[i].before_colon);
    	if(size==0 && str_size==0)
    	{
        	fprintf(listing_file_pointer,"%s\n",asm_data[i].comments);
    	}
    	else
    	{
        	/* Printing in the listing files */
        	temp_counter = asm_data[i].program_counter;
        	for(j=0;j<8;j++)
        	{
            	counter[j] = temp_counter%16;
            	temp_counter = temp_counter/16;
        	}
        	for(j=7;j>=0;j--)
            	fprintf(listing_file_pointer,"%c",hexadecimal_unit(counter[j]));
        	fprintf(listing_file_pointer," ");
        	machine_code_decimal = 0;
        	pow_16 = 1;
        	for(j=7;j>=0;j--)
            	fprintf(listing_file_pointer,"%c",hexadecimal_unit(asm_data[i].machine_code_hexadecimal[j]));
        	for(j=0;j<8;j++)
        	{
            	machine_code_decimal += asm_data[i].machine_code_hexadecimal[j]*pow_16;
            	pow_16*=16;
        	}
        	fprintf(listing_file_pointer," ");
        	if(asm_data[i].is_colon!=0)
        	{
            	fprintf(listing_file_pointer,"%s:",asm_data[i].before_colon);
        	}
        	fprintf(listing_file_pointer,"%s %s\n",asm_data[i].mid_part,asm_data[i].comments);
        	/* Printing in the object file */    
        	if(size>0)
        	{
       		 fwrite(&machine_code_decimal,sizeof(int),1,object_file_pointer);
			}
    	}
	}
	/* Closing the file pointers for listing file & object file */
	fclose(listing_file_pointer);
	fclose(object_file_pointer);
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
/* Function to store number in a 8 size array (8-digit in hexadecimal) */
void bit32(int arr[],int num)
{
	int i;
	int bit32_max;
	int binary[32];
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
/***************************************************** END OF FUNCTIONS FOR PRODUCING LISTING FILE *************************************************************/

