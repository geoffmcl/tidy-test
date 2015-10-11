# tidy-test 20151011

Some simple test cases using a cmake FindTidy.cmake module.

This is also important after the major version change to 5.0.0

Users upgrading to this need to ensure they uninstall any of the Tidy5 installs. There is a build/delete(W|U).txt to give the general files to delete from the install.

#### url2text app

If the [CURL](http://curl.haxx.se/) library is found, this little app is built. It accepts the input of a URL, and will use curl to fetch the html page. That page data will be passed to library Tidy in a buffer, and the tidy node tree will be dumped, to a file or stdout.

Various options control what is shown from the node tree. The default are the text nodes, except comments. The data is trimmed in an attempt to produce a simple readable list of text from the fetched page, hence the idea of a URL to text utility.

The idea was inspired by [edbrowse](http://edbrowse.org/), which similarly fetches a web page using curl, and passes the contents to library tidy to be able to iterate through the nodes collected, again with the idea of getting the readable text into a set of lines...

#### test-tidy app

Acutally has the same functionality as classic tidy since it in fact uses the tidy.c from the HTACG tidy-html5 source.

#### test-opts app

First a test of setting a string option to a blank, and then running libtidy using own constructed memory allocator.

#### test71

Has not been ported to unix yet.

This was a test app added to tidy-html5 while exploring Issue 71. A copy made here and some error checking added.

It does nothing except parse some canned html text, and shows how to get the escaped text, like `&amp;` instead of `&` using tidyNodeGetText(...). And then how to get the `raw` text using tidyNodeGetValue(...).

All input and output is using a `TidyBuffer`, so includes `<tidybuffio.h>`. **NOTE** the name change on Rel 5 - was `buffio.h` before...

Enjoy.

Geoff  
20151011 - 20150701 - 20150610 - 20150520

; eof
