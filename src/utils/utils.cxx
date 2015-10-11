/*\
 * utils.cxx
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <stdio.h>
// other includes
#include "utils.hxx"

static const char *module = "utils";

#define USE_PERF_COUNTER
#if (defined(WIN32) && defined(USE_PERF_COUNTER))
#include <Windows.h>
// QueryPerformanceFrequency( &frequency ) ;
// QueryPerformanceCounter(&timer->start) ;
double get_seconds()
{
    static double dfreq;
    static bool done_freq = false;
    static bool got_perf_cnt = false;
    if (!done_freq) {
        LARGE_INTEGER frequency;
        if (QueryPerformanceFrequency( &frequency )) {
            got_perf_cnt = true;
            dfreq = (double)frequency.QuadPart;
        }
        done_freq = true;
    }
    double d;
    if (got_perf_cnt) {
        LARGE_INTEGER counter;
        QueryPerformanceCounter (&counter);
        d = (double)counter.QuadPart / dfreq;
    }  else {
        DWORD dwd = GetTickCount(); // milliseconds that have elapsed since the system was started
        d = (double)dwd / 1000.0;
    }
    return d;
}

#else // !WIN32
double get_seconds()
{
    struct timeval tv;
    gettimeofday(&tv,0);
    double t1 = (double)(tv.tv_sec+((double)tv.tv_usec/1000000.0));
    return t1;
}
#endif // WIN32 y/n

// see : http://geoffair.org/misc/powers.htm
char *get_seconds_stg( double dsecs )
{
    static char _s_secs_buf[256];
    char *cp = _s_secs_buf;
    sprintf(cp,"%g",dsecs);
    if (dsecs < 0.0) {
        strcpy(cp,"?? secs");
    } else if (dsecs < 0.0000000000001) {
        strcpy(cp,"~0 secs");
    } else if (dsecs < 0.000000005) {
        // nano- n 10 -9 * 
        dsecs *= 1000000000.0;
        sprintf(cp,"%.3f nano-secs",dsecs);
    } else if (dsecs < 0.000005) {
        // micro- m 10 -6 * 
        dsecs *= 1000000.0;
        sprintf(cp,"%.3f micro-secs",dsecs);
    } else if (dsecs <  0.005) {
        // milli- m 10 -3 * 
        dsecs *= 1000.0;
        sprintf(cp,"%.3f milli-secs",dsecs);
    } else if (dsecs < 60.0) {
        sprintf(cp,"%.3f secs",dsecs);
    } else {
        int mins = (int)(dsecs / 60.0);
        dsecs -= (double)mins * 60.0;
        if (mins < 60) {
            sprintf(cp,"%d:%.3f min:secs", mins, dsecs);
        } else {
            int hrs = mins / 60;
            mins -= hrs * 60;
            sprintf(cp,"%d:%02d:%.3f hrs:min:secs", hrs, mins, dsecs);
        }
    }
    return cp;
}



// eof = utils.cxx
