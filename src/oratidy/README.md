# Oracle SQL binding

DOWNLOAD: 20151208: https://sourceforge.net/p/tidy/patches/86/

This is a **`libtidy`** binding to oracle pl/sql. developed and tested on win xp with oracle 11g2 32 bit.

You need at least oracle client tools installed to compile, because you need `oci.lib`, `oci.h`, and `ociextp.h`. You will also need tidy.dll (or tidys.lib) to compile *huh* :-) NOTE: in the patch.zip these files are missing!

MISSING: How to install Oracle SQL? Seeme to download from [Oracle](http://www.oracle.com/technetwork/developer-tools/sql-developer/downloads/index.html) need an account. The `Create Account` looks pretty invasive? Is there another way? 

What **exactly** needs to be installed? ANd what are the unix install steps?

1. compile library make sure you add oracle libs and libtidy
====

on windows I did:

```
gcc liboratidy.c lib\tidy.dll lib\oci.lib -ID:\oracle\product\10.2.0\client_1\oci\include -ID:\tmp\tidy -shared -o liboratidy.dll
```

Hmmm, using `gcc` suggests either `cygwin` or perhaps `MinGW` installed. What tool set it this?

2. copy resulting dll file and tidy.dll to `%ORACLE_HOME%\BIN`
====


3. log into sqlplus and create the library (use the absolute path on windows machines)
====

```
SQL> create library TIDY_LIB is '%ORACLE_HOME%\BIN\liboratidy.dll';
  2  /
```

4. test the procedure
====

```
SQL> @test_oratidy
```

you should see something like this:

```
SQL> @D:\tmp\tidy\oratidy\test_oratidy
1
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"

"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html
xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta name="generator" content= "HTML Tidy for Windows (vers 14 February 2006), see www.w3.org"/>
<title>Foo</title>
</head>
<body>
<p>Foo!&iacute;&frac14;</p>
</body>
</html>

line 1 column 1 - Warning: missing <!DOCTYPE> declaration
Info: Document
content looks like XHTML 1.0 Strict
1 warning, 0 errors were found!


-9802

PL/SQL-Prozedur erfolgreich abgeschlossen.

SQL>
```

5. create wrapper package
====

```
SQL> @D:\tmp\tidy\oratidy\create_tidy_pkg
```

6. test again
====

```
SQL> exec tidy.test;
```

you are done!

; eof
