//
//  print.c
//  Lab3
//
//  Created by Bryce Holton.
//  Copyright (c) 2014 Bryce Holton. All rights reserved.
//

#include "print.h"
#define TOUPPERC(a) ((96 < a && a < 123) ? a - 32 : a)
//static FILE *foutput;

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

void print_line(char line[], char source_name_to_print[], char date_to_print[], int token)
{
	//statically stores the line number
    static int line_count = MAX_LINES_PER_PAGE;
	static int displayed_line_count = 1;
    //checks if the next line will exceed the maximum number of lines per page
    if (++line_count > MAX_LINES_PER_PAGE)
    {
		//if it does, calls print header and resets the line count
        print_page_header(source_name_to_print, date_to_print);
		line_count = 1;
    }

	if(!token)
	{
		//prints the line number and tabs
		printf("%d:\t", displayed_line_count++);
	}

	//if the string length exceed the max string length
    if (strlen(line) > MAX_PRINT_LINE_LENGTH) 
    {
		//creates a temp array and pointer used to seperate the strings
		char Temp[MAX_PRINT_LINE_LENGTH], *tptr;
		//index
		int i;
		//copies from line to temp
		for(i = 0; i < MAX_PRINT_LINE_LENGTH - 1; i++)
		{
			Temp[i] = line[i];
		}
		//adds a null terminator at the end
		Temp[i] = '\0';
		//prints the temp array
		printf("%s\n", Temp);

		//if the next character is not a null
		if(line[i] != '\0')
		{
			//sets the tptr to where we left off
			tptr = &line[i];
			//recursively calls itself until the char fits
			print_line(tptr, source_name_to_print, date_to_print, token);
		}
    }
	//if it does not exceed the max char length
	else
	{
		//it prints
		printf("%s\n", line);
	}
	/*
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
	*/
}
static void print_page_header(char source_name[], char date[])
{
    static int page_number = 0;
	/*
	fwrite("Page\t\n", strlen("Page\t\n"), 1, foutput);
    putchar(FORM_FEED_CHAR);
	*/
	page_number ++;    //increases the page number up by one.
	printf("%s\nPage: %d %s\n", source_name, page_number, date); //prints out a header with the file name, page number and the date
}

//prints the tokens given the head of a list
void print_tokens(struct Token *t, char source_name[], char date[])
{
	//while a token is present
    while(t)
	{
		//makes a string to store the line to print
		char to_print[MAX_PRINT_LINE_LENGTH];
		int i;
		//adds "\t>> " to the start of the line
		strcpy(to_print, "\t>> ");
		//if the code is identifier
		if(IDENTIFIER == t->code)
		{
			//adds "<identifier>" to the string
			strcat(to_print, "<IDENTIFIER>");
			i = strlen(to_print);
			//adds a '\t' and sets the end to '\0'
			to_print[i++] = '\t';
			to_print[i] = '\0';
			//adds the identifier value to the end of the string
			strcat(to_print, t->value);
		}
		//samething for number
		else if(NUMBER == t->code)
		{
			strcat(to_print, SYMBOL_STRINGS[2]);
			i = strlen(to_print);
			to_print[i++] = '\t';
			to_print[i++] = '\t';
			to_print[i] = '\0';
			strcat(to_print, t->value);
		}
		//samething for string
		else if(STRING == t->code)
		{
			strcat(to_print, SYMBOL_STRINGS[3]);
			i = strlen(to_print);
			to_print[i++] = '\t';
			to_print[i++] = '\t';
			to_print[i] = '\0';
			strcat(to_print, t->value);
		}
		//for keywords
		else
		{
			char str[MAX_TOKEN_STRING_LENGTH];
			int j = 0;
			//copies the value to str
			strcpy(str, t->value);
			//capitalizes all the letters
			while(str[j])
			{
				str[j++] = TOUPPERC(str[j]);
			}
			//adds this to the end of the output
			strcat(to_print, str);
			i = strlen(to_print);
			//adds two tabs and sets the last char to newline
			to_print[i++] = '\t';
			to_print[i++] = '\t';
			to_print[i] = '\0';
			strcat(to_print, t->value);
		}
		//prints the line
		print_line(to_print, source_name, date, 1);
		//goes to the next line
		t = t->next;
	}
}

/*
//initializes the output stream
void set_fout(char *fname)
{
	foutput = fopen(fname, "w");
}

//closes the output stram
void close_fout()
{
	fclose(foutput);
}
*/