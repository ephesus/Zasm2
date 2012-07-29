/*  Zasm2 Z80 assembler
 *  by James Lee Rubingh
 *  james@wrive.com
 *
 *  Copyright (2002) James Rubingh
 *  Released under the GPL v2
 *
 *  査読お願いします!
 */

int assemble(struct tab_entry *, FILE *);

static char *calculate_query_string(struct instruction *);
static char *remove_whitespace(char *);

static int add_symbol(struct instruction *);
static int apply_table(struct instruction *, struct tab_entry*);
static int check_for_symbol(const char *);
static int pass_second(struct instruction *);
static int validate_label(char *);

static struct instruction *get_operands(struct instruction *);
static struct instruction *new_instruction();
static struct label_entry *new_label();
static struct instruction *pass_first(FILE *, struct tab_entry*);
static struct instruction *parse_source(FILE *, struct instruction*, struct tab_entry*);
static struct tab_entry *look_with_query_string(char *, struct tab_entry *);
static struct tab_entry *match_mnumonic(struct tab_entry *, struct instruction *);
static struct tab_entry *match_operands_to_mnumonic(struct tab_entry *, const char *);

static void append_string(char *, const char *);
static void attach_label(char *, struct instruction *);
static void calculate_opcode(struct tab_entry*, struct instruction *);
static void found_correct_tab_entry(struct instruction *, struct tab_entry *);

enum Z_DIRECTIVE {
    Z_DB,
    Z_DW,
    Z_ORG,
    Z_END
};

#define NUM_SYMBOLS_TO_CHECK 26
const char *REGISTERS[] = {
    //registers
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "L",
    "BC",
    "DE",
    "HL",
    "AF",
    "IX",
    "IY",
    "IXL",
    "IYL",
    "IXH",
    "IYH",
    "PC",
    "SP",
     //flags
    "NC",
    "NZ",
    "Z",
    "R",
    "PE",
    "PO",
};


