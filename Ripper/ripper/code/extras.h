/*****************************************************************************
 extras.h -- commonly useful prototypes/macros
*****************************************************************************/
#ifndef __EXTRA_H_
#define __EXTRA_H_

/* some extensions to C.... */
#include "vector.h"
//#include <string.h>

/* memory.c functions */
extern char *safe_calloc(int,int);
extern int safe_free(char *);
#define newmem(n,type)  ((type *) safe_calloc((n),sizeof(type)))
#define freemem(addr)   safe_free((char *) addr)

/* error.c functions */
extern void fatal(char *,...);
extern void error(char *,...);
extern void warning(char *,...);
extern void lex_error(char *,...);
extern void dont_printf(char *,...);

/* time.c functions */
extern void start_clock(void);
extern double elapsed_time(void);
extern void randomize(void);

/* trace.c functions */
typedef enum { NONE=0, SUMM=1, LONG=2, DBUG=3, DBG1=4 } trace_lev_t;
extern int Trace_level;

#define set_trace_level(n)    (Trace_level = n)
#define trace_level()         (Trace_level)
#define trace(lev)            if (lev<=Trace_level)

/* some useful functions ... */


#ifndef GCC
extern char *memcpy(char *,char *,int);
extern int memcmp(char *,char *,int);
extern int strcmp(char *,char *);
extern char *strcpy(char *,char *);
extern int strlen(char *);
#endif

extern double atof(const char *);
extern long random(void);
extern int strncmp(char *,char *,int);
extern int qsort(char *,int,int,...);
extern int atoi(char *);
extern long random();
#define ripper_max(a,b)  ((a)>(b)?(a):(b))
/* portable log base 2 function */
#define	 LN2_CONST		0.69314718055994530942
#define	 Log2(x)		((x) <= 0 ? 0.0 : log((float)x) / LN2_CONST)
/* generic copy,equality routines */
#define ripper_copy(type,a1,a2)  ((type *)memcpy((char *)a1,(char *)a2,sizeof(type)))
#define ripper_equal(type,a1,a2)  (!memcmp((char *)a1,(char *)a2,sizeof(type)))

#define streq(s,t) (strcmp((s),(t))==0)

#endif