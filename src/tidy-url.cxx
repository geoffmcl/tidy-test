/*\
 * tidy-url.cxx
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <stdio.h>
#include <string.h> // for strdup(), ...
#include <tidy.h>
#include <tidybuffio.h>
#include "sprtf.h"
#include "httpio.h"
// other includes
#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif
static const char *module = "tidy-url";
static const char *def_url = "http://geoffair.org/vos.htm";
static const char *def_log = "tempurl.txt";

static const char *usr_input = 0;
static HTTPInputSource httpinput;

void give_help( char *name )
{
    SPRTF("%s: usage: [options] usr_input\n", module);
    SPRTF("Options:\n");
    SPRTF(" --help  (-h or -?) = This help and exit(2)\n");
    SPRTF(" If given a URL, fetch and tidy the page given.\n");

    // TODO: More help
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
#ifndef NDEBUG
    if (!usr_input) {
        usr_input = strdup(def_url);
    }
#endif
    if (!usr_input) {
        SPRTF("%s: No user input found in command!\n", module);
        return 1;
    }
    return 0;
}

int get_Data( HTTPInputSource *in, TidyBuffer *pb )
{
    int count = 0;
    byte c;
    for (; in->nextBytePos < MMX_BUFFER 
                   && ((c = in->recvbuffer[ in->nextBytePos ]) != 0); 
                 in->nextBytePos++ )
    {
        tidyBufPutByte( pb, c );
        count++;
    }
    return count;
}

int parse_url( TidyBuffer *pb )
{
    int rc;
    if (!pb->bp || (pb->size == 0)) {
        SPRTF("%s: Nothing to tidy...\n", module );
        return 1;
    }
    TidyDoc doc = tidyCreate();
    if (!doc) {
        SPRTF("%s: Failed to create tidy doc!\n", module );
        return 1;
    }
    TidyBuffer m_errbuf, m_output;
    tidyBufInit( &m_errbuf );
    tidyBufInit( &m_output );

    rc = tidySetErrorBuffer( doc, &m_errbuf );
    if (rc) {
        SPRTF("%s: Failed to set error buffer!\n", module );
        goto exit;
    }
    if (pb->bp && pb->size) {
        SPRTF( "This is the input:\n%s", pb->bp );
    }
    rc = tidyParseBuffer(doc, pb);
    if (rc) {
        SPRTF("%s: tidyParseBuffer() returned %d!\n", module, rc );
    }
    if (m_errbuf.bp && m_errbuf.size) {
       SPRTF( "%s: tidyParseBuffer:\n%s", module, m_errbuf.bp );
       tidyBufFree( &m_errbuf );
    }
    if (rc >= 0) {
        rc = tidyCleanAndRepair( doc );               // Tidy it up!
        if (m_errbuf.bp && m_errbuf.size) {
           SPRTF( "%s: tidyCleanAndRepair:\n%s", module, m_errbuf.bp );
           tidyBufFree( &m_errbuf );
        }
    }
    if ( rc >= 0 ) {
        rc = tidyRunDiagnostics( doc );               // Kvetch
        if (m_errbuf.bp && m_errbuf.size) {
           SPRTF( "%s: tidyRunDiagnostics:\n%s", module, m_errbuf.bp );
           tidyBufFree( &m_errbuf );
        }
    }
    if ( rc > 1 )                                     // If error, force output.
        rc = ( tidyOptSetBool(doc, TidyForceOutput, yes) ? rc : -1 );
    if ( rc >= 0 ) {
        rc = tidySaveBuffer( doc, &m_output );          // Pretty Print
        if (m_errbuf.bp && m_errbuf.size) {
           SPRTF( "%s: tidyRunDiagnostics:\n%s", module, m_errbuf.bp );
           tidyBufFree( &m_errbuf );
        }
    }
    if ( rc >= 0 ) {
        if ( rc > 0 ) {
            SPRTF("%s: tidySaveBuffer() returned %d!\n", module, rc );
        }
        if (m_errbuf.bp && m_errbuf.size) {
           SPRTF( "%s: tidySaveBuffer:\n%s", module, m_errbuf.bp );
           tidyBufFree( &m_errbuf );
        }
        SPRTF( "%s: And here is the result:\n%s", module, m_output.bp );
    } else {
        SPRTF( "%s: A severe error (%d) occurred.\n", module, rc );
    }
    if (m_errbuf.bp && m_errbuf.size) {
        SPRTF( "%s: Diagnostics:\n%s", module, m_errbuf.bp );
        tidyBufFree( &m_errbuf );
   }

exit:
    tidyBufFree( &m_errbuf );
    tidyBufFree( &m_output );
    tidyRelease(doc);
    return rc;
}

void showHTTPInput( PHTTPInputSource ps )
{
    if (ps->pHostName) {
        SPRTF("HostName: '%s', port %d\n", ps->pHostName, ps->nPort );
        if (ps->pResource)
            SPRTF("Resource: '%s'\n", ps->pResource );
    }
}


int tidy_url()
{
    PHTTPInputSource ps = &httpinput;
    memset(ps,0,sizeof(HTTPInputSource));
    int res = parseURL( ps, (tmbstr) usr_input );
    if (res) {
        SPRTF("Parse of URL '%s' FAILED\n", usr_input);
        return 1;
    }
    showHTTPInput( ps );

    res = openURL( ps, (tmbstr) usr_input );
    if (res == 0) {
        TidyBuffer buff;
        tidyBufInit(&buff);
        SPRTF("%s: Received %d bytes...\n", module, res);
        res = get_Data( ps, &buff );
        if (res > 0) {
            SPRTF("%s: Got %d bytes to put to Tidy...\n", module, res);
            res = parse_url( &buff );
            res = 0;
        } else {
            SPRTF("%s: Failed to get data... %d\n", module, res);
            res = -1;
        }
    } else {
        SPRTF("%s: openURL() failed %d\n", module, res );
    }
    closeURL( ps );
    return res;
}

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    set_log_file((char *)def_log, 0);
    iret = parse_args(argc,argv);
    if (iret)
        return iret;

    iret = tidy_url();  // actions of app

    return iret;
}


// eof = tidy-url.cxx
