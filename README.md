# tidy-test 20150610

Some simple test cases using a cmake FindTidy.cmake module.

#### test-tidy app

Acutally has the same functionality as classic tidy since it in fact uses the tidy.c from the HTACG tidy-html5 source.

#### test-opts app

First a test of setting a string option to a blank, and then running libtidy using own constructed memory allocator.

#### test71

This was a test app added to tidy-html5 while exploring Issue 71. A copy made here and some error checking added.

It does nothing except parse some canned html text, and shows how to get the escaped text, like `&amp;` instead of `&` using tidyNodeGetText(...). And then how to get the `raw` text using tidyNodeGetValue(...).

All input and output is using a `TidyBuffer`, so includes `<buffio.h>`.

Enjoy.

Geoff  
20150610 - 20150520

; eof
