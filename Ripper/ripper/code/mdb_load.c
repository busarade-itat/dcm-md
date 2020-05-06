/******************************************************************************
 mdb_load.c - load and print datasets and examples
******************************************************************************/

#include <stdio.h>
#include "ripper.h"
#include "mdb.h"

static symbol_t *Stop=NULL;
static symbol_t *Sep=NULL;
static symbol_t *Colon=NULL;
static symbol_t *Missing=NULL;

/*****************************************************************************/

/* load a dataset */

vec_t *ld_data(file)
char *file;
{
    vec_t *data;
    atom_t *tok;
    example_t *ex;
    atom_t *(*lexer)();
    double tm;

    start_clock(); 
    trace(SUMM) { 
	printf("// loading %s...\n", file? file : "<stdin>"); 
	fflush(stdout);
    }
    if (!lex_open(file)) {
	trace(SUMM) printf("// no data found on file %s\n",
			   file ? file : "<stdin>");
	return NULL;
    }

    data = new_vec(example_t);
    if (names_defined() && (symbolic_field(0) || set_field(0))) {
	lexer = &lex_word;
    } else {
	lexer = lex;
    }

    while ((tok=(*lexer)())!=NULL) {
	if (names_defined()) ex = ld_example(tok);	    
	else ex = ld_unconstrained_example(tok);	    
	if (verify_infer_names(ex)) {
 	    ext_vec(example_t,data,ex);
	}
    }
    lex_close();

    tm = elapsed_time();
    trace(SUMM) { 
	printf("// loaded %d examples %d features %d values in %.2f sec\n",
	       vmax(data),n_fields(),n_symbolic_values(),tm); 
	fflush(stdout);
    };

    return data;
}

/* example_t *ld_attribute_example(atom_t *tok): load an example

   notes:
     syntax is: atom (sep atom)* stop
     called with tok=first symbol
     ends without reading past stop
*/

example_t *ld_example(atom_t *tok)
{
    static example_t ex;
    aval_t av;
    int i;
    char cbuf[BUFSIZ];
    symbol_t *sym;

    if (!Sep) {
	Sep = intern(",");
	Stop = intern(".");
	Colon = intern(":");
	Missing = intern("?");
    }

    /* to hold set values in */
    ex.wt = 1.0;
    ex.inst = new_vecn(aval_t,vmax(Names));
    
    for (i=0; i<vmax(Names); i++) {
	/* read the i-th field, and advance to separator */ 
	if (ignored_field(i)) {
	    av.kind = MISSING_VALUE;
	    /* this might be a set field */
	    while (tok->nom!=Sep) {
		tok = safe_lex_word(); 
	    }
	} else if (tok->nom && tok->nom->kind==MISSING_MARK) {
	    av.kind = MISSING_VALUE; 
	    tok = safe_lex(); 
	} else if (continuous_field(i)) {
	    if (tok->nom!=NULL) {
		lex_error("value of field %d should be continuous",i);
		av.kind = CONTINUOUS; 		
		av.u.num = 0;
	    } else {
		av.kind = CONTINUOUS; 		
		av.u.num = tok->num;
	    }
	    tok = safe_lex(); 
	} else if (symbolic_field(i)) {
	    if (tok->nom==NULL) {
		av.kind = SYMBOL; 		
		sprintf(cbuf,"%g",tok->num);
		av.u.nom = intern(cbuf);
	    } else {
		av.kind = SYMBOL; 		
		av.u.nom = tok->nom;
	    }
	    tok = safe_lex(); 
	} else {  /* set_field(i) */
	    av.kind = SET; 
	    av.u.set = new_vec(symbol_t *); 
	    while (tok->nom!=Sep) {
		if (tok->nom==Stop) {
		    lex_error("the token '.' appears in a set-valued field");
		}
		if (tok->nom==NULL) {
		    /* warning("coercing number %g to a symbol",tok->num); */
		    sprintf(cbuf,"%g",tok->num);
		    sym = intern(cbuf);
		    ext_vec(symbol_t *,av.u.set,&sym);
		} else {
		    ext_vec(symbol_t *,av.u.set,&tok->nom);
		}
		tok = safe_lex_word();
	    }
	} /* else set_field(i) */ 
	/* check that we've advanced to comma */
	lex_verify(tok,Sep);

	/* store the new value */
	ext_vec(aval_t,ex.inst,&av);

	/* go to next token */
	tok = safe_lex();

    } /* for field i */

    /* read class label */
    ripper_copy(atom_t,&ex.lab,tok);
    tok = safe_lex();

    /* read weight if present */
    if (tok->nom==Colon) {
	tok = safe_lex();
	if (tok->nom) lex_error("weight must be numeric");
	ex.wt = tok->num;
	tok = safe_lex(); 
    }

    /* check that we're at end */
    if (tok->nom!=Stop) {
	lex_error("example doesn't end with period ...scanning for period");
	while (tok && tok->nom!=Stop) tok=lex();
	error("scanned to line %d",lex_line());
    }

    /* return new example */
    return &ex;
}

/* assumes that names are NOT defined */
example_t *ld_unconstrained_example(atom_t *tok)
{
    static example_t ex;
    aval_t av;
    symbol_t *last_nom;
    int i;
    char cbuf[BUFSIZ];
    symbol_t *sym;

    if (!Sep) {
	Sep = intern(",");
	Stop = intern(".");
	Colon = intern(":");
	Missing = intern("?");
    }

    /* to hold set values in */
    ex.wt = 1.0;
    ex.inst = new_vec(aval_t);
    
    for (i=0; TRUE; i++) {
	/* read the i-th field */ 
	if (tok->nom && tok->nom->kind==MISSING_MARK) {
	    av.kind = MISSING_VALUE; 
	    tok = safe_lex(); 
	} else if (tok->nom==NULL) {
	    av.kind = CONTINUOUS; 		
	    av.u.num = tok->num;
	    tok = safe_lex(); 
	} else {
	    last_nom = tok->nom;
	    tok = safe_lex();
	    if (tok->nom==Sep || tok->nom==Stop) {
		/* symbolic field */
		av.kind = SYMBOL; 		
		av.u.nom = last_nom;
	    } else {
		/* set valued field */
		av.kind = SET; 
		av.u.set = new_vec(symbol_t *); 
		ext_vec(symbol_t *,av.u.set,&last_nom);
		while (tok->nom != Sep) {
		    if (tok->nom==NULL) {
			/* warning("coercing number %g to a symbol",tok->num); */
			sprintf(cbuf,"%g",tok->num);
			sym = intern(cbuf);
			ext_vec(symbol_t *,av.u.set,&sym);
		    } else {
			if (tok->nom==Stop) {
			    lex_error(
				"the token '.' appears in a set-valued field");
			}
			ext_vec(symbol_t *,av.u.set,&tok->nom);
		    }
		    tok = safe_lex_word();
		} /* while tok->nom!=Sep */
	    } /* else set */
	} /* else symbol */
	
	/* now see if we're at a comma, period, or colon */
	if (tok->nom == Stop || tok->nom == Colon) {
	    if (av.kind != SYMBOL) {
		lex_error("class must be symbolic");
		av.kind = SYMBOL;
		av.u.nom = Missing;
	    } 
	    ex.lab.nom = av.u.nom;
	    if (tok->nom==Colon) {
		tok = safe_lex();
		if (tok->nom) lex_error("weight must be numeric");
		ex.wt = tok->num;
		tok = safe_lex(); 
	    }
	    lex_verify(tok,Stop);
	    break; /* exit for loop */
	} else {
	    /* store the new value */
            lex_verify(tok,Sep);
            tok = safe_lex();
	    ext_vec(aval_t,ex.inst,&av);
	}
    } /* for field i */

    /* return new example */
    return &ex;
}

/*****************************************************************************/

/* void print_example(example_t *ex): print an example
*/

void fprint_aval(fp,av)
FILE *fp;
aval_t *av;
{
    int i;

    if (av->kind==SYMBOL) { 
	fprint_symbol(fp,av->u.nom);
    } else if (av->kind==CONTINUOUS) {
	fprintf(fp,"%g",av->u.num);
    } else if (av->kind==MISSING_VALUE) {
	fprintf(fp,"?");
    } else if (av->kind==SET) {
	for (i=0; i<vmax(av->u.set); i++) {
	    fprint_symbol(fp,*vref(symbol_t *,av->u.set,i));
	    if (i<vmax(av->u.set)-1) fprintf(fp," ");
	}
    } else {
	fprintf(fp,"???");      
    }
}



void fprint_example(fp,ex)
FILE *fp;
example_t *ex;
{
    int i;
    
    for (i=0; i<vmax(ex->inst); i++) {
	fprint_aval(fp,vref(aval_t,ex->inst,i));
	fprintf(fp,",");
    }
    fprint_atom(fp,&ex->lab);
    fprintf(fp,".\n");
}

/* void print_data(vec_t *dataset): print a whole dataset
*/

void fprint_data(fp,data)
FILE *fp;
vec_t *data;
{
    int i;
    
    if (!data) {
	fprintf(fp,"<null dataset>\n");
    } else {
	for (i=0; i<vmax(data); i++) {
	    fprint_example(fp,vref(example_t,data,i));
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
	fatal("syntax: test-dataset names-file data-file");
    }
    if (ld_names(argv[1]))
      print_names();
    else
      fatal("no names file!");

    printf("\n");
    print_data(ld_data(argv[2])); 
    printf("\n");
    print_names();
}
#endif

