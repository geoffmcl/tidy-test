@setlocal
@set TIDY=F:\Projects\tidy-html5\build\issue-329\release\tidy.exe
@if NOT EXIST %TIDY% goto NOEXE

call t %*

@goto END

:NOEXE
@echo Can NOT locate %TIDY%! *** FIX ME ***
:END
