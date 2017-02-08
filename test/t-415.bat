@setlocal
@set TMP_EXE=tidy5
@REM set TMP_FIL=input5\in_415.xml
@REM set TMP_OUT=temp415.xml
@REM set TMP_FIL=input5\in_415-1.xml
@set TMP_FIL=F:\Projects\tidy-test\test\input5\in_415-1.xml
@set TMP_OUT=temp415-1.xml
@set TMP_CMD=tempcmd.txt

@set TMP_OPS=-xml --preserve-entities no --show-info no --escape-cdata yes
@REM set TMP_OPS=-q -xml --preserve-entities no --output-encoding utf8

@%TMP_EXE% -? >nul 2>&1
@if ERRORLEVEL 1 goto NOEXE
@if NOT EXIST %TMP_FIL% goto NOFIL

@echo Doing: '%TMP_EXE% -o %TMP_OUT% %TMP_OPS% %TMP_FIL%'
@%TMP_EXE% -o %TMP_OUT% %TMP_OPS% %TMP_FIL%
@if ERRORLEVEL 1 goto GOTERR
@echo Tidy exited with no error level...
@goto DN_ERR
:GOTERR
@echo Note tidy ERROR level %ERRORLEVEL%
:DN_ERR

fa4 "Bilz" %TMP_OUT%

@goto END

:NOEXE
@echo Can NOT run '%TMP_EXE% -?`! *** FIX ME ***
@goto ISERR

:NOFIL
@echo Can NOT find '%TMP_FIL% -?`! *** FIX ME ***
@goto ISERR


:ISERR
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof
