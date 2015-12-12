/*\
 * test-tidy.cxx
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

/*\
 * An idea to run the tidy test automatically, and 
 * show results.
 *
\*/
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#include <WinSock2.h>   // which include Windows.h
#endif
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

static const char *module = "test-tidy";


typedef struct tagELIST {
    int flag;
    std::string test;
    int used;
    int ec;
    std::string html;
    std::string config;
    std::string basemsg;
    std::string baseout;
}ELIST, *PELIST;

typedef std::vector<std::string> vSTG;
typedef std::vector<ELIST> vELIST;

#ifdef WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

#ifndef TIDY_TEST_ROOT
#ifdef WIN32
#define TIDY_TEST_ROOT "F:\\Projects\\tidy-html5\\test"
#else
#define TIDY_TEST_ROOT "/home/geoff/projects/html_tidy/tidy-html5/test"
#endif
#endif // !TIDY_TEST_ROOT


static const char *usr_input = 0;
static const char *root = TIDY_TEST_ROOT;
static const char *input = TIDY_TEST_ROOT PATH_SEP "input";
#ifdef WIN32
static const char *output = TIDY_TEST_ROOT PATH_SEP "temp-5";
#else
static const char *output = TIDY_TEST_ROOT PATH_SEP "tmp";
#endif
static const char *testbase = TIDY_TEST_ROOT PATH_SEP "testbase";
static const char *testlist = TIDY_TEST_ROOT PATH_SEP "testcases.txt";
static const char *path_sep = PATH_SEP;

static vSTG vFailed;
static vELIST vExceptions;

static std::string current_test;
static int current_itest = 0;
static int current_exit_code = 0;
static const char *exceptions = "445557:500236:647255:649812:658230";

#define MMX_STATIC 1024
static char _static_buff[MMX_STATIC+4];

#ifdef _MSC_VER
#define SNPRINTF _snprintf
#else
#define SNPRINTF snprintf
#endif

#define SPRTF1(a,b) {   \
  SNPRINTF(_static_buff, MMX_STATIC, a, b); \
  vFailed.push_back(_static_buff); \
  SPRTF("%s: %s",module,_static_buff); \
}

// FORWARD REFS
vSTG string_split( const std::string& str, const char* sep = 0, int maxsplit = 0 );
int addException( std::string &test, int ec = -1, std::string html = "", std::string config = "", std::string basemsg = "", std::string baseout = "");

int isException( std::string &test, size_t *poff )
{
    size_t ii, max = vExceptions.size();
    PELIST pe;
    for (ii = 0; ii < max; ii++) {
        pe = &vExceptions[ii];
        std::string s = pe->test;
        if (test == s) {
            pe->used++;
            *poff = ii; // return offset of exception
            return 1;
        }
    }
    return 0;
}

int addException( std::string &test, int ec, std::string html, std::string config, std::string basemsg, std::string baseout )
{
    int iret = 0;
    size_t off = (size_t)-1;
    if (!isException(test, &off)) {
        ELIST el;
        el.test = test;
        el.used = 0;
        el.ec = ec;
        el.html = html;
        el.config = config;
        el.basemsg = basemsg;
        el.baseout = baseout;
        vExceptions.push_back(el);
        iret = 1;
    }
    return iret;
}

void AddExeptionList()
{
    vSTG v = string_split( exceptions, ":" );
    size_t ii, max = v.size();
    for (ii = 0; ii < max; ii++) {
        std::string s = v[ii];
        addException(s);
    }
}

void give_help( char *name )
{
    SPRTF("%s: usage: [options] usr_input\n", module);
    SPRTF("Options:\n");
    SPRTF(" --help  (-h or -?) = This help and exit(2)\n");
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
                SPRTF("%s: Unknown argument '%s'. Try -? for help...\n", module, arg);
                return 1;
            }
        } else {
            // bear argument
            if (usr_input) {
                SPRTF("%s: Already have input '%s'! What is this '%s'?\n", module, usr_input, arg );
                return 1;
            }
            usr_input = strdup(arg);
        }
    }
    //if (!usr_input) {
    //    SPRTF("%s: No user input found in command!\n", module);
    //    return 1;
    //}

    AddExeptionList();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// not exactly a 'trim', but an important 'split' function

vSTG split_whitespace( const std::string &str, int maxsplit )
{
    vSTG result;
    std::string::size_type len = str.length();
    std::string::size_type i = 0;
    std::string::size_type j;
    int countsplit = 0;
    while (i < len) {
        while (i < len && isspace((unsigned char)str[i])) {
		    i++;
		}
        j = i;  // next non-space
        while (i < len && !isspace((unsigned char)str[i])) {
		    i++;
		}

		if (j < i) {
            result.push_back( str.substr(j, i-j) );
		    ++countsplit;
		    while (i < len && isspace((unsigned char)str[i])) {
                i++;
            }

		    if (maxsplit && (countsplit >= maxsplit) && i < len) {
                result.push_back( str.substr( i, len-i ) );
                i = len;
		    }
		}
    }
    return result;
}
/**
  * split a string per a separator - if no sep, use space - split_whitespace 
  */

vSTG string_split( const std::string& str, const char* sep, int maxsplit )
{
    if (sep == 0)
        return split_whitespace( str, maxsplit );

	vSTG result;
	int n = (int)strlen( sep );
	if (n == 0) {
	    // Error: empty separator string
	    return result;
	}
	const char* s = str.c_str();
	std::string::size_type len = str.length();
	std::string::size_type i = 0;
	std::string::size_type j = 0;
	int splitcount = 0;

	while ((i + n) <= len) {
    	if ((s[i] == sep[0]) && (n == 1 || memcmp(s+i, sep, n) == 0)) {
            result.push_back( str.substr(j,i-j) );
		    i = j = i + n;
		    ++splitcount;
		    if (maxsplit && (splitcount >= maxsplit))
                break;
	    } else {
		    ++i;
	    }
	}
	result.push_back( str.substr(j,len-j) );
	return result;
}


///////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#define M_IS_DIR _S_IFDIR
#else // !_MSC_VER
#define M_IS_DIR S_IFDIR
#endif

#define MDT_NONE    0
#define MDT_FILE    1
#define MDT_DIR     2

static struct stat buf;
static int is_file_or_directory( const char *path )
{
	if (!path)
		return MDT_NONE;
	if (stat(path,&buf) == 0)
	{
		if (buf.st_mode & M_IS_DIR)
			return MDT_DIR;
		else
			return MDT_FILE;
	}
	return MDT_NONE;
}
///////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
#define TABSIZE    4

#define DOS_CRLF   0
#define UNIX_LF    1
#define MAC_CR     2

typedef struct
{
	Bool pushed;
	int tabs;
	int curcol;
	int lastcol;
	int maxcol;
	int curline;
	int pushed_char;
	uint bgnline;
	uint size;
	uint length;
	char *buf;
	Bool had_nonspace;
	FILE *fp;
    Bool usetb;
    TidyBuffer tb;
} Stream;

static int tabsize = TABSIZE;
#ifdef WIN32
static int endline = DOS_CRLF;
#else
// TODO: need macro for MAC compiler?
static int endline = UNIX_LF;
#endif
static Bool tabs = yes; /* default is to KEEP tabs in this case */

/*
 Memory allocation functions vary from one environment to
 the next, and experience shows that wrapping the local
 mechanisms up provides for greater flexibility and allows
 out of memory conditions to be detected in one place.
*/
void *MemAlloc(size_t size)
{
	void *p;

	p = malloc(size);

	if (!p)
	{
		fprintf(stderr, "***** Out of memory! *****\n");
		exit(1);
	}

	return p;
}

void *MemRealloc(void *old, size_t size)
{
	void *p;

	p = realloc(old, size);

	if (!p)
	{
		fprintf(stderr, "***** Out of memory! *****\n");
		return NULL;
	}

	return p;
}

void MemFree(void *p)
{
	free(p);
	p = NULL;
}

static Stream *NewStream(FILE *fp)
{
	Stream *in;

	in = (Stream *)MemAlloc(sizeof(Stream));

	memset(in, 0, sizeof(Stream));
	in->fp = fp; /* set file pointer */
	return in;
}

static void FreeStream(Stream *in)
{
	if (in->buf)
		MemFree(in->buf);

	MemFree(in);
}

static void AddByte(Stream *in, uint c)
{
	char *cp;
    if (in->usetb) {
        tidyBufPutByte( &in->tb, c );
        return;
    }
	if (in->size + 1 >= in->length)
	{
		while (in->size + 1 >= in->length)
		{
			if (in->length == 0)
				in->length = 8192;
			else
				in->length = in->length * 2;
		}

		in->buf = (char *)MemRealloc(in->buf, in->length*sizeof(char));
	}
	cp = &in->buf[in->size++];
	*cp++ = (char)c;
	*cp = '\0';  /* ensure zero termination */
	cp = &in->buf[in->bgnline];
}

/*
  Read a character from a stream, keeping track
  of lines, columns etc. This is used for parsing
  markup and plain text etc. A single level
  pushback is allowed with UngetChar(c, in).
  Returns EndOfStream if there's nothing more to read.
*/
static int ReadChar(Stream *in)
{
	int c;

	if (in->pushed)
	{
		in->pushed = no;

		if (in->pushed_char == '\n')
			in->curline--;

		return in->pushed_char;
	}

	in->lastcol = in->curcol;

	/* NOT USED HERE: not chaning anything - certainly not expanding tab ? */
	if (in->tabs > 0)
	{
		in->curcol++;
		in->tabs--;
		return ' ';
	}
    
	/* Else go on with normal buffer: */
	for (;;)
	{
		c = getc(in->fp);

		/* end of file? */
		if (c == EOF)
			break;

		/* coerce \r\n  and isolated \r as equivalent to \n : */
		if (c == '\r')
		{
			c = getc(in->fp);

			if (c != '\n')
				ungetc(c, in->fp);

			c = '\n';
		}

		if (c == '\n')
		{
			if (in->maxcol < in->curcol)
				in->maxcol = in->curcol;

			in->curcol = 1;
			in->curline++;
			in->had_nonspace = no;
			in->bgnline = in->size; /* keep offset to begin of this line in the buffer */
			break;
		}

		if (c == '\t')
		{
			if (tabs)
			{
				/* in this utility, this is ALWAYS true - Tabs are kept as tabs */
  			    in->curcol += tabsize - ((in->curcol - 1) % tabsize);
			}
			else
			{
				/* expand to spaces - NEVER happens in this tool - not tested */
				in->tabs = tabsize - ((in->curcol - 1) % tabsize) - 1;
				in->curcol++;
				c = ' ';
			}
			break;
		}

		if (c == '\033')
			break;

		/* strip control characters including '\r' */

		if (0 < c && c < 32)
			continue;

		if (c > 32) 
		{
			if (in->had_nonspace == no) {
				// first non-space character found
				if( in->curcol >= tabsize )
				{
#ifdef IS_SPACE_TO_TABS
					/* maybe substitute spaces with a tab */
                    int cnt = 0;
					char *ln = &in->buf[in->bgnline + 1];
					int ui = (int)strlen(ln);   // was in->curcol;
					/* 1 - kill the spaces */
					while ((ui >= tabsize) && (*ln == ' ') && (is_all_spaces(ln,tabsize))) {
						if ((int)in->size >= tabsize) {
							in->size -= tabsize;
							ln += tabsize;
							cnt++;
						} else {
							break; /* this should NEVER happen */
						}
						ui -= tabsize;
					}
					/* 2 - substitute a tab for this many spaces of tabsize */
					if (cnt) {
						while(cnt--) {
							AddByte(in, (uint)'\t');
						}
					}
#endif // IS_SPACE_TO_TABS
				}
			}
			in->had_nonspace = yes;
		}
		in->curcol++;
		break;
	}

	return c;
}


static Stream *ReadFile(FILE *fin, int opts)
{
	int c;
	Stream *in  = NewStream(fin);
    if (opts & 0x0001) 
        in->usetb = yes;

	while ((c = ReadChar(in)) >= 0)
		AddByte(in, (uint)c);

	return in;
}

int tidyLoadFile( const char *file, TidyBuffer *pinput )
{
    FILE *fin = fopen(file, "rb");   /* note binary read */
    Stream *in = 0;
    if (!fin) 
        return no;
    in = ReadFile(fin, 1);  // add option to use tidyBuffer
    tidyBufAttach( pinput, in->tb.bp, in->tb.size );    // move this buffer
    tidyBufDetach( &in->tb ); // will be freed when input is...
    FreeStream( in );   // finished with this...
    return yes;

}
int load_tidy_file(const char *file, TidyBuffer *pinput)
{
    return tidyLoadFile(file,pinput) ? 0 : 1;
}

int load_tidy_file2(const char *file, TidyBuffer *pinput)
{
    tidyBufInit( pinput );
    std::ifstream ifs(file);
    if (ifs.bad()) {
        return 1;
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    const std::string file_str = oss.str();    // get a string
    tidyBufAppend( pinput, (void *)file_str.c_str(), file_str.size() );
    return 0;
}

static Bool abort_on_size = no; // want to also skip CR/LF == LF

Bool getUTF8Seq( uint ch, int *count )
{
    Bool hasError = no;
    int bytes = 0;
    uint n = 0;
    if (ch <= 0x7F) /* 0XXX XXXX one byte */
    {
        n = ch;
        bytes = 1;
    }
    else if ((ch & 0xE0) == 0xC0)  /* 110X XXXX  two bytes */
    {
        n = ch & 31;
        bytes = 2;
    }
    else if ((ch & 0xF0) == 0xE0)  /* 1110 XXXX  three bytes */
    {
        n = ch & 15;
        bytes = 3;
    }
    else if ((ch & 0xF8) == 0xF0)  /* 1111 0XXX  four bytes */
    {
        n = ch & 7;
        bytes = 4;
    }
    else if ((ch & 0xFC) == 0xF8)  /* 1111 10XX  five bytes */
    {
        n = ch & 3;
        bytes = 5;
        hasError = yes;
    }
    else if ((ch & 0xFE) == 0xFC)  /* 1111 110X  six bytes */
    {
        n = ch & 1;
        bytes = 6;
        hasError = yes;
    }
    else
    {
        /* not a valid first byte of a UTF-8 sequence */
        n = ch;
        bytes = 1;
        hasError = yes;
    }

    /* successor bytes should have the form 10XX XXXX */

    /* If caller supplied buffer, use it.  Else see if caller
    ** supplied an input source, use that.
    */
    *count = bytes;
    return hasError ? no : yes;
}

////////////////////////////////////////////////////////////////////
//// from : http://stackoverflow.com/questions/2948308/how-do-i-read-utf-8-characters-via-a-pointer
//// for Widows/MSVC needs #include <Winsock2.h>

#define IS_IN_RANGE(c, f, l)    (((c) >= (f)) && ((c) <= (l)))

//u_long readNextChar(char *& p, int len) 
int getNextSeqLen(char *& p, int len, u_long *puc) 
{  
    u_char c1, c2, *ptr = (u_char *) p;
    u_long uc = 0;
    int seqlen = 0;
    int datalen = len; // available length of p ...;    

    if( datalen < 1 )
    {
        // malformed data, do something !!!
        return -1;
    }

    c1 = ptr[0];    // get FIRST character - 8-bits

    if( (c1 & 0x80) == 0 )
    {
        // no high bit set, is just ASCII
        uc = (u_long) (c1 & 0x7F);
        seqlen = 1;
    }
    else if( (c1 & 0xE0) == 0xC0 )
    {
        uc = (u_long) (c1 & 0x1F);
        seqlen = 2;
    }
    else if( (c1 & 0xF0) == 0xE0 )
    {
        uc = (u_long) (c1 & 0x0F);
        seqlen = 3;
    }
    else if( (c1 & 0xF8) == 0xF0 )
    {
        uc = (u_long) (c1 & 0x07);
        seqlen = 4;
    }
    else
    {
        // malformed data, do something !!!
        return -1;
    }

    if( seqlen > datalen )
    {
        // malformed data, do something !!!
        return -1;
    }

    for(int i = 1; i < seqlen; ++i)
    {
        c1 = ptr[i];

        if( (c1 & 0xC0) != 0x80 )
        {
            // malformed data, do something !!!
            return -1;
        }
    }

    switch( seqlen )
    {
        case 2:
        {
            c1 = ptr[0];

            if( !IS_IN_RANGE(c1, 0xC2, 0xDF) )
            {
                // malformed data, do something !!!
                return -1;
            }

            break;
        }

        case 3:
        {
            c1 = ptr[0];
            c2 = ptr[1];

            if( ((c1 == 0xE0) && !IS_IN_RANGE(c2, 0xA0, 0xBF)) ||
                ((c1 == 0xED) && !IS_IN_RANGE(c2, 0x80, 0x9F)) ||
                (!IS_IN_RANGE(c1, 0xE1, 0xEC) && !IS_IN_RANGE(c1, 0xEE, 0xEF)) )
            {
                // malformed data, do something !!!
                return -1;
            }

            break;
        }

        case 4:
        {
            c1 = ptr[0];
            c2 = ptr[1];

            if( ((c1 == 0xF0) && !IS_IN_RANGE(c2, 0x90, 0xBF)) ||
                ((c1 == 0xF4) && !IS_IN_RANGE(c2, 0x80, 0x8F)) ||
                !IS_IN_RANGE(c1, 0xF1, 0xF3) )
            {
                // malformed data, do something !!!
                return -1;
            }

            break;
        }
    }

    for(int i = 1; i < seqlen; ++i)
    {
        uc = ((uc << 6) | (u_long)(ptr[i] & 0x3F));
    }
    *puc = uc;      // unicodeChar???
    p += seqlen;
    return seqlen;  // unicodeChar; 
}

////////////////////////////////////////////////////////////////////////////////

Bool  tidyBufferCompare( TidyBuffer *pout, TidyBuffer *pbase, const char *msg )
{
    Bool res = yes;
    if (pout->size != pbase->size) {
        if (abort_on_size) {
            SPRTF("%s: Test %s: Difference in size %d vs %d\n", module, 
                (msg ? msg : "Unknown"), pout->size, pbase->size);
            return no;
        }
    }
    uint i, j;
    int c, d;
    byte *po, *pi;
    int line_base = 0;
    int line_out = 0;

    for (i = 0, j = 0; (i < pout->size) && (j < pbase->size); i++, j++) {
        // ******************************************************************
        // this has to be different when comparing utf-8 sequences
        po = &pout->bp[i];
        c = *po;
        pi = &pbase->bp[j];
        d = *pi;
        if (c == '\r') {
            if (((i + 1) < pout->size)&&(pout->bp[i+1] == '\n')) {
                i++;
                po = &pout->bp[i];
                c = *po;
            }
        }
        if (c == '\n') {
            line_base++;
        }
        if (d == '\r') {
            if (((j + 1) < pbase->size)&&(pbase->bp[j+1] == '\n')) {
                j++;
                pi = &pbase->bp[j];
                d = *po;
            }
        }
        if (d == '\n')
            line_out++;

        if (c != d) {
            if (res) {
                SPRTF("%s: Test %s: First difference noted, line base %d = 0x%02x, in %d = 0x%02x\n", module,
                    (msg ? msg : "Unknown"), line_base, c, line_out, d );
                // base j
                TidyBuffer info;
                tidyBufInit( &info );

                int max_left = 10;
                int max_right = 10;
                int save = 0;
                int first = j - max_left;
                int last  = j + max_right;
                int nout  = i - max_left;
                int nlast = i + max_right;
                if (first < 0) {
                    max_left = j;
                    first = 0;
                    nout = i - max_left;
                }
                pbase->next = first;
                save = first;
                while (first < (int)pbase->size) {
                    d = tidyBufGetByte( pbase );
                    if (d < ' ') {
                        d = '.';
                    }
                    tidyBufPutByte( &info, d );
                    first++;
                    if (first >= last)
                        break;
                }
                tidyBufPutByte( &info, '\n' );
                first = save;
                while (first < (int)j) {
                    tidyBufPutByte( &info, '-' );
                    first++;
                    if (first >= last)
                        break;
                }
                tidyBufPutByte( &info, '^' );
                tidyBufPutByte( &info, '\n' );

                pout->next = nout;
                while (nout < (int)pout->size) {
                    c = tidyBufGetByte( pout );
                    if (c < ' ') {
                        c = '.';
                    }
                    tidyBufPutByte( &info, c );
                    nout++;
                    if (nout >= nlast)
                        break;
                }

                SPRTF("%s\n", info.bp );
                tidyBufFree(&info);
            }
            res = no;
        }
    }
    return res; /* they are the SAME */
}

int run_tidy_test( std::string &test, int ec, std::string &html, std::string &config, std::string &basemsg, std::string &baseout )
{
    int rc, status, iret = 0;
    TidyDoc tdoc = tidyCreate();
    if (!tdoc) {
        SPRTF("%s: tidyCreate() failed!\n", module );
        return 1;
    }
    TidyBuffer m_errbuf, m_output, m_input;
    tidyBufInit( &m_errbuf );
    tidyBufInit( &m_output );

    rc = tidySetErrorBuffer( tdoc, &m_errbuf );
    if (rc) {
        SPRTF("%s: Failed to set error buffer!\n", module );
        iret = 1;
        goto exit;
    }

    if (!tidyOptParseValue( tdoc, "tidy-mark", "no" )) {
        SPRTF("%s: Failed tidyOptParseValue( tdoc, \"tidy-mark\", \"no\" )!", module);
        iret = 1;
        goto exit;
    }

    status = tidyLoadConfig( tdoc, config.c_str() );
    if ( status != 0 )
        SPRTF("%s: Loading config file \"%s\" failed, err = %d, test = %s\n", module, config.c_str(), status, test.c_str());

    status = tidyParseFile( tdoc, html.c_str() );
    if ( status >= 0 ) {
        status = tidyCleanAndRepair( tdoc );
    } else {
        SPRTF("%s: Test %s: Failed to load '%s'!\n", module, test.c_str(), html.c_str());
        iret = 1;
        goto exit;
    }
    if ( status >= 0 ) {
        status = tidyRunDiagnostics( tdoc );
    } else {
        SPRTF("%s: Test %s: Failed tidyCleanAndRepair()!\n", module, test.c_str() );
        iret = 1;
        goto exit;
    }
    if ( status >= 0 ) {
        rc = tidySaveBuffer( tdoc, &m_output );          // Pretty Print
        if ( rc >= 0 ) {
            if ( rc > 0 ) {
                // SPRTF( "\nDiagnostics:\n\n%s", m_errbuf.bp );
            } else {

            }
        }
        int got = 0;
        int contentErrors   = tidyErrorCount( tdoc );
        int contentWarnings = tidyWarningCount( tdoc );
        int accessWarnings  = tidyAccessWarningCount( tdoc );
        if (contentErrors) {
            got = 2;
        } else if (contentWarnings) {
            got = 1;
        }
        if (ec != got) {
            SPRTF("%s: Test %s: Failed to get same exit indication! got %d, expected %d\n", module, test.c_str(), got, ec );
        }
        if (baseout.size()) {
            tidyBufInit( &m_input );
            if (load_tidy_file( baseout.c_str(), &m_input )) {
                SPRTF("%s: Test %s: Failed to load base html '%s'!\n", module, test.c_str(), baseout.c_str() );
            } else {
                if (!tidyBufferCompare( &m_output, &m_input, test.c_str() )) {
                    SPRTF1("Difference for test '%s'!\n", test.c_str() );
                }
            }
        }

    } else {
        SPRTF("%s: Failed tidyRunDiagnositicsr()!\n", module);
        iret = 1;
        goto exit;

    }


exit:
    tidyBufFree( &m_errbuf );
    tidyBufFree( &m_output );
    tidyRelease( tdoc ); /* called to free hash tables etc. */

    return iret;
}


static ELIST _s_elist;

int getBaseHtml( std::string &test, int ec, std::string &baseout, int verb )
{
    int iret = 0;
    baseout = testbase;
    baseout += path_sep;
    baseout += "out_";
    baseout += test;
    baseout += ".html";
    if (is_file_or_directory(baseout.c_str()) != MDT_FILE) {
        if (ec == 2) {
            baseout = "";
        } else if ((test == "431895")||(test == "431958")) {
            baseout = "";  // seems EXCEPTION - 1. is the emac test anyway 2 is writeback -m WHAT!!! modifies input
        } else {
            if (verb) {
                SPRTF("%s: Failed to find a output for '%s' in testbase '%s'!.\n", module,
                    test.c_str(), testbase );
            }
            iret = -1;  // FAILED!!!
        }
    }
    return iret;
}

int getBaseMsg( std::string &test, int ec, std::string &basemsg, int verb )
{
    int iret = 0;
    basemsg = testbase;
    basemsg += path_sep;
    basemsg += "msg_";
    basemsg += test;
    basemsg += ".txt";
    if (is_file_or_directory(basemsg.c_str()) != MDT_FILE) {
        SPRTF("%s: Failed to find a message for '%s' in testbase '%s'!.\n", module,
                    test.c_str(), testbase );
        iret = -1;  // FAILED!!!
    }
    return iret;
}

int getInputHtml( std::string &test, int ec, std::string &html, int verb )
{
    int iret = 0;
    std::string in_file = input;
    in_file += path_sep;
    in_file += "in_";
    in_file += test;
    html = in_file;
    html += ".xhtml";
    if (is_file_or_directory(html.c_str()) != MDT_FILE) {
        html = in_file;
        html += ".xml";
        if (is_file_or_directory(html.c_str()) != MDT_FILE) {
            html = in_file;
            html += ".html";
            if (is_file_or_directory(html.c_str()) != MDT_FILE) {
                if (verb) {
                    SPRTF("%s: Failed to find test '%s' in input '%s'! Tried xhtml, xml, html.\nRemove test from '%s\n", module,
                        test.c_str(), input, testlist );
                }
                iret = -1;  // FAILED!!!
            }
        }
    }
    return iret;
}

int getConfig( std::string &test, int ec, std::string &config, int verb )
{
    int iret = 0;
    std::string in_file = input;
    in_file += path_sep;
    config = in_file;
    config += "cfg_";
    config += test;
    config += ".txt";
    if (is_file_or_directory(config.c_str()) != MDT_FILE) {
        config = in_file;
        config += "cfg_default.txt";
        if (is_file_or_directory(config.c_str()) != MDT_FILE) {
            if (verb) {
                SPRTF("%s: Failed to find a config for '%s' in input '%s'!.\n", module,
                    test.c_str(), input );
            }
            iret = -1;  // FAILED!!!
        }
    }
    return iret;
}

// flailed flags
#define ff_NoBaseHtml  0x001
#define ff_NoBaseMsg   0x002
#define ff_NoInputHtm  0x004
#define ff_NoConfig    0x008

int get_test_files( PELIST pel, std::string &test, int ec, int verb )
{
    int flag = 0;

    std::string basehtml;
    std::string basemsg;
    std::string html;
    std::string config;

    if (getBaseHtml( test, ec, basehtml, verb ))
        flag |= ff_NoBaseHtml;  // FAILED!!!

    if (getBaseMsg( test, ec, basemsg, verb ))
        flag |= ff_NoBaseMsg;  // FAILED!!!

    if (getInputHtml( test, ec, html, verb ))
        flag |= ff_NoInputHtm;  // FAILED!!!

    if (getConfig( test, ec, config, verb ))
        flag |= ff_NoConfig;  // FAILED!!!

    if (!pel)
        pel = &_s_elist;
    pel->flag = flag;
    pel->test = test;
    pel->ec   = ec;
    pel->used = 0;
    pel->html = html;
    pel->config = config;
    pel->baseout = basehtml;
    pel->basemsg = basemsg;

    return flag;
}

int run_test( std::string &test, int ec )
{
    int iret = 0;
    PELIST pel = &_s_elist;
    iret = get_test_files( pel, test, ec, 0 );
    std::string basehtml;
    std::string basemsg;
    std::string html;
    std::string config;

    if (getBaseHtml( test, ec, basehtml, 1 ))
        return -1;  // FAILED!!!

    if (getBaseMsg( test, ec, basemsg, 1 ))
        return -1;  // FAILED!!!

    if (getInputHtml( test, ec, html, 1 ))
        return -1;  // FAILED!!!

    if (getConfig( test, ec, config, 1 ))
        return -1;  // FAILED!!!

    // Got input, a config, a base msg, and maybe a basehtml - RUN THE TEST
    iret = run_tidy_test( test, ec, html, config, basemsg, basehtml );

    return iret;
}

int run_tidy_tests()
{
    int iret = 0;
    int res = 0;
    const char *file = testlist;
    std::ifstream input(file);
    //std::ifstream input = std::ifstream(file);
    if (input.bad() || !input.is_open()) {
        SPRTF("%s: Unable to open file '%s'!\n", module, file);
        return 1;
    }
    std::string line;
    std::string test;
    int error_exit;
    int file_lines = 0;
    int valid_lines = 0;
    while (std::getline(input, line)) {
        file_lines++;
        vSTG vstg = split_whitespace( line, 0 );
        if (vstg.size() != 2) {
            // forgive and forget
            continue;
        }
        valid_lines++;
        test = vstg[0];
        error_exit = atoi(vstg[1].c_str());
        current_test = test;
        current_itest = atoi(test.c_str());
        current_exit_code = error_exit;

        res = run_test( test, error_exit );

        if (res < 0) {
            fprintf(stderr, "%s: Bad test '%s' failure!\n", module, test.c_str() );
            iret = 1;
            break;
        }

        //int num, ec;
        //std::istringstream iss(line);
        //if (!(iss >> num >> ec)) { 
        //    break; 
        //} // error

        // process pair (num,ec)
    }

    input.close();
    SPRTF("%s: Processed %d line from '%s', ran %d tests.\n", module, file_lines, file, valid_lines);
    if (valid_lines == 0) {
        SPRTF("%s: Warning, no tests run...\n", module );
        return 1;
    }
    // failed report
    if (iret == 0) {
        size_t ii, max = vFailed.size();
        if (max) {
            SPRTF("%s: List of %d tests that failed:\n", module, (int)max);
            for (ii = 0; ii < max; ii++) {
                test = vFailed[ii];
                SPRTF("%s", test.c_str());
            }
        }
    }
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

    iret = run_tidy_tests(); // actions of app

    return iret;
}


// eof = test-tidy.cxx
