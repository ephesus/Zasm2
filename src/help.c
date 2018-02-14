/*  Zasm2 Z80 assembler
 *  by James Lee Rubingh
 *  james@wrive.com
 *
 *  Copyright (2002) James Rubingh
 *  Released under the GPL v2
 *
 *  査読お願いします!
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/zasm.h"

#include "../config.h"

void show_help(int returncode) 
{
    puts("zasm (switches) [infile] [outfile]\n");
    printf("Zasm %s\n",PACKAGE_VERSION);
    puts("(C) 2002 James Lee Rubingh\n");

    puts("-v\tbe --verbose");
    puts("-V\tshow program --version");
    puts("-t\tcreate a ti86 program");
    puts("-s\tcreate a ti86 string");
    puts("-p\t(default) create flat binary, no formatting");
    exit (returncode);
}

