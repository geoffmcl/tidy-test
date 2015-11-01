/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* <DESC>
 * Download a document and use libtidy to parse the HTML.
 * Started as a CURL sample app, part of the curl source
 * Updated to use html-tidy5 library, and some options added.
 * </DESC>
 */
/*
 * LibTidy => https://github.com/htacg/tidy-html5
 * 
  TIDY COPYRIGHT NOTICE:
 
  This software and documentation is provided "as is," and
  the copyright holders and contributing author(s) make no
  representations or warranties, express or implied, including
  but not limited to, warranties of merchantability or fitness
  for any particular purpose or that the use of the software or
  documentation will not infringe any third party patents,
  copyrights, trademarks or other rights. 

  The copyright holders and contributing author(s) will not be held
  liable for any direct, indirect, special or consequential damages
  arising out of any use of the software or documentation, even if
  advised of the possibility of such damage.

  Permission is hereby granted to use, copy, modify, and distribute
  this source code, or portions hereof, documentation and executables,
  for any purpose, without fee, subject to the following restrictions:

  1. The origin of this source code must not be misrepresented.
  2. Altered versions must be plainly marked as such and must
     not be misrepresented as being the original source.
  3. This Copyright notice may not be removed or altered from any
     source or altered source distribution.
 
  The copyright holders and contributing author(s) specifically
  permit, without fee, and encourage the use of this source code
  as a component for supporting the Hypertext Markup Language in
  commercial products. If you use this source code in a product,
  acknowledgment is not required but would be appreciated.
 *
 */

#include <stdio.h>
#include <tidy.h>
#include <tidybuffio.h>
#include <curl/curl.h>

static const char *module = "htmltidy";

static int def_indent = 4;
static char *out_name = 0;
static FILE *out_file = 0;
static char *usr_url = 0;
static int verbosity = 0;
static int show_type = 0;
static int node_count = 0;
static int nul_nodes = 0;

// if given an output file, use it, else stdout
#define OUTFILE (out_file ? out_file : stdout)

/* curl write callback, to fill tidy's input buffer...  */
uint write_cb(char *in, uint size, uint nmemb, TidyBuffer *out)
{
    uint r;
    r = size * nmemb;
    tidyBufAppend( out, in, r );
    return r;
}

static const char *getNodeType( TidyNodeType nt )
{
    switch (nt)
    {
    case TidyNode_Root: return "Root";
    case TidyNode_DocType: return "DOCT";   // "DOCTYPE";
    case TidyNode_Comment: return "Comm";   // "Comment";
    case TidyNode_ProcIns: return "PrIn"; /**< Processing Instruction */
    case TidyNode_Text: return "Text";  /**< Text */
    case TidyNode_Start: return "Star"; /**< Start Tag */
    case TidyNode_End: return  "Endt";    /**< End Tag */
    case TidyNode_StartEnd: return "Clos";    /**< Start/End (empty) Tag */
    case TidyNode_CDATA: return "CDAT";     /**< Unparsed Text */
    case TidyNode_Section: return "XMLs";     /**< XML Section */
    case TidyNode_Asp: return "ASPs";         /**< ASP Source */
    case TidyNode_Jste: return "JSTE";       /**< JSTE Source */
    case TidyNode_Php: return "PHPs";         /**< PHP Source */
    case TidyNode_XmlDecl: return "XMLd";      /**< XML Declaration */
    }
    return "UNKN";
}

void trimTidyBuffer( TidyBuffer *pb )
{
    while (pb->size) {
        pb->size--;
        if (pb->bp[pb->size] > ' ') {
            pb->size++;
            break;
        }
        pb->bp[pb->size] = 0;
    }
}

/* Traverse the document tree */
void dumpNode(TidyDoc doc, TidyNode tnod, int indent )
{
    TidyNode child;
    for ( child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
    {
        TidyNodeType nt = tidyNodeGetType(child);
        const char *type = getNodeType(nt);
        ctmbstr name = tidyNodeGetName( child );
        node_count++;
        if ( name )
        {
            /* if it has a name, then it's an HTML tag ... */
            TidyAttr attr;
            if (show_type)
                fprintf(OUTFILE, "%s ",type);
            if (indent)
                fprintf(OUTFILE, "%*.*s", indent, indent, "");
            fprintf(OUTFILE, "<%s", name);
            /* walk the attribute list */
            for ( attr=tidyAttrFirst(child); attr; attr=tidyAttrNext(attr) ) {
                fprintf(OUTFILE," ");
                fprintf(OUTFILE,"%s",tidyAttrName(attr));
                if (tidyAttrValue(attr))
                    fprintf(OUTFILE,"=\"%s\"", tidyAttrValue(attr));
            }
            fprintf(OUTFILE,">\n");
        }
        else {
            /* if it doesn't have a name, then it's probably text, cdata, etc... */
            TidyBuffer buf;
            tidyBufInit(&buf);
            tidyNodeGetText(doc, child, &buf);
            if (buf.bp && buf.size) {
                trimTidyBuffer( &buf );
                if (buf.size) {
                    if (show_type)
                        fprintf(OUTFILE, "%s ",type);
                    if (indent)
                        fprintf(OUTFILE,"%*.*s", indent, indent, "");
                    fprintf(OUTFILE,"%s\n", buf.bp);
                } else {
                    nul_nodes++;
                }
            } else {
                nul_nodes++;
            }
            tidyBufFree(&buf);
        }
        dumpNode( doc, child, indent + def_indent ); /* recursive */
    }
}

void give_help( char * name )
{
    printf("Usage: %s [options] usr_url\n", module );
    printf("\n");
    printf("Options:\n");
    printf(" --help   (-h or -?) = Give this help, and exit(0)\n");
    printf(" --indent <num> (-i) = Set indent. 0 to disable. (def=%d)\n", def_indent);
    printf(" --verb         (-v) = Set CURL to verbose mode.\n");
    printf(" --out <file>   (-o) = Write node information to a file.\n");
    printf(" --type         (-t) = Add the node type to the output.\n");
    printf("\n");
    printf(" Given a valid URL, fetch web page using CURL,\n");
    printf(" pass the page to libtidy for parsing, cleaning,\n");
    printf(" and enumerate the tidy nodes collected.\n");
    printf("\n");
}

#define ISDIGIT(a) (( a >= '0' ) && ( a <= '9' ))

int is_all_digits( char *num )
{
    size_t i, len = strlen(num);
    int c;
    for (i = 0; i < len; i++) {
        c = num[i];
        if (!ISDIGIT(c))
            return 0;
    }
    if (len)
        return 1;
    return 0;
}

int parse_args( int argc, char **argv )
{
    int c, i, i2, iret = 0;
    char *arg, *sarg;
    for (i = 1; i < argc; i++) {
        i2 = i + 1;
        arg = argv[i];
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
            case 'i':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    if (is_all_digits(sarg)) {
                        def_indent = atoi(sarg);
                    } else {
                        fprintf(stderr,"%s: Expected number to follow '%s', not '%s'!\n", module, arg, sarg );
                        return 1;
                    }
                } else {
                    fprintf(stderr,"%s: Expected number to follow '%s'!\n", module, arg );
                    return 1;
                }
                break;
            case 'o':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    if (out_name) {
                        fprintf(stderr,"%s: Already have output file '%s'! What is this '%s'?\n", module, out_name, sarg);
                        return 1;
                    }
                    out_name = sarg;
                    out_file = fopen(out_name,"w");
                    if (!out_file) {
                        fprintf(stderr,"%s: Unable to open out file '%s'!\n", module, out_name);
                        return 1;
                    }
                } else {
                    fprintf(stderr,"%s: Expected file name to follow '%s'!\n", module, arg );
                    return 1;
                }
                break;
            case 't':
                show_type = 1;
                break;
            case 'v':
                verbosity = 1;
                break;
            default:
                fprintf(stderr,"%s: Unknown option %s!\n", module, arg);
                return 1;
            }
        } else {
            if (usr_url) {
                fprintf(stderr,"%s: Already have '%s'! What is this '%s'!\n", module, usr_url, arg);
                return 1;
            }
            usr_url = arg;
        }
    }
    if (!usr_url) {
        fprintf(stderr,"%s: No URL to fetch found in command line!\n", module);
        return 1;
    }
    return iret;
}

int main(int argc, char **argv )
{
    CURL *curl;
    char curl_errbuf[CURL_ERROR_SIZE];
    TidyDoc tdoc;
    TidyBuffer docbuf = {0};
    TidyBuffer tidy_errbuf = {0};
    int err = parse_args( argc, argv );
    if (err) {
        if (err == 2)
            err = 0;
        return err;
    }
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, usr_url);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, verbosity ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);

    tdoc = tidyCreate();
    tidyOptSetBool(tdoc, TidyForceOutput, yes); /* try harder */
    tidyOptSetInt(tdoc, TidyWrapLen, 4096);
    tidySetErrorBuffer( tdoc, &tidy_errbuf );
    tidyBufInit(&docbuf);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &docbuf);
    printf("\n");
    printf("%s: Will try to fetch '%s', using CURL...\n", module, usr_url );
    err = curl_easy_perform(curl);
    if ( !err ) {
        printf("\n");
        printf("%s: CURL fetched %d bytes, passed to libtidy...\n", module, docbuf.size);
        err = tidyParseBuffer(tdoc, &docbuf); /* parse the input */
        if ( err >= 0 ) {
            err = tidyCleanAndRepair(tdoc); /* fix any problems */
            if ( err >= 0 ) {
                err = tidyRunDiagnostics(tdoc); /* load tidy error buffer */
                if ( err >= 0 ) {
                    printf("%s: Will dump the node tree created by tidy...\n", module );
                    if (out_name) 
                        printf("%s: Output will be to '%s'...\n", module, out_name);
                    dumpNode( tdoc, tidyGetRoot(tdoc), 0 ); /* walk the tree */
                    trimTidyBuffer( &tidy_errbuf );
                    if (tidy_errbuf.size)
                        fprintf(stderr, "%s: %s\n", module, tidy_errbuf.bp); /* show errors */
                    fprintf(stderr, "%s: Processed %d tidy nodes...\n", module, node_count);
                }
            }
        }
    }
    else
      fprintf(stderr, "%s: %s\n", module, curl_errbuf);

    /* clean-up */
    curl_easy_cleanup(curl);
    tidyBufFree(&docbuf);
    tidyBufFree(&tidy_errbuf);
    tidyRelease(tdoc);
    return(err);
}

/* eof */
