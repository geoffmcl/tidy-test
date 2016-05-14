/*\ 
 *  20150713 - Test app for Issue #226
 *
 *  A simple API example of deleting a node, and attributes...
 *
 *  Note: This simple test app has only minimal error checking
 *
\*/

#include <stdio.h>
#include <tidy.h>
#include <tidybuffio.h>
#include <vector>

typedef struct tagNODEATT {
    TidyNode node;
    TidyAttr attr;
}NODEATT;

typedef std::vector<NODEATT> vNDATT;

static vNDATT vNodeAtts;

static const char *sample =
    "<!DOCTYPE html>\n"
    "<head>\n"
    "<meta charset=utf-8>\n"
    "<title>Test app for Issue #226</title>\n"
    "<body>\n"
    "<p class=\"nob\">Para 1</p>\n"
    "<table class=\"tcls\">\n"
    "<tr>\n"
    "<td class=\"txt\">rc: <b>R1C1</b></td>\n"
    "</tr>\n"
    "</table>\n"
    "</body>\n"
    "</html>";

/* Iterate over attribute values */
//TIDY_EXPORT TidyAttr TIDY_CALL    tidyAttrFirst( TidyNode tnod );
//TIDY_EXPORT TidyAttr TIDY_CALL    tidyAttrNext( TidyAttr tattr );
//TIDY_EXPORT ctmbstr TIDY_CALL     tidyAttrName( TidyAttr tattr );
//TIDY_EXPORT ctmbstr TIDY_CALL     tidyAttrValue( TidyAttr tattr );

static TidyNode bold_node = 0;
static TidyNode td_node = 0;
static TidyAttr attr_td = 0;

void dumpNode( TidyDoc tdoc, TidyNode tnod, int indent )
{
    TidyNode child;
    TidyAttr attr;
    bool istd = false;
    NODEATT na;
    for ( child = tidyGetChild(tnod); child; child = tidyGetNext(child) )
    {
        ctmbstr value, name = 0;
        TidyBuffer buff2;
        tidyBufInit(&buff2);
        istd = false;
        switch ( tidyNodeGetType(child) )
        {
        case TidyNode_Root:       name = "Root";                    break;
        case TidyNode_DocType:    name = "DOCTYPE";                 break;
        case TidyNode_Comment:    name = "Comment";                 break;
        case TidyNode_ProcIns:    name = "Processing Instruction";  break;
        case TidyNode_Text:
            name = "Text";
            if (tidyNodeGetText(tdoc, child, &buff2) && buff2.bp && buff2.size ) {
                // fwrite(buff2.bp, buff2.size, 1, stdout);
            }
            break;
        case TidyNode_CDATA:      name = "CDATA";                   break;
        case TidyNode_Section:    name = "XML Section";             break;
        case TidyNode_Asp:        name = "ASP";                     break;
        case TidyNode_Jste:       name = "JSTE";                    break;
        case TidyNode_Php:        name = "PHP";                     break;
        case TidyNode_XmlDecl:    name = "XML Declaration";         break;

        case TidyNode_Start:
        case TidyNode_End:
        case TidyNode_StartEnd:
        default:
            name = tidyNodeGetName( child );
            break;
        }
        if (name) {
            if (strcmp(name,"b") == 0) {
                bold_node = child;
            } else
            if (strcmp(name,"td") == 0) {
                istd = true;
            }
        }
        //assert( name != NULL );
        printf( "%*.*sNode: %s ", indent, indent, " ", (name ? name : "<NULL>") );
        if (buff2.bp && buff2.size) {
            printf("%s ",buff2.bp);
        }

        for (attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr)) {
            name = tidyAttrName(attr);
            value = tidyAttrValue(attr);
            printf("%s=\"%s\" ", (name ? name : "<blank>"), (value ? value : ""));
            if (name && istd && strcmp(name,"class") == 0) {
                td_node = child;
                attr_td = attr;
            }
            na.attr = attr;
            na.node = child;
            vNodeAtts.push_back(na);
        }
        printf("\n");

        tidyBufFree( &buff2 );

        dumpNode( tdoc, child, indent + 4 );
    }
}

#ifdef USE_TIDY5_API
void delete_all_body_attributes( TidyDoc tdoc )
{
    size_t ii, max = vNodeAtts.size();
    if (max)
        printf("Deletion of %d attributes...\n", (int) max );
    for (ii = 0; ii < max; ii++) {
        NODEATT na = vNodeAtts[ii];
        tidyAttrDiscard( tdoc, na.node, na.attr );
    }
}
#endif

/* test266 delete nodes and attributes */
int main(int argc, char **argv) 
{
    int iret = 0;
    int rc;
    TidyNode body = 0;
    TidyNode body_cont = 0;
    TidyNode node, pnode;
    TidyNodeType tnt;
    printf("\nSimple example of HTML Tidy API use.\n");
    if (argc > 1) {
        printf("All parameters are ignored!\n");
    }
    TidyDoc tdoc = tidyCreate();
    TidyBuffer buff, buff2;
    
    tidyBufInit(&buff);
    tidyBufInit(&buff2);
    tidyBufAppend(&buff, (void *)sample, strlen(sample));
    rc = tidyParseBuffer(tdoc, &buff);
    if (rc != 0) {
        iret = 1;
        printf("Impossible: Parsing canned html returned %d\n",rc);
        goto exit;
    }

    // get document BODY element
    body = tidyGetBody(tdoc);
    if (!body) {
        printf("Impossible! Body node not found!\n");
        iret = 1;
        goto exit;
    }

    body_cont = tidyGetChild(body);
    if (!body_cont) {
        printf("Impossible! Body node has no content!\n");
        iret = 1;
        goto exit;
    }

    // got first child of <body>, enumerate children
    rc = 0;
    node = body_cont;
    while (node) {
        rc++;
        tnt = tidyNodeGetType(node);
        node = tidyGetNext( node );         /* siblings */
    }

    printf("Dump of body nodes before deletion...\n");

    bold_node = 0;
    td_node = 0;
    attr_td = 0;
    dumpNode( tdoc, body, 0 );
    if (!bold_node) {
        printf("Failed to find 'b' node...\n");
        iret = 1;
        goto exit;
    }
    
#ifdef USE_TIDY5_API
    // TidyNode TIDY_CALL    tidyDiscardElement( TidyDoc tdoc, TidyNode tnod )
    printf("Deletion of 'bold' node and child text...\n");

    pnode = tidyDiscardElement( tdoc, bold_node );

    if (!td_node || !attr_td) {
        printf("Failed to find 'td' node, with class attribute...\n");
        iret = 1;
        goto exit;
    }

    // void TIDY_CALL tidyAttrDiscard( TidyDoc tdoc, TidyNode tnod, TidyAttr tattr )
    //tidyAttrDiscard( tdoc, td_node, attr_td );
    delete_all_body_attributes( tdoc );



    printf("Dump of body nodes after deletion...\n");
    dumpNode( tdoc, body, 0 );

    printf("Effectively a 'tidy-cleaner'... deleteing unwanted nodes, and attributes...\n");
#endif

    printf("Passed test...\n");

exit:
    // free buffer memory
    tidyBufFree( &buff );
    tidyBufFree( &buff2 );
    tidyRelease( tdoc ); /* called to free hash tables etc. */

    return iret;
}

// eof
