/******************************************************************************
 error.c - print error messages
******************************************************************************/

#include <stdio.h>

#define ERROR_LIMIT 20

static int total_errors=0;

/*VARARGS*/
void fatal(fmt,a1,a2,a3,a4,a5,a6)
char *fmt;
{
    fprintf(stderr,"fatal: ");
    fprintf(stderr,fmt,a1,a2,a3,a4,a5,a6);
    fprintf(stderr,"\n");
    exit(1);
}

/*VARARGS*/
void error(fmt,a1,a2,a3,a4,a5,a6)
char *fmt;
{
    fprintf(stderr,"error: ");
    fprintf(stderr,fmt,a1,a2,a3,a4,a5,a6);
    fprintf(stderr,"\n");

    if (++total_errors > ERROR_LIMIT) {
	fprintf(stderr,"error limit exceeded, goodbye\n");
	exit(1);
    }
}

/*VARARGS*/
void warning(fmt,a1,a2,a3,a4,a5,a6)
char *fmt;
{
    fprintf(stderr,"warning: ");
    fprintf(stderr,fmt,a1,a2,a3,a4,a5,a6);
    fprintf(stderr,"\n");
}

/*VARARGS*/
void dont_printf()
{
}

/*VARARGS*/
void lex_error(fmt,a1,a2,a3,a4,a5,a6)
char *fmt;
{
    char *lex_file(void);
    int lex_line(void);

    fprintf(stderr,"%s:%d: ",lex_file(),lex_line());
    fprintf(stderr,fmt,a1,a2,a3,a4,a5,a6);
    fprintf(stderr,"\n");
    if (++total_errors > ERROR_LIMIT) {
	fprintf(stderr,"error limit exceeded, goodbye\n");
	exit(1);
    }
}

