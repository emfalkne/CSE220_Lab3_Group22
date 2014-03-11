//
//  scanner.h
//  Lab3
//
//  Created by Bryce Holton.
//  Copyright (c) 2014 Bryce Holton. All rights reserved.
//

#ifndef Lab3_scanner_h
#define Lab3_scanner_h

#include "common.h"
#include "print.h"

void init_scanner(char source_name[]);
void close_scanner();
void clear_token_line();
char read_next_line();
char *get_file_line();
struct Token *get_token_line();
#endif
