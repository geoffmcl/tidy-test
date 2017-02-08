@setlocal
@set TMPFIL=%1
@if "%TMPFIL%x" == "x" goto HELP
@if NOT EXIST %TMPFIL% goto NOFIL
@set TMPPY=html5check.py
@if NOT EXIST %TMPPY% goto NOSCR
@python2 --version
@if ERRORLEVEL 1 goto NOPY


python2 %TMPPY% %TMPFIL%

@goto END

:NOSCR
@echo.
@echo Error: Can NOT locate script %TMPPY%! *** FIX ME ***
@echo.
@goto END

:NOPY
@echo.
@echo Error: Can NOT run 'python2 --version'! Has python been setup? Maybe need setuppython.bat first...
@echo.
@goto END

:NOFILE
@echo Error: Can NOT locate %TMPFIL%! Check name, location...
:HELP
@echo Give a valid file to pass to http://html5.validator.nu/
:END
