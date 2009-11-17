struct instruction *pass_first(FILE *, struct tab_entry*);
int pass_second(struct instruction *);
struct instruction *parse_source(FILE *, struct instruction*);
int assemble(struct tab_entry *, FILE *);
int apply_table(struct instruction *, struct tab_entry*);
int validate_label(char *);
void attach_label(char *, struct instruction *);
struct tab_entry *match_opcode(struct instruction *);

