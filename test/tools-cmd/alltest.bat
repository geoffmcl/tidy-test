@setlocal
@REM alltest.bat - execute all test cases
@REM ########################################################
@REM ### *** SET LOCATION OF TIDY EXE TO USE FOR TEST *** ###
@REM ########################################################
@REM ------------------------------------------------
@REM  Allow user to specify a different Tidy.
@REM ------------------------------------------------
@IF NOT "%~1" == "" (
    @echo Setting TY_TIDY_PATH to "%~1"
    @set TY_TIDY_PATH=%~1
)

@REM ------------------------------------------------
@REM  Setup the ENVIRONMENT.
@REM ------------------------------------------------
@call _environment.bat :set_environment
@REM  Show the established environment
@call _environment.bat :report_environment
@REM ------------------------------------------------
@REM  Check for HELP, in many forms...
@REM ------------------------------------------------
@if "%1" == "/help" goto USE
@if "%1" == "/h" goto USE
@if "%1" == "/?" goto USE
@if "%1" == "-h" goto USE
@if "%1" == "--help" goto USE
@if "%1" == "-?" goto USE
@REM ------------------------------------------------
@REM  Requirements checks
@REM ------------------------------------------------

@set TY_ONETEST=onetest.bat
@REM -----------------------------------------------
@if NOT EXIST %TY_EXPECTS_FILE% goto ERR0
@if NOT EXIST %TY_ONETEST% goto ERR3
@if NOT EXIST %TY_CASES_DIR%\nul goto ERR4

@REM  Set the output folder. We will actually build
@REM  into the standard output folder, and then move
@REM  them later if necessary.
@REM  Do not like this, but keep for now
set FINALOUT=%TY_RESULTS_DIR%

@IF NOT "%~2" == "" (
    @echo THIS OPTION NOT YET IMPLEMENTED! Sorry...
    @goto END
    @echo Will move final output to "%~2"
    @set FINALOUT=%~2
    @if EXIST %FINALOUT% goto ERR6
    @if EXIST %FINALOUT%\nul goto ERR6
)

@if NOT EXIST %TY_RESULTS_BASE_DIR%\nul md %TY_RESULTS_BASE_DIR%
@if NOT EXIST %TY_RESULTS_BASE_DIR%\nul goto ERR2

@if "%TY_TIDY_PATH%x" == "x" goto NOTP
%TY_TIDY_PATH% -v
@if ERRORLEVEL 1 goto NOEXE
@echo.
@echo Is this the correct version of tidy you want to use?
@echo The current test version file %TY_VERSION_FILE% show
@type %TY_VERSION_FILE%
@echo.
@pause
@REM ------------------------------------------------
@REM  Setup the report header
@REM ------------------------------------------------
@set TMPCNT=0
@for /F "tokens=1*" %%i in (%TY_EXPECTS_FILE%) do set /A TMPCNT+=1
@echo =============================== > %TY_RESULTS_FILE%
@echo Date %DATE% %TIME% >> %TY_RESULTS_FILE%
@echo Tidy EXE %TY_TIDY_PATH%, version >> %TY_RESULTS_FILE%
%TY_TIDY_PATH% -v >> %TY_RESULTS_FILE%
@echo Input list of %TMPCNT% tests from '%TY_EXPECTS_FILE%' file >> %TY_RESULTS_FILE%
@echo Outut will be to the '%FINALOUT%' folder >> %TY_RESULTS_FILE%
@echo =============================== >> %TY_RESULTS_FILE%

@echo Doing %TMPCNT% tests from '%TY_EXPECTS_FILE%' file...

@REM ------------------------------------------------
@REM  Perform the testing
@REM ------------------------------------------------
@set ERRTESTS=
@for /F "tokens=1*" %%i in (%TY_EXPECTS_FILE%) do @call onetest.bat %%i %%j

@REM ------------------------------------------------
@REM  Output failing test information
@REM ------------------------------------------------
@echo =============================== >> %TY_RESULTS_FILE%
@if "%ERRTESTS%." == "." goto NOERRS
@echo ERROR TESTS [%ERRTESTS%] ... FAILED TEST 1!
@echo ERROR TESTS [%ERRTESTS%] ... FAILED TEST 1! >> %TY_RESULTS_FILE%
@goto DONE1
:NOERRS
@echo Appears there are no changes in the exit value for %TMPCNT% tests - Success 1
@echo Appears there are no changes in the exit value for %TMPCNT% tests - Success 1 >> %TY_RESULTS_FILE%

:DONE1
@REM ------------------------------------------------
@REM  Final testing report
@REM ------------------------------------------------
@diff -v > nul
@if ERRORLEVEL 1 goto NODIFF
@echo.
@echo Running 'diff -ua %TY_EXPECTS_DIR% %TY_RESULTS_DIR%' ... output to log... 
@echo Running 'diff -ua %TY_EXPECTS_DIR% %TY_RESULTS_DIR%' >> %TY_RESULTS_FILE%
@diff -ua %TY_EXPECTS_DIR% %TY_RESULTS_DIR% >> %TY_RESULTS_FILE%
@if ERRORLEVEL 1 goto GOTDIF
@echo =============================== >> %TY_RESULTS_FILE%
@echo Appears there are no changes in the output files for %TMPCNT% tests - Success 2
@echo Appears there are no changes in the output files for %TMPCNT% tests - Success 2 >> %TY_RESULTS_FILE%
@goto DONE
:GOTDIF
@echo =============================== >> %TY_RESULTS_FILE%
@echo Appear to have differences in some output files for %TMPCNT% tests
@echo Appear to have differences in some output files for %TMPCNT% tests >> %TY_RESULTS_FILE%
@echo These results must be carefully checked in %TY_RESULTS_FILE% for details.
@echo.
@goto DONE
:NODIFF
@echo.
@echo Unable to run 'diff' in your environment! 
@echo Do you have a WIN32 port of GNU diff.exe from http://unxutils.sourceforge.net/
@echo Or use an alternative app to compare folder %TY_EXPECTS_DIR% %TY_RESULTS_DIR% ... 
@echo This is needed as the final compare to complete the tests...
@echo.
@echo Unable to run 'diff' in your environment!  >> %TY_RESULTS_FILE%
@echo Do you have a WIN32 port of GNU diff.exe from http://unxutils.sourceforge.net/  >> %TY_RESULTS_FILE%
@echo Or use an alternative app to compare folder %TY_EXPECTS_DIR% %TY_RESULTS_DIR% ...  >> %TY_RESULTS_FILE%
@echo This is needed as the final compare to complete the tests...  >> %TY_RESULTS_FILE%
:DONE
@echo End %DATE% %TIME% >> %TY_RESULTS_FILE%
@echo =============================== >> %TY_RESULTS_FILE%
@echo See %TY_RESULTS_FILE% for full test details...
@goto END

@REM ------------------------------------------------------------
@REM Errors encountered
@REM ------------------------------------------------------------

:ERR0
@echo ERROR: Can not locate '%TY_EXPECTS_FILE%'! Check name, and location ...
@goto END

:ERR2
echo ERROR: Can not create %TY_RESULTS_BASE_DIR% folder! Check name, and location ...
goto END

:ERR3
@echo ERROR: Can not locate '%TY_ONETEST%'! Check name, and location ...
@goto END

:ERR4
@echo ERROR: Can not locate '%TY_CASES_DIR%' folder! Check name, and location ...
@goto END

:ERR6
@echo ERROR: The output directory %FINALOUT% already exists!
@echo Either delete it, or choose another output directory.
@goto END

:NOTP
@echo.
@echo Error: TY_TIDY_PATH not set in environment!
@goto USE2

:NOEXE
@echo.
@echo Error: Can NOT locate %TMPEXE%! Has it been built?
@echo *** FIX ME *** adding the location of the EXE to use for the test
@echo.
@goto END

:USE
@REM FOr now always output this above
@REM echo.
@REM echo  Current environment
@REM call _environment.bat :report_environment
:USE2
@echo.
@echo  Usage of ALLTEST.BAT
@echo  AllTest [tidy.exe [Out_Folder]]
@echo  tidy.exe - This is the Tidy.exe you want to use for the test.
@echo  Out_Folder  - This is the FOLDER where you want the results put,
@echo  relative to the %TY_RESULTS_BASE_DIR% folder.
@echo  This folder will be created if it does not already exist.
@echo  These are both optional, but you must specify [tidy.exe] if you
@echo  wish to specify [Out_Folder].
@echo  ==================================
@echo  ALLTEST.BAT will run a battery of test files in the input folder
@echo  Each test name, has an expected result, given in its table.
@echo  There will be a warning if any test file fails to give this result.
@echo  ==================================
@echo  But the main purpose is to compare the 'results' of two version of
@echo  any two Tidy runtime exe's. Thus after you have two sets of results,
@echo  in separate folders, the idea is to compare these two folders.
@echo  Any directory compare utility will do, or you can download, and use
@echo  a WIN32 port of GNU diff.exe from http://unxutils.sourceforge.net/
@echo  ................................................................
@echo.
@goto END

:END
@REM EOF
