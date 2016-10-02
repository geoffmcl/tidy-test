@setlocal
@set TMPBGN=%TIME%
@set VCVERS=14
@set TMPPRJ=tidy-test
@echo Setup for 64-bit %TMPROJ% build
@set TMPLOG=bldlog-1.txt
@set TMPSRC=..
@set TMP3RD=F:\Projects\software
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
@set TMPINST=F:\Projects\software.x64
@REM ###########################################
@REM ############################################
@REM NOTE: MSVC 10 INSTALL LOCATION
@REM Adjust to suit your environment
@REM ##########################################
@set GENERATOR=Visual Studio %VCVERS% Win64
@set VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio %VCVERS%.0
@set VC_BAT=%VS_PATH%\VC\vcvarsall.bat
@if NOT EXIST "%VS_PATH%" goto NOVS
@if NOT EXIST "%VC_BAT%" goto NOBAT
@set BUILD_BITS=%PROCESSOR_ARCHITECTURE%
@IF /i %BUILD_BITS% EQU x86_amd64 (
    @set "RDPARTY_ARCH=x64"
) ELSE (
    @IF /i %BUILD_BITS% EQU amd64 (
        @set "RDPARTY_ARCH=x64"
    ) ELSE (
        @echo Appears system is NOT 'x86_amd64', nor 'amd64'
        @echo Can NOT build the 64-bit version! Aborting
        @exit /b 1
    )
)
@set CURRDIR=%CD%
@echo Setting environment - CALL "%VC_BAT%" %BUILD_BITS%
@call "%VC_BAT%" %BUILD_BITS%
@if ERRORLEVEL 1 goto NOSETUP
@cd %CURRDIR%

@REM Nothing below need be touched..
@REM if NOT EXIST F:\nul goto NOXD
@REM if NOT EXIST %TMPSRC%\nul goto NOSRC
@REM if NOT EXIST %BOOST_ROOT%\nul goto NOBOOST
@if NOT EXIST %TMPSRC%\CMakeLists.txt goto NOSRC2

@REM if NOT EXIST %TMP3RD%\nul goto NO3RD
@set TMPOPTS=-G "%GENERATOR%" -DCMAKE_INSTALL_PREFIX=%TMPINST%
@REM set TMPOPTS=%TMPOPTS% -DCMAKE_PREFIX_PATH:PATH=%TMP3RD%

:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPOPTS=%TMPOPTS% %1
@shift
@goto RPT
:GOTCMD

@echo Building %TMPPRJ% begin %TMPBGN% > %TMPLOG%
@echo All output to %TMPLOG%...

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
@echo Erro: No directory %TMP3RD% found!
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

