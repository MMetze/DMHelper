@echo off
setlocal

rem Usage:
rem   preparebuilddirectory_msvc64.cmd <build-output-dir>
rem   Stages the x64 runtime payload into the provided build output directory.

if "%~1"=="" (
	echo Usage: %~nx0 ^<build-output-dir^>
	exit /b 1
)

xcopy /s /y ".\bin-win64\*" "%~1\"
xcopy /s /y ".\bestiary\*" "%~1\resources\"
xcopy /s /y ".\doc\*" "%~1\doc\"
xcopy /s /y ".\resources\*" "%~1\resources\"

set "QT_ROOT=C:\Qt\6.10.3\msvc2022_64"
if exist "%QT_ROOT%\bin\windeployqt.exe" (
	"%QT_ROOT%\bin\windeployqt.exe" --compiler-runtime --no-opengl-sw --no-svg "%~1\DMHelper.exe"
) else (
	echo Warning: windeployqt not found at %QT_ROOT%\bin\windeployqt.exe
)

endlocal

