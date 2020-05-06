/******************************************************************************
 fit.c - find a model that fits the data

 uses variant of incremental reduced error pruning (Furnkranz,ML94)
 main twiddles: a postpass to reduce MDL, and repetition of the learning
******************************************************************************/

#include <stdio.h>
#include "ripper.h"

BOOL Sequences=FALSE;
BOOL Simplify=TRUE;
double FP_cost=1.0;
double FN_cost=1.0;
double Max_decompression=64.0;
int Max_sample=0;
int Min_coverage=1;

int Optimizations=2;

/* locally used functions */
static void fit1(vec_t *,symbol_t *,vec_t *);
static void simplify(vec_t *,vec_t *,symbol_t *,vec_t *,int,vec_t *);
static BOOL stop_refining(vec_t *,int,int);
static BOOL reject_refinement(int,int,int,int,int);
static double rule_value(vec_t *,symbol_t *,vec_t *,int,vec_t *);
static BOOL reject_rule(vec_t *,symbol_t *,int,int,vec_t *,vec_t *,double *,double *);
static DATA *adequate_subsample(symbol_t *,DATA *,DATA **);

/*****************************************************************************/

/* iterate fit1 to get good hypothesis */
vec_t *fit(vec_t* data, symbol_t* cl)
{
	int i,trial_num;
	vec_t *hyp;

	/* hyp starts as empty ruleset */
	hyp = new_vec(rule_t);

	/* my algorithm */
	if (!Simplify) {
		fit1(data,cl,hyp);
	} else {
		fit1(data,cl,hyp);
		trace(SUMM) {
			printf("// current ruleset for %s\n",cl->name);
			for (i=0; i<vmax(hyp); i++) {
				printf("//    ");
				print_rule(vref(rule_t,hyp,i));
				printf("\n");
			}
			fflush(stdout);
		}
		/* iterate fit1  */
		for (trial_num=0; trial_num<Optimizations; trial_num++) {
			trace (SUMM) {
				printf("// optimizing rules for class %s---pass %d of %d\n",
					   cl->name,trial_num+1,Optimizations);
				fflush(stdout);
			}
			fit1(data,cl,hyp);
			reduce_dlen(hyp,cl,data);

			trace(SUMM) {
				printf("// current ruleset for %s\n",cl->name);
				for (i=0; i<vmax(hyp); i++) {
					printf("//    ");
					print_rule(vref(rule_t,hyp,i));
					printf("\n");
				}
				fflush(stdout);
			}
		}
	}
	return hyp;
}

/* use a greedy strategy to find rules to cover the examples of class cl */
static void fit1(data,cl,rules)
		vec_t *data;
		symbol_t *cl;
		vec_t *rules;  /* side-effected */
{
	ex_count_t all_p,all_n,rule_p,rule_n;
	rule_t *curr_rule,tmp_rule;
	vec_t data1,grow_data,prune_data;
	vec_t
			*new_form,*new_deriv,
			*old_form, *old_deriv,
			*rev_form,*rev_deriv; /* revised */
	BOOL last_rule_accepted,revising_rule;
	int rule_num;
	double new_val,old_val,rev_val,pr_val;
	double comp,best_comp;
	int i;
	ex_count_t tmp_p,tmp_n;
	char *operation;
	int n_deleted;
	DATA *working_sample;
	static DATA *space=NULL;

	comp = best_comp = 0.0;

	copy_data(&data1,data);

	/* La fréquence des classes doit correspondre aux séquences. */
	count_class_freq(cl,&data1,&all_p,&all_n);

	rule_num = 0;
	n_deleted = 0;
	last_rule_accepted = TRUE;

	/* main loop: learn a set of rules */

	while (all_p>0 && last_rule_accepted) {

		/* determine the starting points for growing rules */
		new_form = new_vec(gsym_t);
		new_deriv = new_vec(deriv_step_t);
		make_the_universe(cl,new_form);
		if (rule_num >= vmax(rules)) {
			revising_rule = FALSE;
			curr_rule = &tmp_rule;
			curr_rule->conseq = cl;
		} else {
			revising_rule = TRUE;
			curr_rule = vref(rule_t,rules,rule_num);
			rev_form = new_vec(gsym_t);
			rev_deriv = new_vec(deriv_step_t);
			copy_vec(gsym_t,rev_form,curr_rule->antec);
			copy_vec(deriv_step_t,rev_deriv,curr_rule->deriv);
			old_form = curr_rule->antec;
			old_deriv = curr_rule->deriv;
		}

		/* don't bother revising a rule that's subsumed by an earlier one */
		if (revising_rule) {
			count_examples(old_form,cl,&data1,&rule_p,&rule_n);
			curr_rule->nposx = rule_p;
			curr_rule->nnegx = rule_n;
			trace(LONG) {
				printf("// old rule %d: ",rule_num+1);
				print_rule(curr_rule); printf("\n");
			}
			if (rule_p==0 && rule_n==0) {
				trace (SUMM) {
					printf("// r%d skipped [null coverage] ",rule_num+1);
					print_rule(curr_rule); printf("\n");
					fflush(stdout);
				}
				free_vec(gsym_t,new_form); free_vec(deriv_step_t,new_deriv);
				free_vec(gsym_t,rev_form); free_vec(deriv_step_t,rev_deriv);
				n_deleted++;
				rule_num++;
				continue;
			}
		}

		/* grow a new rule, and possibly a revised version of old rule */
		if (!Simplify) {
			refine(new_form,new_deriv,cl,&data1);
		} else {
			stratified_partition(&data1,3,&grow_data,&prune_data);
			trace(LONG) printf("// finding new rule %d\n",rule_num+1);
			refine(new_form,new_deriv,cl,&grow_data);

			/* remove examples covered by later rules from pruning set
               since the current rule can't change their classification

               Peut-être qu'il faut changer cette ligne, ou ne pas utiliser
               Simplify */
			for (i=rule_num+1; i<vmax(rules); i++) {
				remove_covered_examples(
						vref(rule_t,rules,i)->antec,&prune_data);
			}

			simplify(new_form,new_deriv,cl,rules,rule_num,&prune_data);
			if (revising_rule) {
				trace(LONG) printf("// revising rule %d\n",rule_num+1);
				refine(rev_form,rev_deriv,cl,&grow_data);
				simplify(rev_form,rev_deriv,cl,rules,rule_num,&prune_data);
			}
		}

		/* decide which rule is best */
		if (!revising_rule) {
			count_examples(new_form,cl,&data1,&rule_p,&rule_n);
			if (reject_rule(new_form,cl,rule_p,rule_n,rules,&data1,&comp,&best_comp)) {
				last_rule_accepted = FALSE;
				/* clean up new_form,deriv */
				free_vec(gsym_t,new_form); free_vec(deriv_step_t,new_deriv);
			} else {
				curr_rule->antec = new_form;
				curr_rule->deriv = new_deriv;
				curr_rule->nposx = rule_p;
				curr_rule->nnegx = rule_n;
				ext_vec(rule_t,rules,curr_rule);
				remove_covered_examples(new_form,&data1);
				trace(SUMM) {
					pr_val =
							Simplify?
							rule_value(new_form,cl,rules,rule_num,&prune_data):
							rule_value(new_form,cl,rules,rule_num,&data1);
					printf("// r%d added [m=%d,v=%.3f] ",
						   rule_num+1,vmax(&data1),pr_val);
					print_rule(curr_rule); printf("\n");
					fflush(stdout);
				}
				rule_num++;
			}
		} else /* revising rule */ {

			working_sample = adequate_subsample(cl,&data1,&space);
			new_val = relative_compression(new_form,cl,rules,rule_num,working_sample);
			rev_val = relative_compression(rev_form,cl,rules,rule_num,working_sample);
			old_val = relative_compression(old_form,cl,rules,rule_num,working_sample);

			trace(LONG) {
				printf("// value: old=%.3f rev=%.3f new=%.3f\n",
					   old_val,rev_val,new_val);
			}
			if (old_val >= new_val && old_val >= rev_val) {
				count_examples(old_form,cl,&data1,&rule_p,&rule_n);
				curr_rule->antec = old_form;
				curr_rule->deriv = old_deriv;
				curr_rule->nposx = rule_p;
				curr_rule->nnegx = rule_n;
				/* clean up new_form,deriv, rev_form,deriv */
				free_vec(gsym_t,new_form); free_vec(deriv_step_t,new_deriv);
				free_vec(gsym_t,rev_form); free_vec(deriv_step_t,rev_deriv);
				trace(SUMM) { operation="retained"; pr_val = old_val; }
			} else if (rev_val>=new_val) {
				count_examples(rev_form,cl,&data1,&rule_p,&rule_n);
				curr_rule->antec = rev_form;
				curr_rule->deriv = rev_deriv;
				curr_rule->nposx = rule_p;
				curr_rule->nnegx = rule_n;
				/* clean up old_form,deriv, new_form,deriv */
				free_vec(gsym_t,old_form); free_vec(deriv_step_t,old_deriv);
				free_vec(gsym_t,new_form); free_vec(deriv_step_t,new_deriv);
				trace(SUMM) { operation="revised"; pr_val = rev_val; }
			} else /* new_val is best */ {
				count_examples(new_form,cl,&data1,&rule_p,&rule_n);
				curr_rule->antec = new_form;
				curr_rule->deriv = new_deriv;
				curr_rule->nposx = rule_p;
				curr_rule->nnegx = rule_n;
				/* clean up old_form,deriv, rev_form,deriv */
				free_vec(gsym_t,old_form); free_vec(deriv_step_t,old_deriv);
				free_vec(gsym_t,rev_form); free_vec(deriv_step_t,rev_deriv);
				trace(SUMM) { operation="replaced"; pr_val = new_val; }
			}
			remove_covered_examples(curr_rule->antec,&data1);
			count_class_freq(cl,&data1,&all_p,&all_n);
			trace(SUMM) {
				printf("// r%d %s [m=%d,c=%.3f] ",
					   rule_num+1,operation,vmax(&data1),pr_val);
				print_rule(curr_rule); printf("\n");
				fflush(stdout);
			}
			rule_num++;
		}
	} /* while allp>0 and last_rule_accepted */
}

/* use a greedy strategy to refine a rule */
void refine(vec_t* ref,vec_t* deriv,symbol_t* cl,vec_t* data)
{
	ex_count_t p,n,p1,n1;
	DATA data1,*working_sample;
	int i, best_i;
	BOOL last_refinement_rejected;
	double best_val, vali, prev_info;
	vec_t *csi;
	int priority;
	static DATA *space=NULL;

	trace(LONG) {
		printf("// refining:  ");
		print_form(ref);
		printf("\n");
	}

	copy_data(&data1,data);

	count_examples(ref,cl,&data1,&p,&n);
	if (form_size(ref)>0) {
		remove_uncovered_examples(ref,&data1);
	}
	prev_info = information(p,n);
	last_refinement_rejected = FALSE;
	while (!stop_refining(ref,p,n) && !last_refinement_rejected) {

		/* if appropriate use a smaller sample */
		working_sample = adequate_subsample(cl,&data1,&space);

		/* find the best refinement */
		priority = 0;
		do {
			best_val = -MAXREAL;
			compute_designated_refinements(ref,deriv,priority++);
			for (i=0; i<n_designated_refinements(); i++) {
				vali = value(refinement(i),cl,working_sample,ref,prev_info,p,n);
				if (vali > best_val) {
					best_val = vali; best_i = i;
				}
			}
		} while (best_val<=0 && priority<=Max_priority);
		/* decide if it's worth keeping */
		count_examples(refinement(best_i),cl,&data1,&p1,&n1);
		if (reject_refinement(priority-1,p,n,p1,n1)) {
			last_refinement_rejected = TRUE;
			trace(LONG) {
				printf("// stopped refining (%g/%g) ",p1,n1);
				print_form(refinement(best_i));
				printf("\n");
			}
		} else {
			copy_vec(gsym_t,ref,refinement(best_i));
			copy_vec(deriv_step_t,deriv,derivation(best_i));
			p = p1; n = n1; prev_info = information(p,n);
			remove_uncovered_examples(ref,&data1);
			trace(LONG) {
				printf("// refined to (%g/%g): ",p,n);
				print_form(ref);
				printf("\n");
			}
		} /* else !reject_refinement */
	} /* while !stop_refining */
}

/* use greedy strategy to simplify a rule */
static void simplify(form,deriv,cl,rules,rule_num,prune_data)
		vec_t *form, *deriv;  /* side-effected here */
		symbol_t *cl;
		vec_t *rules;
		int rule_num;
		vec_t *prune_data;
{
	int i, j, best_j;
	ex_count_t p, n;
	double val, best_val;
	DATA *working_sample;
	static DATA *space=NULL;

	working_sample = adequate_subsample(cl,prune_data,&space);

	best_val = rule_value(form,cl,rules,rule_num,working_sample);
	trace(LONG) {
		count_examples(form,cl,working_sample,&p,&n);
		printf("// simplifying with %d examples\n",vmax(working_sample));
		printf("// initial rule [%g/%g val=%.3f]: ",p,n,best_val);
		print_form(form);
		printf("\n");
	}

	compute_designated_generalizations(cl,form,deriv);
	while (n_designated_generalizations() > 0) {

		/* find generalization better than best_val */
		best_j = -1;
		for (j=0; j<n_designated_generalizations(); j++) {
			val = rule_value(generalization(j),cl,rules,rule_num,working_sample);
			trace(DBUG) {
				count_examples(generalization(j),cl,working_sample,&p,&n);
				printf("// gen %d [%g/%g val=%.3f]: ",j,p,n,val);
				print_form(generalization(j));
				printf("\n");
			}
			if (val >= best_val) {
				best_val = val;
				best_j = j;
			}
		} /*for generalization j */
		if (best_j == -1) {
			/* nothing better found */
			break; /* exit while loop */
		} else {
			/* replace form/deriv with the current best */
			copy_vec(gsym_t,form,generalization(best_j));
			copy_vec(deriv_step_t,deriv,derivation(best_j));
			trace(LONG) {
				count_examples(form,cl,working_sample,&p,&n);
				printf("// simplified to [%g/%g val=%.3f] ",p,n,best_val);
				print_form(form);
				printf("\n");
			}
			compute_designated_generalizations(cl,form,deriv);
		}
	} /* while */
	trace(LONG) {
		val = rule_value(form,cl,rules,rule_num,working_sample);
		count_examples(form,cl,working_sample,&p,&n);
		printf("// final rule [%g/%g val=%.3f]: ",p,n,val);
		print_form(form);
		printf("\n");
	}
}

/* decide when to stop refining and when to reject a refinement */
static BOOL stop_refining(form,p,n)
		vec_t *form;
		int p,n;
{
	int i;
	gsym_t *ci;

	if (n!=0) return FALSE;
	else {
		for (i=0; i<vmax(form); i++)  {
			ci = vref(gsym_t,form,i);
			if (ci->nonterm && !Derives_emptystr[ci->nonterm->index]) {
				return FALSE;
			}
		}
		return TRUE;
	}
}

static BOOL reject_refinement(priority,pi,ni,pj,nj)
		int priority;
		int pi,ni,pj,nj;
{
	if (pj<Min_coverage) return TRUE;
	else if (priority<Max_priority) return FALSE;
	else if (ni==nj) return TRUE;
	else return FALSE;
}

static BOOL reject_rule(form,cl,p,n,rules,data,pcomp,pbest_comp)
		vec_t *form;
		symbol_t *cl;
		int p,n;
		vec_t *rules;
		vec_t *data;
		double *pcomp,*pbest_comp;
{
	char reason[100];
	double decomp;
	BOOL ret;
	int allp,alln;

	/* enforce a description-length based cut-off */
	(*pcomp) += relative_compression(form,cl,rules,vmax(rules),data);
	if ((*pcomp) > (*pbest_comp)) {
		(*pbest_comp) = (*pcomp);
	}
	if ((*pcomp) < (*pbest_comp) - Max_decompression) {
		ret = TRUE;
		sprintf(reason,"description length too large");
	} else if (p==0) {
		ret = TRUE;
		sprintf(reason,"too few positive examples");
	} else if (n*FP_cost/(p*FN_cost + n*FP_cost) >= 0.5) {
		ret = TRUE;
		sprintf(reason,"error rate too large");
	} else {
		ret = FALSE;
		sprintf(reason,"");
	}
	trace (LONG) {
		printf("// final rule compression is %g (best %g)\n",(*pcomp),(*pbest_comp));
	}
	trace (SUMM) {
		if (ret) {
			printf("// rejected [%s]",reason);
			print_form(form);
			printf("\n");
		}
		fflush(stdout);
	}
	return ret;
}

static double rule_value(form,cl,rules,rule_num,data)
		vec_t *form;
		symbol_t *cl;
		vec_t *rules;
		int rule_num;
		vec_t *data;
{

	int i,P,N;
	ex_count_t p,n, cov,uncov,fp,fn;
	double num,denom;
	double ret,errs;

	if (rule_num >= vmax(rules)) {
		/* if adding a rule, use modified Furncrantz metric */
		count_examples(form,cl,data,&p,&n);
		num = (p+1)*FN_cost - (n+1)*FP_cost;
		denom = p*FN_cost + (n+1)*FP_cost + 1;
		ret =  num/denom;
	} else {
		/* if revising, use accuracy of theory resulting
         * from replacing rules[rule_num] with form
         * given that examples covered by later rules
         * are gone, we can just use errors committed by
         * this single rule
         */
		count_rule(form,cl,data,&cov,&uncov,&fp,&fn);
		if (vmax(data)==0) ret=0.0;
		else ret = 1.0 - (fp*FP_cost+fn*FN_cost)/vmax(data);
	}
	return ret;
}


/* construct a subsample of data of size at most Max_sample */
static DATA *adequate_subsample(symbol_t *cl,DATA *data,DATA **space)
{
	DATA *subsample;
	int i;

	if (Max_sample>0 && vmax(data)>Max_sample) {
		/* work with a subsample */
		sample_data(cl,Max_sample,data,space);
		subsample = (*space);
	} else {
		/* work with full dataset */
		subsample = data;
	}
	return subsample;
}
