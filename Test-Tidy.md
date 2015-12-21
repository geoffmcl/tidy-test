# Test Tidy 20151212

#### WIP - Proof of concept ####

The aim is to run some `test` html files, with a given config, through *`libtidy`*, and compare the output to a `testbase`.

[Tidy](http://www.html-tidy.org/) already has a set of cross-platform scripts to do [regression testing](https://github.com/htacg/tidy-html5/tree/master/test). These used CMD/BAT files in windows and shell scripts for unix. These work quite well. There are maybe some OS/CPU differences, but manageable...

There the output is sent to a file, and that file is compared to the `testbase` separately... using say `diff`.

This is an experiment, using *`libtidy`* that ouputs the html/txt to tidy buffers, then load the approriate `testbase` into other tidy buffers, compare the contents of the two buffers, and the results displayed...

Of course the first thing is skipping line endings... not too difficult... dealing with character encoding... a little more difficult, and to my surprise 223 of 228 tests passed... the following is the current output using the tidy testcases...

This may suffer from cut/paste/console/editor mangling...

```
test-tidy: Test 445557: First difference noted, line base 2 = 0x13, in 2 = 0x20
ument had  - I chang
----------^
ument had ? - I chan
test-tidy: Difference for test '445557'!
test-tidy: Test 500236: First difference noted, line base 3 = 0x0d, in 4 = 0x0a
&quot;&gt;?  &lt;a:s
----------^
&quot;&gt;?  &lt;a:s
test-tidy: Difference for test '500236'!
test-tidy: Test 647255: First difference noted, line base 8 = 0x05, in 8 = 0xdb
? ?<?p?>?-¦G+-¦G-¦G-
----------^
? ?<?p?>?-?¦?G?+?-?¦
test-tidy: Difference for test '647255'!
test-tidy: Test 649812: First difference noted, line base 0 = 0x0d, in 1 = 0x0a
/?/?E?N?"??? ? ? ? ?
----------^
/?/?E?N?"??? ? ? ? ?
test-tidy: Difference for test '649812'!
test-tidy: Test 658230: First difference noted, line base 7 = 0x14, in 7 = 0x20
ies: & "    2</p>?<p
----------^
ies: & "  ?  2</p>??
test-tidy: Difference for test '658230'!
test-tidy: Processed 228 line from 'F:\Projects\tidy-html5\test\testcases.txt', ran 228 tests.
test-tidy: List of 5 tests that failed:
Difference for test '445557'!
Difference for test '500236'!
Difference for test '647255'!
Difference for test '649812'!
Difference for test '658230'!
```

Still to look closely at these `failed` testcases...

But this does seem like a cross-platform way to do the these tests, and could be used as say a `# make test` CMake target...

#### WIP - Proof of concept ####

; eof
