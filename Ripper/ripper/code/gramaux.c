/******************************************************************************
 gramaux.c - fill in default definitions in a grammar
******************************************************************************/

#include <stdio.h>
#include "ripper.h"

/* global data */
int N_nonterms;
int N_eff_terms;
int N_suppressed_terms;
int N_intervals=0;
BOOL Eq_negations=FALSE;
BOOL Set_negations=FALSE;
int *Lo_rule;
int *Hi_rule;
symbol_t **Nonterm;
FLAG *Derives_emptystr;

static symbol_t *Body=NULL,*Conds,*Cond;
static atom_t *Wildcard;

#define rules_for(x)  ((x)->kind==NONTERM)

/* locally used functions */
static int grule_cmp(grule_t *,grule_t *);
static BOOL add_default_defn(symbol_t *,vec_t *);
static BOOL define_body(vec_t *);
static BOOL define_conds(vec_t *);
static BOOL define_cond(vec_t *);
static BOOL define_cond_a(symbol_t *,int,vec_t *);
static void add_rule_nn(symbol_t *,symbol_t *,symbol_t *);
static void add_rule_t(symbol_t *,int,int,atom_t *,double);

/*****************************************************************************/

BOOL complete_grammar(data)
vec_t *data;
{
    int i,j;
    BOOL complete;
    vec_t *rhs;
    gsym_t *gsym;

    Body = NULL;
    Conds = NULL;
    Cond = NULL;

    if (!Body) {
	Body=intern("body");
	Conds=intern("body_conds");
	Cond=intern("cond");
	Wildcard = newmem(1,atom_t);
	Wildcard->nom = intern("*");
	Wildcard->num = 0.0;
    }

    /* N_suppressed_terms is side effected in 
       completing the grammar-- yuck! */
    N_suppressed_terms = 0;

    //if (!rules_for(Body))
        define_body(data);

    do {
	complete = TRUE;
	for (i=0; i<vmax(Grammar); i++) {
	    rhs = vref(grule_t,Grammar,i)->rhs;
	    for (j=0; j<vmax(rhs); j++) {
		gsym = vref(gsym_t,rhs,j);
		if (gsym->nonterm && gsym->nonterm->kind!=NONTERM) {
		    complete = FALSE;
		    if (!add_default_defn(gsym->nonterm,data)) return FALSE;
		}
	    }
	}
    } while (!complete);
    return TRUE;
}

 /* sort rules, index the grammar, perform static analysis */
BOOL index_grammar(vec_t* data)
{
    int i,j,k,m;
    symbol_t *lhs, *last_lhs;
    vec_t *rhs;
    BOOL found_one, rhsj_derives_emptystr;
    gsym_t *c;
    REAL last,this;
    atom_t *val;
    atom_t *vk;

    /* sort the grammar rules by nonterminal name */
    for (i=0; i<vmax(Grammar); i++) {
	vref(grule_t,Grammar,i)->user_order = i;
    }
    vsort(grule_t,Grammar,grule_cmp);

    /* assign an index to each nonterm */
    N_nonterms = 0; 
    last_lhs = NULL;
    for (i=0; i<vmax(Grammar); i++) {
	lhs = vref(grule_t,Grammar,i)->lhs;
	if (lhs!=last_lhs) {
	    lhs->index = N_nonterms++;
	    last_lhs = lhs;
	}
    }

    /* count the terminal symbols in the grammar */
    N_eff_terms = N_suppressed_terms;
    for (i=0; i<vmax(Grammar); i++) {
	rhs = vref(grule_t,Grammar,i)->rhs;
	for (j=0; j<vmax(rhs); j++) {
	    c = vref(gsym_t,rhs,j);
	    if (!c->nonterm) {
		/* a terminal of the form X>=? can be bound to 
		 * many different values, so it's wrong to count
		 * it the same as a constant test */
		if (!contains_wildcard(c)) {
		    N_eff_terms++;
		} else {
		    N_eff_terms += n_different_field_values(c->attr_index,data);
		} 
	    } /* if c is terminal */
	} /* for symbol j in rhs */
    } /* for rule i in grammar */

    /* save index of first and last rule for each nonterm */
    if (Lo_rule) freemem(Lo_rule);
    if (Hi_rule) freemem(Hi_rule);
    if (Nonterm) freemem(Nonterm);

    Lo_rule = newmem(N_nonterms,int);
    Hi_rule = newmem(N_nonterms,int);
    Nonterm = newmem(N_nonterms,symbol_t *);
    last_lhs = NULL;
    for (i=0; i<vmax(Grammar); i++) {
	lhs = vref(grule_t,Grammar,i)->lhs;
	if (lhs!=last_lhs) {
	    Nonterm[lhs->index] = lhs;
	    Lo_rule[lhs->index] = i;
	    if (last_lhs) Hi_rule[last_lhs->index] = i;
	    last_lhs = lhs;
	}
    }

    Hi_rule[last_lhs->index] = vmax(Grammar);

    /* static analysis: look for nonterms i with a rhs that
     * contains only nonterms j : Derives_emptystr[j] is true,
     * and then mark Derives_emptystr[i]=TRUE.  repeat until
     * nothing new can be marked.
    */
    Derives_emptystr = newmem(N_nonterms,FLAG);
    for (i=0; i<N_nonterms; i++) Derives_emptystr[i]=FALSE;
    do {
	found_one = FALSE;
	for (i=0; i<N_nonterms && !found_one; i++) {
	    if (Derives_emptystr[i]) continue;
	    for (j=Lo_rule[i]; j<Hi_rule[i] && !Derives_emptystr[i]; j++) {
		rhs = vref(grule_t,Grammar,j)->rhs;
		rhsj_derives_emptystr = TRUE;
		for (k=0; k<vmax(rhs) && rhsj_derives_emptystr; k++) {
		    c = vref(gsym_t,rhs,k);
		    if (!c->nonterm || !Derives_emptystr[c->nonterm->index]) {
			rhsj_derives_emptystr = FALSE;
		    }
		} /*for condition k*/
		if (rhsj_derives_emptystr) {
		    Derives_emptystr[i] = TRUE;
		    found_one = TRUE;
		}
	    } /* for rule k */
	} /* for nonterm i */
    } while (found_one);

    return TRUE;
}

static int grule_cmp(r1,r2)
grule_t *r1, *r2;
{
    int tmp;

    tmp = strcmp(r1->lhs->name,r2->lhs->name);
    if (tmp==0) return r1->user_order - r2->user_order;
    else return tmp;
}

static BOOL add_default_defn(s,data)
symbol_t *s;
vec_t *data;
{
    symbol_t *a;

    if (s==Body) {
	return define_body(data);
    } else if (s==Conds) {
	return define_conds(data);
    } else if (s==Cond) {
	return define_cond(data);
    } else if (!strncmp(s->name,"cond_",5)) {
	a = intern(s->name+5);
	if (a->kind==ATTRIBUTE) { 
	    return define_cond_a(s,a->index,data);
	} else {
	    error("undefined nonterminal %s (%s not an attribute)",
		  s->name,a->name);
	    s->kind = NONTERM; /* prevent looping */
	    return FALSE;
	}
    } else {
	error("undefined nonterminal %s",s->name);
	s->kind = NONTERM; /* prevent looping */
	return FALSE;
    }
}

static BOOL define_body(data)
vec_t *data;
{
    add_rule_nn(Body,Conds,NULL);
    //if (!rules_for(Conds))
        return define_conds(data);
    //else return TRUE;
}

static BOOL define_conds(data)
vec_t *data;
{
    add_rule_nn(Conds,NULL,NULL);
    add_rule_nn(Conds,Cond,Conds);
    //if (!rules_for(Cond))
        return define_cond(data);
    //else return TRUE;
}

static BOOL define_cond(data)
vec_t *data;
{
    int i;
    symbol_t *cond_a;
    char buf[BUFSIZ];
    int mul;

    /* for each attribute a, add a rule r: Cond-->cond_a */
    for (i=0; i<n_fields(); i++) {
        if (suppressed_field(i)) {
            /* record how many nonterminals are being 
	       "suppressed" for MDL stuff 
	    */
	    if (continuous_field(i)) mul = 2;
	    else if (set_field(i) && Set_negations) mul = 2;
	    else if (symbolic_field(i) && Eq_negations 
		     && n_different_field_values(i,data)>2)
	      mul = 2;
	    else 
	      mul = 1;
	    N_suppressed_terms += mul * n_different_field_values(i,data);
        } else if (!ignored_field(i)) {
	    sprintf(buf,"cond_%s",field_name(i));
	    cond_a = intern(buf);
	    add_rule_nn(Cond,cond_a,NULL);
	    /* check that cond_a is defined */
	    //if (!rules_for(cond_a))
            define_cond_a(cond_a,i,data);
	}
    }
    /* succeed if any rules have been added */
    if (!rules_for(Cond)) {
	warning("cannot complete grammar: no non-ignored, non-suppressed attributes");
	return FALSE;
    } else {
	return TRUE;
    }
}

static int rless_than(r1,r2)
REAL *r1, *r2;
{
    if (*r1>*r2) return 1;
    else if (*r1<*r2) return -1;
    else return 0;
}

static BOOL define_cond_a(cond_a,index,data)
symbol_t *cond_a;
int index; /* index of attribute a */
vec_t *data;
{
    int i,deltai;
    double r;
    char buf[BUFSIZ];

    if (symbolic_field(index)) {
	add_rule_t(cond_a,index,OPEQ,Wildcard,0.0);
	if (Eq_negations && n_different_field_values(index,data)>2)
	  add_rule_t(cond_a,index,OPNEQ,Wildcard,0.0);
    } else if (continuous_field(index)) {
	add_rule_t(cond_a,index,OPLE,Wildcard,0.0);
	add_rule_t(cond_a,index,OPGE,Wildcard,0.0);
    } else if (set_field(index)) {
	add_rule_t(cond_a,index,OPIN,Wildcard,0.0);
	if (Set_negations) 
	  add_rule_t(cond_a,index,OPOUT,Wildcard,0.0);
    } else {
	fatal("unknown field type"); 
    }
    return TRUE;
}

/* add a rule r: lhs->nt1,nt2 where nti are nonterminals 
   to the grammar, and mark the lhs as defined */

static void add_rule_nn(lhs,nt1,nt2)
symbol_t *lhs, *nt1, *nt2;
{
    grule_t rule;
    gsym_t gsym;

    /* mark as defined */
    lhs->kind = NONTERM;
    /* build the rule */
    rule.priority = 0;
    rule.lhs = lhs;
    rule.rhs = new_vec(gsym_t);
    if (nt1!=NULL) {
	gsym.nonterm = nt1;
	ext_vec(gsym_t,rule.rhs,&gsym);
    }
    if (nt2!=NULL) {
	gsym.nonterm = nt2;
	ext_vec(gsym_t,rule.rhs,&gsym);
    }
    /* add it to the grammar */
    ext_vec(grule_t,Grammar,&rule);
}

/* add a rule r: lhs-> attr op val num where num is used only
   if val is NULL */

static void add_rule_t(lhs,attrx,op,val,r)
symbol_t *lhs;
int attrx;
int op;
atom_t *val;
double r;
{
    grule_t rule;
    gsym_t gsym;

    /* mark as defined */
    lhs->kind = NONTERM;
    /* build the rule */
    rule.priority = 0;
    rule.lhs = lhs;
    rule.rhs = new_vec(gsym_t);
    gsym.nonterm = NULL;
    gsym.attr_index = attrx;
    gsym.op = op;
    if (val) {
	ripper_copy(atom_t,&gsym.value,val);
    } else {
	gsym.value.nom = NULL;
	gsym.value.num = r;
    }
    ext_vec(gsym_t,rule.rhs,&gsym);
    /* add it to the grammar */
    ext_vec(grule_t,Grammar,&rule);
}

/*****************************************************************************/

void print_grammar_indices()
{
    int i;
    for (i=0; i<N_nonterms; i++) {
	printf("nonterm %d: %s (lo %d hi %d derives_e %c)\n",
	       i,Nonterm[i]->name,
	       Lo_rule[i],Hi_rule[i],
	       "ft"[Derives_emptystr[i]]);
    }
}
