/*  Zasm2 Z80 assembler
 *  by James Lee Rubingh
 *  james@wrive.com
 *
 *  Copyright (2002) James Rubingh
 *  Released under the GPL v2
 *
 *  査読お願いします!
 */

struct instruction *pass_first(FILE *, struct tab_entry*);
int pass_second(struct instruction *);
struct instruction *parse_source(FILE *, struct instruction*);
int assemble(struct tab_entry *, FILE *);
int apply_table(struct instruction *, struct tab_entry*);
int validate_label(char *);
void attach_label(char *, struct instruction *);
struct tab_entry *match_opcode(struct instruction *);
struct instruction *new_instruction();
struct label_entry *new_label();

