//
//  print.c
//  Lab3
//
//  Created by Bryce Holton.
//  Copyright (c) 2014 Bryce Holton. All rights reserved.
//

#include "print.h"
#define TOUPPERC(a) ((96 < a && a < 123) ? a - 32 : a)
static FILE *foutput;

const char* const SYMBOL_STRINGS[] =
{
    "<no token>", "<IDENTIFIER>", "<NUMBER>", "<STRING>",
    "^","*","(",")","-","+","=","[","]",":",";",
    "<",">",",",".","/",":=","<=",">=","<>","..",
    "<END OF FILE>", "<ERROR>",
    "AND","ARRAY","BEGIN","CASE","CONST","DIV","DO","DOWNTO",
    "ELSE","END","FILE","FOR","FUNCTION","GOTO","IF","IN",
    "LABEL","MOD","NIL","NOT","OF","OR","PACKED","PROCEDURE",
    "PROGRAM","RECORD","REPEAT","SET","THEN","TO","TYPE","UNTIL",
    "VAR","WHILE","WITH",
};

static void print_page_header(char source_name[], char date[]);
static char *itostr(int i);

void print_line(char line[], char source_name_to_print[], char date_to_print[])
{
    char save_ch;
    char *save_chp = NULL;
    static int line_count = MAX_LINES_PER_PAGE;
    
    if (++line_count > MAX_LINES_PER_PAGE)
    {
        print_page_header(source_name_to_print, date_to_print);
        line_count = 1;
    }
    if (strlen(line) > MAX_PRINT_LINE_LENGTH) 
    {
        save_chp = &line[MAX_PRINT_LINE_LENGTH];
    }
    if (save_chp)
    {
        save_ch = *save_chp;
        *save_chp = '\0';
    }
    fwrite(line, strlen(line), 1, foutput);
    fwrite("\n", 1, 1, foutput);
    if (save_chp)
    {
        *save_chp = save_ch;
    }
}
static void print_page_header(char source_name[], char date[])
{
    static int page_number = 0;
	fwrite("Page\t", strlen("Page\t"), 1, foutput);
    putchar(FORM_FEED_CHAR);
}
void print_tokens(struct Token *t, char source_name[], char date[])
{
    while(t)
	{
		char to_print[MAX_PRINT_LINE_LENGTH];
		int i;
		strcpy(to_print, "\t>> ");
		if(IDENTIFIER == t->code)
		{
			strcat(to_print, SYMBOL_STRINGS[1]);
			i = strlen(to_print);
			to_print[i++] = '\t';
			to_print[i] = '\0';
			strcat(to_print, t->value);
		}
		else if(NUMBER == t->code)
		{
			strcat(to_print, SYMBOL_STRINGS[2]);
			i = strlen(to_print);
			to_print[i++] = '\t';
			to_print[i++] = '\t';
			to_print[i] = '\0';
			strcat(to_print, t->value);
		}
		else if(STRING == t->code)
		{
			strcat(to_print, SYMBOL_STRINGS[3]);
			i = strlen(to_print);
			to_print[i++] = '\t';
			to_print[i++] = '\t';
			to_print[i] = '\0';
			strcat(to_print, t->value);
		}
		else
		{
			char str[MAX_TOKEN_STRING_LENGTH];
			int j = 0;
			strcpy(str, t->value);
			while(str[j])
			{
				str[j++] = TOUPPERC(str[j]);
			}
			strcat(to_print, str);
			i = strlen(to_print);
			to_print[i++] = '\t';
			to_print[i++] = '\t';
			to_print[i] = '\0';
			strcat(to_print, t->value);
		}
		print_line(to_print, source_name, date);
		t = t->next;
	}
}

void set_fout(char *fname)
{
	foutput = fopen(fname, "w");
}
void close_fout()
{
	fclose(foutput);
}
static char *itostr(int itobeconv)
{
	char temp[128], t;
	int index = 0, j = 0, i = itobeconv;
	temp[0] = '0';
	temp[1] = '\0';
	while(i > 0)
	{
		switch(i % 10)
		{
		case 0:
			temp[index] = '0';
			break;
		case 1:
			temp[index] = '1';
			break;
		case 2:
			temp[index] = '2';
			break;
		case 3:
			temp[index] = '3';
			break;
		case 4:
			temp[index] = '4';
			break;
		case 5:
			temp[index] = '5';
			break;
		case 6:
			temp[index] = '6';
			break;
		case 7:
			temp[index] = '7';
			break;
		case 8:
			temp[index] = '8';
			break;
		case 9:
			temp[index] = '9';
			break;
		};
		index++;
		i = i / 10;
	}
	if(index > 0)
	{
		temp[index--] = '\0';
	}
	while(j < index)
	{
		t = temp[j];
		temp[j++] = temp[index];
		temp[index--] = t;
	}
	return temp;
}