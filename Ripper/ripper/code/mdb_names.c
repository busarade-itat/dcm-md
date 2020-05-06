/******************************************************************************
 mdb_names.c - handle attribute definitions
******************************************************************************/

#include <stdio.h>
#include "ripper.h"
#include "mdb.h"

static void update_field_values(int,symbol_t *);
static void update_class(atom_t *);
static void update_atom_values(atom_t *,attr_def_t *);
static void update_values(symbol_t *,attr_def_t *);
static void init_values(attr_def_t *);
static void bagify(vec_t *bag);
static BOOL valid_value(atom_t *,attr_def_t *);

/*****************************************************************************/

/* global data managed in this file */

/* a vector of attr_def_t */
vec_t *Names = NULL;
/* a vector of atom_t
   note: a vector of (symbol_t *) would be appropriate,
   but vectors of pointers are a bit clumsy....
*/
vec_t *Classes = NULL;

/* next index to assign to a field value */
static int N_values=0;

/* flag: has a names file been read in or not? */
static BOOL Names_defined = FALSE;

#define signsep(r) ((r)<0?" ":"")


/*****************************************************************************/
/* interface to information from names file */

void set_names_defined(){
	Names_defined = TRUE;
}

BOOL names_defined()
{
	return Names_defined;
}

int n_symbolic_values()
{
	return N_values;
}

int n_fields()
{
	return vmax(Names);
}

char *field_name(int i)
{
	attr_def_t *adefp;

	adefp = vref(attr_def_t,Names,i);
	return adefp->name->name;
}

BOOL continuous_field(int i)
{
	return vref(attr_def_t,Names,i)->kind == CONTINUOUS;
}

BOOL ignored_field(int i)
{
	return vref(attr_def_t,Names,i)->kind == IGNORE;
}

BOOL suppressed_field(int i)
{
	return vref(attr_def_t,Names,i)->suppressed;
}


BOOL symbolic_field(int i)
{
	return vref(attr_def_t,Names,i)->kind == SYMBOL;
}

BOOL set_field(int i)
{
	return vref(attr_def_t,Names,i)->kind == SET;
}

static int rless_than(REAL *r1,REAL *r2)
{
	if (*r1>*r2) return 1;
	else if (*r1<*r2) return -1;
	else return 0;
}

BOOL n_different_field_values(int i,DATA *data)
{
	static REAL *rtmp;
	int m,k;
	aval_t *vk;
	REAL last,this;
	int n_values;

	if (symbolic_field(i) || set_field(i)) {
		return vmax(vref(attr_def_t,Names,i)->values);
	} else {
		/* compute the number of distinct non-missing real values */
		n_values = 0;
		if (rtmp==NULL) rtmp=newmem(vmax(data),REAL);
		for (m=0,k=0; k<vmax(data); k++) {
			/*
             * TODO
             * Comprendre pour les asserts (vref à la place de _vref)
             * ne valident pas ces cas.
             */
			vk = vref(aval_t,vref(example_t,data,k)->inst,i);
			//vk = (aval_t*) _vref(((example_t*) _vref(data,k))->inst,i);
			if (vk->kind!=MISSING_VALUE) /* ie a number, not "?" */ {
				rtmp[m++] = vk->u.num;
			}
		}
		if (vmax(data)>0) {
			qsort((char *)rtmp,vmax(data),sizeof(REAL),rless_than);
			last = rtmp[0];
			n_values++;
			for (k=1;k<vmax(data);k++) {
				this = rtmp[k];
				if (this!=last) {
					last=this;
					n_values++;
				}
			}
		}
		return n_values;
	}
}

/*****************************************************************************/

/* recursive-descent parser for names files
   
   syntax:

   names_file ::- classes stop (attr_def stop)*

   attr_def ::- id ':' ['suppressed'] 'continuous'
   attr_def ::- id ':' ['suppressed'] 'symbolic' [ atom (', ' atom)* ]
   attr_def ::- id ':' ['suppressed'] atom (', ' atom)*
*/

static void ld_classes(atom_t *);
static void ld_attr_def(atom_t *);

/* punctuation used in parsing */
static symbol_t *Sep, *Stop, *Continuous, *Symbolic,
		*Set, *Bag, *Ignore, *Colon, *Suppressed;

BOOL ld_names(file)
		char *file;
{
	FILE *fp;
	atom_t *tok;

	if (!Sep) {
		Sep = intern(",");
		Stop = intern(".");
		Colon = intern(":");
		Continuous = intern("continuous");
		Symbolic = intern("symbolic");
		Set = intern("set");
		Bag = intern("bag");
		Ignore = intern("ignore");
		Suppressed = intern("suppressed");
	}

	intern("?")->kind = MISSING_MARK;

	if (!Names) Names = new_vec(attr_def_t);
	else clear_vec(attr_def_t,Names);
	if (!Classes) Classes = new_vec(atom_t);
	else clear_vec(atom_t,Classes);
	if (!lex_open(file)) {
		warning("can't open names file %s",file);
		return FALSE;
	}
	tok = lex();
	if (tok!=NULL) {
		ld_classes(tok);
		while ((tok=lex())!=NULL) {
			ld_attr_def(tok);
		}
		lex_close();
		Names_defined = TRUE;
		return TRUE;
	} else {
		/* names file present but empty */
		lex_close();
		return FALSE;
	}
}

static void ld_classes(tok)
		atom_t *tok;
{
	int i;

	if (!tok->nom) lex_error("class %d is not a symbol",vmax(Classes));
	else ext_vec(atom_t,Classes,tok);

	while ((tok=safe_lex())->nom!=Stop) {
		lex_verify(tok,Sep); tok = safe_lex();
		if (!tok->nom) lex_error("class %d is not a symbol",vmax(Classes));
		else ext_vec(atom_t,Classes,tok);
	}

	/* mark these symbols as classes */
	for (i=0; i<vmax(Classes); i++) {
		vref(atom_t,Classes,i)->nom->index = i;
		vref(atom_t,Classes,i)->nom->kind = CLASS;
	}
}

static void ld_attr_def(tok)
		atom_t *tok;
{
	char cbuf[BUFSIZ];
	attr_def_t adef;
	/* record the attribute name */
	if (!tok->nom) {
		lex_error("attribute name %d is not a symbol",vmax(Names));
		sprintf(cbuf,"a<%d>",vmax(Names));
		adef.name = intern(cbuf);
	} else {
		adef.name = tok->nom;
	}
	/* mark this as an attribute  */
	if (adef.name->kind!=OTHER) {
		lex_error("attribute %d also used as class or operator name",vmax(Names));
		sprintf(cbuf,"a<%s>",adef.name->name);
		adef.name = intern(cbuf);
	}

	/* initialize adef */
	init_values(&adef);
	adef.name->kind = ATTRIBUTE;
	adef.name->index = vmax(Names);

	/* skip the ':' marker */
	tok = safe_lex(); lex_verify(tok,Colon); tok=safe_lex();

	/* check for suppressed keyword */
	if (tok->nom==Suppressed) {
		adef.suppressed = TRUE;
		tok = safe_lex();
	}

	/* parse the remainder */
	if (tok->nom==Continuous) {
		adef.kind = CONTINUOUS;
		tok = safe_lex();
		lex_verify(tok,Stop);
	} else if (tok->nom==Symbolic) {
		adef.kind = SYMBOL;
		tok = safe_lex();
		lex_verify(tok,Stop);
	} else if (tok->nom==Set || tok->nom==Bag) {
		adef.kind = SET;
		adef.isbag = (tok->nom==Bag);
		tok = safe_lex();
		lex_verify(tok,Stop);
	} else if (tok->nom==Ignore) {
		adef.kind = IGNORE;
		tok = safe_lex();
		lex_verify(tok,Stop);
	} else {
		adef.kind = SYMBOL;
		update_atom_values(tok,&adef);
		while ((tok=safe_lex())->nom != Stop) {
			lex_verify(tok,Sep);
			update_atom_values(safe_lex(),&adef);
		}
	}

	/* save the new definition */
	ext_vec(attr_def_t,Names,&adef);
}

/*****************************************************************************/

/* called before an example is stored in a dataset */
BOOL verify_infer_names(ex)
		example_t *ex;
{
	int i,j;
	attr_def_t adef, *adefp;
	aval_t *avp;
	char cbuf[BUFSIZ];
	BOOL all_attrib_defined;

	/* allocate names table */
	if (!Names) {
		Names = new_vec(attr_def_t);
	}
	if (vmax(Names)==0) {
		for (i=0; i<vmax(ex->inst); i++) {
			sprintf(cbuf,"a%d",i+1);
			adef.name = intern(cbuf);
			adef.kind = vref(aval_t,ex->inst,i)->kind;
			init_values(&adef);
			ext_vec(attr_def_t,Names,&adef);
		}
	}
	if (vmax(Names) < vmax(ex->inst)) {
		lex_error("too many fields in example");
	} else if (vmax(Names) > vmax(ex->inst)) {
		lex_error("too few fields in example");
	} else {
		/* check type consistency */
		all_attrib_defined = TRUE;
		for (i=0; i<vmax(Names); i++) {
			adefp = vref(attr_def_t,Names,i);
			avp = vref(aval_t,ex->inst,i);
			if (adefp->kind == IGNORE) {
				/* don't worry, be happy */;
			} else if (adefp->kind == MISSING_VALUE) {
				if (avp->kind == MISSING_VALUE) {
					all_attrib_defined = FALSE;
				} else {
					adefp->kind = avp->kind;
				}
			} else if (avp->kind != MISSING_VALUE) {
				if (adefp->kind != avp->kind) {
					lex_error("field %d is wrong type (%d, should be %d)",
							  i,avp->kind,adefp->kind);
				} else {
					/* types are correct, nothing missing */
					if (avp->kind == SYMBOL) {
						update_field_values(i,avp->u.nom);
					} else if (avp->kind == SET) {
						if (adefp->isbag) bagify(avp->u.set);
						for (j=0; j<vmax(avp->u.set); j++) {
							update_field_values(i,*vref(symbol_t *,avp->u.set,j));
						}
					}
				} /* else types are ok */
			}
		} /* for field i */
		if (all_attrib_defined) Names_defined = TRUE;
		update_class(&ex->lab);
	}

	return TRUE;
}

/*****************************************************************************/

static void update_field_values(int i,symbol_t *s)
{
	update_values(s,vref(attr_def_t,Names,i));
}

static void update_class(atom_t *a)
{
	if (!Classes) Classes = new_vec(atom_t);
	if (a && a->nom && !vmem(atom_t,Classes,a)) {
		ext_vec(atom_t,Classes,a);
		a->nom->kind = CLASS;
		a->nom->index = vmax(Classes)-1;
	}
}

/*****************************************************************************/

static int index_in_tree(symbol_t *,symbol_tree_t *);
static void fshow_symbol_tree(FILE *,symbol_tree_t *);
static symbol_tree_t *update_symbol_tree(symbol_t *,symbol_tree_t *,int,BOOL *);

static void init_values(adefp)
		attr_def_t *adefp;
{
	adefp->values = new_vec(symbol_t *);
	adefp->value_index = NULL;
	adefp->suppressed = FALSE;
}

static void update_atom_values(a,adefp)
		atom_t *a;
		attr_def_t *adefp;
{
	symbol_t *sym;
	char buf[BUFSIZ];

	if (a->nom==NULL) {
		sprintf(buf,"%g",a->num);
		sym = intern(buf);
	} else {
		sym = a->nom;
	}
	update_values(sym,adefp);
}

static void update_values(sym,adefp)
		symbol_t *sym;
		attr_def_t *adefp;
{
	int pos;
	BOOL inserted;
	char *sym_kind_name[] = {
			"?undefined?",
			"missing value marker",
			"wildcard",
			"attribute",
			"attribute or set value",
			"operator",
			"nonterminal symbol",
			"class"
	};
	static vec_t *misused=NULL;

	if (!misused) misused=new_vec(symbol_t);

	if (sym!=NULL) {
		if (sym->kind==OTHER) {
			sym->kind = VALUE;
			sym->index = N_values++;
		} else if (sym->kind!=VALUE) {
			if (!vmem(symbol_t,misused,sym)) {
				lex_error("symbol '%s' is used as type 'attribute value' and also as type '%s'",
						  sym->name,sym_kind_name[sym->kind]);
				warning("attribute value '%s' will be ignored in learning",sym->name);
				ext_vec(symbol_t,misused,sym);
				if (vmax(misused)>50) fatal("too many over-used symbols");
			}
		}
		pos = vmax(adefp->values);
		inserted = FALSE;
		adefp->value_index = update_symbol_tree(sym,adefp->value_index,pos,&inserted);
		if (inserted) ext_vec(symbol_t *,adefp->values,&sym);
	} else {
		lex_error("symbolic attribute %s has continuous value",adefp->name->name);
	}
}

int attr_value_index(sym,adefp)
		symbol_t *sym;
		attr_def_t *adefp;
{
	return index_in_tree(sym,adefp->value_index);
}

/*****************************************************************************/

static void free_symbol_tree(symbol_tree_t *tree);

/* modify bag: replace i-th occurance of term "t" with term "t*i", for i>1 */
static void bagify(vec_t *bag)
{
	static int *count = 0;
	static int count_size = 0;
	symbol_tree_t *symtree;
	symbol_t *s,*snew;
	int is;
	int nextindex;
	BOOL inserted;
	int i;
	char buf[BUFSIZ];

	/* symtree maps symbols s to index i(s) */
	symtree = NULL;
	nextindex = 0;
	for (i=0; i<vmax(bag); i++) {
		s = *vref(symbol_t *,bag,i);
		inserted = FALSE;
		symtree = update_symbol_tree(s,symtree,nextindex,&inserted);
		if (inserted) nextindex++;
	}
	/* count[i(s)] records number of previous occurances of symbol s */
	if (count_size<nextindex || count==0) {
		freemem(count);
		count = newmem(nextindex,int);
		count_size = nextindex;
	}
	for (i=0; i<nextindex; i++) count[i]=0;

	/* do the replacement */
	for (i=0; i<vmax(bag); i++) {
		s = *vref(symbol_t *,bag,i);
		is = index_in_tree(s,symtree);
		count[is]++;
		if (count[is]>1) {
			sprintf(buf,"%s__%d",s->name,count[is]);
			snew = intern(buf);
			vset(symbol_t *,bag,i,&snew);
		}
	}

	/* clean up */
	free_symbol_tree(symtree);
}

/*****************************************************************************/

/* tree utilities */

static int index_in_tree(sym,tree)
		symbol_t *sym;
		symbol_tree_t *tree;
{
	double r;

	if (tree==NULL) return NULLINDEX;
	else {
		r = strcmp(sym->name,tree->cont->name);
		if (r==0) return tree->index;
		else if (r<0) return index_in_tree(sym,tree->left);
		else return index_in_tree(sym,tree->right);
	}
}

static symbol_tree_t *update_symbol_tree(sym,tree,pos,ins)
		symbol_t *sym;
		symbol_tree_t *tree;
		int pos;
		BOOL *ins;
{
	symbol_tree_t *new;
	double r;
	BOOL ins_l,ins_r;

	if (tree==NULL) {
		new = newmem(1,symbol_tree_t);
		new->cont = sym;
		new->index = pos;
		new->left = new->right = NULL;
		(*ins) = TRUE;
		return new;
	} else {
		r = strcmp(sym->name,tree->cont->name);
		ins_l = ins_r = FALSE;
		if (r<0) {
			tree->left = update_symbol_tree(sym,tree->left,pos,&ins_l);
		} else if (r>0) {
			tree->right = update_symbol_tree(sym,tree->right,pos,&ins_r);
		}
		(*ins) = (ins_l || ins_r);
		return tree;
	}
}

static void free_symbol_tree(tree)
		symbol_tree_t *tree;
{
	if (tree!=NULL) {
		free_symbol_tree(tree->left);
		tree->left = NULL;
		free_symbol_tree(tree->right);
		tree->right = NULL;
		freemem(tree);
	}
}

static void fshow_symbol_tree(fp,tree)
		FILE *fp;
		symbol_tree_t *tree;
{
	if (tree==NULL) fprintf(fp,"*");
	else {
		fprint_symbol(fp,tree->cont);
		fprintf(fp,"(");
		fshow_symbol_tree(fp,tree->left);
		fprintf(fp,",");
		fshow_symbol_tree(fp,tree->right);
		fprintf(fp,")");
	}
}

/*****************************************************************************/


void print_attr_def(adef)
		attr_def_t *adef;
{
	int i,end;
	char *elide;

	print_symbol(adef->name);
	printf(":\t");
	if (adef->kind==CONTINUOUS) {
		printf("continuous.\n");
	} else if (adef->kind==SYMBOL) {
		if (vmax(adef->values)==0) {
			printf("dummy_value1, dummy_value2.\n");
		} else if (vmax(adef->values)==1) {
			print_symbol(*vref(symbol_t *,adef->values,0));
			printf(", dummy_value.\n");
		} else {
			for (i=0; i<vmax(adef->values); i++) {
				print_symbol(*vref(symbol_t *,adef->values,i));
				printf( i<vmax(adef->values)-1 ? ", " : ".\n");
			}
		}
	} else if (adef->kind==SET) {
		if (adef->isbag) printf("bag.\n");
		else printf("set.\n");
	} else if (adef->kind==MISSING_VALUE) {
		printf("unknown.\n");
	} else {
		printf(" ???\n");
	}
}

void print_names()
{
	int i;

	if (!Names) {
		printf("<attributes undefined>");
	} else {
		for (i=0; i<vmax(Classes); i++) {
			print_atom(vref(atom_t,Classes,i));
			printf( i==vmax(Classes)-1 ? ".\n" : ",");
		}
		for (i=0; i<vmax(Names); i++) {
			print_attr_def(vref(attr_def_t,Names,i));
		}
	}
}



#ifdef TEST
/*****************************************************************************/
/* main: test driver
*/

main(argc,argv)
int argc;
char *argv[];
{
    if (argc<=2) {
	fatal("syntax: test-dataset namesfile datafile");
    }
    (void) ld_names(argv[1]);
    (void) ld_data(argv[2]);
    print_names();
}
#endif

