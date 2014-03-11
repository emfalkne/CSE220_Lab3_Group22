//
//  print.h
//  Lab3
//
//  Created by Bryce Holton.
//  Copyright (c) 2014 Bryce Holton. All rights reserved.
//

#ifndef Lab3_print_h
#define Lab3_print_h

#include "common.h"

void print_line(char *, char source_name[], char date_to_print[]);
void print_tokens(struct Token *t, char source_name[], char date[]);
void set_fout(char *fname);
void close_fout();

#endif
