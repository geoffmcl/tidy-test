@setlocal
@REM ########################################################################
@REM This is the location of the cmake build output using the MSVC Generator
@set TMPEXE=F:\Projects\tidy-testing\build\win64\release\tidy.exe
@REM set TMPEXE=..\build\win64\Release\tidy.exe
@REM ########################################################################
@if NOT EXIST %TMPEXE% goto NOEXE
@call dirmin %TMPEXE%
@%TMPEXE% -v
@echo Continue with this binary? Only Ctrl+c aborts...
@pause

@REM Set an output file - used in alltest1.cmd and onetest.cmd
@set TMPTEST=temptest.txt
@echo %DATE% %TIME% > %TMPTEST%
@%TMPEXE% -v >> %TMPTEST%
@set TMPOPTS=--strict-tags-attributes no
@echo Adding options %TMPOPTS% >> %TMPTEST%

@call alltest2.cmd %TMPEXE% temp-6

@goto END

:NOEXE
@echo.
@echo Error: Can NOT locate %TMPEXE%! Has it been built?
@echo *** FIX ME *** setting the location of the EXE to use for the test
:END
