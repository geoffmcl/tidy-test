@setlocal
@REM ################################
@REM Adjust for your environment
@REM Use the 'latest' win64 version
@set TMPEXE=..\..\tidy-html5\build\win64\Release\tidy.exe
@set TMPIND=input5

@REM This test could be skipped, but for now...
@if NOT EXIST %TMPEXE% goto NOEXE
@set TMPFIL=%TEMP%\temptidy.txt
@%TMPEXE% -v > %TMPFIL%
@if ERRORLEVEL 1 goto NOTIDY
@set /p TMPVER=<%TMPFIL%
@echo tr: Version: %TMPVER%
@REM call dirmin %TMPEXE%
@if "%~1x" == "x" goto HELP

@REM ################################
@REM Check if 'tail' can be run
@set HVTAIL=type
@tail --help >nul 2>&1
@if ERRORLEVEL 1 goto DNTAIL
@set HVTAIL=tail
:DNTAIL
@REM ###############################

@REM Set/Clear all variables
@set TMPOUT=
@set TMPERR=
@set TMPTAIL=
@set TMPCMD=
@set TMPEFIL=
@set TMPHTML=

:RPT
@if "%~1x" == "x" goto GOTCMD
@if EXIST %1 goto ADDFILE
@set TMPFIL=%TMPIND%\in_%1.html
@if NOT EXIST %TMPFIL% (
@set TMPFIL=%TMPIND%\in_%1.xml
)
@set TMPCFG=%TMPIND%\cfg_%1.txt
@if NOT EXIST %TMPFIL% goto ADDCMD
@if EXIST %TMPCFG% goto SETTAIL
@set TMPCFG=%TMPIND%\cfg_def.txt
:SETTAIL
@REM Check BOTH files EXIST
@if NOT EXIST %TMPCFG% goto ADDCMD
@if NOT EXIST %TMPFIL% goto ADDCMD
@REM Given a TEST input, add to TAIL of commands
@set TMPTAIL=%TMPTAIL% -config %TMPCFG% %TMPFIL%
@set TMPHTML=temp%1.html
@set TMPOUT=-o %TMPHTML%
@set TMPEFIL=temp%1.txt
@set TMPERR=-f %TMPEFIL%
@shift
@if EXIST %TMPEFIL% @del %TMPEFIL%
@goto RPT

:ADDFILE
@REM Given a file input, add to TAIL of commands
@set TMPTAIL=%TMPTAIL% %1
@shift
@goto RPT

:ADDCMD
@set TMPCMD=%TMPCMD% %1
@shift
@goto RPT
:GOTCMD

@echo Run: %TMPEXE% %TMPERR% %TMPOUT% %TMPCMD% %TMPTAIL%
@%TMPEXE% %TMPERR% %TMPOUT% %TMPCMD% %TMPTAIL%
@if ERRORLEVEL 3 goto TIDYERR

@REM #####################
@REM Show the error file
@if "%TMPEFIL%x" == "x" goto DNERRFIL
@if EXIST %TMPEFIL% (
@echo.
@echo tr: %HVTAIL% Error file %TMPEFIL%
@%HVTAIL% %TMPEFIL%
@echo tr: %HVTAIL% End Error file %TMPEFIL%
)
:DNERRFIL
@if "%TMPHTML%x" == "x" goto DNHTMFIL
@if EXIST %TMPHTML% (
@echo tr: Check OUTPUT: %TMPHTML%
)
:DNHTMFIL
@goto END

:TIDYERR
@echo Tidy exited with error %ERRORLEVEL%
@goto END

:NOEXE
@echo Can NOT locate %TMPEXE%! *** FIX ME ***
@goto END

:NOTIDY
@echo Can NOT run %TMPEXE%! *** FIX ME ***
@goto END

:HELP
@echo.
@echo Must at least give an input html file to parse
@echo Alternatively, give an issue number


:END
