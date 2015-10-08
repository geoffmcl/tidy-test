# README.curl.md

Library **cURL** has a great example of using **libyidy** -  [htmltidy](http://curl.haxx.se/libcurl/c/htmltidy.html). This is good to see...

Needed a small patch to bring it up-to-date to use current HTACG Tidy!

```
--- htmltidy.c	Wed Oct 07 18:24:45 2015
+++ ..\src\htmltidy.c	Wed Oct 07 18:02:11 2015
@@ -24,12 +24,12 @@
  * </DESC>
  */
 /*
- * LibTidy => http://tidy.sourceforge.net
+ * LibTidy => https://github.com/htacg/tidy-html5
  */
 
 #include <stdio.h>
-#include <tidy/tidy.h>
-#include <tidy/buffio.h>
+#include <tidy.h>
+#include <tidybuffio.h>
 #include <curl/curl.h>
 
 /* curl write callback, to fill tidy's input buffer...  */
@@ -128,3 +128,4 @@
   return(0);
 }
 
+/* eof */
```

It works perfectly for fetching a URL easily and effiently into a tidy buffer, which is passed to libtidy to clean up... 

Of course this example only dumps the html node tree, but shows how smoothly a URL can be fetched with libcurl...

This project has a `CMakeModules\FindTidy.cmake` which searches for the above `tidy.h`, and `libtidy`. That is, it sets up the correct include paths, and target link libraries for the compile and link...

In Windows, it will first search for the `static` tidys.lib, to save having to set up access to the DLL each time the `htmltidy.exe` app is run. There is no windows installer that sets up a path to the tidy installed DLL, nor include paths...

Unfortunately, the cmake distributed `FindCURL.cmake` module has a search order that looks for the shared library first, and has the windows static libcurl.lib as last on the list. 

It uses an order `curl curllib libcurl_imp curllib_static libcurl`. The `libcurl_imp`, which is the `libcurl.dll` is found in my installation before the static version `libcurl` last on the list... 

My current installation is cURL version 7.35.0, January 29 2014, versus 7.45 on the site... maybe I need to do an update...

And will consider importing a `FindCURL.cmake` to this repo to allow more user control over this... maybe it could use a `CURL_LIB_NAMES` variable where the user can establish what they would prefer to have found first... or something...

But all in all a great example of the use of the two libraries...

; eof
