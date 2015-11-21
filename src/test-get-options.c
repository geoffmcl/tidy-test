/*
    Module: test-get-options.c
    Date: 20151121
    From: https://gist.github.com/benkasminbullock/a57de76e7a294371afd6

    Not sure exactly what is the purpose, but it creates a tidy document,
    gets the option list from the library with API tidyGetOptionList( tdoc ),
    then gets each option using API tidyGetNextOption( tdoc, &pos ),
    and outputs a TAP (https://testanything.org/) list like -

ok 1 - got a tdoc
ok 2 - name is not null
ok 3 - name looks OK
# indent-spaces 1
ok 4 - got a good opt type
ok 5 - default number doesn't look strange
# Integer type default value 2
ok 6 - name is not null
ok 7 - name looks OK
# wrap 1
ok 8 - got a good opt type
ok 9 - default number doesn't look strange
... ending with ...
1..369

    Included here as just another use of libtidy.

 */

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <tidy.h>

/* This number is too big for a sane default, so it looks like
   uninitialized memory. */

#define crazy_number 0x10000

/* Tests "garbage" for being ascii and short C string without
   whitespace. */

#define not_short 0x100

int is_short_ascii_no_ws (const char * garbage)
{
    int i;
    if (! garbage) {
	return 0;
    }
    for (i = 0; i < not_short; i++) {
	int c = garbage[i];
	if (c == '\0') {
	    /* Yes, it was a short ascii string (maybe one byte long). */
	    return 1;
	}
	if (!isprint (c) || c == ' ') {
	    /* Not a printable character, excluding space. */
	    return 0;
	}
    }
    /* Got all the way here and did not find the end. */
    return 0;
}

static void
test (int true_or_false, int * count_ptr, char * format, ...)
{
    va_list va_args;
    int count;
    count = * count_ptr;
    count++;
    if (! true_or_false) {
	printf ("not ");
    }
    printf ("ok %d - ", count);
    va_start (va_args, format);
    vprintf (format, va_args);
    va_end (va_args);
    * count_ptr = count;
    printf ("\n");
}

int main ()
{
    TidyIterator pos;
    TidyDoc tdoc;
    int count;
    count = 0;
    tdoc = tidyCreate();    
    pos = tidyGetOptionList( tdoc );
    test (pos != 0, & count, "got a tdoc");
    while (1) {
	TidyOption to;
	TidyOptionType tot;
	const char * name;
	const char * option_default;
	int option_default_int;
	Bool option_default_bool;
	to = tidyGetNextOption (tdoc, & pos);
	if (to == 0) {
	    break;
	}
	name = tidyOptGetName (to);
	test (name != 0, & count, "name is not null");
	tot = tidyOptGetType (to);
	if (name) {
	    test (is_short_ascii_no_ws (name), & count, "name looks OK");
	    printf ("# %s %d\n", name, tot);
	}
	switch (tot) {
	case TidyString:
	    test (1, & count, "got a good opt type");
	    option_default = tidyOptGetDefault (to);
	    //null seems to be allowed.
	    //test (option_default != 0, & count, "string default is not null");
	    if (option_default) {
		test (is_short_ascii_no_ws (option_default), & count,
		      "looks like a reasonable default string");
		printf ("# %s default value is '%s'.\n", name, option_default);
	    }
	    break;
	case TidyInteger:
	    test (1, & count, "got a good opt type");
	    option_default_int = tidyOptGetDefaultInt (to);
	    test (abs (option_default_int) < crazy_number, & count,
		  "default number doesn't look strange");
	    printf ("# Integer type default value %d\n", option_default_int);
	    break;
	case TidyBoolean:
	    test (1, & count, "got a good opt type");
	    option_default_bool = tidyOptGetDefaultInt (to);
	    test (option_default_bool == 0 || option_default_bool == 1,
		  & count, "boolean default is 0 or 1");
	    printf ("# boolean = %d\n", option_default_bool);
	    break;
	default:
	    test (0, & count, "got a good opt type");
	    printf ("# Unknown value for option type %d.\n", tot);
	}
    }
    // TAP plan
    printf ("1..%d\n", count);
    return 0;
}

/* eof */
