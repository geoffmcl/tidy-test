@setlocal
@REM #======================================================================
@REM # onetest.bat - execute a single test case
@REM #
@REM # (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
@REM # See tidy.c for the copyright notice.
@REM #
@REM # <URL:http://www.html-tidy.org/>
@REM #
@REM # used-by t1.bat, alltest.bat, xmltest.bat
@REM #======================================================================

@REM ------------------------------------------------
@REM  Requirements checks
@REM ------------------------------------------------
@if "%TY_TIDY_PATH%." == "." goto Err1
@%TY_TIDY_PATH% -v > nul
@if ERRORLEVEL 1 goto Err2
@if "%TY_RESULTS_BASE_DIR%." == "." goto Err3
@if NOT EXIST %TY_RESULTS_BASE_DIR%\nul goto Err4
@if NOT EXIST %TY_CASES_DIR%\nul goto Err5
@if "%TY_RESULTS_FILE%x" == "x" goto Err10

@if "%1x" == "x" goto Err8
@if "%2x" == "x" goto Err9

@REM ------------------------------------------------
@REM  Setup test parameters and files
@REM ------------------------------------------------
@set TESTNO=%1
@set EXPECTED=%2

@set INFILES==%TY_CASES_DIR%\case-%1.*ml
@set CFGFILE=%TY_CASES_DIR%\case-%1.conf

@set TIDYFILE=%TY_RESULTS_DIR%\case-%1.html
@set MSGFILE=%TY_RESULTS_DIR%\case-%1.txt

IF NOT EXIST %TY_RESULTS_DIR% mkdir %TY_RESULTS_DIR%

@REM set HTML_TIDY=

@REM If no test specific config file, use default.
@if NOT exist %CFGFILE% @set CFGFILE=%TY_CONFIG_DEFAULT%

@REM ------------------------------------------------
@REM  Get specific input file names, and check them
@REM ------------------------------------------------
@set INFILE=
for %%F in ( %INFILES% ) do @set INFILE=%%F 
@if "%INFILE%." == "." goto Err6
@if NOT EXIST %INFILE% goto Err7

@REM ------------------------------------------------
@REM  Remove any pre-exising test outputs
@REM ------------------------------------------------
@if exist %MSGFILE%  del %MSGFILE%
@if exist %TIDYFILE% del %TIDYFILE%

@REM ------------------------------------------------
@REM  Begin tidying and testing
@REM ------------------------------------------------
@echo Doing: '%TY_TIDY_PATH% -f %MSGFILE% -config %CFGFILE% %3 %4 %5 %6 %7 %8 %9 --tidy-mark no -o %TIDYFILE% %INFILE% >> %TY_RESULTS_FILE%

%TY_TIDY_PATH% -f %MSGFILE% -config %CFGFILE% %3 %4 %5 %6 %7 %8 %9 --tidy-mark no -o %TIDYFILE% %INFILE%
@set STATUS=%ERRORLEVEL%
@REM echo Testing %1, expect %EXPECTED%, got %STATUS%, msg %MSGFILE%
@REM echo Testing %1, expect %EXPECTED%, got %STATUS%, msg %MSGFILE% >> %TY_RESULTS_FILE%
@echo Testing %1, expect %EXPECTED%, got %STATUS%
@echo Testing %1, expect %EXPECTED%, got %STATUS% >> %TY_RESULTS_FILE%

@if %STATUS% EQU %EXPECTED% goto done
@set ERRTESTS=%ERRTESTS% %TESTNO%
@echo *** Failed - got %STATUS%, expected %EXPECTED% ***
@type %MSGFILE%
@echo *** Failed - got %STATUS%, expected %EXPECTED% *** >> %TY_RESULTS_FILE%
@type %MSGFILE% >> %TY_RESULTS_FILE%
@goto done

@REM ------------------------------------------------
@REM  Messages and Exception Handlers
@REM ------------------------------------------------
:Err1
@echo ==============================================================
@echo ERROR: exe not @set in TY_TIDY_PATH environment variable ...
@echo ==============================================================
@goto TRYAT

:Err2
@echo ==============================================================
@echo ERROR: Can not run exe %TY_TIDY_PATH%! Check name, location ...
@echo ==============================================================
@goto TRYAT

:Err3
@echo ====================================================================
@echo ERROR: output folder TY_RESULTS_BASE_DIR not @set in environment ...
@echo ====================================================================
@goto TRYAT

:Err4
@echo ==============================================================
@echo ERROR: output folder %TY_RESULTS_BASE_DIR% does not exist ...
@echo ==============================================================
@goto TRYAT

:Err5
@echo =======================================================================
@echo ERROR: input folder '%TY_CASES_DIR%' does not exist! Check name, location ..
@echo =======================================================================
@goto TRYAT

:TRYAT
@echo Try running alltest.bat %1 %2
@echo ==============================================================
@pause
@goto done

:Err6
@echo ==============================================================
@echo ERROR: Failed to find input matching '%INFILES%'!!!
@echo ==============================================================
@pause
@goto done

:Err7
@echo ==============================================================
@echo ERROR: Failed to find input file '%INFILE%'!!!
@echo ==============================================================
@pause
@goto done

:Err8
@echo.
@echo ERROR: No input test number given!
:Err9
@echo ERROR: No expected exit value given!
@echo.
@goto done

:Err10
@echo ERROR: TY_RESULTS_FILE not @set in evironment!
@echo.
@goto done


:done

@REM EOF
