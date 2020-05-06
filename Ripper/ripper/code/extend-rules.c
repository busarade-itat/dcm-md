/******************************************************************************
 extend-rules.c

 given a hypothesis and a file ptr, write a modified version
 of the hypothesis in which each rule has been "extended" by
 calling the refine routine.  This approximates outputing the
 specific rules that ripper ended up pruning; and thus
 after each rule is extended, the dataset is modified by removing
 the examples covered by the _original_ rule
 
******************************************************************************/

#include <stdio.h>
#include "ripper.h"

void extend_rules(FILE *fp,concept_t *c,DATA *data)
{
    DATA data1;
    rule_t *ri,rbuf;
    int i,n_cl;
    int *cl_offsets;
    symbol_t *last_cl;
    
    copy_data(&data1,data);
    rbuf.antec = new_vec(gsym_t);
    rbuf.deriv = new_vec(deriv_step_t);

    /* for unordered classes, find out where
       the rules for each class start
     */   
    if (c->res!=FIRST) {
	cl_offsets = newmem(vmax(Classes),int);
	last_cl = NULL;
	n_cl = 0;
	for (i=0; i<vmax(c->rules); i++) {
	    ri = vref(rule_t,c->rules,i); 
	    if (ri->conseq != last_cl) {
		cl_offsets[n_cl++] = i;
		last_cl = ri->conseq;
	    }
	}
	trace(LONG) {
	    printf("// class offsets:");
	    for (i=0; i<vmax(Classes); i++) {
		printf(" %d",cl_offsets[i]);
	    }
	    printf("\n");
	}
    } else {
	copy_data(&data1,data);	
    }

    n_cl = 0;
    for (i=0; i<vmax(c->rules); i++) {
	/* make sure we have the right initial dataset */
	if (n_cl<vmax(Classes) && i == cl_offsets[n_cl]) {
	    copy_data(&data1,data);	
	    n_cl++;
	}
	ri = vref(rule_t,c->rules,i); 
	rbuf.conseq = ri->conseq;
	copy_vec(gsym_t,rbuf.antec,ri->antec);
	copy_vec(deriv_step_t,rbuf.deriv,ri->deriv);
	rbuf.nposx = ri->nposx;
	rbuf.nnegx = ri->nnegx;
	trace(LONG) {
	    printf("// Extending rule %d\n// ",i); 
	    print_rule(&rbuf);
	    printf("\n");
	}
	refine(rbuf.antec,rbuf.deriv,rbuf.conseq,&data1);
	count_examples(rbuf.antec,rbuf.conseq,&data1,&rbuf.nposx,&rbuf.nnegx);
	trace(SUMM) {
	    printf("// Extended r%d: ",i+1); 
	    print_rule(&rbuf);
	    printf("\n");
	    fflush(stdout);
	}
	/* write the rule */
	fshow_rule(fp,&rbuf);
	/* remove examples covered by the original rule */
	remove_covered_examples(ri->antec,&data1);
    }
    /* write the stuff that follows the list of rules---from fshow_concept() */
    fprintf(fp,"%s",c->def->nom->name);
    fprintf(fp," %g %g IF .\n.\n",c->nposx,c->nnegx);
    if (c->res!=FIRST) {
	fprintf(fp,"use_best_rule\n");
    }
}

