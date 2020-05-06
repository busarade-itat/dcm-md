#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 protos.h -- function prototypes for grendel system
*****************************************************************************/

#ifndef __PROTOS_H_
#define __PROTOS_H_

/* lex.c functions */
extern BOOL lex_open(char *);
extern void lex_close(void);
extern int lex_line(void);
extern char *lex_file(void);
extern atom_t *lex(void);
extern atom_t *lex_word(void);
extern atom_t *safe_lex(void);
extern atom_t *safe_lex_word(void);
extern atom_t *last_lex(void);
extern void lex_verify(atom_t *, symbol_t *);

/* intern.c functions */
extern symbol_t *intern(const char *);
extern void show_hashtab(void);

/* types.c functions */
extern void fprint_symbol(FILE *, symbol_t *);
extern void show_symbol(symbol_t *);
extern void fprint_atom(FILE *, atom_t *);
extern void show_atom(atom_t *);
#define print_symbol(x) fprint_symbol(stdout,(x))
#define print_atom(x) fprint_atom(stdout,(x))

/* dataset functions */

/* mdb_load.c and mdb_util.c functions */
extern vec_t *ld_data(char *);
extern void copy_data(DATA *, DATA *);
extern DATA *new_data(int);
extern void sample_data(symbol_t *, int, DATA *, DATA **);
extern void count_class_freq(symbol_t *, vec_t *, ex_count_t *, ex_count_t *);
extern void remove_covered_examples(vec_t *, vec_t *);
extern void remove_uncovered_examples(vec_t *, vec_t *);
extern void stratify_and_shuffle_data(vec_t *, int);
extern void ith_stratified_partition(vec_t *, int, int, vec_t *, vec_t *);
extern void stratified_partition(vec_t *, int, vec_t *, vec_t *);

/* mdb_count.c functions */
void count_replaced_ruleset(vec_t *, int, symbol_t *, vec_t *, vec_t *,
							ex_count_t *, ex_count_t *, ex_count_t *, ex_count_t *);
void count_rule(vec_t *, symbol_t *, vec_t *,
				ex_count_t *, ex_count_t *, ex_count_t *, ex_count_t *);
extern void count_examples(vec_t *, symbol_t *, vec_t *, ex_count_t *, ex_count_t *);
extern double error_rate(concept_t *, vec_t *);
extern BOOL form_covers(vec_t *, vec_t *);
extern BOOL contains_symbol(symbol_t *, vec_t *);

/* mdb_names.c functions */
extern void set_names_defined();

extern BOOL ld_names(char *);
extern BOOL names_defined();
extern int n_fields();
extern int n_symbolic_values();
extern char *field_name(int);
extern BOOL continuous_field(int);
extern BOOL ignored_field(int);
extern BOOL symbolic_field(int);
extern BOOL set_field(int);
extern int n_different_field_values(int, DATA *);

/* mdb_opt.c functions */
extern int n_visited_symbols();
extern symbol_t *visited_symbol(int);
extern void compute_field_stats(symbol_t *, int, vec_t *);
extern double pos_symbol_gain(symbol_t *, double);
extern double neg_symbol_gain(symbol_t *, double);
extern void pos_field_stat(symbol_t *, ex_count_t *, ex_count_t *);
extern void neg_field_stat(symbol_t *, ex_count_t *, ex_count_t *);
extern double best_threshold(gsym_t *, symbol_t *, vec_t *, double, ex_count_t, ex_count_t, ex_count_t *, ex_count_t *);
extern symbol_t *best_symbol(gsym_t *, symbol_t *, vec_t *, double, ex_count_t, ex_count_t, ex_count_t *, ex_count_t *);

/* crossval.c functions */
extern void cross_validate(int, vec_t *, void (*)(vec_t *), double (*)(vec_t *));

/* add-redundancy.c functions */
extern void add_redundancy(concept_t *, DATA *);

/* extend-rules.c functions */
extern void extend_rules(FILE *, concept_t *, DATA *);

/* gram.c functions */
extern BOOL ld_grammar(char *, int, vec_t *);
extern void fshow_gsym(FILE *, gsym_t *);
extern gsym_t *ld_gsym(FILE *);
extern void fprint_gsym(FILE *, gsym_t *);
extern void print_grammar(void);
extern BOOL wildcard_threshold(gsym_t *);
BOOL contains_wildcard(gsym_t *);
#define print_gsym(g) fprint_gsym(stdout,(g))

/* gramaux.c functions */
extern BOOL complete_grammar(vec_t *);
extern BOOL index_grammar(vec_t *);
extern void print_grammar_indices(void);

/* desref.c functions */
extern void make_the_universe(symbol_t *, vec_t *);
extern vec_t *refinement(int);
extern vec_t *generalization(int);
extern vec_t *derivation(int);
extern int n_designated_refinements(void);
extern int n_designated_generalizations(void);
extern void compute_designated_refinements(vec_t *, vec_t *, int);
extern void compute_designated_generalizations(symbol_t *, vec_t *, vec_t *);
extern void print_designated_refinements(void);
extern void print_form(vec_t *);
extern void print_deriv(vec_t *);

/* rule.c functions */
extern void fprint_rule(FILE *, rule_t *);
extern void fshow_rule(FILE *, rule_t *);
extern void set_weight(rule_t *, int, int);
extern int rule_size(rule_t *);
extern int form_size(vec_t *);
extern void free_rule(rule_t *);
#define print_rule(r) fprint_rule(stdout,(r))

/* fit.c functions */
extern vec_t *fit(vec_t *, symbol_t *);
extern void refine(vec_t *, vec_t *, symbol_t *, vec_t *);

/* mdl.c functions */
extern void reduce_dlen(vec_t *, symbol_t *, vec_t *);
extern double ruleset_dlen(vec_t *, vec_t *, int, int);
extern double relative_compression(vec_t *, symbol_t *, vec_t *, int, vec_t *);

/* value.c functions */
extern double value(vec_t *, symbol_t *, vec_t *, vec_t *, double, ex_count_t, ex_count_t);
extern double information(ex_count_t, ex_count_t);
/* value functions to optimize */
extern double info_gain(vec_t *, double, ex_count_t, ex_count_t, ex_count_t, ex_count_t);
extern double pos_cvg(vec_t *, double, ex_count_t, ex_count_t, ex_count_t, ex_count_t);
extern double entropy_reduction1(vec_t *, double, ex_count_t, ex_count_t, ex_count_t, ex_count_t);

/* model.c functions */
extern concept_t *model(vec_t *);
extern void count_classes(vec_t *);
extern void reorder_classes(vec_t *);

/* concept.c functions */
extern void fprint_concept(FILE *, concept_t *);
extern void fshow_concept(FILE *, concept_t *);
extern concept_t *ld_concept(FILE *);
extern int concept_size(concept_t *);
extern void free_concept(concept_t *);
extern void count_concept(concept_t *, vec_t *);
extern symbol_t *classify(concept_t *, vec_t *);
extern symbol_t *classify_counts(concept_t *, vec_t *, ex_count_t *, ex_count_t *);
extern BOOL class_counts(concept_t *, vec_t *, symbol_t *, ex_count_t *, ex_count_t *);

#define print_concept(c) fprint_concept(stdout,(c))

/* main and mainaux functions */
extern char *add_ext(char *, char *);
extern void give_help(void);
#ifndef __GETOPT_H__
#ifndef SOLARIS
extern int getopt(int, char **, char *);
#endif
extern char *optarg;
extern int optind;
#endif

#endif
#ifdef __cplusplus
}
#endif