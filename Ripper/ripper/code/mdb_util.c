/******************************************************************************
 mdb_util.c - miscellaneous utilities for datasets
******************************************************************************/

#include <stdio.h>
#include "ripper.h"
#include "mdb.h"
#include "extras.h"

#define Min_pos_fraction 0.2

/****************************************************************************/
/* basic utilities */
/****************************************************************************/

/* make an empty holder for data */

DATA *new_data(int m)
{
    return new_vecn(example_t,m);
}

/* make a "copy" of a dataset, without allocating new storage */

void copy_data(DATA *copy,DATA *data) /* copy is side-effected */ 
{
    share_vec(example_t,copy,data);
}

/* sample about m elements of data and put them in sample */
void sample_data(symbol_t *cl,int m,DATA *data,DATA **space)
{
    int q,r,i;
    DATA data1;
    example_t *exi,*exr;
    static vec_t *index=NULL;
    int npos;
    example_t dummy_ex;
    ex_count_t wfact;

    /* first allocate space, if necessary */
    if ((*space)==NULL) {
	/* allocate space and fill it with examples */
	(*space) = new_data(Max_sample);
	for (i=0; i<Max_sample; i++) {
	    ext_vec(example_t,(*space),&dummy_ex);
	}
    }

    /* next find all positive examples (of class cl) */
    if (index==NULL) index=new_vec(int);
    else clear_vec(int,index);
    for (i=0; i<vmax(data);i++) {
	exi = vref(example_t,data,i);
	if (exi->lab.nom==cl) {
	    ext_vec(int,index,&i);
	}
    }
    /*
    trace(SUMM) printf("// sampling %d from %d/%d\n",
		       m,vmax(index),vmax(data)-vmax(index));
    */		       

    /* below: wfact is such that 
     *  (sum of weights of sample)*wfact = (sum of weights in data)
    */   
    copy_data(&data1,data);
    if (vmax(index)/vmax(data) >= Min_pos_fraction) {
	wfact = ((double)vmax(data))/((double)m);
	for (i=0; i<m; i++) {
	    r = random()%vmax(&data1);
	    exi = vref(example_t,(*space),i);
	    exr = vref(example_t,&data1,r);
	    exi->lab.nom = exr->lab.nom;
	    exi->wt = exr->wt * wfact;	    
	    exi->inst = exr->inst;
	    swap_out_example(&data1,r);
	}
    } else {
	npos = (int)Min_pos_fraction*vmax(data)+0.5;
	if (npos >= vmax(index)) npos=vmax(index);
	wfact = ((double)vmax(index))/((double)npos);
	for (i=0; i<npos; i++) {
	    q = random()%vmax(index);
	    r = *vref(int,index,q);
	    exi = vref(example_t,(*space),i);
	    exr = vref(example_t,&data1,r);
	    exi->lab.nom = exr->lab.nom;
	    exi->wt = exr->wt * wfact;	    
	    exi->inst = exr->inst;
	    swap_out_example(&data1,r);
	    /* "swap out" position q of index */
	    ripper_copy(int,vref(int,index,q),vref(int,index,vmax(index)-1));
	    shorten_vec(int,index);
	}
	wfact = ((double)vmax(data)-vmax(index))/((double)m-npos);
	for ( ;i<m; i++) {
	    exi = vref(example_t,(*space),i);
	    r = random()%vmax(&data1);
	    exr = vref(example_t,&data1,r);
	    exi->lab.nom = exr->lab.nom;
	    exi->wt = exr->wt;	    
	    exi->inst = exr->inst;
	    swap_out_example(&data1,r);
	}
    }
}

/* remove examples in data covered by form */
void remove_covered_examples(vec_t *form,DATA *data)
{
    int i;
    example_t *exi;

    i=0;
    while (i<vmax(data)) {
	exi = vref(example_t,data,i);
	if (form_covers(form,exi->inst)) {
	    swap_out_example(data,i);
	} else i++;
    } 
}

/* remove examples in data NOT covered by form */

void remove_uncovered_examples(vec_t *form,DATA *data)
{
    int i;
    example_t *exi;

    /* update the data by removing uncovered examples */
    i=0;
    while (i<vmax(data)) {
	exi = vref(example_t,data,i);
	if (!form_covers(form,exi->inst)) {
	    swap_out_example(data,i);
	} else i++;
    } 
}

/****************************************************************************/
/* routines used for cross-validation */
/****************************************************************************/

/* randomize order of data so as to preserve 
 * class frequencies in each of several partitions,
 * where partition i of size-n dataset is the range 
 *     n/splits*i...n/splits*(i+1)
 */
void stratify_and_shuffle_data(DATA *data,int splits)
{
    int i,j,k,r,offset;
    static example_t *tmp=NULL, extmp, *exi;
    static int tmpsize=0,n;
    static int *class_tmp=NULL;
    symbol_t *classi;

    /* allocate class_tmp array if needed */
    if (class_tmp==NULL) {
	class_tmp = newmem(vmax(Classes),int);
    }
    /* allocate a larger tmp array, if needed */
    if (tmpsize<vmax(data)) {
	if (tmp) freemem(tmp);
	tmp = newmem(vmax(data),example_t);
	tmpsize = vmax(data);
    } 
    /* set logical size of tmp */
    n = vmax(data);

    /* count #examples of each class */
    for (i=0; i<vmax(Classes); i++) {
	class_tmp[i]=0;
    }
    for (i=0; i<n; i++) {
	exi = vref(example_t,data,i);
	class_tmp[exi->lab.nom->index]++;
    }
    /* next, make class_tmp[i] be offset at which 
       to store examples of class i */   
    for (i=vmax(Classes)-1; i>=0; i--) {
	offset = 0;
	for (j=0; j<i; j++) offset += class_tmp[j];
	class_tmp[i] = offset;
    }

    /* now copy in the data, sorting by class */
    for (i=0; i<n; i++) {
	exi = vref(example_t,data,i);
	offset = class_tmp[exi->lab.nom->index]++;
	ripper_copy(example_t,&tmp[offset],exi);
    }

    /* shuffle each class separately */
    i=0;
    while (i<n) {
	classi = tmp[i].lab.nom;
	for (j=i; j<n && tmp[j].lab.nom==classi; j++)
 	    ; /* do nothing */
	if (j<n) {
	    /* shuffle range i...j */
	    for (k=i; k<j; k++) {
		r = k+random()%(n-k);
		ripper_copy(example_t,&extmp,&tmp[k]);
		ripper_copy(example_t,&tmp[k],&tmp[r]);
		ripper_copy(example_t,&tmp[r],&extmp);
	    }
	}
	i=j;	
    }

    /* store back into original array in set order */
    clear_vec(example_t,data);
    for (j=0; j<splits; j++) {
	for (i=j; i<n; i+=splits) {
	    ext_vec(example_t,data,&tmp[i]);
	}
    }
}

/* assuming data has been shuffled, put the i-th 
 * partition in data2 and the rest in data1
*/

void ith_stratified_partition(DATA *data,int i,int folds,DATA *data1,DATA *data2)
{
    int lo,hi,j,m;

    m = vmax(data)/folds;
    clear_vec(example_t,data1);
    clear_vec(example_t,data2);
    lo = i*m;
    hi = i==folds-1? vmax(data) : (i+1)*m;
    for (j=0; j<vmax(data); j++) {
	if (j>=lo && j<hi) {
	    ext_vec(example_t,data2,vref(example_t,data,j));
	} else {
	    ext_vec(example_t,data1,vref(example_t,data,j));
	}
    }
} 


/* conceptually split data as for a k-fold stratified 
 * cross-validation, where splits=k, and then put
 * the first (k-1) partitions in data1, and the
 * last partition in data2
*/

void stratified_partition(DATA *data,int splits,DATA *data1,DATA *data2)
{
    int cut;

    stratify_and_shuffle_data(data,splits);
    cut = vmax(data)*(splits-1.0)/splits;
    share_subvec(example_t,data1,data,0,cut);
    share_subvec(example_t,data2,data,cut,vmax(data)-cut);
}


/****************************************************************************/

/* mark i-th example as removed --- without destroying any data */
void swap_out_example(data,i)
vec_t *data;
int i;
{
    example_t *exi,*ex_last,tmp;

    exi = vref(example_t,data,i);
    ex_last = vref(example_t,data,vmax(data)-1);
    ripper_copy(example_t,&tmp,exi);
    ripper_copy(example_t,exi,ex_last);
    ripper_copy(example_t,ex_last,&tmp);
    shorten_vec(example_t,data);
}
