/******************************************************************************
 lex.c - lexical analysis

 notes:
  lex_ch points to next unread input
  tokens: letter [alnum|'_']*, special, punct+, or a C number
    where special are isspecial characters (see macro below)
******************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "ripper.h"

/* some data that is only locally used */

static atom_t token;
static FILE *lex_fp;
static char *lex_filename;
static int lex_ch;
static int lex_lineno;


#define isspecial(c) \
  ((c)==',' || (c)=='.' || (c)=='(' || (c)==')' || (c)=='?')

/*****************************************************************************/

/* lex_open(char *file_name): open file for lexing; NULL means open stdin
 * lex_close(void): close lex file
*/

BOOL lex_open(name)
char *name;
{
    if (name!=NULL) {
	lex_fp=fopen(name,"r");
	lex_filename = name;
    } else {
	lex_fp = stdin;
	lex_filename = "standard input";
    }
    
    if (lex_fp!=NULL) {

	lex_ch = getc(lex_fp);
    }
    lex_lineno = 1;
    return lex_fp!=NULL;
}

void lex_close()
{
    lex_lineno = 0;
    if (lex_fp) fclose(lex_fp);
}

/* lex(): return next token
*/

atom_t *lex()
{
    static char string[BUFSIZ]; 
    REAL number=0.0, sign, esign, place, exponent;
    int len=0;

    /* initialize because equal atoms
     * sometimes need to be bitsize identical */
    token.nom = NULL;
    token.num = 0.0;
    start_lexing:
    while (isspace(lex_ch))  {
	if (lex_ch=='\n') lex_lineno++;
	lex_ch = getc(lex_fp);
    }
    printf("lex_ch : %u, EOF : %u \n",lex_ch, EOF);
    if (lex_ch==EOF)
        return NULL;
    if (isalpha(lex_ch) || lex_ch=='_') {
	goto read_symbol;
    } else if (lex_ch=='\'') {
	goto read_quote;
    } else if (isdigit(lex_ch)) {
	sign = 1.0;
	goto read_number;
    } else if (lex_ch=='%') {
	while (lex_ch!='\n' && lex_ch!=EOF) 
	  lex_ch = getc(lex_fp);
	goto start_lexing;
    } else if (lex_ch=='-') {
	lex_ch = getc(lex_fp);
	if (isdigit(lex_ch)) {
	    sign = -1.0;
	    goto read_number;
	} else {
	    ungetc(lex_ch,lex_fp);
	    lex_ch = '-';
	    goto read_punct;
	}
    } else if (isspecial(lex_ch)) {
	goto read_special;
    } else if (ispunct(lex_ch)) {
	goto read_punct;
    } else {
	error("illegal char %c at line %d" ,lex_ch, lex_lineno);
	lex_ch = getc(lex_fp);
	goto start_lexing;
    }

    read_symbol:
    /* alpha alnum* */
    while (isalnum(lex_ch) || lex_ch=='_') {
	string[len++] = lex_ch;
	lex_ch = getc(lex_fp);
    }
    string[len]='\0';
    token.nom = intern(string);
    return &token;

    read_quote:
    lex_ch = getc(lex_fp);
    while (lex_ch!='\'' && lex_ch!='\n' && lex_ch!=EOF) {
	string[len++] = lex_ch;
	lex_ch = getc(lex_fp);
    }
    string[len]='\0';
    if (lex_ch!='\'') lex_error("missing close quote for '%8s...'",string);
    else lex_ch = getc(lex_fp);
    token.nom = intern(string);
    return &token;

    read_number:
    /* digit+ [. digit+][e digit+] */
    while (isdigit(lex_ch)) {
	number = number*10 + lex_ch-'0';
	lex_ch = getc(lex_fp);
    }
    if (lex_ch=='.') {
	lex_ch = getc(lex_fp);
	if (isdigit(lex_ch)) {
	    place = 1.0;
	    while (isdigit(lex_ch)) {
		place /= 10;
		number += place*(lex_ch-'0');
		lex_ch = getc(lex_fp);
	    }
	} else {
	    ungetc('.',lex_fp);
	}
    } 
    if (lex_ch=='e' || lex_ch=='E') {
	lex_ch = getc(lex_fp);
	exponent = 0.0;
	esign = 1.0; 
	if (lex_ch=='+') { 
	    lex_ch=getc(lex_fp); 
	}
	else if (lex_ch=='-') { 
	    esign = -1.0; 
	    lex_ch=getc(lex_fp); 
	}
	while (isdigit(lex_ch)) {
	    exponent = exponent*10 + (lex_ch-'0');
	    lex_ch = getc(lex_fp);
	}
	number *= pow(10.0,esign*exponent);
    }
    token.nom = NULL;
    token.num = sign*number;
    return &token;

    read_special:
    /* special */
    string[0] = lex_ch;
    string[1]='\0';
    lex_ch = getc(lex_fp);
    token.nom = intern(string);
    return &token;

    read_punct:
    /* punct+ */
    while (ispunct(lex_ch) && !isspecial(lex_ch) && lex_ch!='\'') {
	string[len++] = lex_ch;
	lex_ch = getc(lex_fp);
    }
    string[len] = '\0';
    token.nom = intern(string);
    return &token;
}

/* lex_word(): return next token, using
 * simple tokenization rule that only allows only 
 * the tokens comma, period, quoted strings,
 * and white space delimited non-white-space strings
*/
atom_t *lex_word()
{
    static char string[BUFSIZ]; 
    int len=0;

    /* initialize because equal atoms
     * sometimes need to be bitsize identical */
    token.nom = NULL;
    token.num = 0.0;

    start_lexing:
    while (isspace(lex_ch))  {
	if (lex_ch=='\n') lex_lineno++;
	lex_ch = getc(lex_fp);
    }
    if (lex_ch==EOF) 
	return NULL;

    if (lex_ch=='.' || lex_ch==',') {
	string[0] = lex_ch;
	string[1]='\0';
	lex_ch = getc(lex_fp);
	token.nom = intern(string);
	return &token;
    } else if (lex_ch=='\'') {
	lex_ch = getc(lex_fp);
	while (lex_ch!='\'' && lex_ch!='\n' && lex_ch!=EOF) {
	    string[len++] = lex_ch;
	    lex_ch = getc(lex_fp);
	}
	string[len]='\0';
	if (lex_ch!='\'') lex_error("missing close quote for '%8s...'",string);
	else lex_ch = getc(lex_fp);
	token.nom = intern(string);
	return &token;
    } else if (isprint(lex_ch)) {
	while (isprint(lex_ch) && lex_ch!=EOF 
	       && lex_ch!='.' && lex_ch!=',' && !isspace(lex_ch)) 
	{
	    string[len++] = lex_ch;
	    lex_ch = getc(lex_fp);
	}
	string[len]='\0';
	token.nom = intern(string);
	return &token;
    } else {
	error("illegal char %c at line %d" ,lex_ch, lex_lineno);
	lex_ch = getc(lex_fp);
	goto start_lexing;
    }
}

/*****************************************************************************/

/* int lex_line(void): current line number
/* char *lex_file(void): current file name
 * lex_verify(atom_t *tok, symbol_t *s): check that tok contains s
 * atom_t safe_lex(void):  read, check for eof
 * atom_t last_lex(void):  return last token read
*/

int lex_line()
{
    return lex_lineno;
}

char *lex_file()
{
    return lex_filename;
}

void lex_verify(tok,s)
atom_t *tok;
symbol_t *s;
{
    char buf[BUFSIZ];

    if (!tok || !tok->nom || tok->nom!=s) {
	if (tok->nom) sprintf(buf,"%s",tok->nom->name);
	else sprintf(buf,"%g",tok->num);
	lex_error("expected the token '%s' but saw '%s'",s->name,buf);
    }
}

atom_t *safe_lex()
{
    atom_t *tok=lex(); 
    if (!tok) {
	lex_error("premature end of file");
	token.nom = intern(".");
	return &token;
    } else {
	return tok;
    }
}

atom_t *safe_lex_word()
{
    atom_t *tok=lex_word(); 
    if (!tok) {
	lex_error("premature end of file");
	token.nom = intern(".");
	return &token;
    } else {
	return tok;
    }
}

atom_t *last_lex()
{
    return &token;
}

#ifdef TEST
/*****************************************************************************/
/* main: test driver
*/

main(argc,argv)
int argc;
char *argv[];
{
    atom_t *tok;

    if (argc<=1) {
	lex_fp = stdin;
	lex_ch = getchar();
    }
    else (lex_open(argv[1]));

    while (tok=lex()) {
	show_atom(tok);
	printf("\n");
    }
    if (argc>1) lex_close();
}
#endif

