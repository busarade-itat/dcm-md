/******************************************************************************
 gram.c - load and print grammars
******************************************************************************/

#include <stdio.h>
#include "ripper.h"

/*****************************************************************************/

/* global data managed in this file */
vec_t *Grammar = NULL;

int Max_priority;

/* how to print operators */
static char *Op_name[] = { "(null op)", "=", "!=", "<=", ">=", " ~ ", " !~ "};

/*****************************************************************************/

/* recursive-descent parser for grammars

 syntax:

 grammar_file ::- rule*
 rule ::- atom --> rhs
 rhs ::- stop
 rhs ::- gsym (',' gsym)*

 gsym ::- !atom
 gsym ::- atom [ op atom ]

 op ::- '=' 
 op ::- '!=' 
 op ::- '>=' 
 op ::- '<=' 

 convention: everything except ld_rule advances past the last token 
  of the constituent
*/

static void ld_rule(atom_t *,vec_t *);
static vec_t *ld_rhs(atom_t *);
static gsym_t *ld_condition(atom_t *);
static symbol_t *operator(atom_t *);
static int mark_priority(symbol_t *);

/* punctuation -- used globally */
static BOOL Initialized=FALSE;
static symbol_t *Arrow,*Bar,*Stop,*Sep,*Wildcard;

BOOL ld_grammar(file,is_default,data)
char *file;
int is_default;
vec_t *data;
{
    vec_t *grammar;
    atom_t *tok;
    symbol_t *op;

    if (!Initialized) {
	Initialized=TRUE;
	Wildcard = intern("*"); Wildcard->kind = WILDCARD;
	Arrow = intern("-->");
	Bar = intern("|");
	Stop = intern(".");
	Sep = intern(",");
	op = intern("=");
	op->kind = OPERATOR; op->index = OPEQ;
	op = intern("!=");
	op->kind = OPERATOR; op->index = OPNEQ;
	op = intern("<=");
	op->kind = OPERATOR; op->index = OPLE;
	op = intern(">=");
	op->kind = OPERATOR; op->index = OPGE;
	op = intern("~");
	op->kind = OPERATOR; op->index = OPIN;
	op = intern("!~");
	op->kind = OPERATOR; op->index = OPOUT;
    }

    if (!Grammar) Grammar = new_vec(grule_t);
    else clear_vec(grule_t,Grammar);

    Max_priority = 0;

    if (!lex_open(file) && !is_default) {
	warning("can't open grammar file %s--using defaults",file) ;
    } else {
		/*
	while ((tok=lex())!=NULL) {
	    ld_rule(tok,Grammar);
	}
	lex_close();
		 */
    }

    if (!complete_grammar(data)) {
	error("grammar is incomplete");
	return FALSE;
    }

    if (!index_grammar(data)) {
	error("problem in indexing grammar");
	return FALSE;
    }

    return TRUE;
}

static void ld_rule(tok,grammar)
atom_t *tok;
vec_t *grammar;
{
    grule_t rule;
    int p;
    BOOL badrule = FALSE;

    if ((p = mark_priority(tok->nom))!=0) {
        rule.priority = p;
	if (p > Max_priority) Max_priority = p;
	tok = safe_lex();
    } else {
	rule.priority = 0;
    }

    if (!tok->nom) {
	lex_error("grammar rule right-hand side must be a symbol");
	rule.lhs = intern("???");
	badrule = TRUE;
    } else {
	rule.lhs = tok->nom;
    }
    if (tok->nom->kind && tok->nom->kind!=NONTERM) {
	lex_error("symbol '%s' from grammar file is used elsewhere",
		  tok->nom->name);
	warning("rules for grammar symbol '%s' will be ignored",
		tok->nom->name);
	badrule = TRUE;
    } else {
	rule.lhs->kind = NONTERM;
    }
    tok = safe_lex(); 
    lex_verify(tok,Arrow);
    tok = safe_lex();

    rule.rhs = ld_rhs(tok);
    if (!badrule) {
	ext_vec(grule_t,grammar,&rule);
    }

    while ((tok=last_lex())->nom==Bar) {
	tok=safe_lex();
	rule.rhs = ld_rhs(tok);
	if (!badrule) {
	    ext_vec(grule_t,grammar,&rule);
	}
    }
    lex_verify(tok,Stop);
}

static vec_t *ld_rhs(tok)
atom_t *tok;
{
    vec_t *rhs;
    gsym_t *s;

    rhs = new_vec(gsym_t);
    if (tok->nom!=Stop) {
	s = ld_condition(tok);
	if (s) ext_vec(gsym_t,rhs,s);
	while (last_lex()->nom==Sep) {
	    tok = safe_lex();
	    s = ld_condition(tok);
	    if (s) ext_vec(gsym_t,rhs,s);
	}
    }
    return rhs;
}

static gsym_t *ld_condition(tok)
atom_t *tok;
{
    static gsym_t sym;
    atom_t atm;
    symbol_t *n;

    sym.nonterm = NULL;
    sym.attr_index = NULLINDEX;
    sym.op = NULLOP;
    atm.nom = sym.value.nom = NULL;
    atm.num = sym.value.num = 0.0;

    if (!tok->nom) {
	lex_error("illegal grammar symbol %g ignored",tok->num);
	tok = safe_lex();
	return NULL;
    } else if (tok->nom && tok->nom->kind!=ATTRIBUTE) {
	sym.nonterm = tok->nom;
	tok = safe_lex();
	return &sym;
    } else /* tok->nom && tok->nom is an attribute */ {
	sym.attr_index = tok->nom->index;
	tok = safe_lex();
	if (n=operator(tok)) {
	    sym.op = (operator_t) n->index;
	    tok = safe_lex();
	    ripper_copy(atom_t,&sym.value,tok);
	    safe_lex();
	} else {
	    lex_error("expected an operator -- supplying '='");
	    sym.op = OPEQ;
	    tok = safe_lex();
	    ripper_copy(atom_t,&sym.value,tok);
	    safe_lex();
	}
	return &sym;
    }
}

static int mark_priority(mark) /* return n for token "!^n" else 0 */
symbol_t *mark;
{
    char *cp;
    int p=0;
    
    for (cp=mark->name; *cp; cp++) 
      if (*cp=='!') p++;
      else return 0;
    return p;
}

static symbol_t *operator(tok)  /* return NULL if not operator, else op name */
atom_t *tok;
{
    if (tok->nom->kind==OPERATOR) return tok->nom;
    else return NULL;
}

/*****************************************************************************/

void fprint_gsym(FILE *fp, gsym_t *sym)
{
    if (!sym) {
	fprintf(fp,"(null)");
    } else if (sym->nonterm) {
	fprint_symbol(fp,sym->nonterm);
    } else {
	fprintf(fp,"%s",field_name(sym->attr_index));
	fprintf(fp,Op_name[sym->op]);
	fprint_atom(fp,&(sym->value));
    }
}

void fshow_gsym(fp,sym)
FILE *fp;
gsym_t *sym;
{
    if (!sym) {
	fprintf(fp,"(null)");
    } else if (sym->nonterm) {
	fprint_symbol(fp,sym->nonterm);
    } else {
	fprintf(fp,"%s %s ",field_name(sym->attr_index),Op_name[sym->op]);
	if (sym->value.nom==NULL) fprintf(fp,"%g",sym->value.num);
	else fprintf(fp,"%s",sym->value.nom->name);
    }
}

gsym_t *ld_gsym(fp)
FILE *fp;
{
    gsym_t *gsym;
    symbol_t *sym,*op;
    char word_buf[BUFSIZ];
    char *ptr;
    double d,strtod(const char *,char **);

    if (!Initialized) {
	Initialized=TRUE;
	op = intern("=");
	op->kind = OPERATOR; op->index = OPEQ;
	op = intern("!=");
	op->kind = OPERATOR; op->index = OPNEQ;
	op = intern("<=");
	op->kind = OPERATOR; op->index = OPLE;
	op = intern(">=");
	op->kind = OPERATOR; op->index = OPGE;
	op = intern("~");
	op->kind = OPERATOR; op->index = OPIN;
	op = intern("!~");
	op->kind = OPERATOR; op->index = OPOUT;
    }


    fscanf(fp,"%s",word_buf); 
    if (strcmp(word_buf,"(null)")==0) {
	return NULL;
    } else if (strcmp(word_buf,".")==0) {
	return NULL;
    } else {
	gsym = newmem(1,gsym_t);
	/* attribute op value */
	sym = intern(word_buf);
	if (sym->kind!=ATTRIBUTE) {
	    error("bad attribute %s in rule condition",word_buf);
	    gsym->attr_index = 0;
	} else {
	    gsym->attr_index = sym->index;
	}
	fscanf(fp,"%s",word_buf); 
	sym = intern(word_buf);
	if (sym->kind!=OPERATOR) {
	    error("bad operator %s in rule condition",word_buf);
	    gsym->op = OPEQ;
	} else {
	    gsym->op = sym->index;
	}
	fscanf(fp,"%s",word_buf); 
	if (continuous_field(gsym->attr_index)) {
	    d = strtod(word_buf,&ptr);
	    gsym->value.nom = NULL;
	    if (ptr!=word_buf) {
		gsym->value.num = d;
	    }
	    else {
		lex_error("attribute '%s' should have a continuous value",
			  field_name(gsym->attr_index));
		gsym->value.num = 0;
	    }
	} else {
	    gsym->value.nom = intern(word_buf);
	}
    }
    /* printf("   condition: "); print_gsym(gsym); printf("\n"); */
    return gsym;
}

void print_grammar()
{
    int i,j;
    grule_t *rule;

    for (i=0; i<vmax(Grammar); i++) {
	rule = vref(grule_t,Grammar,i);
	for (j=0; j<rule->priority; j++) printf("!");
	print_symbol(rule->lhs); 
	printf(" --> "); 
	for (j=0; j<vmax(rule->rhs); j++) {
	    if (j>0) printf(",");
	    print_gsym(vref(gsym_t,rule->rhs,j));
	}
	printf(".\n");
    }
}

BOOL contains_wildcard(gsym)
gsym_t *gsym;
{
    return 
        gsym->nonterm==NULL 
	&& gsym->value.nom!=NULL 
	&& gsym->value.nom->kind==WILDCARD;
}

/*****************************************************************************/

#ifdef TEST
/* main: test driver
*/

int main(int,char**);

main(argc,argv)
int argc;
char *argv[];
{
    if (argc<=3) {
	fatal("syntax: test-grammar namesfile datafile grammarfile");
    }
    (void)ld_names(argv[1]);
    (void)ld_data(argv[2]);

    ld_grammar(argv[3],NULL);
    printf("\n\n**** names information ****\n\n");
    print_names();
    printf("\n\n**** grammar after completion ****\n\n");
    print_grammar();
    print_grammar_indices();
/*  printf("\n\n**** symbol table ***\n\n");
    show_hashtab();
*/
    return 0;
}
#endif

