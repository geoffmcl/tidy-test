/*\
 * tidy-by-buf.c
 *
 * Copyright (c) 2018 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

/*\
 * # 20190204 Is. #788 maybe exposed a case where output to a TidyBuffer
 * # could have an overrun... but not yet proved. This uses the 
 * # same technique as oss-fuzz projet tidy-html5/tidy_fuzzer.c
 * # to try to repeat the possible bug...
 * test: C:\Users\user\Documents\Tidy\temp-788\clusterfuzz-testcase-minimized-tidy_fuzzer-5639351547985920.nt
\*/

#include <sys/types.h>
#include <sys/stat.h>
//#include <stdlib.h>
#include <stdio.h>
#include <string.h> // for strdup(), ...
#include <stdint.h> // for uint8_t, ...

#include <tidy.h>
#include <tidybuffio.h>


#ifndef SPRTF
#define SPRTF printf
#endif

static const char *module = "tidy-by-buf";

static const char *usr_input = 0;
static FILE* errout = NULL;  // stderr;  /* initialize to stderr */
static const char *err_fil = NULL;
static FILE* htmout = NULL;  // stdout if NULL */
static const char *htm_fil = NULL;
static int verbosity = 0;

#define VERB1 (verbosity >= 1)
#define VERB2 (verbosity >= 2)
#define VERB5 (verbosity >= 5)
#define VERB9 (verbosity >= 9)

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
    show_version();
    printf("%s: usage: [options] usr_input\n", module);
    printf("Options:\n");
    printf(" --help  (-h or -?) = This this help, and exit(0)\n");
    printf(" --version          = Show version, and exit(0)\n");
    printf(" --Verb[n]     (-V) = Bump, or set verbosity. 0,1,2,5,9 (def=%d)\n", verbosity);
    printf(" --out <file>  (-o) = Output html to this file. (def=%s)\n", 
        (htm_fil ? htm_fil : "none"));
    printf(" --file <file> (-f) = Output non-html to this file. (def=%s)\n",
        (err_fil ? err_fil : "none"));
    // TODO: More help
}

/* utility functions */
///////////////////////////////////////////////////////////////
#ifdef _WIN32
#define M_IS_DIR _S_IFDIR
#else // !_WIN32
#define M_IS_DIR S_IFDIR
#endif

#define MDT_NONE    0
#define MDT_FILE    1
#define MDT_DIR     2

static struct stat buf;
static int is_file_or_directory(const char *path)
{
    memset(&buf, 0, sizeof(buf));
    if (!path)
        return MDT_NONE;
    if (stat(path, &buf) == 0)
    {
        if (buf.st_mode & M_IS_DIR)
            return MDT_DIR;
        else
            return MDT_FILE;
    }
    return MDT_NONE;
}
static size_t get_last_file_size() { return buf.st_size; }
///////////////////////////////////////////////////////////////


/* ===================================================== */
int run_tidy_parser(TidyBuffer* data_buffer,
    TidyBuffer* output_buffer,
    TidyBuffer* error_buffer) 
{
    int rc = -1;
    TidyDoc tdoc = tidyCreate();
    if (!tdoc) {
        printf("%s: Failed tidyCreate()\n", module);
        abort();
    }

    rc = tidySetErrorBuffer(tdoc, error_buffer); /* // Capture diagnostics */
    if (rc < 0) {
        printf("%s: Failed tidySetErrorBuffer()\n", module);
        abort();
    }

    /* set options */
    tidyOptSetBool(tdoc, TidyXhtmlOut, yes);
    tidyOptSetBool(tdoc, TidyForceOutput, yes);

    rc = tidyParseBuffer(tdoc, data_buffer);    /* // Parse the input, from a TidyBuffer */
    if (rc >= 0)
        rc = tidyCleanAndRepair(tdoc);          /* // Tidy it up! */
    if (rc >= 0)
        rc = tidyRunDiagnostics(tdoc);          /* // Kvetch */
    if (rc >= 0) 
        rc = tidySaveBuffer(tdoc, output_buffer); /* // Pretty Print, to a TidyBuffer */


    tidyRelease(tdoc);

    return rc;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) 
{
    int rc = -1;
    size_t fw;
    TidyBuffer data_buffer;
    TidyBuffer output_buffer;
    TidyBuffer error_buffer;
    tidyBufInit(&data_buffer);
    tidyBufInit(&output_buffer);
    tidyBufInit(&error_buffer);

    tidyBufAttach(&data_buffer, (byte*)data, (uint)size);
    rc = run_tidy_parser(&data_buffer, &output_buffer, &error_buffer);

    /* don't spray noise, unless requested */
    if (VERB5 || err_fil)
    {
        if (error_buffer.bp && (error_buffer.size > 0))
        {
            int isfile = 0;
            if (err_fil)
            {
                errout = fopen(err_fil, "wb");
                if (errout)
                    isfile = 1;
            }
            if (!errout)
                errout = stderr;
            fw = fwrite(error_buffer.bp, error_buffer.size, 1, errout);
            if (isfile) {
                fclose(errout);
                if (VERB1) {
                    printf("%s: error output, %u, written to '%s' %s\n", module,
                        error_buffer.size, err_fil,
                        (error_buffer.size == (uint)fw) ? "ok" : "FAILED");
                }
            }
        }
    }
    if (VERB5 || htm_fil)
    {
        if (output_buffer.bp && output_buffer.size > 0)
        {
            int isfilo = 0;
            if (htm_fil) {
                htmout = fopen(htm_fil, "wb");
                if (htmout)
                    isfilo = 1;
            }
            if (!htmout)
                htmout = stdout;
            fw = fwrite(output_buffer.bp, output_buffer.size, 1, htmout);
            if (isfilo) {
                fclose(htmout);
                if (VERB1) {
                    printf("%s: html output, %u, written to '%s' %s\n", module,
                        output_buffer.size, htm_fil,
                        (output_buffer.size == (uint)fw) ? "ok" : "FAILED");
                }
            }
        }
    }
    /* CLEAN UP buffers */
    tidyBufFree(&error_buffer);
    tidyBufFree(&output_buffer);
    tidyBufDetach(&data_buffer);
    return rc;
}

/* ===================================================== */
#define ISDIGIT(a) (( a >= '0' ) && ( a <= '9' ))

int parse_args( int argc, char **argv )
{
    int i,i2,c;
    char *arg, *sarg;
    for (i = 1; i < argc; i++) {
        arg = argv[i];
        i2 = i + 1;
        if (strcmp(arg, "--version") == 0) {
            show_version();
            return 2;
        }
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
            case 'V':
                verbosity++;
                sarg++;
                while (*sarg) {
                    if (ISDIGIT(*sarg)) {
                        verbosity = atoi(sarg);
                        break;
                    }
                    else if (*sarg == 'V') {
                        verbosity++;
                    }
                }
                break;
            case 'f':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    err_fil = strdup(sarg);
                }
                else {
                    printf("%s: Expected file name to follow '%s'!\n", module, arg);
                    return 1;
                }
                break;
            case 'o':
                if (i2 < argc) {
                    i++;
                    sarg = argv[i];
                    htm_fil = strdup(sarg);
                }
                else {
                    printf("%s: Expected file name to follow '%s'!\n", module, arg);
                    return 1;
                }
                break;
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

int tidy_by_buffer()
{
    int iret = -1;
    size_t rd, len = 0;
    uint8_t *data;
    FILE *fp = 0;
    int res = is_file_or_directory(usr_input);
    if (res != MDT_FILE) {
        printf("%s: Can not 'stat' input '%s'\n", module, usr_input);
        return -1;
    }
    len = get_last_file_size();
    if (len <= 0) {
        printf("%s: input '%s' length problem...\n", module, usr_input);
        return -1;
    }
    data = (uint8_t *)malloc(len);
    if (!data) {
        printf("%s: memory failed %d!\n", module, (int)len);
        return -1;
    }
    fp = fopen(usr_input, "rb");
    if (!fp) {
        printf("%s: Can not 'open' input '%s'\n", module, usr_input);
        free(data);
        return -1;
    }
    rd = fread(data, 1, len, fp);
    fclose(fp);
    if (rd != len) {
        printf("%s: Can not 'read' input '%s'\n", module, usr_input);
        free(data);
        return -1;
    }

    /* ****************************************** */
    iret = LLVMFuzzerTestOneInput(data, len);
    /* ****************************************** */

    free(data);

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

    iret = tidy_by_buffer(); // actions of app

    /* clean up potential 'strdup' allocations */
    if (usr_input)
        free((char *)usr_input);
    if (htm_fil)
        free((char *)htm_fil);
    if (err_fil)
        free((char *)err_fil);

    return iret;
}


// eof = tidy-by-buf.c
