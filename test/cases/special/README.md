About this test suite:
======================

These test files represent that standard regression testing that must be
performed prior to committing changes to Tidy's code. In some circumstances
results are platform specific and these notices will be displayed in the
testing results.

The particular files in _this_ directory require special handling and so are
excluded from the standard testing suite.

case-431895
 : This test uses `gnu-emacs` and as such warning/error output is dependent
   upon the directory path from which the tests are run.
   
case-431958
 : This test uses `write-back` meaning that it generates no output file other
   than the original file, meaning that (1) it's not put into the results
   directory, and (2) there's a good chance of polluting our test cases.
   
case-500236
 : This case uses `word-2000` and is producing different output on different
   operating systems, reportedly.
   
case-661606
 : This cases shift-jis character set and is producing different output on
   different operating systems.