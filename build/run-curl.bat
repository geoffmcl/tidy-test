@setlocal
@set TMPEXE=Release\htmltidy.exe
@if NOT EXIST %TMPEXE% goto NOEXE
@set TMP3RD=F:\Projects\software\bin
@if NOT EXIST %TMP3RD%\nul goto NO3RD
@set TMPCMD=
:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPCMD=%TMPCMD% %1
@shift
@goto RPT
:GOTCMD

@set PATH=%TMP3RD%;%PATH%

%TMPEXE% %TMPCMD%

@goto END

:NOEXE
@echo Can NOT locate %TMPEXE%! *** FIX ME *** to the location of the EXE
@goto END

:NO3RD
@echo Can NOT locate %TMP3RD%! *** FIX ME *** to the location of the tidy DLL
@goto END

:END

@REM eof
