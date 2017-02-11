@setlocal

@set TMPOPTS=--show-body-only yes --show-info no

@REM Valid pair - no errors
@set TMPTST=483

@call :DOTEST
@echo See output temp%TMPTST%.html

@REM One of 32 out-of-range pairs - warning
@set TMPTST=483-1

@call :DOTEST
@echo See output temp%TMPTST%.html

@REM Leading surrogate pair, but no trainling - warning
@set TMPTST=483-2

@call :DOTEST
@echo See output temp%TMPTST%.html

@REM Trailing surrogate pair, but no leading - warning
@set TMPTST=483-3

@call :DOTEST
@echo See output temp%TMPTST%.html

@goto END

:DOTEST

call tr -o temp%TMPTST%.html %TMPOPTS% input5\in_%TMPTST%.html

@goto :EOF

:END
