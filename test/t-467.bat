@setlocal

@set TMPFIL=input5\in_467-4.html
@if NOT EXIST %TMPFIL% goto NOFIL

@REM set TMPOPTS=-raw -o tempraw1.html -f temperr1.txt --force-output yes %*
@set TMPOPTS=-raw -o temputf1.html -f temperr2.txt --force-output yes --output-encoding utf8 %*

tidy5 %TMPOPTS% %TMPFIL%

@goto END

:NOFIL
@echo NOT EXIST %TMPFIL%! *** FIX ME ***
:END
