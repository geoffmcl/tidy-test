/*\
 * test-locale.c
 *
 * Copyright (c) 2018 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

static const char *module = "test-locale";

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
	char* result = NULL;
 
	result = setlocale(LC_ALL, ""); /* 'LANG'? */

	if (result != NULL) 
    {
		printf("setlocale=%s\n", result);
	}
    else
    {
		printf("setlocale returns NULL.\n");
        iret = 1;
	}
    
    return iret;
}


// eof = test-locale.cxx
