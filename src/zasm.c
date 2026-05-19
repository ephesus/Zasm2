/*  Zasm2 Z80 assembler
    by James Lee Rubingh
    james@wrive.com

    Copyright (2002) James Rubingh
    Released under the GPL v2
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "../config.h"

#include "../include/zasm.h"

//global variables, extern'd in zasm.h
int verbose = 0;
int tistring = 0;
int tiprog = 0;
double linenumber = 0;
unsigned int current_address = 0;
struct label_entry* label_root = NULL; //data segment should be initialized to 0, but lets not assume
struct label_entry *label_current = NULL;
struct symbol_entry* symbol_root = NULL;
struct symbol_entry* symbol_current = NULL;

void do_error()
{
    printf("Error: line %f - Closing\n", linenumber);
    exit(EIO);
}

void do_error_msg(char *message)
{
    printf("Error: line %.0f - %s\n", linenumber, message);
    exit(EIO);
}

// Helper to free the symbol table linked list
void free_symbols(struct symbol_entry *root) {
    struct symbol_entry *current = root;
    while (current != NULL) {
        struct symbol_entry *next_node = current->next;
        // 'name' points to 'tmp_i->mnumonic' (statically allocated array in instruction),
        // do NOT free(current->name) or it will double-free when the instruction is deleted.
        free(current);
        current = next_node;
    }
}

void free_labels(struct label_entry *root) {
    struct label_entry *current = root;
    while (current != NULL) {
        struct label_entry *next_node = current->next;
        if (current->name) {
            free(current->name); // Allocated with malloc in add_label()
        }
        free(current);
        current = next_node;
    }
}

// Helper to free the main instruction abstract syntax tree
void free_instructions(struct instruction *root) {
    struct instruction *current = root;
    while (current != NULL) {
        struct instruction *next_node = current->next;
        
        // Free the dynamically allocated array of operand strings (from get_operands)
        if (current->operands) {
            for (int i = 0; i < current->op_num; i++) {
                if (current->operands[i]) {
                    free(current->operands[i]);
                }
            }
            free(current->operands);
        }
        
        free(current);
        current = next_node;
    }
}

// free the malloc'd Z80 instruction definition table list
void free_tab_entries(struct tab_entry *root) {
    struct tab_entry *current = root;
    while (current != NULL) {
        struct tab_entry *next_node = current->next;
        free(current);
        current = next_node;
    }
}

void free_lists(struct instruction *inst_root, struct tab_entry *t_root)
{
    /* 1. Free symbol_root before instruction_root because s references i */
    if (symbol_root != NULL) {
        free_symbols(symbol_root);
        symbol_root = NULL; // Clear global pointer
        symbol_current = NULL;
    }

    /* 2. Free label_root (contains pointers to instructions) */
    if (label_root != NULL) {
        free_labels(label_root);
        label_root = NULL; // Clear global pointer
        label_current = NULL;
    }

    if (inst_root != NULL) {
        free_instructions(inst_root);
    }

    if (t_root != NULL) {
        free_tab_entries(t_root);
    }
}

struct tab_entry* new_tab_entry(char *buf)
{
    struct tab_entry *new_tab_entry;

    new_tab_entry =    (struct tab_entry *) malloc(sizeof(struct tab_entry));
    memset(new_tab_entry, 0, sizeof(struct tab_entry));
    strcpy(new_tab_entry->mnumonic, buf);
    new_tab_entry->opcode = -1;
    new_tab_entry->size = -1;
    return new_tab_entry;
}

/*! parse the table file and create a list
  with all of the possible instructions */
struct tab_entry *read_table(FILE *tabfile)
{
    struct tab_entry *root = NULL;
    struct tab_entry *temp = NULL;
    struct tab_entry *old_tmp = NULL;
    char buffer[TABFILE_BUFFER_SIZE];
    char hexbuf[TABFILE_BUFFER_SIZE];
    char *buf, *endptr;

    while (fgets(buffer, TABFILE_BUFFER_SIZE, tabfile))
    {
        if ((buf = (char *) strtok(buffer, tab_whitespace))) {
            temp = new_tab_entry(buf);
            strcpy(hexbuf,"0x");

            while ((buf = (char *) strtok(NULL, tab_whitespace)))
            {
                if (*temp->operands == 0) {
                    strcpy(temp->operands, buf);
                } else if (*temp->hex_code == 0) {
                    //fill in hex code and opcode
                    strcpy(temp->hex_code, buf);
                    strcpy(hexbuf+2, buf);
                    temp->opcode = (unsigned int)strtol(hexbuf, (char **)NULL, 16);
                } else if (temp->size == -1){
                    temp->size = atoi(buf);
                }
            }

#ifdef DEBUG2
            printf("mnumonic: %s\n operand: %s\n hexcode: %s\n opcode: %d\n size: %d\n",
                    temp->mnumonic, temp->operands, temp->hex_code, temp->opcode, temp->size);
#endif
            /* start linked list, or add to it  */
            if (root == NULL) {
                root = temp;
                old_tmp = temp;
            } else {
                old_tmp->next = temp;
                old_tmp = temp;
            }
        }
    }

    return root;
}



int main(int ac, char **av)
{
    int c;
    FILE *infile=NULL, *outfile=NULL, *tabfile=NULL;
    struct instruction *root=NULL;
    struct tab_entry *tab_root=NULL;

    while (1)
    {
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
                    optarg = optarg ? optarg : "No file";
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
        if ((tabfile=fopen("./TASM80.TAB","r"))){
            if ((verbose))
                puts("using default tab file TASM80.TAB");
        } else {
            puts("Can't open TASM80.TAB file");
            exit(1);
        }
    }

    if (!(infile = fopen(av[optind], "r"))) {
        if (av[optind]) {
            printf("error: with source file \"%s\"\n", av[optind]);
        } else {
            show_help(0);
        }
        exit(1);
    }

    if (!(outfile = fopen(av[optind+1], "w"))) {
        printf("error: with output file %s\n", av[optind+1]);
        exit(1);
    }

    if (verbose) {
        printf("infile:  %s\n", av[optind]);
        printf("outfile: %s\n", av[optind+1]);
    }

    /* read table file */
    tab_root = read_table(tabfile);

    root = assemble(tab_root, infile);

    if (write_to_file(root)) {
        printf("error\n");
    }

    free_lists(root, tab_root);

    fclose(tabfile); fclose(infile); fclose(outfile);

    return 0;
}

