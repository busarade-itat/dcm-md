/******************************************************************************
 concept.c - manipulate concept type
******************************************************************************/

#include <stdio.h>
#include "ripper.h"

/*****************************************************************************/

void fprint_concept(FILE *fp,concept_t *c)
{
    int i;

    for (i=0; i<vmax(c->rules); i++) {
	fprint_rule(fp,vref(rule_t,c->rules,i));
	fprintf(fp,"\n");
    }
    fprintf(fp,"default ");
    fprint_symbol(fp,c->def->nom);
    fprintf(fp," (%g/%g).\n",c->nposx,c->nnegx);
    if (c->res!=FIRST) {
	fprintf(fp,"resolution=best.\n");
    }
}

/* print a computer-readable version of the concept */
void fshow_concept(fp,c)
FILE *fp;
concept_t *c;
{
    int i;
    rule_t *r;

    for (i=0; i<vmax(c->rules); i++) {
	r = vref(rule_t,c->rules,i);
	fshow_rule(fp,r);
    }
    fprintf(fp,"%s",c->def->nom->name);
    fprintf(fp," %g %g IF .\n.\n",c->nposx,c->nnegx);
    if (c->res!=FIRST) {
	fprintf(fp,"use_best_rule\n");
    }
}

/* read a computer-readable version of the concept */
concept_t *ld_concept(fp)
FILE *fp;
{
    concept_t *c;
    char word_buf[BUFSIZ];
    rule_t r,*pr;
    gsym_t *gsym;

    c = newmem(1,concept_t);
    c->res=FIRST;
    c->rules=new_vec(rule_t);
    
    fscanf(fp,"%s",word_buf);
    while (!strcmp(word_buf,".")==0) {
	r.conseq = intern(word_buf);
	fscanf(fp,"%f %f",&(r.nposx),&(r.nnegx));
	fscanf(fp,"%s",word_buf);
	if (strcmp(word_buf,"IF")) {
	    error("rule %d is ill-formed",vmax(c->rules)+1);
	}
	r.antec = new_vec(gsym_t);
	while ((gsym = ld_gsym(fp))!=NULL) {
	    ext_vec(gsym_t,r.antec,gsym);
	}
	/* printf("rule: "); print_rule(&r); printf("\n"); */
	ext_vec(rule_t,c->rules,&r);
	
	/* advance to beginning of next rule */
	fscanf(fp,"%s",word_buf);
    }
    if (fscanf(fp,"%s",word_buf)>0 && strcmp(word_buf,"use_best_rule")==0) {
	c->res = BEST;
    }

    /* last rule should be default rule */
    pr = vref(rule_t,c->rules,vmax(c->rules)-1);
    if (vmax(pr->antec)!=0) {
	warning("no default rule");
    } else {
	c->nposx = pr->nposx;
	c->nnegx = pr->nnegx;
	c->def = newmem(1,atom_t);
	c->def->nom = pr->conseq;
	shorten_vec(rule_t,c->rules);
    }

    return c;
}


int concept_size(c)
concept_t *c;
{
    int i,sz;

    sz = 0;
    for (i=0; i<vmax(c->rules); i++) {
	sz += rule_size(vref(rule_t,c->rules,i));
    }
    return sz;
}

void free_concept(c)
concept_t *c;
{
    int i;

    for (i=0; i<vmax(c->rules); i++) {
	free_rule(vref(rule_t,c->rules,i));
    }
    free_vec(rule_t,c->rules);
    c->rules = NULL;
}

symbol_t *classify(c,inst)
concept_t *c;
vec_t *inst;
{
    ex_count_t d1,d2;
    return classify_counts(c,inst,&d1,&d2);
}

symbol_t *classify_counts(c,inst,p,n)
concept_t *c;
vec_t *inst;
ex_count_t *p, *n;
{
    int i;
    rule_t *ri;
    symbol_t *best_class;
    double best_con,coni;

    if (c->res==FIRST) {
	for (i=0; i<vmax(c->rules); i++) {
	    ri = vref(rule_t,c->rules,i);
	    if (form_covers(ri->antec,inst)) {
		(*p) = ri->nposx;
		(*n) = ri->nnegx;
		return ri->conseq;
	    }
	}
	(*p) = c->nposx;
	(*n) = c->nnegx;
	return c->def->nom;
    } else {
	assert(c->res==BEST); 
	best_con = -1;
	best_class = NULL;
	for (i=0; i<vmax(c->rules); i++) {
	    ri = vref(rule_t,c->rules,i);
	    if (form_covers(ri->antec,inst)) {
		coni = (ri->nposx+1) / (ri->nposx + ri->nnegx + 2);
		if (coni > best_con) {
		    best_con = coni;
		    best_class = ri->conseq; 
		    (*p) = ri->nposx;
		    (*n) = ri->nnegx;
		}
	    }
	}
	if (best_class!=NULL) {
	    return best_class;
	} else {
	    (*p) = c->nposx;
	    (*n) = c->nnegx;
	    return c->def->nom;
	}
    }
}

/* find counts for best/first rule associated with class cl */
BOOL class_counts(c,inst,cl,p,n)
concept_t *c;
vec_t *inst;
symbol_t *cl;
ex_count_t *p, *n;
{
    int i;
    rule_t *ri;
    double best_con,coni;
    static BOOL warned=FALSE;

    if (c->res==FIRST) {
	if (!warned) {
	    warning("predict -V should not be used on rulesets with ordered classes");
	    warned = TRUE;
	}
	for (i=0; i<vmax(c->rules); i++) {
	    ri = vref(rule_t,c->rules,i);
	    if (ri->conseq==cl && form_covers(ri->antec,inst)) {
		(*p) = ri->nposx;
		(*n) = ri->nnegx;
		return TRUE;
	    }
	}
	/* check default rule */
	if (c->def->nom==cl) {
	    (*p) = c->nposx;
	    (*n) = c->nnegx;
	    return TRUE;
	} else {
	    (*n) = c->nposx;
	    (*p) = c->nnegx;
	    return FALSE;
	}
    } else {
	assert(c->res==BEST); 
	best_con = -1;
	for (i=0; i<vmax(c->rules); i++) {
	    ri = vref(rule_t,c->rules,i);
	    if (ri->conseq==cl && form_covers(ri->antec,inst)) {
		coni = (ri->nposx + 1) / (ri->nposx + ri->nnegx + 2);
		if (coni > best_con) {
		    best_con = coni;
		    (*p) = ri->nposx;
		    (*n) = ri->nnegx;
		}
	    }
	}
	if (best_con > -1) {
	    return TRUE;
	} else {
	    /* use default rule */
	    if (c->def->nom==cl) {
		(*p) = c->nposx;
		(*n) = c->nnegx;
		return TRUE;
	    } else {
		(*n) = c->nposx;
		(*p) = c->nnegx;
		return FALSE;
	    }
	}
    }
}


void count_concept(hyp,data)
concept_t *hyp; /* hyp is side-effected */
vec_t *data;
{
    ex_count_t cov,uncov,fp,true_n,p,n;
    rule_t *ri;
    int i;
    DATA data1;
    atom_t *best_class;
    ex_count_t highest_freq;

    if (hyp->res==FIRST) { 
	/* choose the default class to be the last class */
	hyp->def = vref(atom_t,Classes,vmax(Classes)-1);
	/* NB: passing in default class means that this
	 * routine will count the number of true negatives 
	 */
	count_replaced_ruleset(NULL,-1,hyp->def->nom,hyp->rules,data,
			       &cov,&uncov,&fp,&true_n);
	hyp->nposx = true_n;
	hyp->nnegx = uncov-true_n;
    } else {
	assert(hyp->res==BEST);
	/* count the coverage of the individual rules */
	for (i=0; i<vmax(hyp->rules); i++) {
	    ri = vref(rule_t,hyp->rules,i);
	    count_examples(ri->antec,ri->conseq,data,&p,&n);
	    ri->nposx = p;
	    ri->nnegx = n;
	}
	/* now count the examples covered only by the default rule */
	copy_data(&data1,data);
	for (i=0; i<vmax(hyp->rules); i++) {
	    ri = vref(rule_t,hyp->rules,i);
	    remove_covered_examples(ri->antec,&data1);
	}
	count_classes(&data1);
	/* find the most frequent class among the uncovered examples */
	highest_freq = uncov = 0;
	best_class = NULL;
	for (i=0; i<vmax(Classes); i++) {
	    if (Class_counts[i] > highest_freq) {
		best_class = vref(atom_t,Classes,i);
		highest_freq = Class_counts[i];
	    }
	    uncov += Class_counts[i];
	}
	if (best_class) {
	    hyp->def = best_class;
	    hyp->nposx = highest_freq;
	    hyp->nnegx = uncov-highest_freq;
	} else {
	    /* no examples covered by default */
	    hyp->def = vref(atom_t,Classes,vmax(Classes)-1);
	    hyp->nposx = hyp->nnegx = 0;
	}
    }
}

