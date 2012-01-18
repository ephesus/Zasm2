/*  Zasm2 Z80 assembler
 *  by James Lee Rubingh
 *  james@wrive.com
 *
 *  Copyright (2002) James Rubingh
 *  Released under the GPL v2
 *
 *  査読お願いします!
 */

char *remove_whitespace(char *);

int apply_table(struct instruction *, struct tab_entry*);
int assemble(struct tab_entry *, FILE *);
int pass_second(struct instruction *);
int validate_label(char *);

struct instruction *get_operands(struct instruction *);
struct instruction *new_instruction();
struct label_entry *new_label();
struct instruction *pass_first(FILE *, struct tab_entry*);
struct instruction *parse_source(FILE *, struct instruction*);
struct tab_entry *match_opcode(struct tab_entry *, struct instruction *);

void attach_label(char *, struct instruction *);

