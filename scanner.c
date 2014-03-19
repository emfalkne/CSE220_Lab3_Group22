//
//  scanner.c
//  Lab3
//
//  Created by Bryce Holton.
//  Copyright (c) 2014 Bryce Holton. All rights reserved.
//	Editted by Group 22
//	Scott Dexhimer (SDexh), Emily Falkner (emfalkne), Savannah Puckett (snpucket)
//	Git Hub Url: https://github.com/emfalkne/CSE220_Lab3_Group22/commits/master

#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"

//error message macro
#define ERRORMSG printf("Error.")
//used to shift upper case letters to lower case letters
#define TOLOWERC(a) ((64 < a && a < 91) ? a + 32 : a)

//forward declarations---------------------------------------------------------------------------------------------
void init_scanner(char source_name[]);
void close_scanner();
void clear_token_line();
char read_next_line();
char *get_file_line();
struct Token *get_token_line();
static void read_line(char *last);
static void rem_token(struct Token *t);
static void tokenize_line();
static struct Token *tokenize_word(int *i, char *ch);
static int adv_to_valid(char *ch);
static int adv_to_nl(char *ch);
static int interpret_ch(char *dest, char *src);
static int identify(char ch);
static int is_letter(char ch);
static int is_number(char ch);
static int is_symbol(char ch);
static int parse_word(char *dest, char *src);
static int parse_number(char *dest, char *src);
static int parse_symbol(char *dest, char *src);
static void add_token(struct Token *t);
static void make_sense_token(struct Token *t);

//type definitions/structs ----------------------------------------------------------------------------------------
typedef enum
{
    LETTER, DIGIT, QUOTE, SPECIAL, EOF_CODE,
}CharCode;

typedef struct
{
    char *string;
    TokenCode token_code;
}RwStruct;

/*********************
 Static Variables for Scanner
 Must be initialized in the init_scanner function.
 *********************/
static FILE *fin;
static char src_name[MAX_FILE_NAME_LENGTH];
static CharCode char_table[256];  // The character table
//starts the link list
static char line[MAX_SOURCE_LINE_LENGTH];
static struct Token *tl_head;

const RwStruct rw_table[8][13] = {
    {{"do",DO},{"if",IF},{"in",IN},{"of",OF},{"or",OR},{"to",TO},{"<=",LE},{">=",GE},{"<",LT},{">",GT},{"!=",NE}}, //Reserved words of size 2
    {{"and",AND},{"div",DIV},{"end",END},{"for",FOR},{"mod",MOD},{"nil",NIL},{"not",NOT},{"set",SET},{"var",VAR},{"!",NOT}}, //Reserved words of size 3
    {{"case",CASE},{"else",ELSE},{"file",FFILE},{"goto",GOTO},{"then",THEN},{"type",TYPE},{"with",WITH},{"*",STAR},{"+",PLUS},{"nihil",NIL}}, //Reserved words of size 4
    {{"array",ARRAY},{"begin",BEGIN},{"const",CONST},{"label",LABEL},{"until",UNTIL},{"while",WHILE},{"-",MINUS},{"=",EQUAL},{":",COLON},{",",COMMA},{"/",SLASH},{"<ERROR>",ERROR},{"FILE",FFILE}},  //Reserved words of size 5
    {{"downto",DOWNTO}, {"packed",PACKED},{"record",RECORD}, {"repeat",REPEAT},{"string",STRING},{"(",LPAREN},{")",RPAREN},{"..",DOTDOT},{".",PERIOD},{"nihil",NIL}},  // Reserved words of size 6
	{{"program", PROGRAM},{"^",UPARROW},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL}}, // Reserved words of size 7
    {{"function", FUNCTION},{"<no token>",NO_TOKEN},{"[",LBRACKET},{"]",RBRACKET},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL}}, // Reserved words of size 8
    {{"procedure", PROCEDURE},{";",SEMICOLON},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL},{"nihil",NIL}}  // Reserved words of size 9
};

//initializes the variables necessary to run scanner
void init_scanner(char source_name[])
{
	//copies the source name to src_name variable
    strcpy(src_name, source_name);
	//clears the tokens from memory
	clear_token_line();
	//opens the output file
	fin = fopen(src_name, "r");
	if(fin == NULL){
		printf("%s\n",source_name);
		printf("%s\n",src_name);
		printf("%s\n","fopen failed");
	}
}

//takes care of final business with the scanner class
void close_scanner()
{
	//closes the file
	if(fin != NULL){
		int fclose_success = fclose(fin);
		if(fclose_success == 0){
	//		printf("%s","Fclose success");
		}
		else{
			printf("%s","ERROR");
		}
	}
	//clears the tokens from memory
	clear_token_line();
}

//reads the next line from the input stream
char read_next_line()
{
	char ch;
	//removes the tokens from memory
	clear_token_line();
	//reads the line
	read_line(&ch);
	//tokenizes the line
	tokenize_line();
	//returns the char used to determine when EOF is reached
	return ch;
}

//gets the line that was most recently read
char *get_file_line() {return line;}

//used to convert the file into a char array
static void read_line(char *last)
{
	char stream_line[MAX_SOURCE_LINE_LENGTH];
	int i = 0;
	do
	{
		//gets a new char, if no char could be read
		if(fread(&stream_line[i], 1, 1, fin) == 0)
		{
			//EOF has been reached
			stream_line[i] = EOF;
		}
		//increments i
		i++;
		//while end of line or EOF has not been reached
	}while('\n' != stream_line[i - 1] && EOF != stream_line[i - 1]);
	//the char at the end of the line
	*last = stream_line[i - 1];
	//if it isn't, installs a null character at the end
	stream_line[i - 1] = '\0';
	//copies the data
	strcpy(line, stream_line);
}

//returns the token linked list
struct Token *get_token_line() {return tl_head;}

//removes tokens from memory and sets head to null
void clear_token_line()
{
	//calls the recursive remove function
	rem_token(tl_head);
	//sets head to null
	tl_head = NULL;
}

//used to recursively remove tokens from memory
static void rem_token(struct Token *t)
{
	//if there is a token
	if(t)
	{
		//call rem_token on the next token
		rem_token(t->next);
		//free the token from memory
		free(t);
	}
}

//Savannah: Debug this part ----------------------------------------------------------------------------------------------------------------------------
//tokenizes the line
static void tokenize_line()
{
	char *cur_ch = line;
	int adv = 0;
	struct Token *t = tokenize_word(&adv, cur_ch);
	//if tokenizes_word failes
	if(!t)
	{
		//makes a token with a value of '\0'
		t = (struct Token*)malloc(sizeof(struct Token));
		t->value[0] = '\0';
		t->next = NULL;
		add_token(t);
		t = NULL;
	}
	//adds the token to the list
	add_token(t);
	//while tokenize word succeeds
	while(t)
	{
		//advances by the appropriate amount
		cur_ch += adv;
		//tokenizes starting at cur_ch
		t = tokenize_word(&adv, cur_ch);
		//adds the token
		add_token(t);
	}
}

//returns a token of the next word
static struct Token *tokenize_word(int *i, char *ch)
{
	char word[MAX_TOKEN_STRING_LENGTH];
	int num_adv = 0;
	struct Token *t;
	while(' ' == *ch)
	{
		//advance to the next valid character
		num_adv = adv_to_valid(ch);
		//if num_advance == 0
		if(0 == num_adv)
		{
			//set to one to avoid infinite loop
			num_adv = 1;
		}
		//advances the char pointer
		ch += num_adv;
	}
	//if there is nothing to tokenize on the line, returns null
	if('\0' == *ch)
	{
		return NULL;
	}
	//interprets the "word" and advances the tracker
	num_adv += interpret_ch(word, ch);
	*i = num_adv;
	//makes the token on heap
	t = (struct Token*)malloc(sizeof(struct Token));
	//copies the "word" to value of token
	strcpy(t->value, word);
	//sets the next to null
	t->next = NULL;
	make_sense_token(t);
	return t;
}

//advances to the next valid character
static int adv_to_valid(char *ch)
{
	int num_adv = 0;
	//while the value ch is less than than 32 (all chars and symbols)
	while(*ch < 32 && '\0' != *ch)
	{
		//goes to the next char
		ch++;
		//increments the number
		num_adv++;
	}
	//if the current ch and the next ch are \ es
	if('\\' == *ch && '\\' == *(ch + 1))
	{
		//advances to new line
		num_adv += adv_to_nl(ch);
	}
	return num_adv;
}
//End Debug	---------------------------------------------------------------------------------------------------------------------------------------------

//skips the line
static int adv_to_nl(char *ch)
{
	int i = 0;
	//while the value of ch is not null
	while('\0' != *ch)
	{
		//incrament the location of ch and i
		ch++;
		i++;
	}
	return i;
}

//interprets the character and parses the meaning of the word
static int interpret_ch(char *dest, char *src)
{
	int i = identify(TOLOWERC(*src)), num_adv;
	switch(i)
	{
	case 1:
		num_adv = parse_word(dest, src);
		break;
	case 2:
		num_adv = parse_number(dest, src);
		break;
	case 3:
		 num_adv = parse_symbol(dest, src);
		break;
	}

	return num_adv;
}

//identifies the char as the start of a string, number, or symbol
static int identify(char ch)
{
	if(is_letter(ch))
	{
		return 1;
	}
	else if(is_number(ch))
	{
		return 2;
	}
	return 3;
}

//these three functions look at a char and determin the category it belongs to---------------------------
//letter
static int is_letter(char ch)
{
	if(96 < ch && ch < 123)
	{
		return 1;
	}
	return 0;
}
//number
static int is_number(char ch)
{
	if(47 < ch && ch < 58)
	{
		return 1;
	}
	return 0;
}
//symbol
static int is_symbol(char ch)
{
	//if it is within the valid range and is not a letter or number
	if(32 < ch && ch < 127 && !is_letter(TOLOWERC(ch)) && !is_number(ch))
	{
		return 1;
	}
	return 0;
}
//---------------------------------------------------------------------------------------------------------

//parses a word
static int parse_word(char *dest, char *src)
{
	int num_adv = 0;
	//while the char is a letter, number, or underscore
	while(is_letter(TOLOWERC(*src)) || is_number(*src) || '_' == *src)
	{
		//copies the character to the destination
		*dest = TOLOWERC(*src);
		//increments the destination and source locations
		dest++;
		src++;
		num_adv++;
	}
	//sets the last char to be null (for string)
	*dest = '\0';
	return num_adv;
}

//parses a number
static int parse_number(char *dest, char *src)
{
	int num_adv = 0;
	//accounts for negative numbers
	if('-' == *src)
	{
		*dest = *src;
		dest++;
		src++;
		num_adv++;
	}
	//while the number goes
	while(is_number(*src))
	{
		*dest = *src;
		dest++;
		src++;
		num_adv++;
	}
	*dest = '\0';
	return num_adv;
}

//parses a symbol
static int parse_symbol(char *dest, char *src)
{
	int num_adv = 0;
	//jumps to the appropriate case
	switch(*src)
	{
		//if it is a -
	case '-':
		if(is_number(*(src + 1)))
		{
			num_adv += parse_number(dest, src);
		}
		else
		{
			*dest = *src;
			dest++;
			src++;
			num_adv++;
		}
		break;

		//if it is a :
	case ':':
		if('=' == *(src + 1))
		{
			*dest = *src;
			dest++;
			src++;
			num_adv++;
			*dest = *src;
			dest++;
			num_adv++;
		}
		else
		{
			*dest = *src;
			dest++;
			src++;
			num_adv++;
		}
		break;

		//...
	case '\'':
		*dest = *src;
		dest++;
		src++;
		num_adv++;
		while('\'' != *src && '\0' == *src)
		{
			*dest = *src;
			dest++;
			src++;
			num_adv++;
		}
		*dest = *src;
		dest++;
		num_adv++;
		break;

		//you get the idea
	case '\"':
		*dest = *src;
		dest++;
		src++;
		num_adv++;
		while('\"' != *src && '\0' == *src)
		{
			*dest = *src;
			dest++;
			src++;
			num_adv++;
		}
		*dest = *src;
		dest++;
		num_adv++;
		break;

	case '<':
		if('=' == *(src + 1))
		{
			*dest = *src;
			dest++;
			src++;
			num_adv++;
			*dest = *src;
			dest++;
			num_adv++;
		}
		else
		{
			*dest = *src;
			dest++;
			num_adv++;
		}
		break;

	case '>':
		if('=' == *(src + 1))
		{
			*dest = *src;
			dest++;
			src++;
			num_adv++;
			*dest = *src;
			dest++;
			num_adv++;
		}
		else
		{
			*dest = *src;
			dest++;
			num_adv++;
		}
		break;

	case '.':
		if('.' == *(src + 1))
		{
			*dest = *src;
			dest++;
			src++;
			num_adv++;
			*dest = *src;
			dest++;
			num_adv++;
		}
		else
		{
			*dest = *src;
			dest++;
			num_adv++;
		}
		break;

	case '!':
		if('=' == *(src + 1))
		{
			*dest = *src;
			dest++;
			src++;
			num_adv++;
			*dest = *src;
			dest++;
			num_adv++;
		}
		else
		{
			*dest = *src;
			dest++;
			num_adv++;
		}
		break;

	default:
		*dest = *src;
		dest++;
		num_adv++;
		break;
	};
	*dest = '\0';
	 return num_adv;
}

//adds the token to the list
static void add_token(struct Token *t)
{
	//if no head
	if(tl_head == NULL)
	{
		//adds at head
		tl_head = t;
	}
	else
	{
		//makes a pointer for traversing the list
		struct Token *p = tl_head;
		//while there is a next node
		while(p->next)
		{
			//move to the next node
			p = p->next;
		}
		//adds the token at the end of the list
		p->next = t;
	}
}

//makes sense of the token
static void make_sense_token(struct Token *t)
{
	char ch = *(t->value);
	//switches to the appropriate case
	switch(ch)
	{
		//if a ' was encountered
	case '\'':
		//it's a string
		t->code = STRING;
		t->type = STRING_LIT;
		break;
		//same with "
	case '\"':
		t->code = STRING;
		t->type = STRING_LIT;
		break;

		//if a -
	case '-':
		//if the next char is a number
		if(is_number(*((t->value) + 1)))
		{
			//then it's a number
			t->code = NUMBER;
			t->type = INTEGER_LIT;
		}
		//else it is a minus
		else
		{
			t->code = MINUS;
			t->type = REAL_LIT;
		}
		break;

		//now for a bunch of symbols
	case ':':
		if('=' == *((t->value) + 1))
		{
			t->code = COLONEQUAL;
			t->type = REAL_LIT;
		}
		else
		{
			t->code = COLON;
			t->type = REAL_LIT;
		}
		break;

	case '+':
		t->code = PLUS;
		t->type = REAL_LIT;
		break;

	case '=':
		t->code = EQUAL;
		t->type = REAL_LIT;
		break;

	case '*':
		t->code = STAR;
		t->type = REAL_LIT;
		break;

	case '(':
		t->code = LPAREN;
		t->type = REAL_LIT;
		break;

	case ')':
		t->code = RPAREN;
		t->type = REAL_LIT;
		break;

	case '[':
		t->code = LBRACKET;
		t->type = REAL_LIT;
		break;

	case ']':
		t->code = RBRACKET;
		t->type = REAL_LIT;
		break;

	case ';':
		t->code = SEMICOLON;
		t->type = REAL_LIT;
		break;

	case ',':
		t->code = COMMA;
		t->type = REAL_LIT;
		break;

	case '.':
		if('.' == *((t->value) + 1))
		{
			t->code = DOTDOT;
			t->type = REAL_LIT;
		}
		else
		{
			t->code = PERIOD;
			t->type = REAL_LIT;
		}
		break;

	case '/':
		t->code = SLASH;
		t->type = REAL_LIT;
		break;

	case '<':
		if('=' == *((t->value) + 1))
		{
			t->code = LE;
			t->type = REAL_LIT;
		}
		else
		{
			t->code = LT;
			t->type = REAL_LIT;
		}
		break;

	case '>':
		if('=' == *((t->value) + 1))
		{
			t->code = GE;
			t->type = REAL_LIT;
		}
		else
		{
			t->code = GT;
			t->type = REAL_LIT;
		}
		break;

	case '!':
		if('=' == *((t->value) + 1))
		{
			t->code = NE;
			t->type = REAL_LIT;
		}
		else
		{
			t->code = NOT;
			t->type = REAL_LIT;
		}
		break;

	case EOF:
		t->code = END_OF_FILE;
		t->type = REAL_LIT;
		break;

	case '&':
		t->code = AND;
		t->type = REAL_LIT;
		break;

	case '|':
		t->code = OR;
		t->type = REAL_LIT;
		break;

	case '%':
		t->code = MOD;
		t->type = REAL_LIT;
		break;

	case '\0':
		t->code = NO_TOKEN;
		t->type = REAL_LIT;
		break;

		//ok, so Im pretty sure that there is a bug in here
	default:
		//If char is a number
		if(is_number(ch))
		{
			//then it's a number
			t->code = NUMBER;
			t->type = INTEGER_LIT;
		}
		//otherwise it is going to either be a real_lit or identifier
//Emily: Debug this section --------------------------------------------------------------------------------------------------------------------------------------------------------------------
		else
		{
			int i, j = 0, esc = 0;
			//if the length of the string is greater than 9, it cannot be a real lit
			if(strlen(t->value) > 9)
			{
				//therefore it is an identifier
				t->code = IDENTIFIER;
			}
			else
			{
				//while i < the width of the matrix
				for(i = 0; i < 11 && 0 == esc; i++)
				{
					//while j < the height of the matrix and escape == 0
					while(j < 8 && 0 == esc && NULL != rw_table[j][i].string)
					{
						//if the strings are not equal
						if(0 != strcmp(rw_table[j][i].string, t->value))
						{
							//go to the next row
							j++;
						}
						//if they are equal
						else
						{
							//incrament esc
							esc++;
						}
					}
				}
				//if no match was found
				if(0 == esc)
				{
					//then it is an identifier
					t->code = IDENTIFIER;
				}
				//moves j back one place
				j--;
				//sets the code to the appropriate token_code
				t->code = rw_table[j][i].token_code;
			}
			t->type = REAL_LIT;
		}
//End Section ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		break;
	};
}

