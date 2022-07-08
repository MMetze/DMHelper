@set DIR_MSG="Running build script from %cd%"
@CALL :Message2 "Building DMHelper for Windows x86_64" %DIR_MSG%

:start
@set choice=
@set /p choice=Are you sure you would like to completely rebuild and redeploy the application? (y/n)
if '%choice%'=='n' goto end
if not '%choice%'=='y' goto start

set QT_DIR=C:\Qt
set QT_VERSION=5.15.2
set QT_INSTALLER_VERSION=4.3
set MSVC_VERSION=2019
set SEVENZIP_APP=C:\Program Files\7-Zip\7z
set PATH=%QT_DIR%\%QT_VERSION%\msvc%MSVC_VERSION%_64\bin;%QT_DIR%\Tools\QtInstallerFramework\%QT_INSTALLER_VERSION%\bin;%QT_DIR%\Tools\QtCreator\bin;%QT_DIR%\Tools\QtCreator\bin\jom;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%

rmdir /s /q ..\bin64
mkdir ..\bin64
mkdir ..\bin64\config
mkdir ..\bin64\packages
mkdir ..\bin64\packages\com.dmhelper.app
mkdir ..\bin64\packages\com.dmhelper.app\meta
mkdir ..\bin64\packages\com.dmhelper.app\data
mkdir ..\bin64\packages\com.dmhelper.app\data\bestiary
mkdir ..\bin64\packages\com.dmhelper.app\data\doc
mkdir ..\bin64\packages\com.dmhelper.app\data\pkgconfig
mkdir ..\bin64\packages\com.dmhelper.app\data\plugins
mkdir ..\bin64\packages\com.dmhelper.app\data\resources
mkdir ..\bin64\packages\com.dmhelper.app\data\resources\tables
xcopy /s .\installer\* ..\bin64\*
rename ..\bin64\packages\com.dmhelper.app\meta\installscript64.qs installscript.qs

cd ..

rem Uncomment the following line to skip actually building the SW
rem goto skip_build

@CALL :Message "Compiling DMHelper"

rmdir /s /q .\build-64_bit-release
mkdir build-64_bit-release
cd build-64_bit-release
call vcvarsall.bat x64
@echo on
qmake.exe ..\src\DMHelper.pro -spec win32-msvc "CONFIG+=qtquickcompiler" 
jom.exe /d /f Makefile qmake_all
jom.exe /d /f Makefile.Release
goto build_done

:skip_build
cd build-64_bit-release

:build_done
@CALL :Message "Copy resource content"
xcopy .\release\DMHelper.exe ..\bin64\packages\com.dmhelper.app\data\
xcopy %QT_DIR%\%QT_VERSION%\msvc%MSVC_VERSION%_64\bin\Qt5Xml.dll ..\bin64\packages\com.dmhelper.app\data\
xcopy ..\src\bin-win64\* ..\bin64\packages\com.dmhelper.app\data\*
xcopy /s ..\src\bestiary\* ..\bin64\packages\com.dmhelper.app\data\resources\*
xcopy /s ..\src\doc\* ..\bin64\packages\com.dmhelper.app\data\doc\*
xcopy /s ..\src\bin-win64\pkgconfig\* ..\bin64\packages\com.dmhelper.app\data\pkgconfig\*
xcopy /s ..\src\bin-win64\plugins\* ..\bin64\packages\com.dmhelper.app\data\plugins\*
xcopy /s ..\src\resources\* ..\bin64\packages\com.dmhelper.app\data\resources\*

windeployqt --compiler-runtime --no-opengl-sw --no-angle --no-svg ..\bin64\packages\com.dmhelper.app\data

@CALL :Message "Create the installer"
cd ..\bin64
binarycreator -c config\config_win64.xml -p packages "DMHelper 64-bit release Installer"
cd ..
move ".\bin64\DMHelper 64-bit release Installer.exe" ".\DMHelper 64-bit release Installer.exe"

@CALL :Message "Create the zip-file distribution"
"%SEVENZIP_APP%" a -tzip archive.zip .\bin64\packages\com.dmhelper.app\data\*
del "DMHelper 64-bit release.zip"
rename archive.zip "DMHelper 64-bit release.zip"

:end
@pause
@EXIT /B 0

:Message
@echo ................................................................................
@echo %~1
@echo ................................................................................
@EXIT /B 0

:Message2
@echo ................................................................................
@echo %~1
@echo %~2
@echo ................................................................................
@EXIT /B 0
