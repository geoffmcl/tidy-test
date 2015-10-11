/*\
 *  Project: test-tidy
 *  Repo: https://github.com/geoffmcl/tidy-test
 *  Module: url2text
 *
 * Dependencies:
 * LibTidy => https://github.com/htacg/tidy-html5
 * LibCurl => http://curl.haxx.se/
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
 * Just example code to use library CURL to download a url, and 
 * use library Tidy to output only the text nodes
 *
\*/

#include <stdio.h>
#include <tidy.h>
#include <tidybuffio.h>
#include <curl/curl.h>
#include "sprtf.h"

#ifndef SPRTF
#define SPRTF printf
#endif 

static const char *module = "url2text";
static const char *def_log = "tempu2t.txt";
static const char *usr_input = 0;
static const char *usr_output = 0;
static FILE *usr_file = 0;

static bool show_tags = false;
static int verbosity = 0;

#define VERB1 (verbosity >= 1)
#define VERB2 (verbosity >= 2)
#define VERB5 (verbosity >= 5)
#define VERB9 (verbosity >= 9)

// options
static bool show_title = false;
static bool show_links = false;
static bool show_raw_text = true;
static bool skip_all_one = true;

/* curl write callback, to fill tidy's input buffer...  */
uint write_cb(char *in, uint size, uint nmemb, TidyBuffer *out)
{
    uint r;
    r = size * nmemb;
    tidyBufAppend( out, in, r );
    return r;
}

typedef struct tagNODEDUMP {
    TidyDoc doc;
    TidyNode tnod;
    TidyNodeType nt;
    TidyTagId nid;
} NODEDUMP, *PNODEDUMP;

char *filterBuffer(char *cp)
{
    size_t i, len;
    int c;
    while (*cp && (*cp <= ' '))
        cp++;
    if (*cp) {
        len = strlen(cp);
        for (i = len - 1; i > 0; i--) {
            if (cp[i] > ' ')
                break;
            cp[i] = 0;
        }
    }
    len = strlen(cp);
    if (len && skip_all_one) {
        c = *cp;
        for (i = 1; i < len; i++) {
            if (cp[i] != c)
                break;
        }
        if (i >= len) {
            cp = EndBuf(cp);
        }
    }
    return cp;
}


/* Traverse the document tree */
//void dumpNode(TidyDoc doc, TidyNode tnod, int indent )
void dumpNode( PNODEDUMP pnd, int indent )
{
    TidyDoc doc = pnd->doc;
    TidyNode tnod = pnd->tnod;
    TidyNode child;
    char *cp;
    size_t len;
    for ( child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
    {
        TidyNodeType nt = tidyNodeGetType( child );
        TidyTagId nid = tidyNodeGetId( child );
        ctmbstr name = tidyNodeGetName( child );
        if ( name )
        {
            /* if it has a name, then it's an HTML tag, and may have attributes ... */
            TidyAttr attr;
            ctmbstr  aval;
            if (show_links && (nid == TidyTag_A)) {
                cp = GetNxtBuf();
                strcpy(cp,"LINK: ");
                for ( attr = tidyAttrFirst(child); attr; attr=tidyAttrNext(attr) ) {
                    strcat(cp,tidyAttrName(attr));
                    aval = tidyAttrValue(attr);
                    if (aval && *aval) {
                        sprintf(EndBuf(cp),"=%s ",aval);
                    } else {
                        strcat(cp," ");
                    }
                }
                SPRTF("%s\n",cp);
            } else if (show_tags) {
                SPRTF( "%*.*s%s ", indent, indent, "<", name);
                /* walk the attribute list */
                for ( attr=tidyAttrFirst(child); attr; attr=tidyAttrNext(attr) ) {
                    SPRTF(tidyAttrName(attr));
                    tidyAttrValue(attr) ? SPRTF("=\"%s\" ",tidyAttrValue(attr)) : SPRTF(" ");
                }
                SPRTF( ">\n");
            }
        }
        else 
        {
            /* if it doesn't have a name, then it's probably text, cdata, etc... */
            TidyBuffer buf;
            tidyBufInit(&buf);
            if (show_raw_text) {
                tidyNodeGetText(doc, child, &buf);
            } else {
                tidyNodeGetText(doc, child, &buf);
            }
            cp = (char *)buf.bp;
            if (cp && buf.size) {
                // SPRTF("%*.*s\n", indent, indent, buf.bp?(char *)buf.bp:"");
                cp = filterBuffer(cp);
                len = strlen(cp);
                if (len) {
                    bool show = true;
                    switch (pnd->nid) {
                    case TidyTag_SCRIPT:
                    case TidyTag_STYLE:
                        show = false;
                        break;
                    case TidyTag_TITLE:
                        if ( !show_title ) {
                            show = false;
                        }
                        break;
                    }

                    if (show) {
                        if (nt != TidyNode_Comment) {
                            if (usr_file) {
                                int res = fprintf(usr_file,"%s\n",cp);
                                if (res < 0) {
                                    SPRTF("Error writing to '%s'!\n", usr_output);
                                    fclose(usr_file);
                                    usr_file = 0;
                                    SPRTF("%s\n", cp);
                                }
                            } else {
                                if (pnd->nid == TidyTag_TITLE) {
                                    SPRTF("TITLE: %s\n", cp);
                                } else {
                                    SPRTF("%s\n", cp);
                                }
                            }
                        }
                    }
                }
            }
            tidyBufFree(&buf);
        }
        /* setup the parent */
        pnd->tnod = child;
        pnd->nid  = nid;
        pnd->nt   = nt;
        dumpNode( pnd, indent + 4 ); /* recursive for every node */
    }
}


int load_url()
{
    const char *url = usr_input;
    int iret = 1;
    CURL *curl;
    char curl_errbuf[CURL_ERROR_SIZE];
    TidyDoc tdoc;
    TidyBuffer docbuf = {0};
    TidyBuffer tidy_errbuf = {0};
    int err;

    // code
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
    if (VERB2) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    } else {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);

    // setup libtidy
    tdoc = tidyCreate();
    tidyOptSetBool(tdoc, TidyForceOutput, yes); /* try harder */
    tidyOptSetInt(tdoc, TidyWrapLen, 4096);
    tidySetErrorBuffer( tdoc, &tidy_errbuf );
    tidyBufInit(&docbuf);

    // fetch the URL
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &docbuf);
    err = curl_easy_perform(curl);

    if ( !err ) {
        // got the URL data, give to libtidy
        err = tidyParseBuffer(tdoc, &docbuf); /* parse the input */
        if ( err >= 0 ) {
            err = tidyCleanAndRepair(tdoc); /* fix any problems */
            if ( err >= 0 ) {
                err = tidyRunDiagnostics(tdoc); /* load tidy error buffer */
                if ( err >= 0 ) {
                    NODEDUMP nd;
                    memset(&nd,0,sizeof(NODEDUMP));
                    nd.doc = tdoc;
                    nd.tnod = tidyGetRoot(tdoc);
                    dumpNode( &nd, 0 ); /* walk the tree */
                    if (VERB9) {
                        SPRTF("%s\n", tidy_errbuf.bp); /* show errors */
                    }
                }
            }
        }
    }
    else
    {
        SPRTF("Curl: %s\n", curl_errbuf);
    }
    /* clean-up */
    curl_easy_cleanup(curl);
    tidyBufFree(&docbuf);
    tidyBufFree(&tidy_errbuf);
    tidyRelease(tdoc);

    iret = err;

    return iret;
}

void give_help( char *name );

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
            case 'l':
                show_links = true;
                break;
            case 'a':
                skip_all_one = false;
                break;
            case 't':
                show_title = true;
                break;
            // TODO: Other arguments
            default:
                SPRTF("%s: Unknown argument '%s'. Try -? for help...\n", module, arg);
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
    if (!usr_input) {
        SPRTF("%s: No user input found in command!\n", module);
        return 1;
    }
    return 0;
}

// main OS entry
// MSVC DEBUG: PATH=F:\Projects\software\bin;%PATH%
int main(int argc, char **argv )
{
    int iret = 0;

    set_log_file((char *)def_log, 0);

    iret = parse_args( argc, argv );
    if (iret)
        return iret;

    iret = load_url();  // action of app

    close_log_file();
    return iret;
}

void give_help( char *name )
{
    SPRTF("%s: usage: [options] usr_input\n", module);
    SPRTF("Options:\n");
    SPRTF(" --help  (-h or -?) = This help and exit(2)\n");
    SPRTF(" --links       (-l) = Show links in output\n");
    SPRTF(" --title       (-t) = Show title in output\n");
    SPRTF(" --all         (-a) = Show all text. Default is to skip lines of just one character.\n");

    // TODO: More help
}

/* eof */
