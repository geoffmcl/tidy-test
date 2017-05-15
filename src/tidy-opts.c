/*\
 * tidy-opts.c
 *
 * Copyright (c) 2015-2017 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <stdio.h>
#include <string.h> // for strdup(), ...
#include <tidy.h>
#include <tidybuffio.h>
#include "sprtf.h"


/*\
 *
 * This module is to specifically test the tidyOptSaveSink() function
 * Is has no other purpose
 *
\*/

#ifndef MEOL
#ifdef WIN32
#define MEOL "\r\n"
#else
#define MEOL "\n"
#endif
#endif
#ifdef SPRTF
#define SPTRF SPRTF
#endif


static const char *module = "tidy-opts";

static const char *usr_input = 0;

void give_help( char *name )
{
    SPRTF("%s: usage: [options] [usr_input]\n", module);
    SPRTF("Options:\n");
    SPRTF(" --help  (-h or -?) = This help and exit(2)\n");
    SPRTF(" Test 1: Fetch and display the tidy library options in\n");
    SPRTF(" a sink, and after setting one.\n");
    SPRTF(" Test 2: Set an own 'allocator' and use to create and parse a\n");
    SPRTF(" document\n");
}

int parse_args( int argc, char **argv )
{
    int i,i2,c;
    char *arg, *sarg;
    for (i = 1; i < argc; i++) {
        arg = argv[i];
        i2 = i + 1;
        if (*arg == '-') {
            sarg = &arg[1];
            while (*sarg == '-')
                sarg++;
            c = *sarg;
            switch (c) {
            case 'h':
            case '?':
                give_help(argv[0]);
                return 2;
                break;
            // TODO: Other arguments
            default:
                SPRTF("%s: Unknown argument '%s'. Tyr -? for help...\n", module, arg);
                return 1;
            }
        } else {
            // bear argument
            if (usr_input) {
                SPRTF("%s: Already have input '%s'! What is this '%s'?\n", module, usr_input, arg );
                return 1;
            }
            usr_input = strdup(arg);
        }
    }
    //if (!usr_input) {
    //    SPRTF("%s: No user input found in command!\n", module);
    //    return 1;
    //}
    return 0;
}

static TidyDoc tdoc = 0;    // tidyCreate();
static TidyBuffer output;
static TidyBuffer errbuf;
static TidyBuffer cfgbuf;

Bool initBuffers()
{
    Bool done = no;
    tidyBufInit( &output );
    tidyBufInit( &errbuf );
    tidyBufInit( &cfgbuf );
    if (tidySetErrorBuffer( tdoc, &errbuf ) >= 0) {    // Capture diagnostics
        done = yes;
    }
    return done;
}

void clearBuffers( int flag )
{
    if ((flag == 0) || (flag & 1))
        tidyBufClear( &errbuf );
    if ((flag == 0) || (flag & 2))
        tidyBufClear( &output );
    if ((flag == 0) || (flag & 4))
        tidyBufClear( &cfgbuf );
}

Bool openTidyLib()
{
    Bool done = no;
    if (tdoc == 0) {
        tdoc = tidyCreate();
        done = initBuffers();
    }
    return done;
}

void closeTidyLib()
{
    if (tdoc)  {

        clearBuffers(1|2|4); // release buffer memory - maybe tidyBufFree() also does this???

        // free buffer memory
        tidyBufFree( &output );
        tidyBufFree( &errbuf );
        tidyBufFree( &cfgbuf );

        tidyRelease( tdoc ); /* called to free hash tables etc. */
    }
    tdoc = 0;
}

typedef struct tagSINKDATA2 {
    int context;
    TidyBuffer *tbp;
}SINKDATA2, *PSINKDATA2;

static void TIDY_CALL putByteFunc2(void* sinkData, byte bt )
{
    // do something with the byte
    if (sinkData && bt) {
        PSINKDATA2 psd2 = (PSINKDATA2)sinkData;
        tidyBufPutByte( psd2->tbp, bt );
    }
}

///////////////////////////////////////////////////////
// Config stuff
///////////////////////////////////////////////////////

TidyOptionId getTidyOptionId( const char *item )
{
    return tidyOptGetIdForName(item);
}

// implementation
Bool getConfigBool( const char *item )
{
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS)
        return tidyOptGetBool(tdoc, id);
    return no;
}

Bool setConfigBool( const char *item, Bool val )
{
    Bool done = no;
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS) {
        Bool curr = tidyOptGetBool(tdoc, id);
        if (curr != val) {
            done = tidyOptSetBool(tdoc, id, val);
        }
    }
    return done;
}


// TODO: CHECK AutoBool to triSate mappings
// Tidy AutoBool has no(0), yes(1), auto(2)
// Qt4  triState has no(0), yes(2), part(1)
TidyTriState getConfigABool( const char *item )
{
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS)
        return (TidyTriState)tidyOptGetInt(tdoc,id);
        // return cfgAutoBool(tdoc, id);
    return TidyAutoState;
}

Bool setConfigABool( const char *item, Bool val )
{
    Bool done = no;
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS) {
        Bool curr = (Bool)tidyOptGetInt(tdoc,id);
        if (curr != val) {
            done = tidyOptSetInt(tdoc,id,val);
        }
    }
    return done;
}


int getConfigInt( const char *item )
{
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS)
         return tidyOptGetInt(tdoc,id);

    if (strcmp(item,"indent-spaces") == 0) {
        return 2;
    } else if (strcmp(item,"wrap") == 0) {
        return 68;
    } else if (strcmp(item,"tab-size") == 0) {
        return 8;
    } else if (strcmp(item,"show-errors") == 0) {
        return 6;
    } 
    return 0;
}

Bool setConfigInt( const char *item, int val )
{
    Bool done = no;
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS) {
        int curr = tidyOptGetInt(tdoc,id);
        if (curr != val) {
            done = tidyOptSetInt(tdoc,id,val);
        }
    }
    return done;
}

const char *getConfigStg( const char *item )
{
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS)
         return tidyOptGetValue(tdoc,id);
    return "";
}

Bool setConfigStg( const char *item, const char *stg )
{
    Bool done = no;
    TidyOptionId id = getTidyOptionId(item);
    if (id < N_TIDY_OPTIONS) {
        const char *curr = tidyOptGetValue(tdoc,id);
        if (curr) {
            // have a value, but only set if different
            if (strcmp(curr,stg)) {
                done = tidyOptSetValue(tdoc,id,stg);
            }
        } else {
            // presently NO VALUE, set ONLY if there is LENGTH now
            //if (strlen(stg)) {
                done = tidyOptSetValue(tdoc,id,stg);
            //}
        }
    }
    return done;
}


static SINKDATA2 sd2;
// TIDY_EXPORT int TIDY_CALL         tidyOptSaveSink( TidyDoc tdoc, TidyOutputSink* sink );
static TidyOutputSink sink;

void show_libVersion()
{
#ifdef USE_OLD_API  // did not have tidyLibraryVersion();
#ifdef USE_TIDY5_API
    ctmbstr s = tidyLibraryVersion();
#else    
    ctmbstr s = tidyReleaseDate();
#endif    
    SPRTF("Using HTML Tidy library version %s\n", s);
#else
    ctmbstr v = tidyLibraryVersion();
    ctmbstr d = tidyReleaseDate();
#ifdef PLATFORM_NAME
    SPRTF("Using HTML Tidy library for %s, date %s, version %s\n", PLATFORM_NAME, d, v);
#else
    SPRTF("Using HTML Tidy library date %s, version %s\n", d, v);
#endif
#endif

}

int show_sink()
{
    int iret = 0;
    sd2.context = 2;
    sd2.tbp = &cfgbuf;
    if (tidyInitSink( &sink, &sd2, &putByteFunc2 )) {
        iret = tidyOptSaveSink(tdoc, &sink);
        if ( !cfgbuf.bp || (strlen((const char *)cfgbuf.bp) == 0) ) {
            const char *err1 = "Oops! All configuration items are equal default!" MEOL;
            tidyBufAppend(&cfgbuf,(void *)err1,(uint)strlen(err1));
        }
    } else {
        const char *err2 = "Oops! internal error: tidyInitSink() FAILED" MEOL;
        tidyBufAppend(&cfgbuf,(void *)err2,(uint)strlen(err2));
    }
    SPRTF("%s", cfgbuf.bp);
    return iret;
}

////////////////////////////////////////////////////////////////////////
//// Memory Stats
static size_t total_mem = 0;
static size_t max_mem = 0;
static size_t max_max_mem = 0;
static size_t largest_mem = 0;
typedef struct tagMMSTATS {
    void *link;
    void *mem;
    size_t size;
    Bool freed;
    Bool resized;
    size_t nsize;
    void *nmem;
}MMSTATS, *PMMSTATS;

static MMSTATS mmstats = { 0 };

void add_to_stats( void *vp, size_t size, Bool realloc, void *nvp )
{
    PMMSTATS pmm = &mmstats;
    if (realloc) {
        while (pmm->link) {
            if ((pmm->mem == vp)||(pmm->nmem == nvp)) {
                pmm->resized = yes;
                pmm->nmem = nvp;
                pmm->nsize = size;
                if (size > largest_mem)
                    largest_mem = size;
                total_mem += size - pmm->size;
                max_mem += size - pmm->size;
                if (max_mem > max_max_mem)
                    max_max_mem = max_mem;
                return;
            }
            pmm = (PMMSTATS)pmm->link;
        }
        if ((pmm->mem == vp)||(pmm->nmem == nvp)) {
            pmm->resized = yes;
            pmm->nmem = nvp;
            if (size > largest_mem)
                largest_mem = size;
            pmm->nsize = size;
            total_mem += size - pmm->size;
            max_mem += size - pmm->size;
            if (max_mem > max_max_mem)
                max_max_mem = max_mem;
            return;
        }
        SPRTF("%s: REAllocation %p or %p NOT found!\n", module, vp, nvp );
    } else {
        PMMSTATS nmm = (PMMSTATS)malloc(sizeof(MMSTATS));
        if (!nmm) {
            SPRTF("%s: UGH! Memory FAILED on %d bytes!\n", module, (int)(sizeof(MMSTATS)) );
            exit(2);
        }
        memset(nmm,0,sizeof(MMSTATS));
        nmm->mem = vp;
        nmm->size = size;
        if (size > largest_mem)
            largest_mem = size;
        total_mem += size;
        max_mem += size;
        if (max_mem > max_max_mem)
            max_max_mem = max_mem;
        // get to last link
        while (pmm->link) {
            pmm = (PMMSTATS) pmm->link;
        }
        pmm->link = nmm;
    }
}

size_t free_block( void *vp )
{
    PMMSTATS pmm = &mmstats;
    while (pmm->link) {
        if ((pmm->mem == vp) || ((pmm->nmem == vp)&&(pmm->resized))) {
            if (!pmm->freed) {
                pmm->freed = yes;
                if (pmm->resized) {
                    if (max_mem > pmm->nsize)
                        max_mem -= pmm->nsize;
                    else
                        max_mem = 0;
                    return pmm->nsize;
                } else {
                    if (max_mem > pmm->size)
                        max_mem -= pmm->size;
                    else
                        max_mem = 0;
                    return pmm->size;
                }
            } else {
                SPRTF("%s: block %p already marked free. Maybe reuse...\n", module, vp);
            }
        } 
        pmm = (PMMSTATS) pmm->link;
    }
    if ((pmm->mem == vp) || ((pmm->nmem == vp)&&(pmm->resized))) {
        if (!pmm->freed) {
            pmm->freed = yes;
            if (pmm->resized) {
                if (max_mem > pmm->nsize)
                    max_mem -= pmm->nsize;
                else
                    max_mem = 0;
                return pmm->nsize;
            } else {
                if (max_mem > pmm->size)
                    max_mem -= pmm->size;
                else
                    max_mem = 0;
                return pmm->size;
            }
        } else {
            SPRTF("%s: block %p already marked free. Maybe reuse...\n", module, vp);
        }
    } 
    SPRTF("%s: Block freed not in stats %p!\n", module, vp );
    return 0;
}

void free_stats() 
{
    PMMSTATS pmm = &mmstats;
    int cnt = 0;
    while (pmm->link) {
        void *vp = pmm->link;
        if (cnt) {
            if (!pmm->freed)
                SPRTF("%s: allocation %p (%p) NOT freed\n", module, pmm->mem, pmm->nmem);
            free(pmm);
        }
        pmm = (PMMSTATS)vp;
        cnt++;
    }
    SPRTF("%s: %d allocations, total mem %d, max mem %d, largest %d\n", module, cnt, (int)total_mem, (int)max_max_mem,
        (int)largest_mem);
}

/////////////////////////////////////////////////////////////////////////
//// Experiment with supplying own allocator

typedef struct _MyAllocator {
    TidyAllocator base;
    //   ...other custom allocator state...
} MyAllocator;
 

//void * MyAllocator_alloc(TidyAllocator *base, void *block, size_t nBytes)
void * TIDY_CALL MyAllocator_alloc(TidyAllocator *base, size_t nBytes)
{
    MyAllocator *self = (MyAllocator*)base;
    //   ...
    void *vp = malloc(nBytes);
    if (!vp) {
        self->base.vtbl->panic(base,"Memory FAILED!");
    }
    SPRTF("    Allocate: %p, size %d\n", vp, (int)nBytes);
    add_to_stats( vp, nBytes, no, 0 );
    return vp;
}
void * TIDY_CALL MyAllocator_realloc(TidyAllocator *base, void *block, size_t nBytes)
{
    MyAllocator *self = (MyAllocator*)base;
    //   ...
    void *vp = realloc(block,nBytes);
    if (!vp) {
        self->base.vtbl->panic(base,"Memory FAILED!");
    }
    if (block) {
        SPRTF("    Reallocate: %p, from %p, size %d\n", vp, block, (int)nBytes);
        add_to_stats( block, nBytes, yes, vp );
    } else {
        SPRTF("[Re]allocate: %p, from NULL, size %d\n", vp, (int)nBytes);
        add_to_stats( vp, nBytes, no, 0 );
    }
    return vp;
}
void TIDY_CALL MyAllocator_free(TidyAllocator *base, void *block)
{
    MyAllocator *self = (MyAllocator*)base;
    if (block) {
        size_t size = free_block( block );
        SPRTF("        Free: %p, size %d\n", block, (int)size );
    } else {
        SPRTF("        Free: NULL\n" );
    }
    free(block);
}
void TIDY_CALL MyAllocator_panic(TidyAllocator *base, ctmbstr msg)
{
    SPRTF("Critical Error: %s\n", msg);
    exit(2);
}

static const TidyAllocatorVtbl MyAllocatorVtbl = {
     MyAllocator_alloc,
     MyAllocator_realloc,
     MyAllocator_free,
     MyAllocator_panic
};

static MyAllocator allocator;
static TidyDoc doc;
static TidyBuffer m_errbuf;
static TidyBuffer m_outbuf;
static const char *opt1 = "show-body-only";
static const char *html1 = "<p>hello<em> world</em></p>";

void test_with_allocator()
{
    TidyDoc doc = 0;
    TidyOptionId id;
    Bool done;
    int res, c;
    size_t ii,len;

    SPRTF("%s: Test 2: Set own allocator\n", module );
    memset(&allocator,0,sizeof(MyAllocator));
    allocator.base.vtbl = &MyAllocatorVtbl;
    //...initialise allocator specific state...
    SPRTF("%s: Do intital 'document' creation...\n", module);
    doc = tidyCreateWithAllocator(&allocator.base);
    if (!doc) {
        SPRTF("Failed tidyCreateWithAllocator!\n");
        goto exit;
    }
    SPRTF("%s: Allocate two buffers, err and out buffer...\n", module);
    tidyBufInitWithAllocator( &m_errbuf, &allocator.base);  // tidyBufInit( &m_errbuf );
    tidyBufInitWithAllocator( &m_outbuf, &allocator.base);  // tidyBufInit( &m_errbuf );
    SPRTF("%s: Set error buffer...\n", module );
    res = tidySetErrorBuffer( doc, &m_errbuf );
    if ( !(res == 0) ) {    // Capture diagnostics
        SPRTF("Failed to set error buffer! %d\n", res);
        goto exit;
    }
    SPRTF("%s: Set option '%s: yes'\n", module, opt1);
    id = tidyOptGetIdForName(opt1);
    if (id < N_TIDY_OPTIONS) {
        done = tidyOptSetInt(doc, id, yes);
        if (!done) {
            SPRTF("Failed to set option bool for %s!\n", opt1);
            goto exit;
        }
    } else {
        SPRTF("Failed to set option bool for %s!\n", opt1);
        goto exit;
    }

    res = tidyParseString( doc, html1 );
    SPRTF("%s: Results from tidyParseString(...) = %d\n", module, res );
    if (m_errbuf.bp && strlen((const char *)m_errbuf.bp)) {
        len = strlen((const char *)m_errbuf.bp);
        while (len) {
            len--;
            c = m_errbuf.bp[len];
            if (c > ' ')
                break;
            m_errbuf.bp[len] = 0;
        }
        SPRTF("errbuf: '%s'\n", m_errbuf.bp);
        tidyBufClear( &m_errbuf );
    }

    res = tidyCleanAndRepair( doc );
    SPRTF("%s: Results from tidyCleanAndRepair(...) = %d\n", module, res );
    if (m_errbuf.bp && strlen((const char *)m_errbuf.bp)) {
        SPRTF("errbuf: '%s'\n", m_errbuf.bp);
        len = strlen((const char *)m_errbuf.bp);
        while (len) {
            len--;
            c = m_errbuf.bp[len];
            if (c > ' ')
                break;
            m_errbuf.bp[len] = 0;
        }
        SPRTF("errbuf: '%s'\n", m_errbuf.bp);
        tidyBufClear( &m_errbuf );
    }
    res = tidyRunDiagnostics( doc );
    SPRTF("%s: Results from tidyRunDiagnostics(...) = %d\n", module, res );
    if (m_errbuf.bp && strlen((const char *)m_errbuf.bp)) {
        len = strlen((const char *)m_errbuf.bp);
        while (len) {
            len--;
            c = m_errbuf.bp[len];
            if (c > ' ')
                break;
            m_errbuf.bp[len] = 0;
        }
        SPRTF("errbuf: '%s'\n", m_errbuf.bp);
        tidyBufClear( &m_errbuf );
    }
    
#ifdef USE_TIDY5_API
    res = tidyReportDoctype( doc );
    SPRTF("%s: Results from tidyReportDoctype(...) = %d\n", module, res );
    if (m_errbuf.bp && strlen((const char *)m_errbuf.bp)) {
        len = strlen((const char *)m_errbuf.bp);
        while (len) {
            len--;
            c = m_errbuf.bp[len];
            if (c > ' ')
                break;
            m_errbuf.bp[len] = 0;
        }
        SPRTF("errbuf: '%s'\n", m_errbuf.bp);
        tidyBufClear( &m_errbuf );
    }
#endif // #ifdef USE_TIDY5_API
    
    res = tidySaveBuffer( doc, &m_outbuf );
    SPRTF("%s: Results from tidySaveBuffer(...) = %d\n", module, res );
    if (m_errbuf.bp && strlen((const char *)m_errbuf.bp)) {
        len = strlen((const char *)m_errbuf.bp);
        while (len) {
            len--;
            c = m_errbuf.bp[len];
            if (c > ' ')
                break;
            m_errbuf.bp[len] = 0;
        }
        SPRTF("errbuf: '%s'\n", m_errbuf.bp);
        tidyBufClear( &m_errbuf );
    }

    SPRTF("Input:  '%s'\n", html1 );
    if (m_outbuf.bp) {
        len = strlen((const char *)m_outbuf.bp);
        for (ii = 0; ii < len; ii++) {
            c = m_outbuf.bp[ii];
            if (c < ' ')
                m_outbuf.bp[ii] = ' ';
        }
        while (len) {
            len--;
            c = m_outbuf.bp[len];
            if (c > ' ')
                break;
            m_outbuf.bp[len] = 0;
        }
        SPRTF("Output: '%s'\n", m_outbuf.bp );
    } else {
        SPRTF("Output: '%s'\n", "HUH! none???" );
    }
exit:    
    tidyBufFree( &m_errbuf );
    tidyBufFree( &m_outbuf );
    tidyRelease(doc);
    free_stats();
    SPRTF("%s: Test 2: Done.\n", module );

}

static const char *def_log = "tempopts.txt";

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    const char *cp;
    set_log_file((char *)def_log, 0);
    iret = parse_args(argc,argv);
    if (iret)
        return iret;

    show_libVersion();

    SPRTF("%s: Test 1: Default options and after setting a blank string\n", module);
    if (!openTidyLib()) {
        SPRTF("Some problem initializing libtidy!\n");
        iret = 1;
        goto exit;
    }

    iret = show_sink();

    // Now to make some configuration changes, and try again...
    clearBuffers(1|2|4); // release buffer memory - maybe tidyBufFree() also does this???
    cp = getConfigStg( "alt-text" );
    if (cp) {
        if (setConfigStg( "alt-text", cp )) {
            iret = show_sink();
        }
    } else {
        if (setConfigStg( "alt-text", "" )) {
            iret = show_sink();
        }
    }
    SPRTF("%s: Test 1: Done\n", module);
    test_with_allocator();
exit:
    closeTidyLib();
    SPRTF("%s: All output written to '%s'\n", module, def_log);
    return iret;
}


// eof = tidy-opts.c
