@setlocal
@if "%~1x" == "x" (
@echo Usage: in-file "message"
@exit /b 1
)

@if NOT EXIST %1 (
@echo Can NOT locate %1
@exit /b 1
)
@if "%~2x" == "x" (
@echo No 2nd param - the message
@exit /b 1
)

call git add %1
@if ERRORLEVEL 1 (
@echo Failed to add %1!
@exit /b 1
)

call git commit -m %2 %1
@if ERRORLEVEL 1 (
@echo COmmit failed...
@exit /b 1
)

@echo Done %1 %2 ....

