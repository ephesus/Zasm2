/*  Zasm2 Z80 assembler
    by James Lee Rubingh
    james@wrive.com

    Copyright (2002) James Rubingh
    Released under the GPL v2

    査読お願いします!
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/zasm.h"
#include "ctype.h"
#include "../config.h"

#include "pass.h"

/* starting new file */
int assemble(struct tab_entry *tabroot, FILE *infile)
{
  int result = 0;
  linenumber = 0;

  struct instruction *root;

  root = pass_first(infile, tabroot);
  result = pass_second(root);

  return result;
}

/** Go through the tree of instructions and put in opcodes from the
 * table for things that dont use labels
 */
int apply_table(struct instruction* root, struct tab_entry* table)
{
  struct instruction *tmp_i;
  struct tab_entry *tab_match;
  int num_of_instructions=0;

  if (root == NULL)
    return -1;

  tmp_i = root;
  while (tmp_i) {
    tab_match = match_opcode(tmp_i);
    tmp_i = tmp_i->next;
    num_of_instructions++;
  }
  return num_of_instructions;
}

struct tab_entry *match_opcode(struct instruction *instruction) 
{
  struct tab_entry *tmp_tab = NULL;

  printf("%s - ", instruction->mnumonic);
  printf("%ul\n", instruction->opcode);


  return tmp_tab;
}

/** go through the source file(s) and create a
 * data structure with all the info about instruction sizes
 * and opcodes for all instructions without labels
 */
struct instruction *pass_first(FILE *infile, struct tab_entry *table)
{
  struct instruction *root = NULL;
  label_root = NULL; 

  /** build up the basic tree from the source file(s)
   * descends recursively into included source files
   */
  root = parse_source(infile, root);
  apply_table(root, table);

  return root;
}

/** Parse text file and create
 * tree elements for each instruction 
 * appends instructions to the end of the tree it gets
 * so that you can call this more than once, to include
 * more than one file
 * if you call it passing a root that is null, it will return
 * the beginning of the tree
 * if you pass it something other than null, it returns a pointer
 * to the last link of the tree
 */
struct instruction *parse_source(FILE *infile, struct instruction* initial_root)
{

  struct instruction *cur_old =NULL, *cur = NULL;
  struct instruction *inst_root;
  char buffer[INSTRUCTION_BUFFER_SIZE];
  char b[INSTRUCTION_BUFFER_SIZE];
  char *buf, *ptr;
  int instructions = 0;
  int cur_op_num;

  inst_root = initial_root;
  buf = b;

  while (fgets(buffer, INSTRUCTION_BUFFER_SIZE, infile)) {
    strip_comment(buffer);
    linenumber++;

    if (cur == NULL) {
        cur = (struct instruction *) malloc(sizeof(struct instruction));
        cur->next = NULL;
        cur->operands = NULL;
    }


    if (isblank(buffer[0]) || (buffer[0] == '\n')) {
      /** if the first char is blank, treat as an instruction
       * or a blank line.
       */

      /** split up line, get instruction and operands */
      if ((buf = (char *) strtok(buffer, whitespace))) {
        instructions++;

        /* if first element, point root to it */
        if (!inst_root)
          inst_root = cur;

        /* attach new link to the end of list */
        if (cur_old)
          cur_old->next = cur;

        /* convert to uppercase */
        ptr = buf;
        do {
          *ptr = toupper(*ptr);
        } while (*(ptr++) != '\0');

        strcpy(cur->mnumonic, buf);

        /* check for operands */
        cur_op_num = 0;
        while ((buf = (char *) strtok(NULL, whitespace))) {
#ifdef DEBUG
          printf(":opnd: %s\n", buf);
#endif

          if (!cur->operands) 
            cur->operands = (char **) malloc(sizeof( char *));
          else 
            cur->operands = (char **) realloc(cur->operands, (sizeof(char *)*(cur_op_num+1)));

          cur->operands[cur_op_num] = (char *) malloc(strlen(buf) );
          strcpy(cur->operands[cur_op_num], buf);

          cur->op_num = ++cur_op_num;
        }
        cur->op_num = cur_op_num;
      }

      /* set cur_old for next iteration */
      cur_old = cur;
      cur = (struct instruction *) malloc(sizeof(struct instruction));
      cur->next = NULL;
      cur->operands = NULL;

    } else {
      /* the first char wasnt blank, so see if it's a label
       * or a declaration or whatever
       */
      if (validate_label(buffer)) {
        attach_label(buffer, cur);
      } else {
        do_error_msg(ERR_PARSE);
      }
    }

  }

  /* return either the root or the tail */
  return initial_root == NULL ? inst_root : cur;
}

/*  add a label_entry to the big list */
void attach_label(char *ptr, struct instruction *inst) {
  struct label_entry *tmp;

  tmp = (struct label_entry*) malloc(sizeof(struct label_entry));
  tmp->instruction = inst;

  if (label_root == NULL)
    label_root = tmp;
  else
    label_current->next = tmp;

  label_current = tmp;
  tmp->name = (char *)malloc(strlen(ptr) * sizeof(char));
  strcpy(tmp->name, ptr);
}

/* make sure that the label is a valid label with ascii chars
 * if it's not valid, return a false */
int validate_label(char *ptr) {
  int valid = 1;
  while (*ptr != '\0') {
    if (!isalnum(*ptr++))
      valid = 0;
  }
  return valid;
}

/**  Go back through the instruction tree and using the list of
 *  labels, patch up all the forward references
 */
int pass_second(struct instruction *root)
{
  struct label_entry *cur = NULL;
  struct instruction *instd = NULL;

  cur = label_root;
  while (cur) {
    printf("label: %s  :\n", cur->name);
    instd = cur->instruction;
    printf("   inst: %s  :\n", instd->mnumonic);

    cur = cur->next;
  }
  return 0;
}


