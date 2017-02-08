@setlocal
@set TMPEXE=F:\Projects\tidy-html5\build\win64\Release\tidy.exe
@if NOT EXIST %TMPEXE% goto NOEXE

%TMPEXE% %*

@goto END

:NOEXE
@echo Can NOT locate %TMPEXE%
@goto END

:END

 
