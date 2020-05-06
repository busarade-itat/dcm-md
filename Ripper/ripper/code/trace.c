/******************************************************************************
 trace.c - general trace facility -- not being used currently 
 as the MIPS compiler gets confused...
******************************************************************************/

#include <stdio.h>

int Trace_level=1;

void set_trace_level(n)
int n;
{
    Trace_level = n;
}

int trace_level()
{
    return Trace_level;
}

/*VARARGS*/
void trace(lev,fmt,a1,a2,a3,a4,a5,a6)
char *fmt;
{
    if (lev<=Trace_level) {
	printf(fmt,a1,a2,a3,a4,a5,a6);
	fflush(stdout);
    }
}
