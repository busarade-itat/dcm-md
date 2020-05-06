#include <stdio.h>

extern char *calloc(unsigned int,int);
extern int free(char *);

char *safe_calloc(int n, int size)
{
    char *space;

    space = calloc((unsigned)n,(unsigned)size);

    if (space==NULL) {
	fatal("out of memory");
    }
    return space;
}

safe_free(space)
char *space;
{
    free(space);
}

#ifdef TEST
main()
{
    int n=999,s=12;
    int c;
    
    for (c=0;safe_calloc(n,s)!=NULL;c++)
      ;
    printf("allocated %d times\n",c);
}
fatal(str)
char *str;
{
    fprintf(stderr,str);
}
#endif
