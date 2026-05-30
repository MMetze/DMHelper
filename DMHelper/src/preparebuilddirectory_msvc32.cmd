@echo off
setlocal

rem Usage:
rem   preparebuilddirectory_msvc32.cmd <build-output-dir>
rem   Stages the 32-bit runtime payload into the provided build output directory.

if "%~1"=="" (
	echo Usage: %~nx0 ^<build-output-dir^>
	exit /b 1
)

xcopy /s /y ".\bin-win32\*" "%~1\debug\"
xcopy /s /y ".\bestiary\*" "%~1\debug\resources\"
xcopy /s /y ".\doc\*" "%~1\debug\doc\"
xcopy /s /y ".\resources\*" "%~1\debug\resources\"

set "QT_ROOT=C:\Qt\6.11.1\msvc2022_64"
if exist "%QT_ROOT%\bin\windeployqt.exe" (
	"%QT_ROOT%\bin\windeployqt.exe" --compiler-runtime --no-opengl-sw --no-svg "%~1\debug\DMHelper.exe"
) else (
	echo Warning: windeployqt not found at %QT_ROOT%\bin\windeployqt.exe
)

endlocal

