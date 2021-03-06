@setlocal
@set TMPEXE=F:\Projects\tidy-html5\build\temp\Release\tidy.exe
@if NOT EXIST %TMPEXE% goto NOEXE
@echo Tidy version -
%TMPEXE% -v
@call dirmin %TMPEXE%

@set TMPOUT=
@set TMPERR=
@set TMPTAIL=
@set TMPCMD=
@set TMPEFIL=
:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPFIL=input5\in_%1.html
@if NOT EXIST %TMPFIL% (
@set TMPFIL=input5\in_%1.xml
)
@set TMPCFG=input5\cfg_%1.txt
@if NOT EXIST %TMPFIL% goto ADDCMD
@if EXIST %TMPCFG% goto SETTAIL
@set TMPCFG=input5\cfg_def.txt
:SETTAIL
@set TMPTAIL=-config %TMPCFG% %TMPFIL%
@set TMPOUT=-o temp%1.html
@set TMPEFIL=temp%1.txt
@set TMPERR=-f %TMPEFIL%
@shift
@goto RPT

:ADDCMD
@set TMPCMD=%TMPCMD% %1
@shift
@goto RPT
:GOTCMD

%TMPEXE% %TMPERR% %TMPOUT% %TMPCMD% %TMPTAIL%

@if "%TMPEFIL%x" == "x" goto END
@if EXIST %TMPEFIL% (
@echo.
@echo Error file %TMPEFIL%
type %TMPEFIL%
@echo End Error file %TMPEFIL%
)

@goto END

:NOEXE
@echo Can NOT locate %TMPEXE%! *** FIX ME ***
:END
