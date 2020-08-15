@echo off
call premake\premake5.exe vs2019
popd
IF %ERRORLEVEL% NEQ 0 (
  PAUSE
)