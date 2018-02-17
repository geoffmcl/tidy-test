@setlocal
@REM copy the EXE into C:\MDOS, IFF changed
@REM Note use a 'dirmin.bat' which essentially does a 'dir',
@REM but remove all the extra lines - not essential is missing
@REM Uses 'fc4.exe' to compare binaries - not essential if missing
@set TMPDIR=C:\MDOS
@set TMPFIL=load-dll.exe
@set TMPSRC=Release\%TMPFIL%
@set TMPDST=%TMPDIR%\%TMPFIL%

@REM Check destination...
@if NOT EXIST %TMPDIR%\nul (
@echo Destination dir %TMPDIR% does NOT exist! *** FIX ME ***
@exit /b 1
)

@REM Check SOURCE
@if NOT EXIST %TMPSRC% (
@echo Source file %TMPSRC% does NOT exist! *** FIX ME ***
@exit /b 1
)

@call dirmin %TMPSRC%

@call :CHKCOPY

@goto END

:CHKCOPY
@if NOT EXIST %TMPDST% goto DOCOPY

@echo Current destination %TMPDST%
@call dirmin %TMPDST%

@REM Compare
@fc4 -q -v0 -b %TMPSRC% %TMPDST% >nul
@if ERRORLEVEL 2 goto NOFC4
@if ERRORLEVEL 1 goto DOCOPY
@echo.
@echo Files are the SAME... Nothing done...
@echo.
@goto :EOF

:NOFC4
@echo Can NOT run fc4! so doing copy...
:DOCOPY
copy %TMPSRC% %TMPDST%
@if NOT EXIST %TMPDST% goto ERR3
@call dirmin %TMPDST%
@echo.
@echo Done file update...
@echo.
@goto :EOF

:ERR3
@echo.
@echo An ERROR in copying! Unknown! *** FIX ME ***
@echo.

:END

@REM eof

