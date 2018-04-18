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
#include <vector>



#ifndef SPRTF
#define SPRTF printf
#endif

static const char *module = "tidy-json";

typedef std::vector<std::string> vSTG;

static const char *usr_input = 0;
static std::string json_str = "";
static int add_newline = 1;
static int add_indent = 1;
static int show_raw_text = 0;
static const char *indent = "  ";
static const char *endln = "\n";
static const char *json_file = "tempnode.json";
static const char *msg_json = "tempmsg.json";
static const char *config_file = 0; /* passed to 'libtidy', def <none> */
static const char *lang = 0; /* passed to 'libtidy', def <none> */

static vSTG messages;   /* accumulation of 'messages' arriving at 'callback' */

#define MMX_INDENT  1024
#define flg_needs_comma  0x00001

static char _s_buf[1024];

typedef struct tagJCTX {
    TidyDoc doc;
    int state[MMX_INDENT];
}JCTX, * PJCTX;
static JCTX jcx;
// forward ref
int output_node( PJCTX pjcx, TidyNode node, int lev );

/////////////////////////////////////////////////////////////////////////
static void show_lib_version()
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

static void show_version()
{
    SPRTF("%s version %s, circa %s\n", module, TT_VERSION, TT_DATE);
    show_lib_version();
}
//////////////////////////////////////////////////////////////////////

void give_help( char *name )
{
    printf("\n");
    show_version();
    printf("%s: usage: [options] usr_input\n", module);
    printf("\n");
    printf("Options:\n");
    printf(" --help  (-h or -?) = This help and exit(0)\n");
    printf(" --version     (-v) = Show version and exit(0)\n");
    printf(" --out <file>  (-o) = Output json to this file. (def=%s)\n", json_file );
    printf(" --msg <file>  (-m) = Output message json to this file. (def=%s)\n", msg_json);
    printf(" --newline     (-n) = Toggle newlines. (def=%s)\n", add_newline ? "on" : "off");
    printf(" --indent      (-i) = Toggle indenting. (def=%s)\n", add_indent ? "on" : "off");
    printf(" --space <cnt> (-s) = Set indent spaces. (def=%d)\n", (int)strlen(indent));
    printf(" --config <file> (-c) = Set config file to pass to libtidy. (def=%s)\n",
        config_file ? config_file : "<none>");
    printf(" --lang <lang>   (-l) = Set language to pass to libtidy. (def=%s)\n",
        lang ? lang : "<none>");


    printf("\n");
    printf(" Indenting only applied if 'newline' is on.\n");
    printf(" Will load the assumed HTML input file in libtidy, version %s (%s)\n", tidyLibraryVersion(), tidyReleaseDate());
    printf(" and convert the node tree to a json output.\n");
    printf("\n");
}

#define IS_DIGIT(a) (( a >= '0')&&( a <= '9'))

int is_all_digits(char *arg)
{
    size_t i, len = strlen(arg);
    for (i = 0; i < len; i++) {
        if (!IS_DIGIT(arg[i]))
            return 0;
    }
    return 1;
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
            case 'i':
                if (add_indent)
                    add_indent = 0;
                else
                    add_indent = 1;
                break;
            case 'n':
                if (add_newline)
                    add_newline = 0;
                else
                    add_newline = 1;
                break;

            case 'o':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    json_file = strdup(sarg);
                } else {
                    printf("%s: Expected file name to follow '%s'!\n", module, arg);
                    return 1;
                }
                break;
            case 'm':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    msg_json = strdup(sarg);
                }
                else {
                    printf("%s: Expected file name to follow '%s'!\n", module, arg);
                    return 1;
                }
                break;
            case 'c':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    config_file = strdup(sarg);
                }
                else {
                    printf("%s: Expected file name to follow '%s'!\n", module, arg);
                    return 1;
                }
                break;
            case 'l':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    lang = strdup(sarg);
                }
                else {
                    printf("%s: Expected language name to follow '%s'!\n", module, arg);
                    return 1;
                }
                break;
            case 's':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    if (is_all_digits(sarg)) {
                        i2 = atoi(sarg);
                        if (i2) {
                            indent = (const char *)malloc(i2 + 2);
                            if (!indent) {
                                printf("%s: Memory failed for %d bytes!\n", module, i2 + 2);
                                return 1;
                            }
                            memset((void *)indent,0,i2+1);
                            memset((void *)indent,' ',i2);
                        } else {
                            printf("%s: Expected an integer. Not '%s'!\n", module, sarg);
                            return 1;
                        }
                    } else {
                        printf("%s: Expected an integer. Not '%s'!\n", module, sarg);
                        return 1;
                    }

                } else {
                    printf("%s: Expected space count to follow '%s'!\n", module, arg);
                    return 1;
                }
                break;
            case 'v':
                show_version();
                return 2;
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

void add_msg_string_len(std::string &msg, ctmbstr stg, size_t len, Bool add_quotes)
{
    size_t ii;
    char c;
    if (add_quotes)
        msg += "\"";
    for (ii = 0; ii < len; ii++)
    {
        c = stg[ii];
        if (c >= ' ')
        {
            if (c == '"')
                msg += '\\';
            else if (c == '\\')
                msg += '\\';
            msg += c;
        }
        else if (c & 0x80) {
            msg += c;   /* Add utf-8 chars */
        }
        else {
            if (c == '\n')
                msg += "\\n";
            else if (c == '\t')
                msg += "\\t";
            else if (c == '\r')
                msg += "\\r";
        }
    }
    if (add_quotes)
        msg += "\"";
}

void add_msg_string(std::string &msg, ctmbstr stg)
{
    add_msg_string_len(msg, stg, strlen(stg), yes);
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

void add_newline_to_json(std::string &json)
{
    if (add_newline) {
        size_t len = json.size();
        if (len) {
            len--;
            json += endln;
        }
    }
}

void add_indent_to_json(std::string &json, int lev)
{
    if (add_indent && add_newline) {
        int i;
        for (i = 0; i < lev; i++)
            json += indent;
    }
}


void add_a_newline()
{
    if (add_newline) {
        add_newline_to_json(json_str);
    }
}

void add_indent_str(int lev)
{
    if (add_indent && add_newline) {
        add_indent_to_json(json_str, lev);
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

void add_tbuf_to_json(TidyBuffer *pbuf)
{
    if (pbuf && pbuf->size && pbuf->bp)
        add_msg_string_len(json_str, (ctmbstr)pbuf->bp, pbuf->size, no);
}

void add_tbuf_to_json2( TidyBuffer *pbuf )
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

void output_file( PJCTX pjcx, int lev )
{
    add_a_comma( pjcx, lev );
    add_a_newline();
    add_indent_str(lev);

    json_str += "\"in_file\" : ";
    json_str += "\"";
    add_msg_string_len(json_str, usr_input, strlen(usr_input), no);
    // json_str += usr_input;
    json_str += "\"";

    pjcx->state[lev] |= flg_needs_comma;

    add_a_comma( pjcx, lev );
    add_a_newline();
    add_indent_str(lev);

    json_str += "\"out_file\" : ";
    json_str += "\"";
    add_msg_string_len(json_str, json_file, strlen(json_file), no);
    //json_str += json_file;
    json_str += "\"";

    pjcx->state[lev] |= flg_needs_comma;
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

    output_file( pjcx, 1 );

    iret = output_node(pjcx, node, 1);

    add_a_newline();

    json_str += "}";
    add_a_newline();

    size_t len = json_str.size();
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

typedef struct tagLEV2STG {
    TidyReportLevel lev;
    const char *stg;
    const char *desc;
}LEV2STG, * PLEV2STG;

static LEV2STG Lev2Stg[] = {
    { TidyInfo, "TidyInfo", "Report: Information about markup usage" },
    { TidyWarning, "TidyWarning", "Report: Warning message" },
    { TidyConfig, "TidyConfig", "Report: Configuration error" },
    { TidyAccess, "TidyAccess", "Report: Accessibility message" },
    { TidyError, "TidyError", "Report: Error message - output suppressed" },
    { TidyBadDocument, "TidyBadDocument", "Report: I/O or file system error" },
    { TidyFatal, "TidyFatal", "Report: Crash!" },
    { TidyDialogueSummary, "TidyDialogueSummary", "Dialogue: Summary-related information" },
    { TidyDialogueInfo, "TidyDialogueInfo", "Dialogue: Non-document related information" },
    { TidyDialogueFootnote, "TidyDialogueFootnote", "Dialogue: Footnote" },
    { TidyDialogueDoc, "TidyDialogueDoc", "Dialogue: Deprecated (renamed)" },
    /* MUST BE LAST*/
    { (TidyReportLevel)0, 0, 0 }
};

const char *MsgLeveltoStg(TidyReportLevel lev)
{
    PLEV2STG p = Lev2Stg;
    while (p->stg) {
        if (p->lev == lev)
            return p->stg;
        p++;
    }
    return "Uknown";
}

Bool TIDY_CALL MessageCallback(TidyMessage tmessage)
{
    TidyDoc tdoc = tidyGetMessageDoc(tmessage);
    uint code = tidyGetMessageCode(tmessage);
    ctmbstr mkey = tidyGetMessageKey(tmessage);
    int line = tidyGetMessageLine(tmessage);
    int col = tidyGetMessageColumn(tmessage);
    TidyReportLevel lev = tidyGetMessageLevel(tmessage);
    Bool muted = tidyGetMessageIsMuted(tmessage);
    ctmbstr form = tidyGetMessageFormatDefault(tmessage);
    ctmbstr def = tidyGetMessageDefault(tmessage);
    ctmbstr tmsg = tidyGetMessage(tmessage);
    ctmbstr posd = tidyGetMessagePosDefault(tmessage);
    ctmbstr pos = tidyGetMessagePos(tmessage);
    ctmbstr pred = tidyGetMessagePrefixDefault(tmessage);
    ctmbstr pre = tidyGetMessagePrefix(tmessage);
    ctmbstr outd = tidyGetMessageOutputDefault(tmessage);
    ctmbstr out = tidyGetMessageOutput(tmessage);
    const char *levstg = MsgLeveltoStg(lev);
    Bool addnl = (add_newline ? yes : no);
    Bool addind = ((addnl && add_indent) ? yes : no);

    TidyIterator itArg;
    char *sb = _s_buf;
    std::string msg;

    msg = "";   /* start blank */
    if (addind) add_indent_to_json(msg, 2);
    /*   msg += "\"message\": {"; - this seems WRONG, so */
    msg += "{";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 3);
    msg += "\"messageLine\": ";
    sprintf(sb, "%d", line);
    msg += sb;
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 3);
    msg += "\"messageColumn\": ";
    sprintf(sb, "%d", col);
    msg += sb;
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 3);
    msg += "\"messageLevel\": ";
    add_msg_string(msg, levstg);
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 3);
    msg += "\"messageIsMuted\": ";
    msg += (muted ? "true" : "false");
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 3);
    msg += "\"messageKey\": ";
    add_msg_string(msg, mkey);
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 3);
    msg += "\"messagePreDefault\": ";
    add_msg_string(msg, pred);
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 3);
    msg += "\"messagePre\": ";
    add_msg_string(msg, pre);
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 3);
    msg += "\"messageDefault\": ";
    add_msg_string(msg, def);
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 3);
    msg += "\"messageLang\": ";
    add_msg_string(msg, tmsg);
    if (addnl) add_newline_to_json(msg);

    itArg = tidyGetMessageArguments(tmessage);
    while (itArg) {
        int arg_int;
        uint arg_uint;
        ctmbstr arg_val;
        double arg_dbl;
        TidyMessageArgument my_arg = tidyGetNextMessageArgument(tmessage, &itArg);
        TidyFormatParameterType pt = tidyGetArgType(tmessage, &my_arg);
        ctmbstr arg_form = tidyGetArgFormat(tmessage, &my_arg);
        switch (pt)
        {
        case tidyFormatType_INT:
            arg_int = tidyGetArgValueInt(tmessage, &my_arg);
            break;
        case tidyFormatType_UINT:
            arg_uint = tidyGetArgValueUInt(tmessage, &my_arg);
            break;
        case tidyFormatType_STRING:
            arg_val = tidyGetArgValueString(tmessage, &my_arg);
            break;
        case tidyFormatType_DOUBLE:
            arg_dbl = tidyGetArgValueDouble(tmessage, &my_arg);
            break;
        case tidyFormatType_UNKNOWN:
        default:
            break;
        }
    }

    if (addind) add_indent_to_json(msg, 2);
    msg += "}";

    messages.push_back(msg);

    return no;
}

void output_message_json()
{
    Bool addnl = (add_newline ? yes : no);
    Bool addind = ((addnl && add_indent) ? yes : no);
    ctmbstr lang = tidyGetLanguage();
    size_t ii, len = messages.size();
    if (!len)
        return;
    FILE *out = fopen(msg_json, "w");
    if (!out)
        return;
    std::string msg;
    std::string s;
    msg = "{";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 1);
    msg += "\"filename\": ";
    add_msg_string(msg, usr_input);
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 1);
    msg += "\"language\": ";
    add_msg_string(msg, lang);
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 1);
    msg += "\"libtidyVersion\": ";
    add_msg_string(msg, tidyLibraryVersion());
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 1);
    msg += "\"libtidyDate\": ";
    add_msg_string(msg, tidyReleaseDate());
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 1);
    msg += "\"libtidyPlatform\": ";
    add_msg_string(msg, tidyPlatform());
    msg += ",";
    if (addnl) add_newline_to_json(msg);

    if (addind) add_indent_to_json(msg, 1);
    msg += "\"messages\": [";
    if (addnl) add_newline_to_json(msg);
    for (ii = 0; ii < len; ii++) {
        s = messages[ii];
        //msg += "  \"message\": ";
        msg += s;
        if ((ii + 1) < len)
            msg += ",";
        if (addnl) add_newline_to_json(msg);
    }

    if (addind) add_indent_to_json(msg, 1);
    msg += "]";
    if (addnl) add_newline_to_json(msg);

    msg += "}";
    if (addnl) add_newline_to_json(msg);

    len = msg.size();
    ii = fwrite(msg.c_str(), 1, len, out);
    fclose(out);
    if (ii == len)
        printf("%s: Message json to file '%s'.\n", module, msg_json);
     else 
        printf("%s: Message json file '%s' failed!\n", module, msg_json);

}


int process_input()
{
    int status, iret = 0;
    Bool b;
    const char *htmlfil = usr_input;
    TidyDoc tdoc;
    tdoc = tidyCreate();
    b = tidySetMessageCallback(tdoc, &MessageCallback);
    if (!b) {
        printf("WARNING: Failed to set message callback\n");
    }
    if (config_file) {
        if (tidyLoadConfig(tdoc, config_file)) {
            printf("Failed to load config file '%s'!\n", config_file);
            iret = 1;
            iret = 1;
            goto exit;
        }
    }
    if (lang) {
        if (!tidySetLanguage(lang))
        {
            printf(tidyLocalizedString(TC_STRING_LANG_NOT_FOUND),
                lang, tidyGetLanguage());
            printf("\n");
            iret = 1;
            goto exit;
        }
    }
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

    if (tidyErrorCount(tdoc) || tidyWarningCount(tdoc))
        tidyErrorSummary(tdoc);

    tidyGeneralInfo(tdoc);

exit:

    output_message_json();

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
void out_contents( const std::istream &input )
{
    std::ostringstream oss;
    oss << input.rdbuf();
    const std::string file_str = oss.str();    // xml2json( oss.str().data() );
    std::cout << file_str << std::endl;
}

void show_file( const char *file )
{
    out_contents( std::ifstream(file) );
}

// eof = tidy-json.cxx
