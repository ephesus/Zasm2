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
#include "../config.h"
#include "pass.h"

void strip_comment(char *ptr)
{
    /*! replace any ; or \n with a terminating null */
    while (*ptr != '\0')
    {
        if ((*ptr == ';')||(*ptr == '\n')||(*ptr == ':')) {
            *ptr = '\0';
            break;
        }

        ptr++;
    }
}

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

int write_to_file(struct instruction * root)
{
    struct instruction *instw;

    instw = root;

    while (instw) {
        instw = NULL;
    }
    return 0;
}

int check_for_symbol(const char *operand)
{
    int i;

    for (i=0; i<NUM_SYMBOLS_TO_CHECK; i++)
    {
        if (strcmp(REGISTERS[i], operand) == 0) {
            return 1;
        }
    }

    return 0;
}

void append_string(char *target, const char *addition)
{
    char *tmp;

    tmp = target;
    while (*tmp)
    {
        tmp++;
    }

    //this is a potential buffer overflow but, oh well
    strcpy(tmp, addition);
}

char *capitalize(char *buf)
{
    char *tmp;

    tmp = buf;

    while (*tmp)
    {
        if (isalpha(*tmp))
            *tmp = toupper(*tmp);
        tmp++;
    }

    return buf;
}

/* Figure out the generic string representing the individual
 * instruction in the TABLE file. The instruction is already
 * matched so it's only the operands
 */
char *calculate_query_string(struct instruction *tmp_i)
{
    char *query;
    char tmp_str[10];
    int i;

    query = (char *) malloc(INSTRUCTION_BUFFER_SIZE);
    query[0] = '\0';

    for (i = 0; i < tmp_i->op_num; i++)
    {
        if (i > 0)
            append_string(query, ",");

        if (check_for_symbol(tmp_i->operands[i])) {
            append_string(query, tmp_i->operands[i]);
        } else {
            //if first char is alphanumeric, it's not a register
            if (isalpha(tmp_i->operands[i][0])) {
                tmp_i->not_reduced =1;
                append_string(query, "*");
            } else if ((isdigit(tmp_i->operands[i][0])) || (tmp_i->operands[i][0] == '$')) {
                //if first char is digit or '$' assume it's a value
                append_string(query, "*");
            } else {
                //anything else, just copy. things like (HL) for example
                append_string(query,tmp_i->operands[i]);
            }
        }
    }

    return query;
}

struct tab_entry *match_operands_to_mnumonic(struct tab_entry *tab_match, const char *query_string)
{
    char current_mnumonic[MNUMONIC_TXT_LENGTH];
    struct tab_entry *tab_tmp;

    strncpy(current_mnumonic, tab_match->mnumonic, MNUMONIC_TXT_LENGTH);
    tab_tmp = tab_match;

    //loop until we get to the next mnumonic or the end
    while ((strcmp(current_mnumonic, tab_tmp->mnumonic) == 0) && (tab_tmp->next))
    {
        if (strcmp(tab_tmp->operands, query_string) == 0) {
            //return a matched instruction
            return tab_tmp;
        }
        tab_tmp = tab_tmp->next;
    }

    return NULL;
}

/* wasn't a label, so assume it's an instruction */
void calculate_opcode(struct tab_entry *tabroot, struct instruction *tmp_i)
{
    struct tab_entry *tab_match;
    struct tab_entry *tab_temp;
    char *query_string;

    if (tab_match = match_mnumonic(tabroot, tmp_i)) {
        //mnumonic is found in tab file
        query_string = calculate_query_string(tmp_i);

        //only if there are operands try to match them
        if (tmp_i->op_num)
            tab_match = match_mnumonic_with_query_string(query_string, tab_match);

        found_correct_tab_entry(tmp_i, tab_match);

        free(query_string);
    } else {
        // wasn't in tab file, preprocessor directive?
        if ((strcmp(tmp_i->mnumonic, ".DW") == 0) ||
                (strcmp(tmp_i->mnumonic, ".WORD") == 0)) {

        } else if ((strcmp(tmp_i->mnumonic, ".DB") == 0) ||
                (strcmp(tmp_i->mnumonic, ".BYTE") == 0)) {

        } else if ((strcmp(tmp_i->mnumonic, ".INCLUDE") == 0) ||
                (strcmp(tmp_i->mnumonic, ".REQUIRE") == 0)) {

        } else if (strcmp(tmp_i->mnumonic, ".ORG") == 0) {
            //set assumed PC register value

        } else {
            if (add_symbol(tmp_i)) {
                //reached word not in table file, also not understood by zasm2
                printf("bad instruction: %s\n", tmp_i->mnumonic);
                do_error_msg(ERR_PARSE);
            }
        }
    }
}

/* add symbol to symbol table (_textShadow = $f34e) etc */
int add_symbol(struct instruction *tmp_i)
{
    struct symbol_entry *cur;

    if (tmp_i->operands && (tmp_i->operands[0][0] == '=')) {
        if (validate_label(tmp_i->mnumonic)) {

            cur = new_symbol();
            cur->name = tmp_i->mnumonic;
            cur->instruction = tmp_i;

            //if first symbol, set it to root
            if (!symbol_root) {
                symbol_root = cur;
                symbol_current = cur;
            } else {
                symbol_current->next = cur;
                symbol_current = cur;
            }
            return 0;
        }
    }

    return 1;
}

struct tab_entry *match_mnumonic_with_query_string(char *query_string, struct tab_entry *tab_match)
{
    if (!(tab_match = match_operands_to_mnumonic(tab_match, query_string))) {
        //check if one operand is value or label
        printf("***query_string: %s\n", query_string);
        printf("something fell through - not good\n");
    }

    return tab_match;
}

struct tab_entry *match_mnumonic(struct tab_entry *tabroot, struct instruction *instruction)
{
    struct tab_entry *tmp_tab = NULL;

    tmp_tab = tabroot;

    while (tmp_tab)
    {
        if (strcmp(instruction->mnumonic, tmp_tab->mnumonic) == 0)
            return tmp_tab;

        tmp_tab = tmp_tab->next;
    }

    return NULL;
}

void found_correct_tab_entry(struct instruction *tmp_i, struct tab_entry *match)
{
    tmp_i->matched_tab = match;
}

struct symbol_entry *new_symbol()
{
    struct symbol_entry *tmp;

    if (!(tmp = (struct symbol_entry*) malloc(sizeof(struct symbol_entry)))) {
        do_error_msg(ERR_MALLOC);
    }
    memset(tmp, 0, sizeof(struct symbol_entry));
    return tmp;
}

struct label_entry *new_label()
{
    struct label_entry *tmp;

    if (!(tmp = (struct label_entry *) malloc(sizeof(struct label_entry)))) {
        do_error_msg(ERR_MALLOC);
    }
    memset(tmp, 0, sizeof(struct label_entry));
    return tmp;
}

struct instruction *new_instruction()
{
    struct instruction *cur;

    if (!(cur = (struct instruction *) malloc(sizeof(struct instruction)))) {
        do_error_msg(ERR_MALLOC);
    }

    memset(cur, 0, sizeof(struct instruction));

    return cur;
}

char *remove_whitespace(char * buf)
{
    int i, t;
    char tmp[INSTRUCTION_BUFFER_SIZE];

    i = t = 0;
    while (buf[i])
    {
        if (!isblank(buf[i]))
            tmp[t++] = buf[i];

        i++;
    }
    tmp[t] = '\0';
    strcpy(buf, tmp);
}

/** go through the source file(s) and create a
 * data structure with all the info about instruction sizes
 * and opcodes for all instructions without labels
 */
struct instruction *pass_first(FILE *infile, struct tab_entry *tabroot)
{
    struct instruction *root = NULL;
    label_root = NULL;

    /** build up the basic tree from the source file(s)
     * descends recursively into included source files
     */
    root = parse_source(infile, root, tabroot);

    return root;
}

/* current line has first token removed (instruction) and comments
 * removed. Now go through the rest and assume they're instructions
 * separated with ','
 */
struct instruction *get_operands(struct instruction *cur)
{
    int cur_op_num;
    char *buf;

    /* check for operands */
    cur_op_num = 0;
    while ((buf = (char *) strtok(NULL, comma)))
    {
        remove_whitespace(buf); //remove any tabs etc.
        if (strlen(buf) > 0) {
            if (!cur->operands)
                cur->operands = (char **) malloc(sizeof( char *));
            else
                cur->operands = (char **) realloc(cur->operands, (sizeof(char *)*(cur_op_num+1)));

            capitalize(buf);
            cur->operands[cur_op_num] = (char *) malloc(strlen(buf) );
            strcpy(cur->operands[cur_op_num], buf);

            cur->op_num = ++cur_op_num;
        }
    }
    cur->op_num = cur_op_num;
    return cur;
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
struct instruction *parse_source(FILE *infile, struct instruction* initial_root, struct tab_entry *tabroot)
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

    while (fgets(buffer, INSTRUCTION_BUFFER_SIZE, infile))
    {
        strip_comment(buffer);
        linenumber++;

        if (cur == NULL)
            cur = new_instruction();

        if (isblank(buffer[0]) || (buffer[0] == '\n') || (buffer[0] == '#') || (buffer[0] == '.')) {
            if (buffer[0] == '#') //make preprocessor directives .<directive>
                buffer[0] = '.';

            /** split line, get instruction and operands */
            if ((buf = (char *) strtok(buffer, whitespace))) {
                instructions++;

                if (!inst_root)
                    inst_root = cur;

                if (cur_old)
                    cur_old->next = cur;

                capitalize(buf);

                strncpy(cur->mnumonic, buf, MNUMONIC_TXT_LENGTH);

                get_operands(cur);
            }

            calculate_opcode(tabroot, cur);

            cur_old = cur;
            cur = new_instruction();

        } else {
            /* see if it's a valid label */
            if (strlen(buffer) > 0) {
                if (validate_label(buffer))
                    attach_label(buffer, cur);
                else
                    do_error_msg(ERR_BADLABEL);

            }
        }
    }

    /* return either the head or the tail */
    return initial_root == NULL ? inst_root : cur;
}

/*  add a label_entry to the big list */
void attach_label(char *ptr, struct instruction *inst)
{
    struct label_entry *tmp;

    tmp = new_label();
    tmp->instruction = inst;

    if (label_root == NULL)
        label_root = tmp;
    else
        label_current->next = tmp;  //label_current is global extern'd

    label_current = tmp;
    tmp->name = (char *)malloc(strlen(ptr) * sizeof(char));
    strcpy(tmp->name, ptr);
}

/* make sure that the label is a valid label with ascii chars
 * if it's not valid, return a false
 * */
int validate_label(char *ptr)
{
    int valid = 1;
    while (*ptr != '\0')
    {
        if (!(isalnum(*ptr) || (*ptr == '-') || (*ptr == '_') || (*ptr == '.') || (*ptr == '@')))
            valid = 0;
        ptr++;
    }
    return valid;
}

/**  Go back through the instruction tree and using the list of
 *  labels, patch up all the forward references
 */
int pass_second(struct instruction *root)
{
    struct label_entry *cur = NULL;
    struct symbol_entry *cur_sym = NULL;
    struct instruction *instd = NULL;
    int i = 0;

#ifdef DEBUG
    instd = root;
    //loop through instructions
    while (instd)
    {
        if (instd->not_reduced) {
            printf("   inst: %s  :\n", instd->mnumonic);
            printf("    matched_tab: %p  :\n",  instd->matched_tab);
            for(i = 0; i < instd->op_num; i++)
                printf("\t:opnd: %s\n", instd->operands[i]);
        }
        instd = instd->next;
    }

    cur = label_root;
    while (cur)
    {
        instd = cur->instruction;

        //print all labels and operands
        printf("label: %s  :\n", cur->name);
        printf("   inst: %s  :\n", instd->mnumonic);
        for(i = 0; i < instd->op_num; i++)
            printf("\t:opnd: %s\n", instd->operands[i]);


        cur = cur->next;
    }
#endif

    if (symbol_root) {
        cur_sym = symbol_root;
        while (cur_sym)
        {
            printf("symbol: %s %s\n", cur_sym->name, cur_sym->instruction->operands[0]);
            cur_sym = cur_sym->next;
        }
    }

    return 0;
}

