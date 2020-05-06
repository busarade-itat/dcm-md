/******************************************************************************
 desref.c - generate  the set of "designated refinements" of a rule

 designated refinements of a sentential form desref(x) are defined as follows:
 - x in desref(x)
 - IF x=uAv AND A-->y in grammar AND z in desref(y) THEN uzv in desref(x)

 This actually generates 
  { y in desref(x) : y!=x and no rule is used more than once in generating y }

******************************************************************************/

#include <stdio.h>
#include "ripper.h"

/* an "antecedent" is a sentential form
 * plus its derivation */

typedef struct antec_s {
    vec_t *sform;
    vec_t *deriv;
} antec_t;

/* an extensible array of ptrs to antec_t 
 * note this array is never cleared, because that
 * would require freeing and later reallocating
 * the antec_t's that it points to; instead
 * N_slots keeps track of the number of meaningful
 * values in the array  */

vec_t *Slots;
int N_slots=0;

/* array of flags, one for each grammar rule */
FLAG *used = NULL; 

static void store_refinements(antec_t *,int,int,int);
static void clear_slots(void);
static antec_t *avail_ref(void);
static void do_deriv_step(vec_t *,deriv_step_t *,vec_t *);
static void replay_deriv(vec_t *,vec_t *,vec_t *,int,int *,int *);
static void copy_wildcards(vec_t *,vec_t *);

/*****************************************************************************/

/* make u the start symbol */
void make_the_universe(cl,u)
symbol_t *cl;
vec_t *u;
{
    char str[BUFSIZ];
    gsym_t ucond;
    symbol_t *body_cl;

    clear_vec(gsym_t,u);
    sprintf(str,"body_%s",cl->name);
    body_cl = intern(str);
    if (body_cl->kind == NONTERM) {
	ucond.nonterm = body_cl;
    } else {
	ucond.nonterm = intern("body");
    }
    ext_vec(gsym_t,u,&ucond);
}

/*****************************************************************************/

/* return i-th computed refinement/generalization */
vec_t *refinement(index) int index;
{
    return (*vref(antec_t *,Slots,index))->sform;
}
vec_t *generalization(index) int index;
{
    return (*vref(antec_t *,Slots,index))->sform;
}

/* return derivation of the i-th computed refinement/generalization */
vec_t *derivation(index) int index;
{
    return (*vref(antec_t *,Slots,index))->deriv;
}

/* return number of computed refinements/generalizations */
int n_designated_refinements()
{
    return N_slots;
}
int n_designated_generalizations()
{
    return N_slots;
}


/*****************************************************************************/

/* compute all refinements of a sentential form */
void compute_designated_refinements(form,deriv,max_priority)
vec_t *form;
vec_t *deriv;
int max_priority;
{
    antec_t antec;
    int i;
    /* clear flags */
    if (!used) used = newmem(vmax(Grammar),FLAG);
    for (i=0; i<vmax(Grammar); i++) used[i] = FALSE;

    /* clear out old refinements */
    clear_slots();
    /* generate new refinements */
    antec.sform = form;
    antec.deriv = deriv;
    store_refinements(&antec,0,vmax(antec.sform),max_priority);
}

static void store_refinements(antec_t* antec, int lo, int hi, int max_priority)
{
    symbol_t *nt;
    int i,j;
    antec_t *newref;
    deriv_step_t step;
    grule_t *rj;


    trace(DBG1) {
	printf("// refining: "); 
	print_form(antec->sform); 
	printf("\n"); 
    }

    for (i=lo; i<hi; i++) {

	nt = vref(gsym_t,antec->sform,i)->nonterm;
	if (!nt) continue;

	/* for each rule r */
        for (j=Lo_rule[nt->index]; j<Hi_rule[nt->index]; j++) {
            rj = vref(grule_t,Grammar,j);

            /* make sure rule is unused so far, and has priority */
            if (used[j] || rj->priority > max_priority) {
            continue;
            }

            /* allocate space */
            newref = avail_ref();

            /* record the derivation */
            if(antec->deriv) copy_vec(deriv_step_t,newref->deriv,antec->deriv);
            step.posn = i; step.rulex = j;
            ext_vec(deriv_step_t,newref->deriv,&step);

            /* perform the derivation */
            do_deriv_step(antec->sform,&step,newref->sform);

            /* recursively refine the new guy  */
            used[j] = TRUE;
            store_refinements(newref,i,i+vmax(rj->rhs),max_priority);
            used[j] = FALSE;
        }
    }
}

/*****************************************************************************/

/* compute all generalizations of a sentential form */
void compute_designated_generalizations(cl,form,deriv)
symbol_t *cl;
vec_t *form;
vec_t *deriv;
{
    int i,j,lo,hi,n_remove;
    vec_t *form1, *deriv1, *tmp;
    deriv_step_t *step;
    antec_t *new_antec;
    gsym_t dummy;

    clear_slots();
    form1 = new_vec(gsym_t);
    tmp = new_vec(gsym_t);
    deriv1 = new_vec(deriv_step_t);
    make_the_universe(cl,form1);
    for (i=0; i<vmax(deriv); i++) {
	step = vref(deriv_step_t,deriv,i);
	if (Derives_emptystr[vref(gsym_t,form1,step->posn)->nonterm->index]) {

	    trace(DBG1) {
		printf("delete at %d ",step->posn); print_form(form1); 
		printf("; "); print_deriv(deriv1); printf("\n");
	    }

	    /* make a copy with the subtree rooted here marked in lo,hi */
	    new_antec = avail_ref();
	    copy_vec(gsym_t,new_antec->sform,form1);
	    copy_vec(deriv_step_t,new_antec->deriv,deriv1);
	    lo = step->posn; hi = step->posn+1;
	    replay_deriv(new_antec->sform,new_antec->deriv,deriv,i,&lo,&hi);
	    copy_wildcards(new_antec->sform,form);

	    trace(DBG1) {
		printf("got [%d-%d]",lo,hi); print_form(new_antec->sform); 
		printf("; "); print_deriv(new_antec->deriv); printf("\n");
	    }

	    /* delete marked conditions (between lo and hi) */
	    n_remove = hi-lo;
	    for (j=lo; j<vmax(new_antec->sform)-n_remove; j++) {
		vset(gsym_t,new_antec->sform,j,
		     vref(gsym_t,new_antec->sform,j+n_remove));
	    }
	    shorten_vecn(gsym_t,new_antec->sform,n_remove);

	    /* insert root of subtree in their place */
	    dummy.nonterm = intern("????");
	    ext_vec(gsym_t,new_antec->sform,&dummy);
	    for (j=vmax(new_antec->sform)-1; j>lo; j--) {
		vset(gsym_t,new_antec->sform,j,
		     vref(gsym_t,new_antec->sform,j-1));
	    }
	    vset(gsym_t,new_antec->sform,step->posn,
		 vref(gsym_t,form1,step->posn));
	}
	do_deriv_step(form1,step,tmp);
	copy_vec(gsym_t,form1,tmp);
	ext_vec(deriv_step_t,deriv1,step);
    }
    free_vec(gsym_t,form1);
    free_vec(gsym_t,tmp);
    free_vec(deriv_step_t,deriv1);
}

/* replay a derivation, starting at step start */ 
static void replay_deriv(form,cnd_deriv,deriv,start,lo,hi)
vec_t *form, *cnd_deriv; /* side effected */
int start, *lo,*hi;      /* lo,hi side effected */
vec_t *deriv;
{
    int i;
    vec_t *tmp;
    deriv_step_t *step, step1;

    tmp = new_vec(gsym_t);
    for (i=start; i<vmax(deriv); i++) {

	step = vref(deriv_step_t,deriv,i);
	do_deriv_step(form,step,tmp);

	/* update derivation */
	if (step->posn < *lo) {
	    ext_vec(deriv_step_t,cnd_deriv,step);
	} else if (step->posn >= *hi) {
	    step1.rulex = step->rulex;
	    step1.posn = step->posn - (*hi - *lo) + 1;
	    ext_vec(deriv_step_t,cnd_deriv,&step1);
	} /* else ignore this step if posn between lo and hi */

	/* update lo, hi */
	if (step->posn < *hi) *hi += vmax(tmp)-vmax(form);
	if (step->posn < *lo) *lo += vmax(tmp)-vmax(form);

	copy_vec(gsym_t,form,tmp);
    }
    free_vec(gsym_t,tmp);
}

/* copy thresholds into a new_form, assuming it is
 * a copy of old_form (as made by replay_derivation) 
*/   

static void copy_wildcards(new_form,old_form)
vec_t *new_form, *old_form;
{
    int i;
    gsym_t *old_gsymi,*new_gsymi;

    assert(vmax(new_form)==vmax(old_form));

    for (i=0; i<vmax(old_form); i++) {
	new_gsymi = vref(gsym_t,new_form,i);
	if (contains_wildcard(new_gsymi)) {
	    old_gsymi = vref(gsym_t,old_form,i);
	    new_gsymi->value.nom = old_gsymi->value.nom;
	    new_gsymi->value.num = old_gsymi->value.num;
	}
    }
}

/*****************************************************************************/

/* utilities used by compute_designated_xxx() functions */

static void clear_slots()
{
    int i;
    antec_t *refi;

    if (!Slots) {
	Slots = new_vec(antec_t *);
    } else {
	N_slots = 0;
	for (i=0; i<vmax(Slots); i++) {
	    refi = *vref(antec_t *,Slots,i);
	    clear_vec(gsym_t,refi->sform);
	    clear_vec(deriv_step_t,refi->deriv);
	}
    }
}

static void do_deriv_step(oldform,step,newform)
vec_t *oldform, *newform; /* newform is side-effected */
deriv_step_t *step;
{
    int j;
    int i=step->posn;
    grule_t *r = vref(grule_t,Grammar,step->rulex);

    clear_vec(gsym_t,newform);
    /* copy in oldform 0..i-1 */
    for (j=0; j<i; j++) 
      ext_vec(gsym_t,newform,vref(gsym_t,oldform,j));
    /* copy in rhs of rule */
    for (j=0; j<vmax(r->rhs); j++) 
      ext_vec(gsym_t,newform,vref(gsym_t,r->rhs,j));
    /* copy in rest of oldform */
    for (j=i+1; j<vmax(oldform); j++) 
      ext_vec(gsym_t,newform,vref(gsym_t,oldform,j));
}

/* find/allocate space for new refinement */
static antec_t *avail_ref()
{
    antec_t *newref;

    /* create a new antecedent if needed */
    if (N_slots == vmax(Slots)) {
	newref = newmem(1,antec_t);
	newref->sform = new_vec(gsym_t);
	newref->deriv = new_vec(deriv_step_t);
	ext_vec(antec_t *,Slots,&newref);
    } 
    newref = *vref(antec_t *,Slots,N_slots++);
    clear_vec(gsym_t,newref->sform);
    clear_vec(deriv_step_t,newref->deriv);
    return newref;
}

/*****************************************************************************/

void print_form(cs)
vec_t *cs;
{
    int j;
    for (j=0; j<vmax(cs); j++) {
	printf(" "); print_gsym(vref(gsym_t,cs,j));
    }
}

void print_deriv(der)
vec_t *der;
{
    int j;
    for (j=0; j<vmax(der); j++) {
	printf(" %d@%d", 
          vref(deriv_step_t,der,j)->rulex,vref(deriv_step_t,der,j)->posn);
    }
}

void print_designated_refinements()
{
    int i;
    for (i=0; i<N_slots; i++) {
	printf("%5d:",i); 
	print_form((*vref(antec_t *,Slots,i))->sform); 
	printf("\t(");
	print_deriv((*vref(antec_t *,Slots,i))->deriv); 
	printf(")\n");
    }
}

/*****************************************************************************/

#ifdef TEST
/* main: test driver
*/

int main(int,char**);

main(argc,argv)
int argc;
char *argv[];
{
    vec_t *form,*deriv,*choosen_ref,*choosen_deriv;
    int choice;

    if (argc<=3) {
	fatal("syntax: test-grammar namesfile datafile grammarfile");
    }
    (void)ld_names(argv[1]);
    (void)ld_data(argv[2]);

    if (!ld_grammar(argv[3]),NULL)
      warning("can't find grammar file!");
    print_grammar();

    form = new_vec(gsym_t);
    make_the_universe(form);
    deriv = new_vec(deriv_step_t);
    do {
	compute_designated_refinements(form,deriv);
	print_designated_refinements();
	printf("\nrefinement (-1 to stop)? "); scanf("%d",&choice);
	if (choice>=0 && choice<N_slots) {
	    choosen_ref = refinement(choice);
	    choosen_deriv = derivation(choice);
	    copy_vec(gsym_t,form,choosen_ref);
	    copy_vec(deriv_step_t,deriv,choosen_deriv);
	    printf("You choose "); print_form(choosen_ref); printf("\n");
	    printf("copied to: "); print_form(form); printf("\n");
	}
    } while (choice>=0 && choice<N_slots);
    printf("\nGeneralizations of:"); print_form(form); 
    printf("\n       (derivation:"); print_deriv(deriv); printf(")\n");
    compute_designated_generalizations(form,deriv);
    print_designated_refinements();
    return 0;
}
#endif

