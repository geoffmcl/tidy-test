/*\
 * issue-452.cxx
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <stdio.h>
#include <iostream>
#include <tidy.h>
#include <tidybuffio.h>

using namespace std;

static const char *module = "issue-452";

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    TidyDoc tidyDoc = tidyCreate();
    auto showErrors = tidyOptGetValue(tidyDoc, TidyShowErrors);

    if (showErrors!=nullptr) {
        cout << showErrors << endl;
    }
    tidyRelease(tidyDoc);

    return iret;
}


// eof = issue-452.cxx
