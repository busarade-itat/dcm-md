#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 mdb.h - include file for memory-based datamanager part of RIPPER
******************************************************************************/

/****************************************/

/* storing data: 
   instance: vector of attribute values 
   example: instance plus a class label 
   dataset: vector of instances 
*/

typedef enum {
    MISSING_VALUE,
    SYMBOL,
    CONTINUOUS,
    SET,
    IGNORE
} attr_kind_t;

typedef struct attribute_value_s {
    attr_kind_t kind;
    union val_u {
        double num;
        /* numeric value */
        symbol_t *nom;
        /* symbolic value */
        vec_t *set;     /* vector of symbols */
    } u;
} aval_t;

typedef struct example_s {
    vec_t *inst;
    /* vector of aval_t */
    atom_t lab;
    ex_count_t wt;
    unsigned sid; /* used by sequence count */
} example_t;

/* interpreting data:
   attr_def: atom, type, vector of possible values, min, max
*/

typedef struct symbol_tree_s {
    symbol_t *cont;
    int index;
    struct symbol_tree_s *left;
    struct symbol_tree_s *right;
} symbol_tree_t;

typedef struct attr_def_s {
    symbol_t *name;
    attr_kind_t kind;
    /* type check with min/max or values? */
    vec_t *values;
    /* vector of symbol_t--possible values */
    symbol_tree_t *value_index;   /* binary tree of atom_t */
    BOOL isbag;                   /* true if bag-valued attribute */
    BOOL suppressed;              /* store, don't use in learning,
				     but do use in computing MDL.
				     Used to reduce variability of ripper
				     in using different subsets of attributes
				   */
} attr_def_t;

extern vec_t *SID;

extern vec_t *Names;      /* vector of attr_def_t */

/* internally used names functions */
extern void print_attr_def(attr_def_t *);

extern void print_names(void);

extern BOOL verify_example(example_t *);

extern int attr_value_index(symbol_t *, attr_def_t *);

/* internally used dataset functions */
extern void swap_out_example(vec_t *, int);

extern void fprint_aval(FILE *, aval_t *);

extern void fprint_example(FILE *, example_t *);

extern void fprint_data(FILE *, vec_t *);

#define print_example(x) fprint_example(stdout,(x))
#define print_data(x) fprint_data(stdout,(x))

extern example_t *ld_example(atom_t *);

extern example_t *ld_unconstrained_example(atom_t *);

extern BOOL verify_infer_names(example_t *ex);

#ifdef __cplusplus
}
#endif