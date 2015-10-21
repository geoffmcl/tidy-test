/*\
 * space2tab.cxx
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
 * DISCLAIMER: This source is as is. It does what it does. 
 *             If it works, praise me. If it fails, do not blame me!
\*/
/*\
 *
 * 20151021: Utility to fix a unique problem.
 *
 * I was working on a project edbrowse, which uses all tab indenting.
 * They run each file through 'indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0'
 * My MSVC 10 IDE editor is set to indent using 4 spaces, so files where I
 * added or modified a source line would be converted to all space indenting.
 * This tool, if it works, converts those leading spaces to a tab, and 
 * will also fix the file to consistent line endings of CR LF.
 * 
 * Usage: space2tab [options] input-file [output-file]
 *
 * It does no other indenting, or file modifications.
 *
 * It is added here since it uses some defines from tidyplatform.h, and 
 * it was originally styled after html tidy tool, tab2spaces.c, which
 * does the reverse - replaces all tabs with spaces.
 *
\*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tidyplatform.h>

#ifndef S2T_VERSION
#define S2T_VERSION "21 Oct, 2015"
#endif

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
    int space_cnt;
	FILE *fp;
} Stream;

static int tabsize = TABSIZE;
#ifdef WIN32
static int endline = DOS_CRLF;
#else
// TODO: need macro for MAC compiler?
static int endline = UNIX_LF;
#endif
static Bool tabs = yes; /* default is to KEEP tabs in this case */

static const char *module = "space2tab";

static const char *usr_input = 0;
static const char *usr_output = 0;

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

//////////////////////////////////////////////////////////////////
// check a buffer commences with at least this many spaces
/////////////////////////////////////////////////////////////////
Bool is_all_spaces( char *ln, int cnt )
{
    Bool ret = no;
    while (*ln && cnt)
    {
        if (*ln == ' ') 
        {
            cnt--;
            if (cnt == 0)
                return yes;
        }
        else
            return no;
        ln++;

    }
    return no;
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
	int c, cnt;

	if (in->pushed)
	{
		in->pushed = no;

		if (in->pushed_char == '\n')
			in->curline--;

		return in->pushed_char;
	}

	in->lastcol = in->curcol;

	/* expanding tab ? */
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
            in->space_cnt = 0;
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
					/* maybe substitute spaces with a tab */
					char *ln = &in->buf[in->bgnline + 1];
					int ui = (int)strlen(ln);   // was in->curcol;
					cnt = 0;
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
				}
			}
			in->had_nonspace = yes;
            in->space_cnt = 0;
		}
        else if ( c == ' ' )
        {
            in->space_cnt++;
            if (in->space_cnt > 3)
                cnt = 0;
        }
		in->curcol++;
		break;
	}

	return c;
}

static Stream *ReadFile(FILE *fin)
{
	int c;
	Stream *in  = NewStream(fin);

	while ((c = ReadChar(in)) >= 0)
		AddByte(in, (uint)c);

	return in;
}

static void WriteFile(Stream *in, FILE *fout)
{
	int i, c;
	char *p;

	i = in->size;
	p = in->buf;

	while (i--)
	{
		c = *p++;

		if (c == '\n')
		{
			if (endline == DOS_CRLF)
			{
				putc('\r', fout);
				putc('\n', fout);
			}
			else if (endline == UNIX_LF)
				putc('\n', fout);
			else if (endline == MAC_CR)
				putc('\r', fout);

			continue;
		}

		putc(c, fout);
	}
}

void give_help( char *name )
{
	printf("\n");
	printf("%s: usage: [options] usr_input [usr_output]\n", module);
	printf("%s Version: " S2T_VERSION "\n", module);
	printf("\n");
	printf("Options:\n");
	printf(" --help  (-h or -?) = This help and exit(0)\n");
	printf(" --tab <num>   (-t) = Replace <num> leading spaces with a tab (def=%d)\n", tabsize);
	printf(" --win         (-w) = Set line ends to CRLF (Windows)\n");
	printf(" --mac         (-m) = Set line ends to CR (Mac)\n");
	printf(" --unix        (-u) = Set line ends to LF (Unix)\n");
	printf("  Default is %s line endings.\n", 
		((endline == DOS_CRLF) ? "Windows (CRLF)" : (endline == UNIX_LF) ? "Unix (LF)" : "Mac (CR)"));
	printf("\n");
	printf(" Utility to replace leading spaces with a tab.\n");
	printf(" This utility reads the usr_input, and only deals with converting lines\n");
	printf(" with leading spaces, converting the above space count to tabs.\n");
	printf(" And can be coerced into changing the output line endings.\n");
	printf(" It does nothing about current indenting otherwise.\n");
}

#define ISDIGIT(a) (( a >= '0') && ( a <= '9' ))

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
			case 'w':
				endline = DOS_CRLF;
				break;
			case 'm':
				endline = MAC_CR;
				break;
			case 'u':
				endline = UNIX_LF;
				break;
			case 't':
				while (*sarg && !ISDIGIT(*sarg))
					sarg++;
				if (ISDIGIT(*sarg)) {
					if (sscanf(sarg, "%d", &tabsize) != 1) {
						printf("Option '%s' failed to yield in integer!\n", arg);
						return -1;
					}
				} else if (i2 < argc) {
					/* assume digit is next param */
					i++;
					sarg = argv[i];
					if (ISDIGIT(*sarg)) {
						if (sscanf(sarg, "%d", &tabsize) != 1) {
							printf("Option '%s %s' failed to yield in integer!\n", arg, sarg);
							return -1;
						}
					} else {
						printf("Expected integer to follow '%s'!\n", arg);
						return -1;
					}
				} else {
					printf("Expected integer to follow '%s'!\n", arg);
					return -1;
				}
				if ((tabsize < 1) || (tabsize > 255)) 
				{
					printf("Option '%s' yield bad tabsize %d. Range 1 - 255!\n", arg, tabsize);
					return -1;
				}
				break;

			default:
				printf("%s: Unknown argument '%s'. Try -? for help...\n", module, arg);
				return -1;
			}
		} else {
			// bear argument
			if (usr_input) {
				if (usr_output) {
					printf("%s: Already have input '%s', and output '%s'! What is this '%s'?\n", module, usr_input, usr_output, arg );
					return -1;
				} else {
					usr_output = strdup(arg);
				}
			} else {
				usr_input = strdup(arg);
			}
		}
	}
	if (!usr_input) {
		printf("%s: No user input found in command!\n", module);
		return -1;
	}
	return 0;
}

// main() OS entry
int main( int argc, char **argv )
{
	FILE *fin, *fout;
	Stream *in = NULL;
	int iret = 0;

	iret = parse_args(argc,argv);
	if (iret != 0) {
		if (iret == 2)
			return 0;
		return iret;
	}

	fin = fopen(usr_input, "rb");   /* note binary read */
	if (!fin) {
		printf("%s: Failed to open input file '%s'\n", module, usr_input);
		return -1;
	}
	in = ReadFile(fin);
	fclose(fin);

	if (usr_output) {
		fout = fopen(usr_output, "wb"); /* note binary write */
		if (!fout) {
			printf("%s: Failed to open output file '%s'\n", module, usr_output);
			return -1;
		}
	} else {
		fout = stdout;
	}

	WriteFile(in, fout);

	if (usr_output) {
		fclose(fout);
	}

	FreeStream(in);

	return iret;
}

/* eof */
