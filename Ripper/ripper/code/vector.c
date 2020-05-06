/******************************************************************************
 vector.c - implement "vector" type, an extensible array of things
******************************************************************************/

#include <stdio.h>
#include "vector.h"

extern char *safe_calloc(int,int);
extern int safe_free(char *);
#define newmem(n,type)  ((type *) safe_calloc((n),sizeof(type)))
#define freemem(addr)   safe_free((char *) addr)

#ifndef GCC
extern char *memcpy(char *,char *,int);
extern int memcmp(char *,char *,int);
#endif

/*****************************************************************************/

/* print function for debugging */
void show_vec(v)
vec_t *v;
{
    int i;
    printf("<vec:%d sz %d:",v->max,v->item_sz);
    for (i=0;i<v->max*v->item_sz;i++) {
	printf(" %3d",((char *)v->space)[i]);
    }
    printf(">");
}

vec_t *_new_vec(int sz,int initlen) {
    vec_t *v = newmem(1,vec_t);
    v->item_sz = sz;
    v->space_sz = initlen;
    v->space = (void *)safe_calloc(initlen,v->item_sz);
    return v;
}

/* free a vector */
void _free_vec(vec_t* v)
{
    freemem((char *)v->space);
    freemem((char *)v);
}

/* return ptr to ith element of v */
void *_vref(vec_t* v,int i)
{
    assert(i>=0 && i<v->max);
    return (void *) ((char *)v->space + i*v->item_sz);
}

/* set ith thing in v to contents of a */
void *_vset(vec_t* v,int i,void* a)
{
    assert(i>=0 && i<v->max);
    return (void *)memcpy((char *)v->space+i*v->item_sz,(char *)a,v->item_sz);
}

/* test if a is in v */
int _vmem(vec_t* v,void* a)
{
    int i;
    char *start;
    for (i=0; i<v->max; i++) {
	start = ((char *) v->space) + i*v->item_sz;
	if (!memcmp(start,(char *)a,v->item_sz)) {
	    return 1;
	}
    }
    return 0;
}

/* make v1 a copy of v2, duplicating memory */
vec_t *_copy_vec(vec_t* v1,vec_t* v2)
{
    if (v1->space_sz*v1->item_sz < v2->max*v2->item_sz) {
	freemem((char *)v1->space);
	v1->space_sz = v2->max;
	v1->space = safe_calloc(1,v2->max*v2->item_sz);
    }
    v1->max = v2->max;
    v1->item_sz = v2->item_sz;
    memcpy(v1->space,v2->space,v2->max*v2->item_sz);
    return v1;
}

/* add a copy of item a to end of v */
vec_t *_ext_vec(vec_t* v,void* item)
{
    void *tmp;

    if (v->max >= v->space_sz) {
	/* double space size */
	tmp = v->space;
	v->space = safe_calloc(GROW(v->space_sz),v->item_sz);
	memcpy((char *)v->space, (char *)tmp, v->item_sz*v->space_sz);
	v->space_sz = GROW(v->space_sz);
	freemem(tmp);
    }
    memcpy((char *)v->space+v->item_sz*v->max++,(char *)item,v->item_sz);
    return v;
}

/* remove last item in v */ 
vec_t *_shorten_vecn(vec_t* v,int n)
{
    if (v->max>=n) v->max -= n;
    return v;
}
