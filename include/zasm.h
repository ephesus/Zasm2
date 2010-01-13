/* 
   This is a zasm header file.   
   */

#ifndef ZASM_H
#define ZASM_H

#define INSTRUCTION_BUFFER_SIZE 1000
#define TABFILE_BUFFER_SIZE 500
#define whitespace " \t\r\n,"
#define tab_whitespace " \t\r\n"
#define DEBUG 
#define ERR_PARSE "Couldn't parse"
#define ERR_MALLOC "Out of Memory"

typedef unsigned char byte;

struct instruction {
    char mnumonic[6];
    char **operands;
    unsigned int opcode;
    unsigned short int op_num;
    struct instruction *next, *previous;
} __attribute__((__packed__));

struct tab_entry {
    char mnumonic[6];
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
extern struct label_entry *label_latest_unset;
extern struct tab_entry *read_table(FILE*);
extern void strip_comment(char *);
extern void do_error_msg(char *);
extern void do_error();
extern void free_lists();
void post_process(int mode);
struct tab_entry *new_tab_entry(char *);

/* pass.c */
extern int pass(struct instruction *, struct tab_entry *, FILE * infile);

/* help.c */
extern void show_help(int ) __attribute__((__noreturn__));

#endif

