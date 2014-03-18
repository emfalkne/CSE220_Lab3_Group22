//
//  main.c
//  Lab3
//
//  Created by Bryce Holton.
//  Copyright (c) 2014 Bryce Holton. All rights reserved.
//

#include <stdio.h>
#include "common.h"
#include "print.h"
#include "scanner.h"

void init_lister(const char *name, char source_file_name[], char dte[]);

int main()//int argc, const char * argv[])
{
    char source_name[MAX_FILE_NAME_LENGTH];
    char date[DATE_STRING_LENGTH];
	//for debugging ------------------------------------------------------------
	char *DEBUG = "NEWTON(1).PAS";
    init_lister(DEBUG, source_name, date);//argv[1], source_name, date);
	//--------------------------------------------------------------------------
	//initializes the scanner
    init_scanner(source_name);
	//initialies the output
	set_fout("output.txt");
	//while the end of the file is not reached
	while(EOF != read_next_line())
	{
		//prints the line
		print_line(get_file_line(), source_name, date);
		//then the token
		print_tokens(get_token_line(), source_name, date);
	}
	//closes the scanner
	close_scanner();
	//closes the printer
	close_fout();
	//returns 1
    return 1;
}

void init_lister(const char *name, char source_file_name[], char dte[])
{
    time_t timer;
    
    strcpy(source_file_name, name);
    //file = fopen(source_file_name, "r");
    time(&timer);
    strcpy(dte, asctime(localtime(&timer)));
}

