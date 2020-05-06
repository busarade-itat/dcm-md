/******************************************************************************
 model.c - use fit() on each class to find a model that fits the data
******************************************************************************/

#include <stdio.h>
#include "ripper.h"

static concept_t *model1(vec_t *);

BOOL Uniq_rule_class = FALSE;
ex_count_t *Class_counts=NULL;
order_t Class_ordering=INCFREQ;

/*****************************************************************************/

concept_t *model(vec_t* data)
{
    concept_t *hyp; 
    rule_t *rulej;

    count_classes(data);
    reorder_classes(data);
    hyp = model1(data);
    if (Add_redundancy) add_redundancy(hyp,data);
    count_concept(hyp,data);
    return hyp;
}

static concept_t *model1(vec_t* data)
{
    int i,j;
    vec_t *fit_rules;
    DATA data1;
    atom_t *classi;
    rule_t *rulej;
    concept_t *hyp;
    int end_class;

    hyp = newmem(1,concept_t);
    hyp->rules = new_vec(rule_t);
    copy_data(&data1,data);
    if (Class_ordering==UNORDERED) {
	hyp->res = BEST;

	end_class = vmax(Classes);
    } else {
	hyp->res = FIRST;
	end_class = vmax(Classes)-1;
    }

    /* Modification de Ripper pour ne pas chercher à extraire des règles sur la classe des négatifs. */
    if (Uniq_rule_class) end_class = 1;

    for (i=0; i<end_class; i++) {
        classi = vref(atom_t,Classes,i);
        fit_rules = fit(&data1,classi->nom);
        for (j=0; j<vmax(fit_rules); j++) {
                rulej = vref(rule_t,fit_rules,j);
                ext_vec(rule_t,hyp->rules,rulej);
                /* remove covered examples for ordered classes */
                if (Class_ordering!=UNORDERED) {
                remove_covered_examples(rulej->antec,&data1);
                }
        }
	    free_vec(rule_t,fit_rules);
    } /* for class i */
    return hyp;
}

/****************************************************************************/

/* count #examples of each class, for use by MDL, etc */
void count_classes(data)
vec_t *data;
{
    ex_count_t p,n;
    int k;

    if (Class_counts==NULL) Class_counts = newmem(vmax(Classes),ex_count_t);
    for (k=0; k<vmax(Classes); k++) {
	count_class_freq(vref(atom_t,Classes,k)->nom,data,&p,&n);
	Class_counts[k] = p;
    }
}

/****************************************************************************/

static double *Class_val=NULL;
static int dec_class_val(char *,char *);
static int inc_class_val(char *,char *);

static int dec_class_val(char* cl1, char* cl2)
{
    return -inc_class_val(cl1,cl2);
}

static int inc_class_val(char* cl1, char* cl2)
{
    double v1,v2;

    v1 = Class_val[((atom_t *)cl1)->nom->index];
    v2 = Class_val[((atom_t *)cl2)->nom->index];

    if (v1>v2) return 1;
    else if (v1<v2) return -1;
    else return 0;
}

/* sort classes by prevalence, least prevalent first */
void reorder_classes(vec_t* data)
{
    int i,j,tmp;
    atom_t *cli;
    char *how_ordered = "?";
    vec_t *rules;
    atom_t *classi;
    
    if (Class_val==NULL) {
	Class_val = newmem(vmax(Classes),double);
    }

    switch (Class_ordering) {
    case UNORDERED:
    case GIVEN:
	for (i=0; i<vmax(Classes); i++) Class_val[i] = i+1;
	how_ordered = "given";
	break;
    case INCFREQ:
    case DECFREQ:
	for (i=0; i<vmax(Classes); i++) Class_val[i] = Class_counts[i];
	if (Class_ordering==INCFREQ) {
	    vsort(atom_t,Classes,&inc_class_val);
	    how_ordered = "increasing frequency";
	} else {
	    vsort(atom_t,Classes,&dec_class_val);
	    how_ordered = "increasing frequency";
	}
	break;
    case MDL:
	/* find quick-and-dirty hypotheses for each class, 
	   and record the description lengths */

	/* save old values */
	/* use quick-and-dirty approach */
	tmp = Optimizations;
	Optimizations = 0; 

	for (i=0; i<vmax(Classes); i++) {
	    classi = vref(atom_t,Classes,i);
	    trace(SUMM) {
		printf("// estimating cost of rules for class %s\n",
		       classi->nom->name);
		fflush(stdout);
	    }
	    rules = fit(data,classi->nom);
	    Class_val[i] = combined_dlen(rules,classi->nom,data);
	    for (j=0;j<vmax(rules);j++) {
		free_rule(vref(rule_t,rules,j));
	    }
	    free_vec(rule_t,rules);
	    trace(SUMM) {
		printf("// class %s rules need %.3f bits\n",
		       classi->nom->name,Class_val[i]);
		fflush(stdout);
	    }
	}

	/* restore old value */
	Optimizations = tmp; 

	vsort(atom_t,Classes,&inc_class_val);
	how_ordered = "description length";
	break;
    }

    trace(SUMM) {
	printf("// classes in %s order:\n",how_ordered);
	for (i=0; i<vmax(Classes); i++) {
	    cli = vref(atom_t,Classes,i);
	    printf("// class %s:\t%.3f\n",
		   cli->nom->name,Class_val[cli->nom->index]);
	}
	fflush(stdout);
    }

    /* re-assign indices and re-order class counts */
    
    for (i=0; i<vmax(Classes); i++) {
	/* save original value of count here */
	Class_val[i] = Class_counts[i];
    }

    for (i=0; i<vmax(Classes); i++) {
	j = vref(atom_t,Classes,i)->nom->index;
	/* move class w/ index j to index i */ 
	vref(atom_t,Classes,i)->nom->index = i;
	Class_counts[j] = Class_val[i];
    }

    trace(LONG) {
	printf("// class frequencies:\n");
	for (i=0; i<vmax(Classes); i++) {
	    cli = vref(atom_t,Classes,i);
	    printf("// class %d (%s): %g\n",
		   cli->nom->index,cli->nom->name,Class_counts[cli->nom->index]);
	}
    }

    if ((FN_cost != 1.0 || FP_cost!=1.0) && vmax(Classes)!=2) {
	warning("loss ratio (-L) only supported for 2-class problems!");
    }
}

