/******************************************************************************
 mdb_count.c - "counting" utilities for datasets
******************************************************************************/

#include <stdio.h>
#include "ripper.h"
#include "mdb.h"

#define weight(exi) (exi->wt)
vec_t* SID = NULL;

void count_examples(form,cl,data,pos,neg)
vec_t *form;
symbol_t *cl;
DATA *data;
ex_count_t *pos,*neg;
{
    ex_count_t fp,fn,cov,uncov;

    count_rule(form,cl,data,&cov,&uncov,&fp,&fn);
    *pos = cov-fp;
    *neg = fp;
}

/*****************************************************************************/

/* compute coverage of a ruleset[index..end] on data
 * after ruleset[index] has been replaced with a given form 
 * return compression obtained by deleting sub-optimal rules
 * with replace == NULL and index == -1 just this counts the rules
*/

void count_replaced_ruleset(replace,index,cl,rules,data,pcov,puncov,pfp,pfn)
vec_t *replace; /* form to replace rules[index] or NULL for empty rule */
vec_t *rules;   
symbol_t *cl;
int index;      /* assume counts for rules 0...index-1 are ok */
DATA *data;    /* assume data is the portion of larger set 
		   not covered by rules 0 ... index -1 */
ex_count_t *pcov,*puncov,
    *pfp,*pfn;  /* side-effected to hold computed counts*/ 
{
    int i,j;
    BOOL covered; 
    example_t *exi;
    rule_t *rj;
    
    /* compute fp and cov of early rules */
    *pcov = *puncov = *pfp = *pfn = 0;
    for (j=0; j<index; j++) {
	rj = vref(rule_t,rules,j);
	*pcov += rj->nnegx+rj->nposx;
	*pfp += rj->nnegx;
    } 

    /* clear counts for later rules */
    for (j=index+1; j<vmax(rules); j++) {
	rj = vref(rule_t,rules,j);
	rj->nnegx = rj->nposx = 0;
    } 

    /* count coverage on data of later rules */
    for (i=0; i<vmax(data);i++) {
		exi = vref(example_t,data,i);
		if (!SID || *vref(BOOL, SID, exi->sid)) {
			if (replace != NULL && form_covers(replace, exi->inst)) {
				*pcov += weight(exi);
				if (exi->lab.nom != cl) *pfp += weight(exi);
				if (SID) *vref(BOOL, SID, exi->sid) = FALSE;
			} else {
				/* see if later rule covers exi */
				for (covered = FALSE, j = index + 1; j < vmax(rules) && !covered; j++) {
					rj = vref(rule_t, rules, j);
					if (form_covers(rj->antec, exi->inst)) {
						covered = TRUE;
						*pcov += weight(exi);
						if (exi->lab.nom != rj->conseq) {
							*pfp += weight(exi);
							rj->nnegx += weight(exi);
						} else {
							rj->nposx += weight(exi);
						}
						if (SID) *vref(BOOL, SID, exi->sid) = FALSE;
					}
				}
				if (!covered && !SID) {
					*puncov += weight(exi);
					if (exi->lab.nom == cl) *pfn += weight(exi);
				}
			}
		}
    } /* for datum i */

	if (SID) {
		for (i = 0; i < vmax(data); i++) {
			exi = vref(example_t, data, i);
			if (*vref(BOOL, SID, exi->sid)) {
				*puncov += weight(exi);
				if (exi->lab.nom == cl) *pfn += weight(exi);
			}
		}

		for (i = 0; i < vmax(SID); i++) *vref(BOOL, SID, i) = TRUE;
	}
}

/* count members of a class in a dataset
*/
void count_class_freq(cl,data,p,n)
symbol_t *cl;
DATA *data;
ex_count_t *p, *n;
{
    example_t *exi;
    int i;

    (*p) = (*n) = 0;
    for (i=0; i<vmax(data); i++) {
		exi = vref(example_t,data,i);

		/* Ajout de l'utilisation de SID dans le code de ripper */
		if (!SID || *vref(BOOL, SID, exi->sid)) {
			if (exi->lab.nom == cl) (*p) += weight(exi);
			else (*n) += weight(exi);
			if (SID) *vref(BOOL, SID, exi->sid) = FALSE;
		}
    }

	if (SID) for (i = 0; i < vmax(SID); i++) *vref(BOOL, SID, i) = TRUE;
}

/* count coverage of a single sentential form
*/

void count_rule(form,cl,data,pcov,puncov,pfp,pfn)
vec_t *form;
symbol_t *cl;
DATA *data;
ex_count_t *pcov, *puncov, *pfp, *pfn;
{
#define UNOPTIMIZED
#ifdef UNOPTIMIZED
    int i;
    example_t *exi;

    *pcov = *puncov = *pfp = *pfn = 0;

	if (SID) {
		for (i = 0; i < vmax(data); i++) {
			exi = vref(example_t, data, i);

			/*
             * Le test de couverture d'un exemple se fait ici.
             * C'est donc ici qu'on doit choisir si l'occurrence compte ou non dans le calcul de fréquence.
             */

			if (*vref(BOOL, SID, exi->sid)) {
				if (form_covers(form, exi->inst)) {
					if (exi->lab.nom != cl) {
						*pfp += weight(exi);
					}
					*pcov += weight(exi);
					*vref(BOOL, SID, exi->sid) = FALSE;
				}
			}
		}
		for (i = 0; i < vmax(data); i++) {
			exi = vref(example_t, data, i);

			/*
             * Le test de couverture d'un exemple se fait ici.
             * C'est donc ici qu'on doit choisir si l'occurrence compte ou non dans le calcul de fréquence.
             */

			if (*vref(BOOL, SID, exi->sid)) {
				if (!form_covers(form, exi->inst)) {
					*puncov += weight(exi);
					if (exi->lab.nom == cl) {
						*pfn += weight(exi);
					}
					*vref(BOOL, SID, exi->sid) = FALSE;
				}
			}
		}

		for (i = 0; i < vmax(SID); i++) *vref(BOOL, SID, i) = TRUE;
	}
	else {
		for (i = 0; i < vmax(data); i++) {
			exi = vref(example_t, data, i);

			if (form_covers(form, exi->inst)) {
				if (exi->lab.nom != cl) {
					*pfp += weight(exi);
				}
				*pcov += weight(exi);
			}
			else {
				*puncov += weight(exi);
				if (exi->lab.nom == cl) {
					*pfn += weight(exi);
				}
			}
		}
	}
#else
    gsym_t *condp;
    symbol_t *cond_nom;
    REAL cond_num;
    operator_t cond_op;
    aval_t *exi_val;
    example_t *exi,*eximax;
    BOOL covered;

    *pcov = *puncov = *pfp = *pfn = 0;

    /* first a special case for length-one forms */
    if (vmax(form)==1 && !(vref(gsym_t,form,0)->nonterm)) {
	condp = vref(gsym_t,form,0);
	cond_num = condp->value.num; 
	cond_nom = condp->value.nom;
	cond_op = condp->op;
	eximax = vbase(example_t,data)+vmax(data);
	for (exi=vbase(example_t,data); exi<eximax; exi++) {
	    exi_val = vbase(aval_t,exi->inst)+condp->attr_index;
	    /* TEST: does condition cover the example? */
	    covered = FALSE;
	    if (exi_val->kind!=MISSING_VALUE) { 
		switch (cond_op) {
		  case OPEQ:
		    if (exi_val->kind==SYMBOL && exi_val->u.nom==cond_nom) 
		      covered = TRUE;
		    if (exi_val->kind==CONTINUOUS && exi_val->u.num==cond_num)
		      covered = TRUE;
		    break;
		  case OPNEQ:
		    if (exi_val->kind==SYMBOL && exi_val->u.nom!=cond_nom) 
		      covered = TRUE;
		    if (exi_val->kind==CONTINUOUS && exi_val->u.num!=cond_num)
		      covered = TRUE;
		    break;
		  case OPLE:
		    if (exi_val->u.num<=cond_num) 
		      covered = TRUE;
		    break;
		  case OPGE:
		    if (exi_val->u.num>=cond_num) 
		      covered = TRUE;
		    break;
		  case OPIN:
		    if (contains_symbol(cond_nom,exi_val->u.set))
		      covered = TRUE;
		    break;
		  case OPOUT:
		    if (!contains_symbol(cond_nom,exi_val->u.set))
		      covered = TRUE;
		    break;
		  default:
		    fatal("unknown operator");
		} /* switch cond_op */
	    }/* else not missing */
	    if (covered) {
		*pcov += weight(exi);
		if (exi->lab.nom != cl) *pfp += weight(exi);
	    } else {
		*puncov += weight(exi);		
		if (exi->lab.nom == cl) *pfn += weight(exi);
	    }
	} /* for each example */ 
    } else {
	/* case for forms of length greater than 1 */
	eximax = vbase(example_t,data)+vmax(data);
	for (exi=vbase(example_t,data); exi<eximax; exi++) {
	    if (form_covers(form,exi->inst)) {
		*pcov += weight(exi);
		if (exi->lab.nom != cl) *pfp += weight(exi);
	    } else {
		*puncov += weight(exi);		
		if (exi->lab.nom == cl) *pfn += weight(exi);
	    }
	}
    }
#endif
}

double error_rate(c,data)
concept_t *c;
DATA *data;
{
    example_t *exp,*exmax;
    double err;
    double tot;

    err = tot = 0;
    exmax = vbase(example_t,data)+vmax(data);
    for (exp=vbase(example_t,data); exp<exmax; exp++) {
	tot += weight(exp);
	if (classify(c,exp->inst)!=exp->lab.nom) err += weight(exp);
	
    }
    if (tot==0) return 0.0;
    else return err/tot;
}

/*****************************************************************************/

BOOL form_covers(sform,inst)
vec_t *inst;
vec_t *sform;
{
    register gsym_t *cp,*cpmax;
    aval_t *ex_val;
    symbol_t *cp_nom;
    REAL cp_num;

    /* optimized loop over all conditions */
    cpmax = vbase(gsym_t,sform)+vmax(sform);
    for (cp=vbase(gsym_t,sform); cp<cpmax; cp++) {
	if (!cp->nonterm) {
	    cp_nom = cp->value.nom;
	    cp_num = cp->value.num;
	    /* TEST: does condition cover instance? */
	    ex_val = vbase(aval_t,inst)+cp->attr_index;
	    if (ex_val->kind==MISSING_VALUE) {
	      return (cp->op==OPEQ && cp_nom && cp_nom->kind==MISSING_MARK);
	    }
	    switch (cp->op) {
	      case OPEQ:
		if (ex_val->kind==SYMBOL && ex_val->u.nom!=cp_nom) return FALSE;
		if (ex_val->kind==CONTINUOUS && ex_val->u.num!=cp_num) return FALSE;
		break;
	      case OPNEQ:
		if (ex_val->kind==SYMBOL && ex_val->u.nom==cp_nom) return FALSE;
		if (ex_val->kind==CONTINUOUS && ex_val->u.num==cp_num) return FALSE;
		break;
	      case OPLE:
		if (ex_val->u.num>cp_num) return FALSE;
		break;
	      case OPGE:
		if (ex_val->u.num<cp_num) return FALSE;
		break;
	      case OPIN:
		if (!contains_symbol(cp_nom,ex_val->u.set)) return FALSE;
		break;
	      case OPOUT:
		if (contains_symbol(cp_nom,ex_val->u.set)) return FALSE;
		break;
	      default:
		assert(0);
	    }/* switch*/
	} /* if !nonterm */
    } /* for each gsym */
    return TRUE;
}

BOOL contains_symbol(sym,inst)
symbol_t *sym;
vec_t *inst;
{
    int i; 
    for (i=0; i<vmax(inst); i++) {
	if (*vref(symbol_t *,inst,i) == sym) return TRUE;
    }
    return FALSE;
}
