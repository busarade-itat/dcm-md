#ifndef __RIPPER_H_
#define __RIPPER_H_

/******************************************************************************
 ripper.h - main include file for RIPPER

 Modification history for RIPPER.

 Release 1. 
   Roughly as described in ML95 paper, with addition of a -t flag
     that changes the way data is represented for text problems.

 Release 2.
   Replaced -t option with new type of feature, "set"
   Reorganized much of the code.  mdb_* implements an abstraction,
     a "main-memory database" for holding sets of examples
   Subsampling via -M option
   Quick & dirty versions of test-rules, eliminate, and pprint-rules
   Changed mdl.c so that any rule that has error>50% (in a generalized
     sense for loss ratios) is automatically deleted. This shouldn't be
     necessary but it apparently is.

 Release 2.1
     Fixed a bug in mdb_opt.c that was preventing OPOUT from being used

 Release 2.2
     Extended rocchio-main.c to handle loss ratios
     Added -A option, and add-redundacy main program

 Release 2.3
     Updated everything, and fixed a divide-by-zero error
     that caused problems for Linux.

 Release 2.4
     Extended so that missing attributes can be tested for
     explicitly in rules; sped up hashing function; added
     "suppress" declaration in names file, which makes ripper
     behave more predictably when attributes are removed;
     and added an initial implementation of "boosting".

 Release 2.5
     Added -aunordered option - fixed bug in assignment of
       default classes for -au
     Fixed bug in grammar completion - loops when all attributes
       are ignored.
     Clarified attribute value/class overlap error message.
     Added -V option to predict. 
     Added 'bag' data type.
     Stopped saving value_index for set types.
     Fixed memory leak (thanks, Wei Fan!) and added compilation
      option SAVEMEMORY which lets vector types grow more slowly.
     Clarified an error message occuring when grammar rules 
      contain an attribute value 

  patch 1 - bug fix in mdb_names 

******************************************************************************/

#define VERSION "V1 Release 2.5 (patch 1)"

static char *COPYRIGHT  = "Copyright (c) AT&T Labs";

/* #define NDEBUG */
#include <assert.h>
#include <limits.h>
#include <math.h>
#include "extras.h" 

/*****************************************************************************/

/* some useful types and constants */

#define REAL float
#define MAXREAL ((float)HUGE_VAL)
#define HUGEINT INT_MAX
#define NULLINDEX (-1)
#define BOOL int
#define FLAG char
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/****************************************/

/* symbol type: a char string, plus some properties */
typedef enum { 
    OTHER=0,     /* type not yet determined */
    MISSING_MARK,/* the symbol "?" */
    WILDCARD,    /* the symbol "*" */
    ATTRIBUTE,   /* name of an attribute */
    VALUE,       /* value of an symbolic attribute or name of a element */
    OPERATOR,    /* name of an operator */
    NONTERM,     /* name of a nonterminal symbol */
    CLASS        /* name of a class */
} sym_kind_t;
/* see also: sym_kind_name in mdb_names.c */

typedef struct symbol_s {
    char *name;
    sym_kind_t kind;
    int index;
} symbol_t;

/****************************************/

/* operator types and names */

typedef enum { NULLOP, OPEQ, OPNEQ, OPLE, OPGE, OPIN, OPOUT} operator_t;

/****************************************/

/* atom type: contains real number or symbol */
typedef struct atom_s {
    symbol_t *nom;  /* NULL if numeric atom */
    REAL num;
} atom_t;

/****************************************/

/* a dataset is a vector of examples */
typedef vec_t DATA;
typedef float ex_count_t;

extern vec_t *Classes;           /* vector of atoms */
extern ex_count_t *Class_counts; /* vector of counts */
extern vec_t *Words;             /* vector of words */

/****************************************/

/* grammars
   grule: lhs, priority, vector of rhs gsyms
   gsym: logically:
    (symbol feature, atom op, atom value) or (symbol nonterm)
    nonterm==NULL indicates a terminal
*/

typedef struct grule_s {
    symbol_t *lhs;
    int priority;           /* learner uses low-priority rules first */
    vec_t *rhs;             /* vector of gsym_t */
    int user_order;         /* used to minimize impact of completion */
} grule_t;

typedef struct gsym_s {
    symbol_t *nonterm;      /* NULL if a terminal symbol */
    int attr_index;         /* index of attribute */
    operator_t op;          /* code for operator used */
    atom_t value;           /* value attribute is compared against */
} gsym_t;


/* data structures encoding the grammar */

extern vec_t *Grammar;      /* a vector of grule_t */
extern int Max_priority;    /* highest "priority" of a grammar rule */
extern int N_nonterms;      /* number of nonterminal symbols */
extern int N_eff_terms;     /* number of terminal symbols */

/* next few items are dynamically-allocated
 * arrays indexed by a nonterminal's index field */

extern int *Lo_rule;        /* array of int: index in Grammar of first rule */
extern int *Hi_rule;        /* array of int: index in Grammar of last rule */
extern FLAG *Derives_emptystr;  /* array of FLAG */
extern symbol_t **Nonterm;  /* array of (symbol_t *) */

/****************************************/

/* rules (if-then rules used in concepts)
  rule: antec -> conseq (nnegx, nposx, weight)
	deriv (used in simplification)
*/
typedef struct rule_s {
    symbol_t *conseq;
    vec_t *antec;       /* a vector of gsym_t */
    ex_count_t nposx;   /* #examples correctly covered */
    ex_count_t nnegx;   /* #examples incorrectly covered */
    vec_t *deriv;       /* a vector of deriv_step_t */
} rule_t;

/* one step in derivation of a rule */
typedef struct deriv_step_s {
    int posn;  /* place rule was applied */
    int rulex; /* index of grammar rule used */
} deriv_step_t;

/****************************************/

/* concepts 
   concept: 
       strategy,
       vector of rules,
       default class (nposx,nnegx)
*/
typedef enum {
    BEST,     /* use highest-weighted rule on conflicts */
    FIRST     /* use first rule on conflicts */
} strategy_t;

typedef struct concept_s {
    strategy_t res;         /* conflict resolution strategy */
    atom_t *def;            /* default class */
    ex_count_t nposx;       /* #examples default is right */
    ex_count_t nnegx;       /* #examples default is wrong */
    vec_t *rules;           /* vector of rule_t */
} concept_t;

/*****************************************************************************/

/* global parameters */

typedef enum { GIVEN,MDL,INCFREQ,DECFREQ,UNORDERED } order_t;

/* Modification de ripper */
extern BOOL Sequences;             /* use a SID to count the frequency of a rule */
extern BOOL Uniq_rule_class;       /* generate rule uniquely for the first class */

extern int N_intervals;            /* number of intervals to discretize into */
extern BOOL Simplify;              /* whether to use incremental REP */
extern BOOL Eq_negations;          /* inequality tests appear in rules if true */
extern BOOL Set_negations;         /* not-element-of to appear in rules if true */
extern order_t Class_ordering;     /* how to order classes */
extern int FW_irep;                /* use Furncrantz & Widmer IREP algorithm */            
extern double Max_decompression;   /* stopping criteria for adding rules */
extern int Initial_rule_limit;     /* limit on #rules used in pass 1 */
extern int Rule_limit_multiplier;  /* how to increase rule limit */ 
extern int Optimizations;          /* times to optimize the ruleset */
extern double MDL_theory_factor;   /* to multiply coding cost of theory by */
extern double FN_cost;             /* cost of false negative */
extern double FP_cost;             /* cost of false positive */
extern int Max_sample;             /* max size subsample to use in 
				      choosing conditions for rules, etc */
extern BOOL Add_redundancy;        /* add "redundant" conditions to rules */
extern BOOL Force_independent_rules;
extern int Min_coverage;           /* min coverage of a rule */

/* function to optimize in value() routines */

/* value_function: f(form,oldinfo,oldp,oldn,p,n) --> double */
typedef double 
  (*value_f)(vec_t *,double,ex_count_t,ex_count_t,ex_count_t,ex_count_t);

extern value_f Value_function;

/*****************************************************************************/

/* function prototypes */

#include "protos.h"
#endif
