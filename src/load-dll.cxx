/*\
 * load-dll.cxx
 *
 * Copyright (c) 2017 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/
/*
    WIN32 ONLY - Example to DYNAMICALLY load a 'Tidy' DLL
    Use LoadLibrary to load user's DLL
    Use GetProcAddress to get "tidyLibraryVersion" and "tidyReleaseDate"
    and show results....
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <Windows.h>
#include <stdio.h>
#include <string.h> // for strdup(), ...
#include <time.h> // for ctime
#include <tidy.h>
#include "sprtf.h"

// other includes

static const char *module = "load-dll";

static const char *usr_input = 0;

//TIDY_EXPORT ctmbstr TIDY_CALL  tidyReleaseDate(void);
//TIDY_EXPORT ctmbstr TIDY_CALL  tidyLibraryVersion(void);
typedef ctmbstr (TIDY_CALL *LibVersion)(void);
static const char *procVers = "tidyLibraryVersion";
static const char *procDate = "tidyReleaseDate";

void give_help( char *name )
{
    printf("%s: usage: [options] usr_input\n", module);
    printf("Options:\n");
    printf(" --help  (-h or -?) = This help and exit(0)\n");
    printf("\n");
    printf(" WIN32 ONLY - Example to DYNAMICALLY load a 'Tidy' DLL\n");
    printf(" Use LoadLibrary to load user's DLL\n");
    printf(" Use GetProcAddress to get \"tidyLibraryVersion\" and \"tidyReleaseDate\"\n");
    printf(" and show results of calling these 2 API's\n");
}

/////////////////////////////////////////////////////////
//// UTILITIES
#ifdef _MSC_VER
#define M_IS_DIR _S_IFDIR
#else // !_MSC_VER
#define M_IS_DIR S_IFDIR
#endif

static struct stat buf;
enum DiskType {
    MDT_NONE,
    MDT_FILE,
    MDT_DIR
};

static DiskType is_file_or_directory(const char * path)
{
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
static time_t get_last_file_time() { return buf.st_mtime; }

#define P64 "%I64u"
#define nnf_NoClean 0x01

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : InStr
// Return type: INT 
// Arguments  : LPTSTR lpb
//            : LPTSTR lps
// Description: Return the position of the FIRST instance of the string in lps
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
int InStr(char *lpb, char *lps)
{
    int   iRet = 0;
    int   i, j, k, l, m;
    char  c;
    i = (int)strlen(lpb);
    j = (int)strlen(lps);
    if (i && j && (i >= j))
    {
        c = *lps;   // get the first we are looking for
        l = i - (j - 1);   // get the maximum length to search
        for (k = 0; k < l; k++)
        {
            if (lpb[k] == c)
            {
                // found the FIRST char so check until end of compare string
                for (m = 1; m < j; m++)
                {
                    if (lpb[k + m] != lps[m])   // on first NOT equal
                        break;   // out of here
                }
                if (m == j)   // if we reached the end of the search string
                {
                    iRet = k + 1;  // return NUMERIC position (that is LOGICAL + 1)
                    break;   // and out of the outer search loop
                }
            }
        }  // for the search length
    }
    return iRet;
}

// return nice number - with comas - get_nn
char *get_NiceNumber(char *lpn, int flag)
{
    size_t i, j, k;
    char *lpr = GetNxtBuf();
    *lpr = 0;
    i = strlen(lpn);
    k = 0;
    if (i) {
        if (i > 3) {
            for (j = 0; j < i; j++) {
                // FIX20070923 - avoid adding FIRST comma
                if (k && (((i - j) % 3) == 0))
                    lpr[k++] = ',';
                lpr[k++] = lpn[j];
            }
            lpr[k] = 0;
        }
        else {
            strcpy(lpr, lpn);
        }
    }
    if (k && !(flag & nnf_NoClean)) {
        i = InStr(lpr, (char *)".");
        if (i) {
            while (k--) {
                if (lpr[k] == '.') {
                    lpr[k] = 0;
                    break;
                }
                else if (lpr[k] == '0') {
                    if (k > 0)
                        lpr[k] = 0;
                }
                else
                    break;
            }
        }
    }

    return lpr;
}


char *get_NiceNumberStg64(unsigned long long num, int flag)   // get_nn()
{
    char * lpr = GetNxtBuf();
    sprintf(lpr, P64, num);
    return (get_NiceNumber(lpr,flag));
}

char *get_file_date(time_t t)   // get_file_time - uses strftime(), fall back ctime()
{
    char *cp = GetNxtBuf();
    struct tm *ltp = localtime(&t); // epoch time to tm structure
    if (ltp) {  // succeeded
        //  format                      YEAR-MM-DD HH:MM
        size_t len = strftime(cp, 1024, "%Y-%m-%d %I:%M", ltp);
        if (len)
            return cp;
    }
    // above failed, fall back to 'ctime'....
    char *tp = ctime(&t);
    if (tp) {
        strcpy(cp, tp);
        size_t len = strlen(cp);
        while (len) {
            len--;
            if (cp[len] > ' ')
                break;
            cp[len] = 0;
        }
    }
    else {
        strcpy(cp, "Invalid Time");
    }
    return cp;
}

///////////////////////////////////////////////////////////
DWORD GetLastErrorMsg(LPTSTR lpm, DWORD dwLen, DWORD dwErr, ...)
{
    PVOID lpMsgBuf = 0;
    DWORD    dwr;
    va_list a_list;

    va_start(a_list, dwErr);
    // FORMAT_MESSAGE_IGNORE_INSERTS
    dwr = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dwErr,   //	GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&lpMsgBuf,
        0,
        &a_list);

    //dwr = strlen(lpMsgBuf);
    if (!lpMsgBuf || (dwr == 0) || (dwr >= dwLen))
        dwr = 0;
    else
        strcpy(lpm, (LPTSTR)lpMsgBuf);

    //	printf("%s:%s\n",lpm,(LPCTSTR)lpMsgBuf);
    // Free the buffer.
    if (lpMsgBuf)
        LocalFree(lpMsgBuf);

    return dwr;
}

///////////////////////////////////////////////////////////
static DiskType dt;

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
    dt = is_file_or_directory(usr_input);
    if (dt == MDT_FILE) {
        printf("%s: User input file\n%s %s %s\n", module, usr_input, get_NiceNumberStg64(get_last_file_size(),0),
            get_file_date(get_last_file_time()));
    }
    else if (dt == MDT_DIR) {
        printf("%s: Error: User input '%s' is a directory!\n", module, usr_input);
        return 1;
    }
    else {
        printf("%s: Error: Unable to 'stat' user input '%s'!\n", module, usr_input);
        return 1;
    }
    return 0;
}

static int load_dll(const char *file) // actions of app
{
    int iret = 0;
    HMODULE h = LoadLibrary(file);
    ctmbstr v;
    if (!h) {
        DWORD dwRet, dwErr = GetLastError();
        char *cp = GetNxtBuf();
        *cp = 0;
        printf("%s: Error: Unable to 'LoadLibrary(%s)'! Error %d\n", module, file, (int)dwErr);
        dwRet = GetLastErrorMsg(cp, 1024, dwErr, file);
        if (dwRet)  // success
            printf(" Error: %s\n", cp);
        return 1;
    }
    LibVersion ver = (LibVersion)GetProcAddress(h, procVers);   // "tidyLibraryVersion"
    if (ver) {
        v = ver();
        if (v) {
            printf(" Proc: %s = %s\n", procVers, v);
        }
        else {
            printf(" Proc: %s = %s\n", procVers, "FAILED");
            iret |= 1;
        }
    }
    else {
        printf("%s: Failed `GetProcAddress(h,%s)'!\n", module, procVers);
        iret |= 2;
    }
    ver = (LibVersion)GetProcAddress(h, procDate);   // "tidyReleaseDate"
    if (ver) {
        v = ver();
        if (v) {
            printf(" Proc: %s = %s\n", procDate, v);
        }
        else {
            printf(" Proc: %s = %s\n", procDate, "FAILED");
            iret |= 4;
        }
    }
    else {
        printf("%s: Failed `GetProcAddress(h,%s)'!\n", module, procDate);
        iret |= 8;
    }

    FreeLibrary(h);
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
    UINT em = GetErrorMode();
    SetErrorMode(em | SEM_FAILCRITICALERRORS);
    iret = load_dll(usr_input); // actions of app
    SetErrorMode(em);
    return iret;
}


// eof = load-dll.cxx
