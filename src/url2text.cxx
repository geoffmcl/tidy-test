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
 * use library Tidy to output the text nodes, and other things...
 *
\*/

#include <stdio.h>
#include <tidy.h>
#include <tidybuffio.h>
#include <curl/curl.h>
#include "sprtf.h"
#include "utils.hxx"

#ifndef SPRTF
#define SPRTF printf
#endif 
#ifndef ISDIGIT
#define ISDIGIT(a) ( ( a >= '0' ) && ( a <= '9' ) )
#endif
#ifndef CHKMEM
#define CHKMEM(a) if (!a) { SPRTF("%s: memory allocation failed!\n"); exit(1); }
#endif
static const char *module = "url2text";

static const char *def_log = "tempu2t.txt";
static const char *usr_input = 0;
static const char *usr_output = 0;
static FILE *usr_file = 0;

static int show_tags = 0;
static int verbosity = 0;

#define VERB1 (verbosity >= 1)
#define VERB2 (verbosity >= 2)
#define VERB5 (verbosity >= 5)
#define VERB9 (verbosity >= 9)

// options
static int show_errors = 0;
static int show_title = 0;
static int show_links = 0;
static int show_raw_text = 1;
static int skip_all_one = 1;
static int show_script = 0;
static int skip_not_letters = 1;
static int add_indenting = 0;
#define MX_IND 256
static int tab_size = 4;

////////////////////////////////
static double bgn_secs = 0;

/* used to classify characters for lexical purposes */
#define MAP(c) ((unsigned)c < 128 ? lexmap[(unsigned)c] : 0)
static uint lexmap[128];
static int done_map = 0;

/* lexer character types
*/
#define digit       1u
#define letter      2u
#define namechar    4u
#define white       8u
#define newline     16u
#define lowercase   32u
#define uppercase   64u
#define digithex    128u

static void MapStr( ctmbstr str, uint code )
{
    while ( *str )
    {
        uint i = (byte) *str++;
        lexmap[i] |= code;
    }
}

static void InitMap()
{
    MapStr("\r\n\f", newline|white);
    MapStr(" \t", white);
    MapStr("-.:_", namechar);
    MapStr("0123456789", digit|digithex|namechar);
    MapStr("abcdefghijklmnopqrstuvwxyz", lowercase|letter|namechar);
    MapStr("ABCDEFGHIJKLMNOPQRSTUVWXYZ", uppercase|letter|namechar);
    MapStr("abcdefABCDEF", digithex);
    done_map = 1;
}


static Bool IsLetter(uint c)
{
    uint map;
    if (!done_map)
        InitMap();

    map = MAP(c);

    return ((map & letter) != 0) ? yes : no;
}


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

///////////////////////////////////////////////////////////////
// gathering text nodes can give some strange strings
// this is a filter of what will be displayed
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
    len = strlen(cp);
    if (len && skip_not_letters) {
        for (i = 1; i < len; i++) {
            c = cp[i];
            if (IsLetter(c))
                break;
        }
        if (i >= len) {
            cp = EndBuf(cp);
        }
    }
    return cp;
}
static char _s_ind[MX_IND+4];
char *getIndent(int indent)
{
    char *cp = _s_ind;
    int i, len = indent * tab_size;
    if (len > MX_IND)
        len = MX_IND;
    for (i = 0; i < len; i++) {
        cp[i] = ' ';
    }
    cp[i] = 0;
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
    int res;
    TidyNode parent = tidyGetParent(tnod);
    const char *ind = "";
    if (add_indenting)
        ind = getIndent(indent);
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
                sprintf(cp,"%sLINK: ", ind);
                for ( attr = tidyAttrFirst(child); attr; attr=tidyAttrNext(attr) ) {
                    strcat(cp,tidyAttrName(attr));
                    aval = tidyAttrValue(attr);
                    if (aval && *aval) {
                        sprintf(EndBuf(cp),"=%s ",aval);
                    } else {
                        strcat(cp," ");
                    }
                }
                if (usr_file) {
                    res = fprintf(usr_file,"%s\n",cp);
                    if (res < 0) {
                        SPRTF("Error writing to '%s'!\n", usr_output);
                        fclose(usr_file);
                        usr_file = 0;
                        SPRTF("%s\n", cp);
                    }
                } else {
                    SPRTF("%s\n",cp);
                }
            } else if (show_tags) {
                cp = GetNxtBuf();
                sprintf(cp, "%s<%s", ind, name);
                /* walk the attribute list */
                attr = tidyAttrFirst(child);
                for ( ; attr; attr=tidyAttrNext(attr) ) {
                    sprintf(EndBuf(cp)," %s", tidyAttrName(attr));
                    if (tidyAttrValue(attr))
                        sprintf(EndBuf(cp),"=\"%s\"",tidyAttrValue(attr));
                }
                strcat(cp,">");
                if (usr_file) {
                    res = fprintf(usr_file,"%s\n",cp);
                    if (res < 0) {
                        SPRTF("Error writing to '%s'!\n", usr_output);
                        fclose(usr_file);
                        usr_file = 0;
                        SPRTF("%s\n", cp);
                    }
                } else {
                    SPRTF("%s\n",cp);
                }
            }
        }
        else 
        {
            /* if it doesn't have a name, then it's probably text, cdata, etc... */
            TidyBuffer buf;
            tidyBufInit(&buf);
            if (show_raw_text) {
                tidyNodeGetValue(doc, child, &buf);
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
                                if (pnd->nid == TidyTag_TITLE) {
                                    res = fprintf(usr_file,"%sTITLE: %s\n",ind,cp);
                                } else {
                                    res = fprintf(usr_file,"%s%s\n",ind,cp);
                                }
                                if (res < 0) {
                                    SPRTF("Error writing to '%s'!\n", usr_output);
                                    fclose(usr_file);
                                    usr_file = 0;
                                    SPRTF("%s%s\n", ind,cp);
                                }
                            } else {
                                if (pnd->nid == TidyTag_TITLE) {
                                    SPRTF("%sTITLE: %s\n", ind, cp);
                                } else {
                                    SPRTF("%s%s\n", ind, cp);
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
        dumpNode( pnd, indent + 1 ); /* recursive for every node */
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
    curl_errbuf[0] = 0;
    curl = curl_easy_init();
    if (!curl) {
        SPRTF("%s: curl_easy_init() failed!\n", module );
        return 1;
    }
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
        char *elap = get_seconds_stg( get_seconds() - bgn_secs );
        SPRTF("Fetched %d bytes from '%s', in %s\n", docbuf.size, url, elap );
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
                    if (VERB9 || show_errors) {
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
            case 'a':
                skip_all_one = 0;
                break;
            case 'b':
                skip_not_letters = 0;
                break;
            case 'e':
                show_errors = 1;
                break;
            case 'l':
                show_links = 1;
                break;
            case 't':
                show_title = 1;
                break;
            case 'v':
                verbosity++;
                sarg++;
                while (*sarg) {
                    if (ISDIGIT(*sarg)) {
                        verbosity = atoi(sarg);
                        break;
                    } else if (*sarg == 'v') {
                        verbosity++;
                    }
                }
                break;
            case 'o':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    usr_output = strdup(sarg);
                    CHKMEM(usr_output);
                    usr_file = fopen(usr_output,"w");
                    if (!usr_file) {
                        SPRTF("%s: Error: Failed to open output file '%s'!\n", module, usr_output);
                        return 1;
                    }
                } else {
                    SPRTF("%s: Error: Expected output file to follow '%s'!\n", module, arg);
                    return 1;
                }
                break;
            // TODO: Other arguments
            default:
                SPRTF("%s: Unknown argument '%s'. Try -? for help...\n", module, arg);
                return 1;
            }
        } else {
            // bear argument - assume input url
            if (usr_input) {
                SPRTF("%s: Already have input '%s'! What is this '%s'?\n", module, usr_input, arg );
                return 1;
            }
            usr_input = strdup(arg);
            CHKMEM(usr_input);
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
    bgn_secs = get_seconds();
    set_log_file((char *)def_log, 0);

    iret = parse_args( argc, argv );
    if (iret)
        return iret;

    /////////////////////////////////////
    iret = load_url();  // action of app
    /////////////////////////////////////

    // CLEAN UP //
    if (usr_file) {
        fclose(usr_file);
        SPRTF("%s: output written to '%s'\n", module, usr_output);
    }
    usr_file = 0;
    if (usr_output)
        free((void *)usr_output);
    if (usr_input)
        free((void *)usr_input);
    SPRTF("%s: Ran for %s...\n", module, get_seconds_stg( get_seconds() - bgn_secs) );
    close_log_file();
    return iret;
}

void give_help( char *name )
{
    SPRTF("\n");
    SPRTF("%s: usage: [options] usr_input\n", module);
    SPRTF("\n");
    SPRTF("Options:\n");
    SPRTF(" --help  (-h or -?) = This help and exit(2)\n");
    SPRTF(" --all         (-a) = Show all text. On skips lines of just 1 char. (def=%d)\n", skip_all_one);
    SPRTF(" --bare        (-b) = Show non-letters. On skips lines with no letters. (def=%d)\n", skip_not_letters);
    SPRTF(" --errors      (-e) = Show warnings and errors in output. (def=%d)\n", show_errors);
    SPRTF(" --links       (-l) = Show links in output. (def=%d)\n", show_links);
    SPRTF(" --title       (-t) = Show title in output. (def=%d)\n", show_title);
    SPRTF(" --verb[n]     (-v) = Bump or set verbosity. 0,1,2,5,9 (def=%d)\n", verbosity);
    SPRTF(" --out <file>  (-o) = Write output to this file. (def=%s)\n",
        (usr_output ? usr_output : "<none>") );
    SPRTF("\n");
    SPRTF(" Use library CURL to fetch the URL text, into a tidy buffer,\n");
    SPRTF(" and pass the html to library tidy, then enumerate the nodes\n");
    SPRTF(" collected.\n");

    // TODO: More help
}

/* eof */
