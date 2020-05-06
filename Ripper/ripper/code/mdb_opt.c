/******************************************************************************
 mdb_opt.c - find optimal values for thresholds, etc
******************************************************************************/

#include <stdio.h>
#include "ripper.h"
#include "mdb.h"

#define weight(exi) (exi->wt)

/****************************************************************************/

/* Pairs will contain an sorted list of attribute-value/class-label pairs
 * to be used in finding numerical thresholds.  the class-label will be TRUE 
 * or FALSE depending on whether label of example matches the current class.
*/

typedef struct pair_s {
    REAL val,wt;
    BOOL lab;
} pair_t;

static vec_t *Pairs=NULL;

static int compare_pair(pi,pj)
pair_t *pi,*pj;
{
    if (pi->val > pj->val) return 1;
    else if (pi->val == pj->val) return 0;
    else return -1;
}

/* find the best value for an "wildcard" threshold */
double best_threshold(gsym_t *gsym,symbol_t *cl,vec_t *data,double old_info,
		      ex_count_t oldp,ex_count_t oldn,
		      ex_count_t *pos,ex_count_t *neg)

/* ex_count_t *pos, *neg hold output of function */
{
    ex_count_t ptmp,ntmp;
    int i,index;
    ex_count_t num_pos,num_neg;
    ex_count_t num_pos_gt,num_neg_gt;
    ex_count_t num_pos_lt,num_neg_lt;
    ex_count_t num_pos_eq,num_neg_eq;
    ex_count_t wti;
    double best_val,best_th,th,val;
    example_t *exi;
    aval_t *exi_val;
    pair_t p;

    trace(DBG1) {
	printf("// finding threshold for ");
	print_gsym(gsym);
	printf("\n");
    }

    if (Pairs==NULL) Pairs=new_vecn(pair_t,vmax(data));
    else clear_vec(pair_t,Pairs);

    /* populate and sort the Pairs array */
    index = gsym->attr_index;
    num_pos = num_neg = 0;
    for (i=0; i<vmax(data); i++) {
	exi = vref(example_t,data,i);
	exi_val = vref(aval_t,exi->inst,index);
	if (exi_val->kind!=MISSING_VALUE) /* not a missing attribute */ {
	    p.wt = weight(exi);
	    p.lab = (exi->lab.nom == cl);
	    p.val = exi_val->u.num;
	    ext_vec(pair_t,Pairs,&p);
	    if (p.lab) num_pos += weight(exi); 
	    else num_neg += weight(exi);
	}
    }
    vsort(pair_t,Pairs,&compare_pair);

    /* with no negative data, pick either the smallest or largest pair */
    if (num_neg==0) {
	(*pos) = num_pos;
	(*neg) = num_neg;
	if (num_pos==0) return 0.0;
	else if (gsym->op==OPGE) return vref(pair_t,Pairs,0)->val;
	else return vref(pair_t,Pairs,vmax(Pairs)-1)->val;
    }

    /* now find best threshold in the general case */
    num_pos_lt = num_neg_lt = 0;
    num_pos_eq = num_neg_eq = 0;
    num_pos_gt = num_pos;
    num_neg_gt = num_neg;
    best_val = -MAXREAL;
    best_th = 0.0;
    for (i=0; i<vmax(Pairs); ) {
	/* invariants: at this point
	   i is the first entry in Pairs with val = new threshold 
	   num_[pos|neg]_[eq|lt|gt] are the number of pos (neg)
	   examples equal to (less than, greater than) the last
	   threshold 
	*/
	th = vref(pair_t,Pairs,i)->val;
	num_pos_lt += num_pos_eq;
	num_neg_lt += num_neg_eq;
	num_pos_eq = num_neg_eq = 0; 
	while (i<vmax(Pairs) && vref(pair_t,Pairs,i)->val==th)
	{
	    wti = vref(pair_t,Pairs,i)->wt;
	    if (vref(pair_t,Pairs,i)->lab) num_pos_eq += wti;
	    else num_neg_eq += wti;
	    i++;
	}
	num_pos_gt -= num_pos_eq;
	num_neg_gt -= num_neg_eq;
	/* now i points after last entry in pairs with val == new threshold 
	   num_[pos|neg]_[lt|eq|gt] apply to new threshold 
	 */   
	if (gsym->op==OPLE) {
	    ptmp = num_pos_lt+num_pos_eq;
	    ntmp = num_neg_lt+num_neg_eq;
	} else { /* OPGE */
	    ptmp = num_pos_gt+num_pos_eq;
	    ntmp = num_neg_gt+num_neg_eq;
	}
	
	val = (*Value_function)(NULL,old_info,oldp,oldn,ptmp,ntmp);
	trace(DBG1) {
	    printf("// threshold %g: lt=%g/%g,eq=%g/%g,gt=%g/%g,val=%g\n",
		   th,num_pos_lt,num_neg_lt,num_pos_eq,num_neg_eq,
		   num_pos_gt,num_neg_gt);
	}
	if (val >= best_val) {
	    best_val = val;	    
	    best_th = th;
	    (*pos) = ptmp;
	    (*neg) = ntmp;
	}
    } /* for i=0,..vmax(Pairs) */
    
    trace(DBG1) {
	printf("// threshold is %g [val=%g, %d/%d]\n",
	       best_th,best_val,(*pos),(*neg));
    }

    return best_th; 
}



/*****************************************************************************/

/* find the best value for an "wildcard" word */
symbol_t *best_symbol(gsym_t *gsym,symbol_t *cl,vec_t *data,double old_info,
		      ex_count_t oldp,ex_count_t oldn,ex_count_t *pos,ex_count_t *neg)
/* ex_count_t *pos, *neg hold output of function */
{
    int i;
    symbol_t *best_sym;
    double val,best_val;
    BOOL invert;
    symbol_t *sym;
    ex_count_t ptmp,ntmp;
    
    compute_field_stats(cl,gsym->attr_index,data);
    invert = (gsym->op == OPOUT) || (gsym->op == OPNEQ);

    /* find the symbol with highest val  */
    best_val = -MAXREAL;
    best_sym = NULL;
    (*pos) = (*neg) = 0;
    for (i=0; i<n_visited_symbols(); i++) {
	sym = visited_symbol(i);
	if (invert) {
	    neg_field_stat(sym,&ptmp,&ntmp);
	    val = (*Value_function)(NULL,old_info,oldp,oldn,ptmp,ntmp);
	} else {
	    pos_field_stat(sym,&ptmp,&ntmp);
	    val = (*Value_function)(NULL,old_info,oldp,oldn,ptmp,ntmp);
	}
	if (val > best_val) {
	    best_val = val;
	    best_sym = sym;
	}
	trace(DBUG) {
	    ex_count_t p,n;
	    pos_field_stat(sym,&p,&n);
	    printf("// %sval(%s)=%g (covers  %g/%g)\n",
		   invert?"n":"p",sym->name,val,p,n);
	}
    }
    if (best_sym!=NULL) {
	if (invert) neg_field_stat(best_sym,pos,neg);
	else pos_field_stat(best_sym,pos,neg);
    }
    return best_sym;
}

/* these are used above, but also by verify, filter-text and rocchio */

/*
 * these structures counts the number of occurances of each symbol in
 * pos and negative examples in the dataset. Lastx contains
 * the index of the last example that contains the word; this
 * is used to avoid double counting symbols that appear more than
 * once in a set.  Lastx also records if a symbol has been seen
 * before in this iteration of compute_field_stats
 */ 

static ex_count_t *Nump=NULL;
static ex_count_t *Numn=NULL;
static int *Lastx=NULL;
static ex_count_t Allp,Alln;
static symbol_t **Visited_symbol;
static int N_visited;

int n_visited_symbols()
{
    return N_visited;
}
symbol_t *visited_symbol(int i)
{
    return Visited_symbol[i];
}

/* compute some statistics for the symbols that appear 
 * as values of a given field of a dataset */
void compute_field_stats(symbol_t *cl,int attr_index,vec_t *data)
{
    int i,j,symx;
    symbol_t *sym;
    example_t *exi;
    BOOL posi;
    vec_t *set;
    int n;

    n = n_symbolic_values();

    if (!Nump) {
	/* allocate and initialize arrays  */
	Nump  = newmem(n,ex_count_t);
	Numn  = newmem(n,ex_count_t);
	Lastx = newmem(n,int);
	Visited_symbol = newmem(n,symbol_t *);
	for (i=0; i<n; i++) {
	    Nump[i] = Numn[i] = 0;
	    Lastx[i] = NULLINDEX;
	}
    } else {
	/* reset the arrays */
	for (i=0; i<N_visited; i++) {
	    symx = Visited_symbol[i]->index;
	    Nump[symx] = Numn[symx] = 0;
	    Lastx[symx] = NULLINDEX;
	}
	N_visited = 0;
    }

    /* count pos and neg examples and occurances of each symbol */
    Allp = Alln = 0;
    N_visited = 0;
    for (i=0; i<vmax(data); i++) {
	exi = vref(example_t,data,i);
	if (vref(aval_t,exi->inst,attr_index)->kind!=MISSING_VALUE) {
	    posi = (exi->lab.nom==cl);
	    if (posi) Allp += weight(exi); 
	    else Alln += weight(exi);
	    if (symbolic_field(attr_index)) {
		sym = vref(aval_t,exi->inst,attr_index)->u.nom;
		symx = sym->index;
		if (posi) Nump[symx] += weight(exi);
		else Numn[symx] += weight(exi);
		if (Lastx[symx]==NULLINDEX) {
		    Visited_symbol[N_visited++] = sym;
		    Lastx[symx]=i;
		}
	    } else if (set_field(attr_index)) {
		set = vref(aval_t,exi->inst,attr_index)->u.set;
		for (j=0; j<vmax(set); j++) {
		    /* update counts for j-th word in the set */ 
		    sym = *vref(symbol_t *,set,j);
		    symx = sym->index;
		    if (Lastx[symx] != i) {
			/* haven't seen this word in this example yet */
			if (Lastx[symx]==NULLINDEX) {
			    Visited_symbol[N_visited++] = sym;
			}
			Lastx[symx] = i;
			if (posi) Nump[symx] += weight(exi);
			else Numn[symx] += weight(exi);
		    } 
		    trace(DBG1) printf("// sym %d: stats for %s[%d] now %g/%g\n",
				       i,sym->name,symx,Nump[symx],Numn[symx]);
		} /* for word j in set */
	    } else {
		assert(0);
	    } 
	} /*not missing */
    } /* for example i */ 
}

double pos_symbol_gain(symbol_t *s,double old_info)
{
    ex_count_t p,n;
    
    p = Nump[s->index];
    n = Numn[s->index];
    return p*(old_info - information(p,n));
}

double neg_symbol_gain(symbol_t *s,double old_info)
{
    ex_count_t p,n;
    
    p = Allp-Nump[s->index];
    n = Alln-Numn[s->index];
    return p*(old_info - information(p,n));
}

/* statistics for a word */
void pos_field_stat(symbol_t *s,ex_count_t *p,ex_count_t *n)
{
    (*p) = Nump[s->index];
    (*n) = Numn[s->index];
}

/* statistics for a word */
void neg_field_stat(symbol_t *s,ex_count_t *p,ex_count_t *n)
{
    (*p) = Allp-Nump[s->index];
    (*n) = Alln-Numn[s->index];
}

