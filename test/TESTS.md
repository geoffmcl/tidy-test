# Tests 20160601

### Brief Description

This is a developing set of **new** tests. Unlike the current so called `Regression Tests` these only test for specific issues.

All tests are in the `input5` folder. Each is numbered in the for `in_xxxx.html`. In many cases each may be a series of test inputs, and these will be of the form `in_zzzz.html`, usually the first reported sample. Additional tests, as the issue progresses, will be of the form `in_zzzz-1.html`, `in_zzzz-2.html`, etc.

In general the `xxxx` numbering will match the htacg tidy-html5 issue number. This can also be `in_sfNNN.html` if the test is from the old Sourceforge [Bugs](https://sourceforge.net/p/tidy/bugs/) list, or `in_ebXXXX.html` is reported by the [EdBrowse](https://github.com/CMB/edbrowse) group, which uses libtidy to parse html.

For some tests there may also be a configuration file of the form `cfg_xxxx.txt`, and there is a `cfg_def.txt` for others.

### Test Information

Below are the details of some of the tests. This listing was only started recently, so many of the older tests are undocumented, but clues can be had by looking up the `issue` given in the numbering...

#### Infinite loop parsing in tidy #380

This bug crept in due to another fix. It is clear when it happens - Tidy will never exit, repeating a set of warnings over and over. It does not require any special config.

#### stack exhaustion in tidy-html5 5.1.25 #343

Due to the design of tidy it is easily possible to exhaust the stack. Two test files can do this.

 - input5\exhaustion.html
 - input5\in_343.html
 
Obviously these should be **EXCLUDED** from any test run since it is a known, and ever present `feature` of tidy.

This could ONLY be removed by a complete **rewrite** ot Tidy.

### Future

Some of these tests should be selected and added to the https://github.com/htacg/tidy-html5-tests repository, a base output provided, and form part of the `official` regression tests...

; eof 20160601 - 20160306
