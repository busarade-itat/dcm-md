/******************************************************************************
 rule.c - manipulate rule types
******************************************************************************/

#include <stdio.h>
#include "ripper.h"
#include "mdb.h"

/*****************************************************************************/

void fprint_rule(FILE *fp,rule_t *r)
{
    int i; 
    char *sep = " :- ";
    gsym_t *gsym;

    fprint_symbol(fp,r->conseq);
    for (i=0; i<vmax(r->antec); i++) {
	gsym = vref(gsym_t,r->antec,i);
	if (!gsym->nonterm) {
	    fprintf(fp,"%s",sep);
	    fprint_gsym(fp,gsym);
	    sep = ", ";
	}
    }
    fprintf(fp," (%g/%g).", r->nposx,r->nnegx);
}

void fshow_rule(FILE *fp,rule_t *r)
{
    int i;
    gsym_t *gsym;

    fprintf(fp,"%s",r->conseq->name);
    fprintf(fp," %g %g IF",r->nposx,r->nnegx);
    for (i=0; i<vmax(r->antec); i++) {
	gsym = vref(gsym_t,r->antec,i);
	if (!gsym->nonterm) {
	    fprintf(fp," ");
	    fshow_gsym(fp,gsym);
	}
    }
    fprintf(fp," .\n");
}

void free_rule(r)
rule_t *r;
{
    int i;

    free_vec(gsym_t,r->antec);
    free_vec(derive_step_t,r->deriv);
    r->antec = NULL;
    r->deriv = NULL;
}

int rule_size(r)
rule_t *r;
{
    int i,sz;

    sz = 1;
    for (i=0; i<vmax(r->antec); i++) 
      if (vref(gsym_t,r->antec,i)->nonterm==NULL) sz++;
    return sz;
}

int form_size(vec_t *form)
{
    int i,sz;

    sz = 0;
    for (i=0; i<vmax(form); i++) 
      if (vref(gsym_t,form,i)->nonterm==NULL) sz++;
    return sz;
}
