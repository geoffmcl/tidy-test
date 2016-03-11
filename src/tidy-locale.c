/*\
 * tidy-locale.c
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <stdio.h>
#include <string.h> // for strdup(), ...
#include <tidy.h>
#include "language.h"
#include "locale.h"
#if defined(_WIN32)
#include <windows.h> /* Force console to UTF8. */
#endif

static const char *module = "tidy-locale";

static const char *usr_input = 0;

void give_help( char *name )
{
    printf("\n");
    printf("%s: usage: [options] [language]\n", module);
    printf("Options:\n");
    printf(" --help  (-h or -?) = This help and exit(0)\n");
    printf("\n");
    printf(" Show what language tidy selects by default, and\n");
    printf(" if a language is given on the command line, show\n" );
    printf(" what tidy would select given that language string.\n" );
    printf("\n");
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
    //if (!usr_input) {
    //    printf("%s: No user input found in command!\n", module);
    //    return 1;
    //}
    return 0;
}

#if defined(_WIN32)
static uint win_cp = 0; /* original Windows code page */
#endif

/**
 **  Handles exit cleanup.
 */
static void tidy_cleanup(void)
{
#if defined(_WIN32)
    /* Restore original Windows code page. */
    if (win_cp) {
        printf("%s: at exit - tidy_cleanup doing SetConsoleOutputCP(%u);!\n", module, win_cp);
        SetConsoleOutputCP(win_cp);
    }
    win_cp = 0;
#endif
}

/**
 **  Handles the -xml-strings service.
 **  This service was primarily helpful to developers and localizers to
 **  compare localized strings to the built in `en` strings. It's probably
 **  better to use our POT/PO workflow with your favorite tools, or simply
 **  diff the language header files directly.
 **  **Important:** The attribute `id` is not a specification, promise, or
 **  part of an API. You must not depend on this value.
 */
static void xml_strings( void )
{
    uint i;
    TidyIterator j;

    ctmbstr current_language = tidyGetLanguage();
    Bool skip_current = strcmp( current_language, "en" ) == 0;
    Bool matches_base;

    printf( "<?xml version=\"1.0\"?>\n"
           "<localized_strings version=\"%s\">\n", tidyLibraryVersion());

    j = getStringKeyList();
    while (j) {
        i = getNextStringKey(&j);
        printf( "<localized_string id=\"%u\">\n", i );
        printf( " <string class=\"%s\">", "en" );
        printf("%s", tidyDefaultString(i));
        printf( "</string>\n" );
        if ( !skip_current ) {
            matches_base = strcmp( tidyLocalizedString(i), tidyDefaultString(i) ) == 0;
            printf( " <string class=\"%s\" same_as_base=\"%s\">", tidyGetLanguage(), matches_base ? "yes" : "no" );
            printf("%s", tidyLocalizedString(i));
            printf( "</string>\n" );
        }
        printf( "</localized_string>\n");
    }

    printf( "</localized_strings>\n" );
}



static int show_locale()
{
    tmbstr locale = NULL;
    ctmbstr lang;

    /* Set an atexit handler. */
    atexit( tidy_cleanup );
    
    /* Set the locale for tidy's output. */
    locale = tidySystemLocale(locale);
    tidySetLanguage(locale);
    if ( locale ) {
        printf("%s: tidySystemLocale returned '%s'\n", module, locale);
        free( locale );
    } else {
        printf("%s: tidySystemLocale returned 'NULL'\n", module);
    }
    /* Gets the current language used by Tidy. */
    lang = tidyGetLanguage();
    if (lang) {
        printf("%s: tidyGetLanguage returned '%s'\n", module, lang);
    } else {
        printf("%s: tidyGetLanguage returned 'NULL'\n", module);
    }

    if (usr_input) {
        printf("%s: Doing tidySetLanguage(%s)\n", module, usr_input);
        tidySetLanguage(usr_input);
        lang = tidyGetLanguage();
        if (lang) {
            printf("%s: tidyGetLanguage returned '%s'\n", module, lang);
        } else {
            printf("%s: tidyGetLanguage returned 'NULL'\n", module);
        }

    }

    //xml_strings();  // what does this do??? show strings?

#if defined(_WIN32)
    /* Force Windows console to use UTF, otherwise many characters will
     * be garbage. Note that East Asian languages *are* supported, but
     * only when Windows OS locale (not console only!) is set to an
     * East Asian language.
     */
    win_cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif

    return 0;
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
    iret = show_locale(); // actions of app

    return iret;
}


// eof = tidy-locale.c
