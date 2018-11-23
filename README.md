# tidy-test 20181123

Some simple test cases using a cmake FindTidy.cmake module to find libtidy as a 3rdParty library, and then using the Tidy API.

This is also important after the major version change to 5.0.0

Users upgrading to this need to ensure they uninstall any of the Tidy5 installs. There is a build/delete(W|U).txt to give the general files to delete from the install.

### Project List

 * tidy-test - This is actually a mirror of the tidy binary. Uses 99.9% same source
 * tidy-locale - To test and mirror the language selection by Tidy.
 * htmltidy - Needs CURL. Fetch a URL, parse with Tidy, and enumerated tree of nodes
 * test226 - Issue #226 - test node, attribute deletion
 * test71 - Issue #71 - Difference between cooked tidyNodeGetText() and raw tidyNodeGetValue()
 * tidy-opts - Test code to set some options, and compare to default.
 * tidy-tree - Parse a html file, and output of the tidy nodes
 * tidy-url - WIN32 Only - Fetch and show a URL page using MS HTTPInputSource.
 * url2text - Needs CURL. Fetch a page, but only show the text nodes...
 * space2tab - Very specific tool to convert indent spaces added by my MSVC IDE editor to a tab.
 * tidy-test-prev - This is actually a 99.9% mirror of the previous tidy binary, before localization.

#### tidy-test app

This uses the same source as the [tidy-html5](https://github.com/htacg/tidy-html5) repo, with the small addition of outputting the date with the version, and before parsing a document, output the selected language for the messages. In all other respects it is a duplicate of standard console tidy.
 
#### url2text app

If the [CURL](http://curl.haxx.se/) library is found, this little app is built. It accepts the input of a URL, and will use curl to fetch the html page. That page data will be passed to library Tidy in a buffer, and the tidy node tree will be dumped, to a file or stdout.

Various options control what is shown from the node tree. The default are the text nodes, except comments. The data is trimmed in an attempt to produce a simple readable list of text from the fetched page, hence the idea of a URL to text utility.

The idea was inspired by [edbrowse](http://edbrowse.org/), which similarly fetches a web page using curl, and passes the contents to library tidy to be able to iterate through the nodes collected, again with the idea of getting the readable text into a set of lines...

#### test-opts app

First a test of setting a string option to a blank, and then running libtidy using own constructed memory allocator.

#### test71

Has not been ported to unix yet.

This was a test app added to tidy-html5 while exploring Issue 71. A copy made here and some error checking added.

It does nothing except parse some canned html text, and shows how to get the escaped text, like `&amp;` instead of `&` using tidyNodeGetText(...). And then how to get the `raw` text using tidyNodeGetValue(...).

All input and output is using a `TidyBuffer`, so includes `<tidybuffio.h>`. **NOTE** the name change on Rel 5 - was `buffio.h` before...

### Build Project

This is a [CMake](https://cmake.org/) project, and must be installed. CMake is a build system that uses a single script (defined in a CMakeLists.txt file) to generate appropriate platform-specific build scripts. A `cmake --help` will show the list of **generators** available on your system. Adjust the `Build System` below to your choice, but usually cmake defaults to what is available.

 * cd build
 * cmake .. [-G "Build System"] [-DCMAKE_BUILD_TYPE=Release] [... other options ...]
 
This will generate the native build files appropriate to your choice, so the next step depends on these tools. 

If that is the common `Unix Makefiles`, then you just need to run `$ make`. There is no install target.

If it is a version of Windows MSVC, like `-G "Visual Studio 14 Win64"`, then the next step can be `cmake --build . --config Release`. Again there is no install target.

CMake offer a GUI setup. Just set the source code and binary build directories, choose a `generator`, `configure` and `generate`...

During the `configure` stage cmake **must** find an installed tidy library, static or shared. For certain apps in the suite, CURL must be found, and ZLIB... 

If these 3rd party packages are installed in a `custom` folder, and this must be passed to cmake, like `-DCMAKE_INSTALL_PREFIX=F:\Projects\software.x64`, or using like  `CMAKE_PREFIX_PATH:PATH=X:\3rdParty.x64`...

The `build` directory in the source contains a convenient `build-me.sh` and `build-me.bat` which may work for you. Or at least serve as a model.

Enjoy.

Geoff  
20181123 - 20160311 - 20151101 - 20151011 - 20150701 - 20150610 - 20150520

; eof
