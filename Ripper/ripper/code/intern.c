/******************************************************************************
 intern.c - manage oblist of "symbols" 
******************************************************************************/

#include <stdio.h>
#include "ripper.h"

/* hash table sizes... */
static int Sizes[] = {5003,100003,2000003,40000003,0};
static int S = 0;

/* global data managed in this file */
typedef struct hash_tab_entry_s {
    symbol_t data;
    struct hash_tab_entry_s *next;
} hash_tab_entry_t; 

static hash_tab_entry_t **Hash_tab=NULL;

static int hash(char *);
static hash_tab_entry_t *findslot(char *,int);
static hash_tab_entry_t *addslot(char *,int);

/* map a string to the corresponding symbol, creating one if needed */
symbol_t *intern(str) 
const char *str;
{
    static int n_hashed=0;
    hash_tab_entry_t *slot,*nextslot;
    hash_tab_entry_t **old_hash_tab;
    int i;
    int index;

    /* initialize hash table if necessary */
    if (Hash_tab==NULL) {
	Hash_tab = newmem(Sizes[S],hash_tab_entry_t *);
    }

    index = hash(str);
    if ((slot=findslot(str,index))==NULL) {
	n_hashed++;
	slot = addslot(str,index);
    } 

    /* if not found, add to table */
    if (n_hashed == 2*Sizes[S]) {
	if (Sizes[S+1]==0) {
	    warning("hash table is crowded, performance may suffer");
	} else {
	    /* warning("hash table is too crowded...rehashing"); */
	    old_hash_tab = Hash_tab;
	    Hash_tab = newmem(Sizes[++S],hash_tab_entry_t *);
	    for (i=0; i<Sizes[S-1]; i++) {
		for (slot=old_hash_tab[i]; slot!=NULL; slot=nextslot) {
		    nextslot = slot->next;
		    index = hash(slot->data.name);
		    slot->next = Hash_tab[index];
		    Hash_tab[index] = slot;
		}
	    }
	    /* warning("finished rehashing"); */
	    slot=findslot(str,hash(str));
	}
    }

    return &(slot->data);
}

static int hash(char *cp)
{
    unsigned int index=0;

    /* compute hash function */
    while (*cp) index = ((index << 7) + (index << 3) + index) ^ *cp++;
    return (index%Sizes[S]);
}

static hash_tab_entry_t *findslot(char *str,int index)
{
    hash_tab_entry_t *slot;

    /* look up in table */
    for (slot=Hash_tab[index]; slot!=NULL; slot=slot->next) {
	if (strcmp(slot->data.name,str)==0) {
	    return slot;
	}
    }
    return NULL;
}

static hash_tab_entry_t *addslot(char *str,int index)
{
    hash_tab_entry_t *slot;

    /* if not found, add to table */
    slot = newmem(1,hash_tab_entry_t);
    slot->next = Hash_tab[index];
    Hash_tab[index] = slot;
    slot->data.name = newmem(strlen(str)+1, char);
    strcpy(slot->data.name,str);
    slot->data.kind = OTHER;
    slot->data.index = NULLINDEX;

    return slot;
}

void show_hashtab()
{
    int i;
    hash_tab_entry_t *slot;

    for (i=0; i<Sizes[S]; i++) {
	if (Hash_tab[i]) {
	    printf("slot %d: ",i);
	    for (slot=Hash_tab[i]; slot!=NULL; slot=slot->next) {
		printf("   "); show_symbol(&(slot->data)); printf("\n");
	    }
	}
    }
}
