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

const RwStruct rw_table[8][10] = {
    {{"do",DO},{"if",IF},{"in",IN},{"of",OF},{"or",OR},{"to",TO},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL}}, //Reserved words of size 2
    {{"and",AND},{"div",DIV},{"end",END},{"for",FOR},{"mod",MOD},{"nil",NIL},{"not",NOT},{"set",SET},{"var",VAR},{NULL,NIHIL}}, //Reserved words of size 3
    {{"case",CASE},{"else",ELSE},{"file",FFILE},{"goto",GOTO},{"then",THEN},{"type",TYPE},{"with",WITH},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL}}, //Reserved words of size 4
    {{"array",ARRAY},{"begin",BEGIN},{"const",CONST},{"label",LABEL},{"until",UNTIL},{"while",WHILE},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL}},  //Reserved words of size 5
    {{"downto",DOWNTO}, {"packed",PACKED},{"record",RECORD}, {"repeat",REPEAT},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL}},  // Reserved words of size 6
	{{"program", PROGRAM},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL}}, // Reserved words of size 7
    {{"function", FUNCTION},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL}}, // Reserved words of size 8
    {{"procedure", PROCEDURE},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL},{NULL,NIHIL}}  // Reserved words of size 9
};

//initializes the variables necessary to run scanner
void init_scanner(char source_name[])
{
    strcpy(src_name, source_name);
	clear_token_line();
    tl_head = NULL;
	fin = fopen(src_name, "r");
}

void close_scanner()
{
	fclose(fin);
	clear_token_line();
}

char read_next_line()
{
	char ch;
	clear_token_line();
	read_line(&ch);
	tokenize_line();
	return ch;
}

char *get_file_line()
{
	return line;
}

//used to convert the file into a char array
static void read_line(char *last)
{
	char stream_line[MAX_SOURCE_LINE_LENGTH];
	int i = 0;
	do
	{
		//gets a new char and increments i
		if(fread(&stream_line[i], 1, 1, fin) == 0)
		{
			stream_line[i] = EOF;
		}
		i++;
	}while('\n' != stream_line[i - 1] && EOF != stream_line[i - 1]);
	*last = stream_line[i - 1];
	//if it isn't, installs a null character at the end
	stream_line[i - 1] = '\0';
	//copies the data
	strcpy(line, stream_line);
}

struct Token *get_token_line()
{
	return tl_head;
}

void clear_token_line()
{
	rem_token(tl_head);
	tl_head = NULL;
}

static void rem_token(struct Token *t)
{
	if(t)
	{
		rem_token(t->next);
		free(t);
	}
}

static void tokenize_line()
{
	char *cur_ch = line;
	int adv = 0;
	struct Token *t = tokenize_word(&adv, cur_ch);
	if(!t)
	{
		t = (struct Token*)malloc(sizeof(struct Token));
		t->value[0] = '\0';
		t->next = NULL;
		add_token(t);
		t = NULL;
	}
	add_token(t);
	while(t)
	{
		cur_ch += adv;
		t = tokenize_word(&adv, cur_ch);
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
		num_adv = adv_to_valid(ch);
		if(0 == num_adv)
		{
			num_adv = 1;
		}
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
	while(*ch < 32 && '\0' != *ch)
	{
		ch++;
		num_adv++;
	}
	if('\\' == *ch && '\\' == *(ch + 1))
	{
		num_adv += adv_to_nl(ch);
		return num_adv;
	}
	return num_adv;
}

//skips the line
static int adv_to_nl(char *ch)
{
	int i = 0;
	while('\0' != *ch)
	{
		ch++;
		i++;
	}
	return i;
}

static int interpret_ch(char *dest, char *src)
{
	int i = identify(TOLOWERC(*src)), num_adv;
	switch(i)
	{
	case 1:
		num_adv= parse_word(dest, src);
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
	while(is_letter(TOLOWERC(*src)) || is_number(*src) || '_' == *src)
	{
		*dest = TOLOWERC(*src);
		dest++;
		src++;
		num_adv++;
	}
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
	switch(*src)
	{
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
	switch(ch)
	{
	case '\'':
		t->code = STRING;
		t->type = STRING_LIT;
		break;
	case '\"':
		t->code = STRING;
		t->type = STRING_LIT;
		break;
	case '-':
		if(is_number(*((t->value) + 1)))
		{
			t->code = NUMBER;
			t->type = INTEGER_LIT;
		}
		else
		{
			t->code = MINUS;
			t->type = REAL_LIT;
		}
		break;
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
	default:
		if(is_number(ch))
		{
			t->code = NUMBER;
			t->type = INTEGER_LIT;
		}
		else
		{
			int i = 0, j = 0, esc = 0;
			/*
			while('\0' != t->value[i])
			{
				i++;
			}
			*/
			i = strlen(t->value);
			if(i > 9)
			{
				t->code = IDENTIFIER;
			}
			else
			{
				for(i = 0; i < 11; i++)
				{
				while(j < 10 && 0 == esc && NULL != rw_table[j][i].string)
				{
					if(0 != strcmp(rw_table[j][i].string, t->value))
					{
						j++;
					}
					else
					{
						esc++;
					}
				}
				}
				if(0 == esc)
				{
					t->code = IDENTIFIER;
				}
				j--;
				t->code = rw_table[j][i].token_code;
			}
			t->type = REAL_LIT;
		}
		break;
	};


/*
    NO_TOKEN, UPARROW,	ERROR, ARRAY, BEGIN, CASE, CONST,
	DIV, DO, DOWNTO, ELSE, END, FFILE,
    FOR, FUNCTION, GOTO, IF, IN, LABEL, NIL, OF, PACKED,
    PROCEDURE, PROGRAM, RECORD, REPEAT, SET, THEN, TO, TYPE, UNTIL,
    VAR, WHILE, WITH,
*/
}