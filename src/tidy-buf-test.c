/*\
 * tidy-buf-test.c
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
 * 20160528 - To test issue #413
 * Load file contents into a tidy buffer, and use tidyParseBuffer
 *
\*/

#include <stdio.h>
#include <string.h> // for strdup(), ...
#include <tidy.h>
#include <tidybuffio.h>
#ifndef SPRTF
#define SPRTF printf
#endif
static const char *module = "tidy-buf-test";

static const char *usr_input = 0;
static const char *def_input = "F:\\Projects\\tidy-test\\test\\input5\\in_413.html";

void give_help( char *name )
{
    printf("%s: usage: [options] usr_input\n", module);
    printf("Options:\n");
    printf(" --help  (-h or -?) = This help and exit(2)\n");
    printf(" Given a valid html input file, will load the contents into a tidy buffer,\n");
    printf(" and use tidyParseBuffer() to parse the html, and will then show all the nodes\n");
    printf(" in the tidy tree.\n");
    printf(" In other words more or less show using tidyParseBuffer() does nothing different than\n");
    printf(" cosole tidy parsing and outputting the same html.\n");
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
                printf("%s: Unknown argument '%s'. Try -? for help...\n", module, arg);
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
#if (defined(WIN32) && !defined(NDEBUG))
    if (!usr_input) {
        usr_input = def_input;
        printf("%s: No user input found. Using default %s!\n", module, usr_input);
    }
#endif

    if (!usr_input) {
        printf("%s: No user input found in command!\n", module);
        return 1;
    }
    return 0;
}

static void countNodes( TidyNode tnod, int *pcnt )
{
    TidyNode child;
    for ( child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
    {
        *pcnt += 1;
        countNodes( child, pcnt );
    }
}

static TidyBuffer txtbuf;
static TidyDoc tdoc = 0;
static int total_txt = 0;
static int txt_nodes = 0;
static int ind_step = 2;
#define MY_MX_TEXT 80
static byte btext[MY_MX_TEXT+16];

byte *getText(byte *text)
{
    byte *bp = btext;
    byte b;
    int i, dots, out, len = strlen((char *)text);
    out = 0;
    for (i = 0; i < len; i++) {
        b = text[i];
        if (b < 0x20) {
            bp[out++] = '^';
            // 010 c 0A - 013 l 0D -> 110 n 6E - 077 M 4D 074 J 4A
            //b += 'A';
            b += 64;
        }
        bp[out++] = b;
        if (out >= MY_MX_TEXT) {
            if ((i + 1) < len) {
                dots = len - (i + 1);
                if (dots > 3)
                    dots = 3;
                while (dots--)
                    bp[out++] = '.';
            }
            break;
        }
    }
    bp[out] = 0;
    return bp;
}
void dumpNode( TidyNode tnod, int indent, int *pcnt )
{
    TidyNode child;
    TidyNodeType nt;
    TidyAttr attr;
    ctmbstr value, aname = 0;
    byte *text;
    for ( child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
    {
        ctmbstr name = NULL;
        *pcnt += 1;
        nt = tidyNodeGetType(child);
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
        for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
            aname = tidyAttrName(attr);
            value = tidyAttrValue(attr);
            printf("%s=\"%s\" ", (aname ? aname : "<blank>"), (value ? value : ""));
        }
        if (txtbuf.bp && txtbuf.size) {
            total_txt += txtbuf.size;
            text = getText(txtbuf.bp);
            SPRTF("(%d) %s", txtbuf.size, text);
            txt_nodes++;
        }
        SPRTF("\n");
        dumpNode( child, indent + ind_step, pcnt );
    }
}


#define MY_MX_BUF 256
int do_buf_test()
{
    int res, iret = 0;
    FILE *fp = 0;
    char buffer[MY_MX_BUF+16];
    TidyBuffer buff;
    TidyNode node = 0;
    
    fp = fopen(usr_input,"r");
    if (!fp) {
        printf("%s: Can NOT open file %s!\n", module, usr_input);
        return 1;
    }
    tdoc = tidyCreate();
    tidyBufInit(&buff);
    while (( res = fread(buffer,1,MY_MX_BUF,fp) ) > 0 ) {
        buffer[res] = 0;
        tidyBufAppend(&buff, buffer, res);
    }
    fclose(fp);
    if ( !buff.bp || !buff.size ) {
        printf("%s: Failed to get any data from '%s'\n", module, usr_input);
        return 1;
    }
    res = tidyParseBuffer(tdoc, &buff);
    if (res != 0) {
        printf("%s: Error: Parsing html file data returned %d\n",res);
        return 1;
    }
    node = tidyGetRoot(tdoc);
    if (!node) {
        printf("%s: Failed to get ROOT node!\n");
        iret = 1;
        goto exit;
    }
    res = 0;
    countNodes(node, &res);
    printf("%s: Got %d nodes in tidy tree...\n", module, res );
    tidyBufInit(&txtbuf);
    res = 0;
    dumpNode( node, 0, &res );

exit:
    tidyBufFree( &buff );
    tidyRelease( tdoc ); /* called to free hash tables etc. */

    return iret;
}

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    iret = parse_args(argc,argv);
    if (iret)
        return iret;

    iret = do_buf_test();   // actions of app

    return iret;
}


// eof = tidy-buf-test.c
