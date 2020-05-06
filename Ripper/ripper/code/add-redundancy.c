/******************************************************************************
 add-redundancy.c - add "redundant" conditions to a rule

 ie all all conditions that cover all positive examples of the rule,
 even those that add no additional discriminatory power
******************************************************************************/

#include <stdio.h>
#include "ripper.h"

BOOL Add_redundancy=0;
BOOL Force_independent_rules = FALSE;
static int Max_added_conditions=20; 

static void add_rule_redundancy(rule_t *,DATA *);
static void choose_wildcards(vec_t *,symbol_t *,DATA *);
static symbol_t *choose_wildcard(gsym_t *,symbol_t *,DATA *,vec_t *);

void add_redundancy(concept_t *c,DATA *data)
{
    DATA data1;
    rule_t *ri;
    int i;
    
    copy_data(&data1,data);
    for (i=0; i<vmax(c->rules); i++) {
	ri = vref(rule_t,c->rules,i); 
	trace(LONG) {
	    printf("// Adding redundancy to rule %d\n// ",i); 
	    print_rule(ri);
	    printf("\n");
	}
	add_rule_redundancy(ri,&data1);
	trace(SUMM) {
	    printf("// Redundant version of rule %d\n// ",i); 
	    print_rule(ri);
	    printf("\n");
	    fflush(stdout);
	}
	if (c->res!=BEST && !Force_independent_rules) { 
	    remove_covered_examples(ri->antec,&data1);
	}
    }
}

static void add_rule_redundancy(rule_t *r,DATA *data)
{
    DATA rule_data;
    ex_count_t p,n,p1,n1;
    int i;
    vec_t *form,*refi;
    vec_t *deriv;
    symbol_t *cl;
    double prev_info;
    BOOL refined;
    int s,s1;
    int n_added;
    
    /* optimize #positive examples covered, not gain */
    Value_function = &pos_cvg;

    cl = r->conseq;
    form = r->antec;
    deriv = r->deriv;
    refined = FALSE;
    copy_data(&rule_data,data);
    remove_uncovered_examples(form,&rule_data);
    count_examples(form,cl,&rule_data,&p,&n);
    s = form_size(form);
    prev_info = information(p,n);
    trace(LONG) {
	printf("// %d examples covered by rule\n",vmax(&rule_data)); 
	printf("// adding redundant conditions to (%g/%g):\n// ",p,n); 
	print_form(form); 
	printf("\n");
    }
    n_added = 0;
    do {
	compute_designated_refinements(form,deriv,0);
	refined = FALSE;
	for (i=0; i<n_designated_refinements() && !refined; i++) {

	    /* force a choice to be made for some missing values */
	    choose_wildcards(refinement(i),cl,&rule_data);

	    /* force choices for remaining ones */ 
	    (void)value(refinement(i),cl,&rule_data,form,p,n,prev_info);

	    /* decide if this refinement is worth keeping */
	    count_examples(refinement(i),cl,&rule_data,&p1,&n1);
	    s1 = form_size(refinement(i));
	    if (s1>s && p1==p) {
		n_added++;
		refined = TRUE;
		copy_vec(gsym_t,form,refinement(i));
		copy_vec(deriv_step_t,deriv,derivation(i));
		s = s1;
		if (n1<n) {
		    remove_uncovered_examples(form,&rule_data);
		    n = n1; 
		    prev_info = information(p,n);
		}
		trace(LONG) {
		    printf("// refined to (%g/%g): ",p1,n1); 
		    print_form(form); 
		    printf("\n");
		}
	    } else {
		trace(DBUG) {
		    printf("// invalid (%g,%g): ",p1,n1);
		    print_form(refinement(i));
		    printf("\n");
		}
	    }
	} /* for refinement i */
    } while (refined && n_added<Max_added_conditions);
    r->nposx = p;
    r->nnegx = n;
}

static void choose_wildcards(vec_t *form,symbol_t *cl,DATA *data)
{
    int i;
    atom_t best;
    gsym_t *gsymi;

    /* process the wildcard new forms */
    for (i=0; i<vmax(form); i++) {
	gsymi = vref(gsym_t,form,i);
	if (contains_wildcard(gsymi) 
	    && gsymi->op!=OPGE && gsymi->op!=OPLE) 
	{
	    gsymi->value.nom = choose_wildcard(gsymi,cl,data,form);
	}
    }
}

static symbol_t *choose_wildcard(gsym_t *gsym,symbol_t *cl,DATA *data,vec_t *form)
{
    int i,j;
    BOOL used;
    symbol_t *best_sym;
    double cvg,best_cvg;
    BOOL invert;
    symbol_t *sym;
    ex_count_t ptmp,ntmp;
    
    compute_field_stats(cl,gsym->attr_index,data);
    invert = (gsym->op == OPOUT) || (gsym->op == OPNEQ);

    /* find the symbol covering most positive examples  */
    best_cvg = -MAXREAL;
    best_sym = NULL;
    for (i=0; i<n_visited_symbols(); i++) {
	sym = visited_symbol(i);
	/* see if this symbol has been used before in the rule */
	for (used=FALSE,j=0;!used && j<vmax(form); j++) {
	    if (vref(gsym_t,form,j)->value.nom == sym) {
		trace(LONG) printf("// stop-listing %s\n",sym->name);
		used=TRUE;
	    }
	}
	if (!used) {
	    if (invert) neg_field_stat(sym,&ptmp,&ntmp);
	    else pos_field_stat(sym,&ptmp,&ntmp);
	    cvg = ptmp;
	    if (cvg > best_cvg) {
		best_cvg = cvg;
		best_sym = sym;
	    }
	    trace(DBUG) {
		pos_field_stat(sym,&ptmp,&ntmp);
		printf("// %scvg(%s)=%g (covers  %g/%g)\n",
		       invert?"n":"p",sym->name,cvg,ptmp,ntmp);
	    }
	} /* unused word */
    }
    return best_sym;
}
