@setlocal
@if "%~1x" == "x" (
@echo Usage: "message" in-file [in-file2 ...]
@exit /b 1
)

@set TMPMSG=%~1
@shift

@set TMPFIL=%1
@if NOT EXIST %1 (
@echo Can NOT locate file %1
@exit /b 1
)
@shift

:RPT
@if "%~1x" == "x" goto GOTCMD
@if NOT EXIST %1 (
@echo Can NOT locate file %1
@exit /b 1
)
@set TMPFIL=%TMPFIL% %1
@shift
@goto RPT

:GOTCMD

@echo Will do 'call git add %TMPFIL%'
@echo Followed by 'call git commit -m "%TMPMSG%" %TMPFIL%'

@pause

call git add %TMPFIL%
@if ERRORLEVEL 1 (
@echo Failed to add %TMPFIL%!
@exit /b 1
)

call git commit -m "%TMPMSG%" %TMPFIL%
@if ERRORLEVEL 1 (
@echo Commit failed...
@exit /b 1
)

@echo.
@echo Done "%TMPMSG%" %TMPFIL%

@REM eof

