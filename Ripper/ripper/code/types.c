/******************************************************************************
 types.c - implement basic types: vector and atom
******************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "ripper.h"

static BOOL needs_quote(char *);

/*****************************************************************************/

void fprint_symbol(fp,s)
FILE *fp;
symbol_t *s;
{
    if (needs_quote(s->name)) fprintf(fp,"'%s'",s->name);
    else fprintf(fp,"%s",s->name);
}

void show_symbol(s) /* debugging print function */
symbol_t *s;
{
    printf("<symbol %s type=%d indx=%d>",s->name,s->kind,s->index);
}


void fprint_atom(fp,a)
FILE *fp;
atom_t *a;
{
    if (a->nom) {
	if (needs_quote(a->nom->name)) 
	  fprintf(fp,"'%s'",a->nom->name);
	else 
	  fprintf(fp,"%s",a->nom->name);
    }
    else fprintf(fp,"%g",a->num);
}

/* print function for debugging */
void show_atom(a)
atom_t *a;
{
    if (a->nom) printf("<atom%o:'%s'>",a->nom,a->nom->name);
    else printf("<atom%o:%.3f>",a,a->num);
}

static BOOL needs_quote(str)
char *str;
{
    if (strcmp(str,"?")==0) {
	return FALSE;
    } else if (!isalpha(*str) && (*str)!='_') {
	return TRUE;
    }
    while (*++str) {
	if (!isalnum(*str) && (*str)!='_') {
	    return TRUE;
	} 
    }
    return FALSE;
}

