/******************************************************************************
 value.c - compute information gain and other statistics for a sentential form
******************************************************************************/

#include <stdio.h>
#include "ripper.h"

static void set_wildcards(vec_t *,symbol_t *,vec_t *,double,ex_count_t,ex_count_t,ex_count_t *,ex_count_t *);
static void best_wildcard(atom_t *,gsym_t *,symbol_t *,vec_t *,double,ex_count_t,ex_count_t,ex_count_t *,ex_count_t *);

#define supported_wildcard(g) (contains_wildcard(g) && (g)->op!=NULLOP)

/* function to optimize */
value_f Value_function = &info_gain;

/* foil's measure */
double info_gain(vec_t *form,double oldinfo,
		 ex_count_t oldp,ex_count_t oldn,ex_count_t p,ex_count_t n)
{
    if (oldn>0) return p*(oldinfo - information(p,n));
    else return pos_cvg(form,oldinfo,oldp,oldn,p,n);
}

/* coverage of positive examples */ 
double pos_cvg(vec_t *form,double oldinfo,
	       ex_count_t oldp,ex_count_t oldn,ex_count_t p,ex_count_t n)
{
    if (form!=NULL) {
	return ((double) p)/vmax(form);
    } else {
	return 	(double) p;
    }
}
#define H(p,n) (p*information(p,n)+n*information(n,p))/(p+n)
/*
 * used in Rob's experiments -- largest reduction in entropy
 * such that entropy of pos case is greater than entropy of
 * negative case.  This basically assumes that every condition
 * and its negation are added to rules.
*/ 
double entropy_reduction1(vec_t *form,double oldinfo,
			 ex_count_t oldp,ex_count_t oldn,
			 ex_count_t p,ex_count_t n)
{
    double H0,Hcov,Huncov;
    ex_count_t uncovp,uncovn;

    /* counts for examples not covered by the rule */ 
    uncovp = oldp - p;
    uncovn = oldn - n;
    /* entropies */
    H0 = H(oldp,oldn);
    Hcov = H(p,n);
    Huncov = H(uncovp,uncovn);
    if (Hcov<Huncov) return -MAXREAL;
    else return H0-Hcov;
}


/*****************************************************************************/

double value(vec_t *form,symbol_t *cl,vec_t *data,vec_t *oldform,
	     double old_info,ex_count_t oldp,ex_count_t oldn)
{
    int i,j,k;
    ex_count_t p,n;
    gsym_t *gsymj;
    double val;
    static vec_t *newform = NULL;
    BOOL contains_wildcard_value;

    /* removing a nonterminal doesn't change coverage */
    if (vmax(form)<vmax(oldform)) return 0.0;

    if (newform==NULL) newform = new_vec(gsym_t);
    else clear_vec(gsym_t,newform);

    /* compute newform = form\oldform: take advantage of fact 
     * that oldform = xAz and form = xyz, for some y: A-->* y  */
    for (i=0; i<vmax(oldform); i++) 
      if (!ripper_equal(gsym_t,vref(gsym_t,oldform,i),vref(gsym_t,form,i))) 
	break;
    for (j=0; j<vmax(form)-vmax(oldform)+1; j++) {
	gsymj = vref(gsym_t,form,i+j);
	if (!gsymj->nonterm) ext_vec(gsym_t,newform,gsymj);
    }
    
    /* check that there are some new conditions */
    if (vmax(newform)==0) return 0;

    /* see if there are any >=x or <=y wildcards in new_form */
    contains_wildcard_value=FALSE;
    for (j=0; j<vmax(newform) && !contains_wildcard_value; j++) {
	if (supported_wildcard(vref(gsym_t,newform,j))) {
	    contains_wildcard_value=TRUE;
	}
    }
    
    if (contains_wildcard_value) {
	trace(DBG1) {
	    printf("// wildcard values in "); 
	    print_form(form);
	    printf("\n");
	}
	/* find thresholds and put them in newform */
	set_wildcards(newform,cl,data,old_info,oldp,oldn,&p,&n);
	/* copy best wildcards from newform into form */
	for (k=0,j=0; j<vmax(form)-vmax(oldform)+1; j++) {
	    gsymj = vref(gsym_t,form,i+j);
	    if (!gsymj->nonterm) {
		vset(gsym_t,form,i+j,vref(gsym_t,newform,k++));
	    }
	}	
	trace(DBUG) {
	    printf("// wildcards set to "); 
	    print_form(form);
	    printf("\n");
	}
    } else {
	count_examples(newform,cl,data,&p,&n); 
    }
    val = (*Value_function)(form,old_info,oldp,oldn,p,n);
  
    trace(DBUG) {
	printf("// value of"); 
	print_form(newform); 
	printf(" = %g (%g/%g)\n",val,p,n);
    }

    return val;
}

double information(ex_count_t p,ex_count_t n)
{
    if (p==0) return 0.0;
    else return Log2((p+n+1.0)/(p+1.0));
}

/*****************************************************************************
 stuff to deal with finding values for terminals with wildcard values
 *****************************************************************************/

/* set each of the supported wildcard thresholds in form
 * to the value that optimizes gain on the given data; 
 * or set each of the wildcard words in form to the word
 * that optimizes gain
 * 
 * return the number of pos/neg examples covered by the
 * chosen value in pos and neg
*/

static void set_wildcards(vec_t *form,symbol_t *cl,vec_t *data,double old_info,
			  ex_count_t oldp,ex_count_t oldn,
			  ex_count_t *pos,ex_count_t *neg)
/* ex_count_t *pos, *neg hold output of function */
{
    int i,j;
    static vec_t *tmp_form=NULL;
    atom_t best;
    vec_t data1;
    gsym_t *gsymi;

    if (vmax(form)==1) {
	/* the easy case -- one wildcard */
	gsymi = vref(gsym_t,form,0);

	/* find best threshold and associated pos/neg coverage */
	best_wildcard(&best,gsymi,cl,data,old_info,oldp,oldn,pos,neg);

	/* save the wildcard into form */
	gsymi->value.nom = best.nom;
	gsymi->value.num = best.num;
    } else {
	/* the hard case--many wildcard thresholds, maybe some known also */

	copy_data(&data1,data);

	/* collect the known conditions, and apply to data1 */
	if (tmp_form==NULL) tmp_form = new_vecn(gsym_t,vmax(form));
	else clear_vec(gsym_t,tmp_form);
	for (i=0; i<vmax(form); i++) {
	    gsymi = vref(gsym_t,form,i);
	    if (!supported_wildcard(gsymi)) {
		ext_vec(gsym_t,tmp_form,gsymi);
	    }
	}
	remove_uncovered_examples(tmp_form,&data1);

	/* now process the new forms */
	for (i=0; i<vmax(form); i++) {
	    gsymi = vref(gsym_t,form,i);
	    if (supported_wildcard(gsymi)) {

		/* set the wildcard values */
		best_wildcard(&best,gsymi,cl,&data1,old_info,oldp,oldn,pos,neg);
		gsymi->value.nom = best.nom;
		gsymi->value.num = best.num;

		/* update data1, removing conditions not covered */
		clear_vec(gsym_t,tmp_form);
		ext_vec(gsym_t,tmp_form,gsymi);
		remove_uncovered_examples(tmp_form,&data1);
	    }
	}
    }/* hard case */
}


static void best_wildcard(atom_t *best,gsym_t *gsym,symbol_t *cl,vec_t *data,
			  double old_info,ex_count_t oldp,ex_count_t oldn,
			  ex_count_t *pos,ex_count_t *neg)
/* atom_t *best returns value */
/* ex_count_t *pos, *neg hold output of function */
{
    if (gsym->op==OPLE || gsym->op==OPGE) {
	best->num = best_threshold(gsym,cl,data,old_info,oldp,oldn,pos,neg);
	best->nom = NULL;
    } else if (gsym->op==OPIN || gsym->op==OPOUT 
	       || gsym->op==OPEQ || gsym->op==OPNEQ) 
    {
	best->nom = best_symbol(gsym,cl,data,old_info,oldp,oldn,pos,neg);
    } else {
      fatal("unimplemented use of '?' in grammar");
    }
}







