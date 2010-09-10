/* 

   This is a zasm header file.   
   */

#ifndef ZASM_H
#define ZASM_H

#define INSTRUCTION_BUFFER_SIZE 1000
#define TABFILE_BUFFER_SIZE 500
#define MNUMONIC_TXT_LENGTH 16
#define whitespace " \t\r\n,"
#define comma ","
#define tab_whitespace " \t\r\n"
#define DEBUG 
#define ERR_PARSE "Couldn't parse"
#define ERR_BADLABEL "Invalid Label"
#define ERR_MALLOC "Out of heap memory - seriously?"

typedef unsigned char byte;

struct instruction {
    char mnumonic[MNUMONIC_TXT_LENGTH];
    char **operands;
    unsigned int opcode;
    unsigned int address;
    unsigned short int op_num;
    unsigned short int assembled;
    unsigned short int not_reduced;
    struct tab_entry *matched_tab;
    struct instruction *next, *previous;
} __attribute__((__packed__));

struct tab_entry {
    char mnumonic[MNUMONIC_TXT_LENGTH];
    char operands[10];
    char hex_code[10];
    int opcode;
    int size;
    /* NOP R1 COMBINE etc */
    int type;
    /* not sure what these do */
    unsigned int UNKNOWN_VALUE;
    int UNKNOWN_VALUE2;
    int UNKNOWN_VALUE3;
    struct tab_entry *next;
} __attribute__((__packed__));

struct symbol_entry {
    char *name;
    struct instruction *instruction;
    unsigned int address;
    struct symbol_entry *next;
} __attribute__((__packed__));

struct label_entry {
    char *name;
    struct instruction *instruction;
    struct label_entry *next;
} __attribute__((__packed__));

/* zasm.c */
extern int verbose;
extern int tiprog;
extern int tistring;
extern double linenumber; 
extern struct label_entry *label_root;
extern struct label_entry *label_current;
extern struct symbol_entry *symbol_root;
extern struct symbol_entry *symbol_current;
extern struct tab_entry *read_table(FILE*);
extern void do_error_msg(char *);
extern void do_error();
static void free_lists();
struct tab_entry *new_tab_entry(char *);

/* pass.c */
extern unsigned int current_address;
extern void strip_comment(char *);

/* shunt.c */
extern int shunt(char *);
extern int shunt_error;

/* help.c */
extern void show_help(int ) __attribute__((__noreturn__));

#endif

