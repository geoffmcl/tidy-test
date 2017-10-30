/*
 * sample2.c
 * Relatively simple example use of LibTidy
 * Only a small number of options supported
 *
 */
#include <stdio.h>
#include <errno.h>
#include <tidy.h>
#include <tidybuffio.h>

#ifndef SPRTF
#define SPRTF printf
#endif

static const char* input = 0;
static ctmbstr errfil = NULL;
static FILE *errout = NULL;
static const char *outfile = 0;
static const char *msgfile = 0;
/**
**  Handles the -version service.
*/
static void version(void)
{
    SPRTF("HTML Tidy library date %s, version %s\n", tidyReleaseDate(), tidyLibraryVersion());
}


static void give_help(char *name)
{   
    version();
    SPRTF("Sample2 Usage:\n");
    SPRTF("%s [options] html_input\n", name);
    SPRTF("Options:\n");
    SPRTF(" --help   (-h or -?) = This help and exit(0)\n");
    SPRTF(" --version      (-v) = Show library version, and exit(0)\n");
    SPRTF(" --<option> <value>  = Will be treated as a libTidy option and value\n");
    SPRTF(" --lang <locale>     = This special option set the language tidy will\n");
    SPRTF("                       use for messages. '--lang help' will show the list.\n");
    SPRTF(" -config <file> (-c) = Pass a config file to libTidy\n");
    SPRTF(" -out <file>    (-o) = Write tidy HTML to this file.\n");
    SPRTF(" -msg <file>    (-m) = Write tidy messages to this file.\n");
}
/**
**  Indicates whether or not two filenames are the same.
**  Of course it does *not* always give the exact results,
**  like say ./outfile.html and outfile.html could be the same!
*/
static Bool samefile(ctmbstr filename1, ctmbstr filename2)
{
#if FILENAMES_CASE_SENSITIVE
    return (strcmp(filename1, filename2) == 0);
#else
    return (strcasecmp(filename1, filename2) == 0);
#endif
}

/*
* Look for default configuration files using any of
* the following possibilities:
*  - TIDY_CONFIG_FILE - from tidyplatform.h, typically /etc/tidy.conf
*  - HTML_TIDY        - environment variable
*  - TIDY_USER_CONFIG_FILE - from tidyplatform.h, typically ~/tidy.conf
*/

int ProcessDefConfig(TidyDoc tdoc)
{
    int status = 0;
    static ctmbstr cfgfil = NULL;
#ifdef TIDY_CONFIG_FILE
    if (tidyFileExists(tdoc, TIDY_CONFIG_FILE))
    {
        status = tidyLoadConfig(tdoc, TIDY_CONFIG_FILE);
        if (status != 0) {
            SPRTF("Error: Failed to process TIDY_CONFIG_FILE '%s' define!\n", TIDY_CONFIG_FILE);
            return 1;
        }
    }
    else {
        SPRTF("Error: Compiler defined TIDY_CONFIG_FILE '%s' does not exits!\n", TIDY_CONFIG_FILE);
        return 1;
    }
#endif /* TIDY_CONFIG_FILE */

    if ((cfgfil = getenv("HTML_TIDY")) != NULL)
    {
        status = tidyLoadConfig(tdoc, cfgfil);
        if (status != 0) {
            SPRTF("Error: Failed to load environment HTML_TIDY=%s!\n", cfgfil);
            return 1;
        }
    }
#ifdef TIDY_USER_CONFIG_FILE
    if (tidyFileExists(tdoc, TIDY_USER_CONFIG_FILE))
    {
        status = tidyLoadConfig(tdoc, TIDY_USER_CONFIG_FILE);
        if (status != 0) {
            SPRTF("Error: Failed to process TIDY_USER_CONFIG_FILE '%s' define!\n", TIDY_USER_CONFIG_FILE);
            return 1;
        }
    }
    else {
        SPRTF("Error: Compiler defined TIDY_USER_CONFIG_FILE '%s' does not exits!\n", TIDY_USER_CONFIG_FILE);
        return 1;
    }
#endif /* TIDY_USER_CONFIG_FILE */
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// 20171030 - additions
/** Prints the Windows language names that Tidy recognizes, using the specified
** format string.
** @param format A format string used to display the Windows language names,
**        or NULL to use the built-in default format.
*/
void tidyPrintWindowsLanguageNames(ctmbstr format)
{
    const tidyLocaleMapItem *item;
    TidyIterator i = getWindowsLanguageList();
    ctmbstr winName;
    ctmbstr posixName;

    while (i) {
        item = getNextWindowsLanguage(&i);
        winName = TidyLangWindowsName(item);
        posixName = TidyLangPosixName(item);
        if (format)
            SPRTF(format, winName, posixName);
        else
            SPRTF("%-20s -> %s\n", winName, posixName);
    }
}

/** Prints the languages the are currently built into Tidy, using the specified
** format string.
** @param format A format string used to display the Windows language names,
**        or NULL to use the built-in default format.
*/
void tidyPrintTidyLanguageNames(ctmbstr format)
{
    ctmbstr item;
    TidyIterator i = getInstalledLanguageList();

    while (i) {
        item = getNextInstalledLanguage(&i);
        if (format)
            SPRTF(format, item);
        else
            SPRTF("%s\n", item);
    }
}


/** Handles the -lang help service.
** @remark We will not support console word wrapping for the tables. If users
**         really have a small console, then they should make it wider or
**         output to a file.
** @param tdoc The Tidy document.
*/
static void lang_help(TidyDoc tdoc)
{
    SPRTF("\n");
    SPRTF("%s", tidyLocalizedString(TC_TXT_HELP_LANG_1));
    SPRTF("\n");
    tidyPrintWindowsLanguageNames("  %-20s -> %s\n");
    SPRTF("\n");
    SPRTF("%s", tidyLocalizedString(TC_TXT_HELP_LANG_2));
    SPRTF("\n");
    tidyPrintTidyLanguageNames("  %s\n");
    SPRTF("\n");
    SPRTF(tidyLocalizedString(TC_TXT_HELP_LANG_3), tidyGetLanguage());
    SPRTF("\n");
}
////////////////////////////////////////////////////////////////////////////

static int ParseArgs(TidyDoc tdoc, int argc, char **argv)
{
    int c, i, i2;
    char *arg, *sarg;
    ctmbstr post;

    if (ProcessDefConfig(tdoc))
        return 1;

    for (i = 1; i < argc; i++) {
        i2 = i + 1;
        arg = argv[i];
        c = *arg;
        if (c == '-') {
            sarg = &arg[1];
            c = *sarg;
            if (c == '-') {
                sarg++;
                if (strcmp(sarg, "help") == 0) {
                    give_help(argv[0]);
                    return 2;
                }
                else if (strcmp(sarg, "version") == 0) {
                    version();
                    return 2;
                }
                else {
                    /* treat ALL other '--' as configuration arguments */
                    if (i2 < argc) {
                        i++;
                        /* EXCEPT --language xx */
                        if (strcmp(sarg, "language") == 0 ||
                            strcmp(sarg, "lang") == 0) {
                            if (strcmp(argv[i], "help") == 0) {
                                lang_help(tdoc);
                                return 2;
                                // exit(0);
                            }
                            if (!tidySetLanguage(argv[i]))
                            {
                                SPRTF(tidyLocalizedString(TC_STRING_LANG_NOT_FOUND),
                                    argv[i], tidyGetLanguage());
                                SPRTF("\n");
                            }
                            else {
                                SPRTF(tidyLocalizedString(TC_TXT_HELP_LANG_3), tidyGetLanguage());
                                SPRTF("\n");
                            }
                        }
                        else {

                            if (tidyOptParseValue(tdoc, sarg, argv[i])) {
                                /* config value accepted */
                                /* Set new error output stream if setting changed */
                                post = tidyOptGetValue(tdoc, TidyErrFile);
                                if (post && (!errfil || !samefile(errfil, post)))
                                {
                                    errfil = post;
                                    errout = tidySetErrorFile(tdoc, post);
                                }
                            }
                            else {
                                SPRTF("Error: Configuration option '%s %s' failed!\n", arg, argv[i]);
                                return 1;
                            }
                        }
                    }
                    else {
                        SPRTF("Error: Expect config value following '%s'\n", arg);
                        return 1;
                    }
                }

            }
            else {
                /* have a single '-' option */
                switch (c)
                {
                case 'h':
                case '?':
                    give_help(argv[0]);
                    return 2;
                case 'c':
                    /* assume '-config <file> */
                    if (i2 < argc) {
                        i++;
                        if (tidyLoadConfig(tdoc, argv[i])) {
                            SPRTF("Error: Tidy failed to load config file '%s'!\n", argv[i]);
                            return 1;
                        }
                        else {
                            /* Set new error output stream if setting changed */
                            post = tidyOptGetValue(tdoc, TidyErrFile);
                            if (post && (!errfil || !samefile(errfil, post)))
                            {
                                errfil = post;
                                errout = tidySetErrorFile(tdoc, post);
                            }

                        }
                    }
                    else {
                        SPRTF("Error: Expect config <file> following '%s'\n", arg);
                        return 1;
                    }
                    break;
                case 'v':
                    version();
                    return 2;
                case 'o':
                    /* assume '-out <file> */
                    if (i2 < argc) {
                        i++;
                        outfile = strdup(argv[i]);
                    }
                    else {
                        SPRTF("Error: Expected html output file name after %s\n", arg);
                        return 1;
                    }
                    break;
                case 'm':
                    /* assume '-msg <file> */
                    if (i2 < argc) {
                        i++;
                        msgfile = strdup(argv[i]);
                    }
                    else {
                        SPRTF("Error: Expected message output file name after %s\n", arg);
                        return 1;
                    }
                    break;
                default:
                    SPRTF("Error: Unknown option '%s'! Try -?\n", arg);
                    return 2;
                }
            }
        }
        else {
            if (input) {
                SPRTF("Error: Already have input '%s'. What is this '%s'\n", input, arg);
                return 1;
            }
            input = strdup(arg);
        }
    }
    if (!input) {
        give_help(argv[0]);
        SPRTF("Error: No input file found in the command!\n");
        return 1;
    }
    return 0;
}

/* main OS entry */
int main(int argc, char **argv )
{
    TidyBuffer output = {0};
    TidyBuffer errbuf = {0};
    int rc = -1;
    Bool ok = yes;
    ctmbstr htmlfil = NULL;

    /* init state */
    TidyDoc tdoc = tidyCreate(); // Initialize "document"
    tidyBufInit(&output);
    tidyBufInit(&errbuf);

    rc = ParseArgs(tdoc, argc, argv);
    if (rc) {
        if (rc == 2)
            rc = 0;
        goto Exit;
    }

    SPRTF( "Tidying:\t%s\n", input );

    if ( ok )
        rc = tidySetErrorBuffer( tdoc, &errbuf );      // Capture diagnostics
    if (rc >= 0) {
        htmlfil = input;
        if (tidyOptGetBool(tdoc, TidyEmacs))
            tidySetEmacsFile(tdoc, htmlfil);
        rc = tidyParseFile(tdoc, htmlfil);          // Parse the input file
    }
    if ( rc >= 0 )
        rc = tidyCleanAndRepair( tdoc );               // Tidy it up!
    if ( rc >= 0 )
        rc = tidyRunDiagnostics( tdoc );               // Kvetch
    if ( rc > 1 )                                    // If error, force output.
        rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );
    if ( rc >= 0 )
        rc = tidySaveBuffer( tdoc, &output );          // Pretty Print result to output buffer
    if (rc < 0)
        SPRTF("A severe error (%d) occurred.\n", rc);

    SPRTF("Add information summary:(%u)\n", errbuf.size);
    tidyErrorSummary(tdoc);
    SPRTF("Add general information:(%u)\n", errbuf.size);
    tidyGeneralInfo(tdoc);
    if (msgfile && errbuf.size) {
        FILE *msg = fopen(msgfile, "w");
        if (msg) {
            size_t len = fwrite(errbuf.bp, 1, errbuf.size, msg);
            fclose(msg);
            if (len == errbuf.size) {
                SPRTF("\nMessage results:(%u) written to %s\n", errbuf.size, msgfile);
            }
            else {
                SPRTF("\nFailed to write messages:%u to %s!\n", errbuf.size, msgfile);
                rc |= 1;
            }
        }
        else {
            SPRTF("\nFailed to create message:%u file %s!\n", errbuf.size, msgfile);
            rc |= 1;
        }

    }
    else {
        SPRTF("Diagnostics errors:(%u)\n\n%s", errbuf.size, errbuf.bp);
    }
    if (outfile && output.size) {
        FILE *out = fopen(outfile, "w");
        if (out) {
            size_t len = fwrite(output.bp, 1, output.size, out);
            fclose(out);
            if (len == output.size) {
                SPRTF("\nResults results:(%u) written to %s\n", output.size, outfile);
            }
            else {
                SPRTF("\nFailed to write results:%u to %s!\n", output.size, outfile);
                rc |= 1;
            }
        }
        else {
            SPRTF("\nFailed to create results:%u file %s!\n", output.size, outfile);
            rc |= 1;
        }
    }
    else {
        SPRTF("\nAnd here are the result:(%u)\n\n%s", output.size, output.bp);
    }

Exit:
    tidyBufFree( &output );
    tidyBufFree( &errbuf );
    tidyRelease( tdoc );
    return rc;
}

/* eof */
