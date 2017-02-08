@echo off

rem # Setup a common environment for all batch scripts to use. This will ensure
rem # portability for all of the test scripts without having to figure out which
rem # items to always edit.

rem # This script is run at the start of all batch scripts; batch scripts can
rem # accept parameters to override these.


rem ###########################################################################
rem # Call our appropriate function.
rem #  Note: we're not going to take the Unix approach and embed too many
rem #  functions here. CMD doesn't have a source/include mechanism, and so
rem #  this file would be reloaded every time a function is called!
rem ###########################################################################
call %* 
exit /b


rem ###########################################################################
rem # Change these if necessary:
rem #   You should not call this function; it's used internally only.
rem ###########################################################################
:preset_environment

    if "%TY_TIDY_PATH%x" == "x" (
        set TY_TIDY_PATH=..\..\..\tidy-html5\build\cmake\release\tidy.exe
    )
    
    rem # Relative path from this script to the top-level tidy-html5-tests directory.
    set TY_PROJECT_ROOT_DIR=..

    rem # These are all relative from the TY_PROJECT_ROOT_DIR directory.
    set TY_CASES_BASE_DIR=cases
    set TY_RESULTS_BASE_DIR=cases

    rem # These are relative to the TY_CASES_BASE_DIR directory.
    set TY_TMP_DIR=_tmp

    rem # These are expected to be in cases_base_dir directory.
    set TY_VERSION_FILE=_version.txt

    rem # This file must exist in any test cases directory.
    set TY_CONFIG_DEFAULT=config_default.conf
    
    rem # The default cases set name
    set cases_setname=testbase

GOTO :EOF

rem ###########################################################################
rem # Unset environment variables.
rem ###########################################################################
:unset_environment

    set TY_PROJECT_ROOT_DIR=
    set TY_CASES_BASE_DIR=
    set TY_CASES_DIR=
    set TY_EXPECTS_DIR=
    set TY_EXPECTS_FILE=
    set TY_CONFIG_DEFAULT=
    set TY_VERSION_FILE=
    set TY_RESULTS_BASE_DIR=
    set TY_RESULTS_DIR=
    set TY_RESULTS_FILE=
    set TY_TMP_DIR=
    set TY_TMP_FILE=
    
    rem # TY_TIDY_PATH
    rem # TY_CASES_SETNAME
    
GOTO :EOF


rem ###########################################################################
rem # Set environment variables.
rem ###########################################################################
:set_environment

    call :preset_environment

    rem # We'll use this to get the full, absolute path.
    pushd %TY_PROJECT_ROOT_DIR%
    set TY_PROJECT_ROOT_DIR=%CD%
    popd
    
    rem # *Only* set TY_CASES_SETNAME if it's not already set!
    IF NOT DEFINED TY_CASES_SETNAME (set TY_CASES_SETNAME=%cases_setname%)

    set TY_PROJECT_ROOT_DIR=%TY_PROJECT_ROOT_DIR%
    set TY_CASES_BASE_DIR=%TY_PROJECT_ROOT_DIR%\%TY_CASES_BASE_DIR%
    set TY_CASES_DIR=%TY_CASES_BASE_DIR%\%TY_CASES_SETNAME%
    set TY_EXPECTS_DIR=%TY_CASES_BASE_DIR%\%TY_CASES_SETNAME%-expects
    set TY_EXPECTS_FILE=%TY_CASES_DIR%\_manifest.txt
    set TY_CONFIG_DEFAULT=%TY_CASES_DIR%\%TY_CONFIG_DEFAULT%
    set TY_VERSION_FILE=%TY_CASES_BASE_DIR%\%TY_VERSION_FILE%
    set TY_RESULTS_BASE_DIR=%TY_PROJECT_ROOT_DIR%\%TY_RESULTS_BASE_DIR%
    set TY_RESULTS_DIR=%TY_RESULTS_BASE_DIR%\%TY_CASES_SETNAME%-results
    set TY_RESULTS_FILE=%TY_RESULTS_BASE_DIR%\%TY_CASES_SETNAME%-results.txt
    set TY_TMP_DIR=%TY_RESULTS_BASE_DIR%\%TY_TMP_DIR%
    set TY_TMP_FILE=%TY_TMP_DIR%\temp.txt

    set cases_setname=

GOTO :EOF

rem ###########################################################################
rem # Print out the environment variables.
rem ###########################################################################
:report_environment

    echo        TY_TIDY_PATH = %TY_TIDY_PATH%
    echo TY_PROJECT_ROOT_DIR = %TY_PROJECT_ROOT_DIR%
    echo   TY_CASES_BASE_DIR = %TY_CASES_BASE_DIR%
    echo        TY_CASES_DIR = %TY_CASES_DIR%
    echo      TY_EXPECTS_DIR = %TY_EXPECTS_DIR%
    echo     TY_EXPECTS_FILE = %TY_EXPECTS_FILE%
    echo   TY_CONFIG_DEFAULT = %TY_CONFIG_DEFAULT%
    echo     TY_VERSION_FILE = %TY_VERSION_FILE%
    echo TY_RESULTS_BASE_DIR = %TY_RESULTS_BASE_DIR%
    echo      TY_RESULTS_DIR = %TY_RESULTS_DIR%
    echo     TY_RESULTS_FILE = %TY_RESULTS_FILE%
    echo          TY_TMP_DIR = %TY_TMP_DIR%
    echo         TY_TMP_FILE = %TY_TMP_FILE%

GOTO :EOF

REM EOF
