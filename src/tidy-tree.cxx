/*\
 * tidy-tree.cxx
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h> // for strdup(), ...
#include "tidy.h"
#include "tidybuffio.h"
#include "sprtf.h"

#ifndef SPRTF
#define SPRTF printf
#endif

static const char *module = "tidy-tree";
static const char *def_log = "temptree.txt";
static const char *usr_input = 0;
static int ind_step = 2;
static const char *def_test = "F:\\Projects\\tidy-html5\\test\\input5\\in_273-3.html";
static bool debug_on = false;
static TidyDoc tdoc = 0;
static TidyBuffer txtbuf;
static size_t total_txt = 0;
static int txt_nodes = 0;

#if !defined(TT_VERSION) || !defined(TT_DATE)
#error "This must be compiled setting TT_VERSION and TT_DATE strings!"
#endif

void show_lib_version()
{
    ctmbstr prd = tidyReleaseDate();
    ctmbstr plv = tidyLibraryVersion();
#ifdef  PLATFORM_NAME
    SPRTF("%s: Using library HTML Tidy for %s, circa %s, version %s\n", module,
        PLATFORM_NAME, prd, plv);
#else
    SPRTF("%s: Using library HTML Tidy, circa %s, version %s\n", module,
        prd, plv);
#endif

}

void show_version()
{
    SPRTF("%s version %s, circa %s\n", module, TT_VERSION, TT_DATE);
    show_lib_version();
}

void give_help( char *name )
{
    show_version();
    SPRTF("Usage:\n");
    SPRTF("%s [options] usr_input\n", module);
    SPRTF("Options:\n");
    SPRTF(" --help  (-h or -?) = This help and exit(0)\n");
    SPRTF(" --version     (-v) = show version and exit(0)\n");
    SPRTF("\n");
    SPRTF(" The user input file will be passed to 'libtidy' for\n");
    SPRTF(" parsing, as html, and the resultant DOM like tidy node tree will\n");
    SPRTF(" be enumerated.\n");
}

void dumpNode( TidyNode tnod, int indent, int *pcnt )
{
    TidyNode child;
    TidyAttr tattr;
    uint len;
    for ( child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
    {
        ctmbstr name = NULL;
        *pcnt += 1;
        TidyNodeType nt = tidyNodeGetType(child);
        tidyBufClear( &txtbuf );

        if (tidyNodeHasText(tdoc, child)) {
            if (!tidyNodeGetText(tdoc, child, &txtbuf)) {
                SPRTF("Warning: tidyNodeGetText failed?!?\n");
            }
        }
        switch (nt)
        {
        case TidyNode_Root:
            name = "Root";
            break;
        case TidyNode_DocType:
            name = "DOCTYPE";
            break;
        case TidyNode_Comment:
            name = "Comment";
            break;
        case TidyNode_ProcIns:
            name = "Processing Instruction";
            break;
        case TidyNode_Text:
            name = "Text";
            break;
        case TidyNode_CDATA:
            name = "CDATA";
            break;
        case TidyNode_Section:
            name = "XML Section";
            break;
        case TidyNode_Asp:
            name = "ASP";
            break;
        case TidyNode_Jste:
            name = "JSTE";
            break;
        case TidyNode_Php:
            name = "PHP";
            break;
        case TidyNode_XmlDecl:
            name = "XML Declaration";
            break;

        case TidyNode_Start:
        case TidyNode_End:
        case TidyNode_StartEnd:
        default:
            name = tidyNodeGetName( child );
            break;
        }
        if ( name == NULL ) {
            SPRTF("Internal Error: Failed to get node name/type!!! *** FIX ME ***\n");
            exit(1);
        }
        SPRTF( "%*.*sNode:%d: %s ", indent, indent, " ", *pcnt, name );
        // dump attributes of node
        for (tattr = tidyAttrFirst(child); tattr; tattr = tidyAttrNext(tattr)) {
            ctmbstr anam = tidyAttrName(tattr);
            ctmbstr aval = tidyAttrValue(tattr);
            if (aval)
                SPRTF("%s=\"%s\" ", anam, aval);
            else
                SPRTF("%s ", anam);
        }
        if (txtbuf.bp && txtbuf.size) {
            total_txt += txtbuf.size;   // NOTE: This size may inclue trailing spaces chars...
            SPRTF("(%d) ", txtbuf.size);
            txt_nodes++;
            // trim end of buffer
            len = txtbuf.size;
            while (len--) {
                if (txtbuf.bp[len] > ' ')
                    break;
                txtbuf.bp[len] = 0;
                txtbuf.size--;
            }
            if (txtbuf.size) {
                // zero terminate the buffer
                tidyBufAppend(&txtbuf, (void *)"\0", 1);
                SPRTF("%s", txtbuf.bp);
            }

        }
        SPRTF("\n");
        dumpNode( child, indent + ind_step, pcnt );
    }
}

void countNodes( TidyNode tnod, int *pcnt )
{
    TidyNode child;
    for ( child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
    {
        *pcnt += 1;
        countNodes( child, pcnt );
    }
}


void dumpDoc( TidyDoc doc )
{
    TidyNode node = tidyGetRoot(doc);
    int count = 0;
    countNodes( node, &count );
    SPRTF("Dump of %d nodes...\n", count );
    count = 0;
    tidyBufInit( &txtbuf ); // inti TEXT BUFFER - to use for TEXT
    dumpNode( node, 0, &count );
    SPRTF("Found %ld text bytes on %d nodes...\n", total_txt, txt_nodes );
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
            case 'v':
                show_version();
                return 2;
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

    if (debug_on && !usr_input) {
         usr_input = strdup(def_test);
        SPRTF("%s: Debug ON: Using DEFAULT input %s!\n", module, usr_input);

    }

    if (!usr_input) {
        SPRTF("%s: No user input found in command!\n", module);
        return 1;
    }
    return 0;
}

#ifdef _MSC_VER
#define M_IS_DIR _S_IFDIR
#else // !_MSC_VER
#define M_IS_DIR S_IFDIR
#endif

static struct stat buf;
enum DiskType {
    MDT_NONE,
    MDT_FILE,
    MDT_DIR
};

DiskType is_file_or_directory( const char * path )
{
    if (!path)
        return MDT_NONE;
	if (stat(path,&buf) == 0)
	{
		if (buf.st_mode & M_IS_DIR)
			return MDT_DIR;
		else
			return MDT_FILE;
	}
	return MDT_NONE;
}
size_t get_last_file_size() { return buf.st_size; }
///////////////////////////////////////////////////////////

#ifdef ADD_ISSUE_457
// Code from Issue #457
static int node_count = 0;
void IterateNode(TidyDoc doc, TidyNode tnod)
{
	TidyNode child;
	for (child = tidyGetChild(tnod); child; child = tidyGetNext(child))
	{
		ctmbstr name = tidyNodeGetName(child);

		node_count++;
		SPRTF("%2d: ", node_count);

		if (name)
		{
			TidyNodeType nodeType = tidyNodeGetType(child);

			// if it has a name, then it's an HTML tag ...
			TidyAttr attr;
			SPRTF("<%s ", name);
			// walk the attribute list
			for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr))
			{
				printf(tidyAttrName(attr));
				tidyAttrValue(attr) ? SPRTF("=\"%s\" ", tidyAttrValue(attr)) : SPRTF(" ");
			}
			SPRTF(">\n");
		}
		else
		{
			/* if it doesn't have a name, then it's probably text, cdata, etc... */
			TidyBuffer buf;
			tidyBufInit(&buf);
			tidyNodeGetText(doc, child, &buf);
			SPRTF("%s\n", buf.bp ? (char *)buf.bp : "");
			tidyBufFree(&buf);
		}
		IterateNode(doc, child); /* recursive */
	}
}
#endif // #ifdef ADD_ISSUE_457
///////////////////////////////////////////////////////////////////////

int show_tidy_nodes()
{
    int iret = 0;
    int status = 0;
    const char *htmlfil = usr_input;
    if (is_file_or_directory(htmlfil) != MDT_FILE) {
        SPRTF("Error: Unable to stat file '%s'!\n", htmlfil);
        return 1;
    }
    size_t sz = get_last_file_size();
    SPRTF("Processing file '%s', %ld bytes...\n", htmlfil, sz );

    tdoc = tidyCreate();
    tidyOptSetBool( tdoc, TidySkipNested, yes );
    status = tidyParseFile( tdoc, htmlfil );
    if ( status >= 0 )
        status = tidyCleanAndRepair( tdoc );

    if ( status >= 0 ) {
        status = tidyRunDiagnostics( tdoc );
#ifdef USE_TIDY5_API
        if ( !tidyOptGetBool(tdoc, TidyQuiet) ) {
            /* NOT quiet, show DOCTYPE, if not already shown */
            if (!tidyOptGetBool(tdoc, TidyShowInfo)) {
                tidyOptSetBool( tdoc, TidyShowInfo, yes );
                tidyReportDoctype( tdoc );  /* FIX20140913: like warnings, errors, ALWAYS report DOCTYPE */
                tidyOptSetBool( tdoc, TidyShowInfo, no );
            }
        }
#endif // #ifdef USE_TIDY5_API
    }
    if ( status > 1 ) /* If errors, do we want to force output? */
            status = ( tidyOptGetBool(tdoc, TidyForceOutput) ? status : -1 );

    dumpDoc(tdoc);

#ifdef ADD_ISSUE_457
	SPRTF("Issue #457 - Ignoring close elements\n");
	IterateNode(tdoc, tidyGetRoot(tdoc));
	SPRTF("End of testing - done %d nodes...\n", node_count);
#endif // ADD_ISSUE_457

    tidyRelease( tdoc ); /* called to free hash tables etc. */
    iret = ((status < 0) && (status > 1)) ? 1 : 0;
    return iret;
}

void test_no_doc()
{
    TidyDoc td = 0;
	// ctmbstr vers = tidyLibraryVersion();
	// ctmbstr date = tidyReleaseDate();
	// SPRTF("Testing 'libtidy' version %s (%s)\n", vers, date);
    show_lib_version();
    td = tidyCreate();
    int status = tidyCleanAndRepair( td );
    tidyRelease( td ); /* called to free hash tables etc. */
}

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    set_log_file((char *)def_log, 0);
    iret = parse_args(argc,argv);
    if (iret) {
        if (iret == 2)
            iret = 0;
        return iret;
    }

    test_no_doc();

    iret = show_tidy_nodes();    // actions of app

    return iret;
}


// eof = tidy-tree.cxx
