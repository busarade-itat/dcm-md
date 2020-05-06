#include <stdio.h>
#include <math.h>
#include "ripper.h"

extern char *Help_str[];
extern char *Program;

char *add_ext(str,ext)
char *str, *ext;
{
    char *ret;

    ret = newmem(strlen(str)+strlen(ext)+1,char);
    sprintf(ret,"%s%s",str,ext);
    return ret;
}

void give_help()
{
    static BOOL gave_help=FALSE;
    int i;

    //printf("Program: %s --- %s\n\n",Program,VERSION);
    if (!gave_help) {
	//for (i=0; Help_str[i]; i++) printf("%s\n",Help_str[i]);
	gave_help = TRUE;
    }
}

