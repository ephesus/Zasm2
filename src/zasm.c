/*  Zasm2 Z80 assembler
    by James Lee Rubingh
    james@wrive.com

    Copyright (2002) James Rubingh
    Released under the GPL v2

    査読お願いします!
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "../config.h"

#include "../include/zasm.h"

//global variables, extern'd in zasm.h
int verbose = 0;
int tistring = 0;
int tiprog = 0;
struct label_entry* label_root;
struct label_entry *label_latest_unset;
struct label_entry *label_current;

void strip_comment(char *ptr)
{
  /*! replace any ; or \n with a terminating null */
  while (*ptr != '\0') {
    if ((*ptr == ';')||(*ptr == '\n')||(*ptr == ':')) {
      *ptr = '\0';
      break;
    }

    ptr++;
  }
}

/*! parse the table file and create a list 
  with all of the possible instructions */
struct tab_entry *read_table(FILE *tabfile) {
  struct tab_entry *root, *temp = NULL;
  char buffer[TABFILE_BUFFER_SIZE];
  char hexbuf[TABFILE_BUFFER_SIZE];
  char *buf, *endptr;

  while (fgets(buffer, TABFILE_BUFFER_SIZE, tabfile)) {
    if (buf = (char *) strtok(buffer, tab_whitespace)) {
      temp = (struct tab_entry *) malloc(sizeof(struct tab_entry));
      memset(temp, 0, sizeof(struct tab_entry));
      strcpy(temp->mnumonic, buf);
      temp->opcode = -1;
      temp->size = -1;
      strcpy(hexbuf,"0x");

      while ((buf = (char *) strtok(NULL, tab_whitespace))) {
        if (*temp->operands == 0) {
          strcpy(temp->operands, buf);
        } else if (*temp->hex_code == 0) {
          //fill in hex code and opcode
          strcpy(temp->hex_code, buf);
          strcpy(hexbuf+2, buf);
          temp->opcode = (unsigned int)strtol(hexbuf, (char **)NULL, 0);
        } else if (temp->size == -1){
          temp->size = atoi(buf);
        }
      }

#ifdef DEBUG
      printf("mnumonic: %s\n operand: %s\n hexcode: %s\n opcode: %d\n size: %d\n",
          temp->mnumonic, temp->operands, temp->hex_code, temp->opcode, temp->size);
#endif
    }
  }
#ifdef DEBUG
      puts("Done exporting Table List");
#endif

  return root;
}



int main(int ac, char **av)
{
  int c;
  FILE *infile=NULL, *outfile=NULL, *tabfile=NULL;
  struct instruction *root=NULL;
  struct tab_entry *tab_root=NULL;

  while (1) {
    int option_index=0;
    static struct option long_options[] = {
      {"version", 0, 0, 'V'},
      {"tabfile", 1, 0, 'T'},
      {"verbose", 0, 0, 'v'},
      {"help", 0, 0, 'h'},
      {"tiprog", 0, 0, 't'},
      {"tistrng", 0, 0, 's'},
      {0, 0, 0, 0}
    };

    c = getopt_long (ac, av, "vVdtsT:hp",
        long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 'h':
      case 'V':
        show_help(0);
        break;
      case 'v':
        verbose = 420;
        break;
      case 's':
        tistring = 420;
        break;
      case 'T':
        /* set up table file handle */
        if (!(tabfile = fopen(optarg, "r"))) {
          printf("Error opening tabfile: %s\n", optarg);
          exit(1);
        }
        break;
      default:
        /* invalid switch, so failing */
        exit(1);
    }
  }

  /* check if tabflie is open, if not use default */
  if (!tabfile){
    if (tabfile=fopen("./TASM80.TAB","r")){
      if (verbose)
        puts("using default tab file TASM80.TAB");
    } else {
      puts("Can't open TASM80.TAB file");
      exit(1);
    }
  }

  if (!(infile = fopen(av[optind], "r"))) {
    printf("error: with source file \"%s\"\n", av[optind]);
    show_help(1);
    exit(1);
  }

  if (!(outfile = fopen(av[optind+1], "w"))) {
    printf("error: with output file %s\n", av[optind+1]);
    show_help(1);
    exit(1);
  }

  if (verbose) {
    printf("infile:  %s\n", av[optind]);
    printf("outfile: %s\n", av[optind+1]);
  }

  /* read table file */
  tab_root = read_table(tabfile);

  assemble(tab_root, infile);

  return 0;
}

