/*\
 * tidy-json.cxx
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
 * Example: <div><span>Text1</span>Text2</div>
{
  "type" : "div",
  "content" : [
    {
      "type" : "span",
      "content" : [
        "Text2"
      ]
    },
    "Text1"
  ]
}

 *
\*/

#include <stdio.h>
#include <string.h> // for strdup(), ...
#include <tidy.h>
#include <tidybuffio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#ifndef SPRTF
#define SPRTF printf
#endif

static const char *module = "tidy-json";

static const char *usr_input = 0;
static std::string json_str = "";
static int add_newline = 1;
static int add_indent = 1;
static int show_raw_text = 0;
static const char *indent = "  ";
static const char *endln = "\n";

#define MMX_INDENT  1024
#define flg_needs_comma  0x00001

typedef struct tagJCTX {
    TidyDoc doc;
    int state[MMX_INDENT];
}JCTX, * PJCTX;
static JCTX jcx;
// forward ref
int output_node( PJCTX pjcx, TidyNode node, int lev );

void give_help( char *name )
{
    printf("%s: usage: [options] usr_input\n", module);
    printf("Options:\n");
    printf(" --help  (-h or -?) = This help and exit(2)\n");
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
    if (!usr_input) {
        printf("%s: No user input found in command!\n", module);
        return 1;
    }
    return 0;
}

ctmbstr getNodeName( TidyNode node )
{
    ctmbstr name = 0;
    TidyNodeType nt = tidyNodeGetType(node);
    switch (nt)
    {
    case TidyNode_Root:
        name = "#Root";
        break;
    case TidyNode_DocType:
        name = "#DOCTYPE";
        break;
    case TidyNode_Comment:
        name = "#Comment";
        break;
    case TidyNode_ProcIns:
        name = "#Processing Instruction";
        break;
    case TidyNode_Text:
        name = "#Text";
        break;
    case TidyNode_CDATA:
        name = "#CDATA";
        break;
    case TidyNode_Section:
        name = "#XML Section";
        break;
    case TidyNode_Asp:
        name = "#ASP";
        break;
    case TidyNode_Jste:
        name = "#JSTE";
        break;
    case TidyNode_Php:
        name = "#PHP";
        break;
    case TidyNode_XmlDecl:
        name = "#XML Declaration";
        break;

    case TidyNode_Start:
    case TidyNode_End:
    case TidyNode_StartEnd:
    default:
        name = tidyNodeGetName( node );
        break;
    }
    if ( name == 0 ) {
        SPRTF("Internal Error: Failed to get node name/type!!! *** FIX ME ***\n");
        //exit(1);
        name = "#Internal Error";
    }
    return name;
}

void add_a_newline()
{
    if (add_newline) {
        size_t len = json_str.size();
        if (len) {
            len--;
            json_str += endln;
        }
    }
}

void add_indent_str(int lev)
{
    if (add_indent && add_newline) {
        int i;
        for (i = 0; i < lev; i++)
            json_str += indent;
    }
}

void clear_upper_levels( PJCTX pjcx, int lev )
{
    while (++lev < MMX_INDENT) {
        pjcx->state[lev] = 0;
    }
}

void add_a_comma( PJCTX pjcx, int lev )
{
    if (pjcx->state[lev] & flg_needs_comma)
        json_str += ",";

}

void output_attribute( PJCTX pjcx, TidyAttr attr, int lev )
{
    ctmbstr nam = tidyAttrName(attr);
    ctmbstr aval = tidyAttrValue(attr);

    add_a_comma( pjcx, lev );
    add_a_newline();
    add_indent_str(lev);
    json_str += "{";

    add_a_newline();
    add_indent_str(lev);

    json_str += "\"name\" : ";
    json_str += "\"";
    json_str += nam;
    json_str += "\"";

    pjcx->state[lev] |= flg_needs_comma;

    if (aval) {

        add_a_comma( pjcx, lev );
        add_a_newline();
        add_indent_str(lev);

        json_str += "\"value\" : ";
        json_str += "\"";
        json_str += aval;
        json_str += "\"";

    }

    add_a_newline();
    add_indent_str(lev);
    json_str += "}";

}

void out_child_node( PJCTX pjcx, TidyNode child, int lev )
{
    add_a_comma( pjcx, lev );
    add_a_newline();
    add_indent_str(lev);
    json_str += "{";

    for ( ; child; child = tidyGetNext(child) )
        output_node( pjcx, child, lev + 1 );

    add_a_newline();
    add_indent_str(lev);
    json_str += "}";

    pjcx->state[lev] |= flg_needs_comma;
}

void add_tbuf_to_json( TidyBuffer *pbuf )
{
    // json_str += (char *)pbuf->bp;
    uint i;
    char c;
    for (i = 0; i < pbuf->size; i++) {
        c = pbuf->bp[i];
        if (c >= ' ') {
            if (c == '"') 
                json_str += "\\";
            json_str += c;
            if (c == '\\')
                json_str += c;
        } else if (c == '\t') {
            json_str += "\\t";
        } else if (c == '\n') {
            json_str += "\\n";
        } else if (c == '\r') {
            json_str += "\\r";
        }
    }
}


//////////////////////////////////////////////////////
// output a single node
int output_node( PJCTX pjcx, TidyNode node, int lev )
{
    int iret = 0;
    TidyDoc doc = pjcx->doc;
    TidyBuffer buf;
    tidyBufInit(&buf);

    TidyNode child = tidyGetChild(node);
    TidyNodeType nt = tidyNodeGetType(node);
    ctmbstr name = getNodeName(node);
    TidyAttr attr = tidyAttrFirst(node);

    add_a_comma( pjcx, lev );
    add_a_newline();
    add_indent_str(lev);

    json_str += "\"name\" : ";
    json_str += "\"";
    json_str += name;
    json_str += "\"";

    pjcx->state[lev] |= flg_needs_comma;

    if (nt == TidyNode_Text) {
        if (show_raw_text) {
            tidyNodeGetValue(doc, node, &buf);
        } else {
            tidyNodeGetText(doc, node, &buf);
        }

        if (buf.bp && buf.size) {

            add_a_comma( pjcx, lev );
            add_a_newline();
            add_indent_str(lev);

            json_str += "\"value\" : ";
            json_str += "\"";
            add_tbuf_to_json( &buf );
            json_str += "\"";

            pjcx->state[lev] |= flg_needs_comma;
        }
    }

    if (attr) {

        add_a_comma( pjcx, lev );
        add_a_newline();
        add_indent_str(lev);

        json_str += "\"attributes\" : [";

        for ( ; attr; attr = tidyAttrNext(attr) ) {
            output_attribute( pjcx, attr, lev + 1 );
        }

        add_a_newline();
        add_indent_str(lev);
        json_str += "]";
        pjcx->state[lev] |= flg_needs_comma;
    }

    if (child) {

        add_a_comma( pjcx, lev );
        add_a_newline();
        add_indent_str(lev);

        json_str += "\"content\" : [";

        out_child_node( pjcx, child, lev + 1 );

        add_a_newline();
        add_indent_str(lev);
        json_str += "]";
        pjcx->state[lev] |= flg_needs_comma;
    }

    tidyBufFree(&buf);
    clear_upper_levels( pjcx, lev );
    return iret;
}

int output_json( PJCTX pjcx, TidyDoc tdoc )
{
    int iret = 0;
    TidyNode node = tidyGetRoot(tdoc);
    json_str += "{";

    iret = output_node(pjcx, node, 1);

    add_a_newline();

    json_str += "}";
    add_a_newline();

    size_t len = json_str.size();
    const char *json_file = "temp.json";
    FILE *fp = fopen(json_file,"w");
    if (fp) {
        size_t res = fwrite(json_str.c_str(),1,len,fp);
        fclose(fp);
        if (len == res) {
            printf("%s: Written to file '%s'.\n", module, json_file);
        } else {
            printf("%s: Write to file '%s' failed!\n", module, json_file);
            iret = 1;

        }
    } else {
        std::cout << json_str << std::endl;
        printf("%s: Write to file '%s' failed!\n", module, json_file);
        iret = 1;
    }
    return iret;
}

int process_input()
{
    int status, iret = 0;
    const char *htmlfil = usr_input;
    TidyDoc tdoc;
    tdoc = tidyCreate();
    status = tidyParseFile( tdoc, htmlfil );
    if ( status >= 0 ) {
        status = tidyCleanAndRepair( tdoc );
    } else {
        printf("%s: Failed to load '%s'!\n", module, htmlfil);
        iret = 1;
        goto exit;
    }
    if ( status >= 0 ) {
        status = tidyRunDiagnostics( tdoc );
    } else {
        printf("%s: Failed tidyCleanAndRepair()!\n", module);
        iret = 1;
        goto exit;
    }
    if ( status >= 0 ) {
        memset(&jcx,0,sizeof(JCTX));
        jcx.doc = tdoc;
        iret = output_json( &jcx, tdoc );
    } else {
        printf("%s: Failed tidyCleanAndRepair()!\n", module);
        iret = 1;
    } 

exit:
    tidyRelease( tdoc ); /* called to free hash tables etc. */
    return iret;
}

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    iret = parse_args(argc,argv);
    if (iret) {
        if (iret == 2)
            iret = 0;
        return iret;
    }
    iret = process_input(); // actions of app

    return iret;
}

// sample code
auto out_contents( const std::istream &input ) -> void
{
    std::ostringstream oss;
    oss << input.rdbuf();
    const auto file_str = oss.str();    // xml2json( oss.str().data() );
    std::cout << file_str << std::endl;
}

void show_file( const char *file )
{
    out_contents( std::ifstream(file) );
}

// eof = tidy-json.cxx
