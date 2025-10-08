@echo off

:: Clear previous run status
COLOR

set SHARPMAKE_EXECUTABLE=%~dp0Vendor\Sharpmake\Sharpmake.Application\bin\Release\net6.0\Sharpmake.Application.exe


pushd Vendor
pushd Sharpmake
:: First compile sharpmake to insure we are trying to deploy using an executable corresponding to the code.
if not exist %SHARPMAKE_EXECUTABLE% dotnet build Sharpmake.sln /p:Configuration=Release /p:Platform="Any CPU"
if %errorlevel% NEQ 0 goto error
popd
popd

echo sharpmake executable is built, fetching executable...

if not exist %SHARPMAKE_EXECUTABLE% echo Cannot find sharpmake executable in %~dp0%~dp0Vendor\Sharpmake\Sharpmake.Application\bin\Release\net6.0 & pause & goto error



echo sharpmake building solution...

call %SHARPMAKE_EXECUTABLE% /sources('ProjectDefinitions/SharpmakeEntrypoint/main.sharpmake.cs') /generateDebugSolution

pause
Exit

:error
@COLOR 4F
pause
