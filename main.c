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

int main(int argc, const char * argv[])
{
    char source_name[MAX_FILE_NAME_LENGTH];
    char date[DATE_STRING_LENGTH];
    init_lister(argv[1], source_name, date);

	//initializes the scanner
    init_scanner(source_name);

	//while the end of the file is not reached
	while(EOF != read_next_line())
	{
		//prints the line
		print_line(get_file_line(), source_name, date, 0);
		//then the token
		print_tokens(get_token_line(), source_name, date);
	}
	//closes the scanner
	close_scanner();

	//returns 1
    return 1;
}

void init_lister(const char *name, char source_file_name[], char dte[])
{
    time_t timer;    
    strcpy(source_file_name, name);
    time(&timer);
    strcpy(dte, asctime(localtime(&timer)));
}

