/*****************************************************************************
 time.c -- implement a simple timer
*****************************************************************************/

#ifdef PORTABLE
void start_clock() {;}
double elapsed_time() { return 0.0; } 
void randomize() {;}
#else

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

static long User_last_microsec=0;
static long System_last_microsec=0;
static long User_last_sec=0;
static long System_last_sec=0;


void start_clock()
{
    struct rusage use;

    getrusage(RUSAGE_SELF,&use);
    User_last_sec = use.ru_utime.tv_sec;
    User_last_microsec = use.ru_utime.tv_usec;
    System_last_sec = use.ru_stime.tv_sec;
    System_last_microsec = use.ru_stime.tv_usec;
}

/* return time since last start_clock() 
*/
double elapsed_time() 
{
    double t=0.0;
    struct rusage use;

    getrusage(RUSAGE_SELF,&use);
    t += use.ru_utime.tv_sec - User_last_sec;
    t += ((double)((use.ru_utime.tv_usec - User_last_microsec)/10000))/100.0;
#ifdef NEVER
    /*  for now only measure user time */
    t += use.ru_stime.tv_sec - System_last_sec;
    t += ((double)((use.ru_stime.tv_usec - System_last_microsec)/10000))/100.0;
#endif
    return t;
}

void randomize()
{
    time_t tloc;

    srandom((unsigned) time(&tloc));
}
#endif
