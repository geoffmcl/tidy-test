/*\ 
 *  20150611 - 20150206 - Test app for Issue #71
 *
 *  A simple API example of getting the body text, first as html,
 *  and then as a raw text stream.
 *
 *  Note: This simple test app has only minimal error checking
 *  20150611 - Add this simple API use to tidy-test
 *
\*/

#include <stdio.h>
#include <tidy.h>
#include <tidybuffio.h>

static const char *sample =
    "<!DOCTYPE html>\n"
    "<head>\n"
    "<meta charset=utf-8>\n"
    "<title>Test app for Issue #71</title>\n"
    "<body>something &amp; escaped</body>\n"
    "</html>";

int main(int argc, char **argv) 
{
    int rc;
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
        printf("Impossible: Parsing canned html returned %d\n",rc);
    }
    TidyNode body = tidyGetBody(tdoc);
    if (!body) {
        printf("Impossible! Body node not found!\n");
        goto exit;
    }

    TidyNode text_node = tidyGetChild(body);
    if (!text_node) {
        printf("Impossible! Body node has no content!\n");
        goto exit;
    }

    printf("This is the 'escaped' text, from tidyNodeGetText(...), suitable for html use...\n");
    if (tidyNodeGetText(tdoc, text_node, &buff2) &&
        buff2.bp && buff2.size ) {
        fwrite(buff2.bp, buff2.size, 1, stdout);
    } else {
        printf("Impossible! Text node is empty!\n");
    }
    printf("This is the 'raw' lexer values, from tidyNodeGetValue(...).\n");
    if (tidyNodeGetValue(tdoc, text_node, &buff2) &&
        buff2.bp && buff2.size ) {
        fwrite(buff2.bp, buff2.size, 1, stdout);
        printf("\n");
    } else {
        printf("Impossible! Text node has no value\n");
    }

exit:
    // free buffer memory
    tidyBufFree( &buff );
    tidyBufFree( &buff2 );
    tidyRelease( tdoc ); /* called to free hash tables etc. */

    return 0;
}

// eof
