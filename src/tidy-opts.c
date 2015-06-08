/*\
 * tidy-opts.c
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <stdio.h>
#include <string.h> // for strdup(), ...
#include <tidy.h>
#include <buffio.h>

/*\
 *
 * This module is to specifically test the tidyOptSaveSink() function
 * Is has no toher purpose
 *
\*/

#ifndef MEOL
#ifdef WIN32
#define MEOL "\r\n"
#else
#define MEOL "\n"
#endif
#endif

static const char *module = "tidy-opts";

static const char *usr_input = 0;

void give_help( char *name )
{
    printf("%s: usage: [options] [usr_input]\n", module);
    printf("Options:\n");
    printf(" --help  (-h or -?) = This help and exit(2)\n");
    printf(" It is only to fetch and display the tidy library options in\n");
    printf(" in a sink.\n");
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
                printf("%s: Unknown argument '%s'. Tyr -? for help...\n", module, arg);
                return 1;
            }
        } else {
            // bear argument
            if (usr_input) {
                printf("%s: Already have input '%s'! What is this '%s'?\n", module, usr_input, arg );
                return 1;
            }
            usr_input = strdup(arg);
        }
    }
    //if (!usr_input) {
    //    printf("%s: No user input found in command!\n", module);
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
    ctmbstr s = tidyLibraryVersion();
    printf("Using HTML Tidy library version %s\n", s);
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
            tidyBufAppend(&cfgbuf,(void *)err1,strlen(err1));
        }
    } else {
        const char *err2 = "Oops! internal error: tidyInitSink() FAILED" MEOL;
        tidyBufAppend(&cfgbuf,(void *)err2,strlen(err2));
    }
    printf("%s", cfgbuf.bp);
    return iret;
}

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    const char *cp;
    iret = parse_args(argc,argv);
    if (iret)
        return iret;

    show_libVersion();

    if (!openTidyLib()) {
        printf("Some problem initializing libtidy!\n");
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

exit:
    closeTidyLib();
    return iret;
}


// eof = tidy-opts.c
