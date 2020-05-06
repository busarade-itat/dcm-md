/******************************************************************************
 mdl.c - description-length routines used (mostly) by fit.c
******************************************************************************/

#include <stdio.h>
#include "ripper.h"

/* locally used functions */

/*******************/

static double data_dlen(symbol_t *,ex_count_t,ex_count_t,ex_count_t,ex_count_t);
static double subset_dlen(ex_count_t,double,double);
static double form_dlen(vec_t *);
static void count_ruleset(
	        symbol_t *,vec_t *,vec_t *,
                ex_count_t *,ex_count_t *,ex_count_t *,ex_count_t *);
static double replaced_ruleset_compression(
                vec_t *,symbol_t *,vec_t *,int,vec_t *,
	        ex_count_t *,ex_count_t *,ex_count_t *,ex_count_t *);
static BOOL delete_and_update(
	        rule_t *,double *,
		ex_count_t *,ex_count_t *,ex_count_t *,ex_count_t *);

/* to account for redundancy between attributes and conditions */
double MDL_theory_factor=1.0;    /* default */
#define REDUNDANCY_FACTOR 0.5


/****************************************************************************/

/* combined description length of the rules plus the data */
double combined_dlen(rules,cl,data)
vec_t *rules;
symbol_t *cl;
vec_t *data;
{
    ex_count_t cov,uncov,fp,fn;
    double rule_bits;
    int i;
    rule_t *ri;

    count_ruleset(cl,rules,data,&cov,&uncov,&fp,&fn);
    for (rule_bits=0,i=0; i<vmax(rules); i++) {
	rule_bits += form_dlen(vref(rule_t,rules,i)->antec);
    }
    return rule_bits + data_dlen(cl,cov,uncov,fp,fn);
}

/****************************************************************************/

/* compression obtained by inserting form into ruleset at position index  
 * relative to inserting a null-coverage rule in position index */

double relative_compression(form,cl,rules,index,data)
vec_t *form;
symbol_t *cl;
vec_t *rules;
int index;
vec_t *data;
{
    ex_count_t cov,uncov,fp,fn; 
    double data_dlen_with,data_dlen_without;
    double comp_without,comp_with;
    double dlen_without,dlen_with;

    comp_without = 
      replaced_ruleset_compression(
         NULL,cl,rules,index,data,&cov,&uncov,&fp,&fn);
    data_dlen_without = data_dlen(cl,cov,uncov,fp,fn);

    comp_with = 
      replaced_ruleset_compression(
         form,cl,rules,index,data,&cov,&uncov,&fp,&fn);
    data_dlen_with = data_dlen(cl,cov,uncov,fp,fn);

    dlen_without = data_dlen_without-comp_without;
    dlen_with = data_dlen_with+form_dlen(form)-comp_with;
    
    return dlen_without - dlen_with;
}

/****************************************************************************/

/* delete rules so as to reduce total description length */
void reduce_dlen(rules,cl,data)
vec_t *rules;           /* side-effected */
symbol_t *cl;
vec_t *data;
{
    ex_count_t cov,uncov,fp,fn;
    BOOL need_recount;
    int i,j;
    rule_t *ri;
    double compression;
    
    trace(SUMM) {
	printf("// reduce_dlen: %d examples, %d rules\n",
	       vmax(data), vmax(rules));
    }
    count_ruleset(cl,rules,data,&cov,&uncov,&fp,&fn);
    need_recount=FALSE;

    /* look at all rules, latest first, and consider deletion */
    for (i=vmax(rules)-1; i>=0; i--) {
	ri = vref(rule_t,rules,i);
	if (delete_and_update(ri,&compression,&cov,&uncov,&fp,&fn)) {
	    trace(SUMM) {
		printf("// deleting r%d of %d [saving %g bits]: ",
		       i+1,vmax(rules),compression);
		print_rule(ri);
		printf("\n");
		fflush(stdout);
	    }
	    if (i!=vmax(rules)-1) need_recount=TRUE;
	    
	    /* delete the ri from rules */
	    free_rule(ri);
	    for (j=i; j+1<vmax(rules); j++) {
		vset(rule_t,rules,j,vref(rule_t,rules,j+1));
	    }
	    shorten_vec(rule_t,rules);
	}
    }
    if (need_recount) {
	count_ruleset(cl,rules,data,&cov,&uncov,&fp,&fn);
    }
}

/****************************************************************************/

/* update counts as if rule were deleted 
 * also return (in pcompression) compression obtained by deletion and
 * (as return value) if the rule should be deleted
*/

static BOOL delete_and_update(r,pcompression,pcov,puncov,pfp,pfn)
rule_t *r;
double *pcompression;           /* holds compression obtained by deletion */
ex_count_t *pcov, *puncov, *pfp, *pfn; /* updated to reflect deletion */
{
    ex_count_t cov1,uncov1,fp1,fn1;
    double with,without;

    /* counts after deletion, assuming no other rules cover same data */
    cov1 = (*pcov) - (r->nnegx + r->nposx);
    uncov1 = (*puncov) + (r->nnegx + r->nposx);
    fp1 = (*pfp) - r->nnegx;
    fn1 = (*pfn) + r->nposx;

    with = data_dlen(r->conseq,*pcov,*puncov,*pfp,*pfn);
    without = data_dlen(r->conseq,cov1,uncov1,fp1,fn1);
    (*pcompression) = with + form_dlen(r->antec) - without;

    if ((*pcompression) >= 0.0 
        || (r->nnegx*FP_cost/(r->nposx*FN_cost + r->nnegx*FP_cost) >= 0.5))
    {
	/* update counts to reflect deletion */
	*pcov = cov1;
	*puncov = uncov1;
	*pfp = fp1;
	*pfn = fn1;
	return TRUE;
    } else {
	return FALSE;
    }
}


/****************************************************************************/


/* send subset of e elements of a set of size n, where expected number 
 * of such elements is n*p --- see (Quinlan,ML95) */
static double subset_dlen(ex_count_t n,double e,double p)
{
    return -Log2(p)*e + -Log2(1.0-p)*(n-e); 
}


/****************************************************************************/


/* description length of the data given a ruleset---see (Quinlan,ML95) */
static double data_dlen(symbol_t *cl,
			ex_count_t cov,ex_count_t uncov,
			ex_count_t fp,ex_count_t fn)
{
    double e,proportion;
    ex_count_t all;
    int i,start_class;

    assert(cov>0 || uncov>0); 

    start_class = Class_ordering==UNORDERED? 0 : cl->index ;

    /* desired proportion of fp/errors is same as proportion of class */
    all = 0.0;
    for (i=start_class; i<vmax(Classes); i++) {
	all += Class_counts[i];
    }
    proportion = ((double)Class_counts[cl->index])/all;
    e = fn*FN_cost + fp*FP_cost;

    /* desired proportion of fp/errors is 1/2  
    proportion = 0.5;
    */

    if (cov >= uncov) {
	return 
	  Log2(cov+uncov+1.0)+
	  subset_dlen(cov,fp*FP_cost,proportion*e/cov) + 
	  (uncov>0 ? subset_dlen(uncov,fn*FN_cost,(double)fn/uncov) : 0.0);
    } else {
	proportion = 1.0-proportion;
	return 
	  Log2(cov+uncov+1.0)+
	  subset_dlen(uncov,fn*FN_cost,proportion*e/uncov) + 
	  (cov>0 ? subset_dlen(cov,fp*FP_cost,(double)fp/cov) : 0.0);
    }
}


/****************************************************************************/

static double form_dlen(form)
vec_t *form;
{
    int i,form_len;
    double dlen;

    form_len=0;
    for (i=0; i<vmax(form); i++) {
	if (vref(gsym_t,form,i)->nonterm==NULL) form_len++;
    }

    /* send form_len then the terminals in the form */

    dlen = Log2((double)form_len);
    if (dlen>1) dlen += 2*Log2(dlen);
    dlen += subset_dlen(N_eff_terms,(double)form_len,((double)form_len)/N_eff_terms);

    return MDL_theory_factor * REDUNDANCY_FACTOR * dlen;
}

/****************************************************************************/

static void count_ruleset(cl,rules,data,pcov,puncov,pfp,pfn)
symbol_t *cl;   /* class predicted by the rules */
vec_t *rules;   /* will side-effect counts  */
vec_t *data;    
ex_count_t *pcov,*puncov,
    *pfp,*pfn;  /* side-effected to hold computed counts*/ 
{
    count_replaced_ruleset(NULL,-1,cl,rules,data,pcov,puncov,pfp,pfn);
}

/*
 * return compression obtained by making the appropriate replacement
 * and deleting unnecessary rules, and count the ruleset as a side-effect
*/
static double 
replaced_ruleset_compression(replace,cl,rules,index,data,pcov,puncov,pfp,pfn)
vec_t *replace; /* form to replace rules[index] or NULL for empty rule */
symbol_t *cl;   /* class predicted by the rules */
vec_t *rules;   
int index;      /* assume counts for rules 0...index-1 are ok */
vec_t *data;    /* assume data is the portion of larger set not covered by
		   rules 0 ... index -1 */
ex_count_t *pcov,*puncov,
    *pfp,*pfn;  /* side-effected to hold computed counts*/ 
{
    double compression,compj;
    int j;
    rule_t *rj;

    /* count the rules */
    count_replaced_ruleset(replace,index,cl,rules,data,pcov,puncov,pfp,pfn);
    
    /* now see if any rules can be deleted */
    compression = 0.0;
    for (j=index+1; j<vmax(rules); j++) {
	rj = vref(rule_t,rules,j);
	if (delete_and_update(rj,&compj,pcov,puncov,pfp,pfn)) {
	    compression += compj;
	}
    }
    return compression;
}
