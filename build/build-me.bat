@setlocal
@set TMPBGN=%TIME%

@set TMPPRJ=tidy-test
@echo Setup for 64-bit %TMPROJ% build
@set TMPLOG=bldlog-1.txt
@set TMPSRC=..
@set TMP3RD=D:\Projects\3rdParty.x64
@set ADDINST=0
@REM set BOOST_ROOT=X:\install\msvc100\boost
@REM if NOT EXIST %BOOST_ROOT%\nul goto NOBOOST

@call chkmsvc %TMPPRJ%
@REM call setupqt32
@REM if EXIST build-cmake.bat (
@REM call build-cmake
@REM if ERRORLEVEL 1 goto NOBCM
@REM )

@REM ###########################################
@REM NOTE: Specific install location
@REM ###########################################
@rem set TMPINST=%TMP3RD%
@set TMPINST=D:\Projects\install\MSVC140-64
@REM ###########################################

@REM Nothing below need be touched..
@REM if NOT EXIST F:\nul goto NOXD
@REM if NOT EXIST %TMPSRC%\nul goto NOSRC
@REM if NOT EXIST %BOOST_ROOT%\nul goto NOBOOST
@if NOT EXIST %TMPSRC%\CMakeLists.txt goto NOSRC2

@if NOT EXIST %TMP3RD%\nul goto NO3RD

@set TMPTIDY=D:\Projects\install\MSVC140-64\tidy-5.7.29.I884
@if NOT EXIST %TMPTIDY%\include\tidy.h goto NOTIDY

@set TMPOPTS=-DCMAKE_INSTALL_PREFIX=%TMPINST%
@REM set TMPOPTS=%TMPOPTS% -DCMAKE_PREFIX_PATH:PATH=%TMP3RD%
@set TMPOPTS=%TMPOPTS% -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON

:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPOPTS=%TMPOPTS% %1
@shift
@goto RPT
:GOTCMD

@echo Building %TMPPRJ% begin %DATE% %TMPBGN% > %TMPLOG%
@echo All output to %TMPLOG%...

@set TIDY_ROOT=%TMPTIDY%
@echo Set ENV TIDY_ROOT=%TIDY_ROOT% >> %TMPLOG%
@REM echo Set ENV BOOST_ROOT=%BOOST_ROOT% >> %TMPLOG%

@echo Doing 'cmake %TMPSRC% %TMPOPTS%' out to %TMPLOG%
@echo Doing 'cmake %TMPSRC% %TMPOPTS%' >> %TMPLOG%
@cmake %TMPSRC% %TMPOPTS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR1

@echo Doing 'cmake --build . --config Debug'
@echo Doing 'cmake --build . --config Debug'  >> %TMPLOG%
@cmake --build . --config Debug  >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR2

@echo Doing: 'cmake --build . --config Release'
@echo Doing: 'cmake --build . --config Release'  >> %TMPLOG%
@cmake --build . --config Release  >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR3

@REM type %TMPLOG%
@fa4 "***" %TMPLOG%
@echo.
@echo Appears a successful build... see %TMPLOG%
@call elapsed %TMPBGN%
@if "%ADDINST%" == "0" (
@echo.
@echo Install to %TMPINST% disabled at this time. Set ADDINST=1
@echo.
@goto END
)
@echo.
@echo Proceed with an install - Debug then Release
@echo.
@echo *** CONTINUE? *** Only Ctrl+C aborts... all other keys continue...
@echo.
@pause
@echo.
@echo Doing: 'cmake --build . --config Debug --target INSTALL'
@echo Doing: 'cmake --build . --config Debug --target INSTALL' >> %TMPLOG%
@cmake --build . --config Debug --target INSTALL >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR4
@echo.
@echo Doing: 'cmake --build . --config Release --target INSTALL'
@echo Doing: 'cmake --build . --config Release --target INSTALL' >> %TMPLOG% 2>&1
@cmake --build . --config Release --target INSTALL >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR5
@echo.
@echo.
@fa4 " -- " %TMPLOG%
@echo.
@call elapsed %TMPBGN%
@echo.
@echo All done %TMPPRJ%... build and install to %TMPINST%
@echo See %TMPLOG% for details...
@echo.
@goto END

:ERR1
@echo ERROR: Cmake config or geneation FAILED!
@goto ISERR

:ERR2
@echo ERROR: Cmake build Debug FAILED!
@goto ISERR

:ERR3
@echo ERROR: Cmake build Release FAILED!
@goto ISERR

:ERR4
@echo ERROR: Cmake install debug FAILED!
@goto ISERR

:ERR5
@echo ERROR: Cmake install release FAILED!
@goto ISERR

:NOXD
@echo Error: X:\ drive NOT found!
@goto ISERR
 
:NOSRC
@echo Error: No %TMPSRC% found!
@goto ISERR

:NO3RD
@echo Error: No directory %TMP3RD% found!
@goto ISERR

:NOTIDY
@echo Error: No directory %TMPTIDY% found!
@goto ISERR

:NOBOOST
@echo Error: Boost directory %BOOST_ROOT% not found!
@goto ISERR
 
:NOSRC2
@echo Error: File %TMPSRC%\CMakeLists.txt not found!
@goto ISERR

:NOBCM
@echo Error: Running build-cmake.bat caused an error!
@goto ISERR

:ISERR
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof

