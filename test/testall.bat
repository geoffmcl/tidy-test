@setlocal
@REM Run tidy per testall.txt
@set TMPFIL=testall.txt
@if NOT EXIST %TMPFIL% goto NOLIST
@set TMPTIDY=tidy5
@if "%~1x" == "x" goto GOTCMD
@set TMPTIDY=%1
:GOTCMD

%TMPTIDY% -v
@if ERRORLEVEL 1 goto NOTIDY
@echo Above is version of tidy being used...

@set TMPCNT=0
@set TMPCNT2=0

@for /F %%i in (%TMPFIL%) do @(call :CNTIT %%i)
@if "%TMPCNT%" == "0" goto NOFILS
@echo Have %TMPCNT% files to process...

@for /F %%i in (%TMPFIL%) do @(call :DOIT %%i)


@goto END

:CNTIT
@set /a TMPCNT+=1
@goto :EOF

:DOIT
@set /a TMPCNT2+=1
@echo Doing %TMPCNT2% of %TMPCNT%: %1
@if %TMPCNT2% LSS 210 goto :EOF
%TMPTIDY% --tidy-mark no --show-info no %1
@echo Exit level %ERRORLEVEL%
@echo Done %TMPCNT2% of %TMPCNT%: %1
@choice /D Y /T 3 /M "Pausing for 3 seconds. Def=Y"
@if ERRORLEVEL 2 goto GOTNO
@goto :EOF

:GOTNO
@echo Use Ctrl+C to abort...
@pause
@goto GOTNO

:NOLIST
@echo Can NOT locate file %TMPFIL%! *** FIX ME ***
@goto END

:NOTIDY
@echo Can NOT run tidy %TMPTIDY%! *** Check name, location ***
@goto END

:NOFILS
@echo There appears no files in %TMPFIL%!
@goto END

:END

@REM eof
