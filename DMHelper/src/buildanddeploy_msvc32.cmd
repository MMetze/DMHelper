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
set PATH=%QT_DIR%\%QT_VERSION%\msvc%MSVC_VERSION%\bin;%QT_DIR%\Tools\QtInstallerFramework\%QT_INSTALLER_VERSION%\bin;%QT_DIR%\Tools\QtCreator\bin;%QT_DIR%\Tools\QtCreator\bin\jom;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%

rmdir /s /q ..\bin32
mkdir ..\bin32
mkdir ..\bin32\config
mkdir ..\bin32\packages
mkdir ..\bin32\packages\com.dmhelper.app
mkdir ..\bin32\packages\com.dmhelper.app\meta
mkdir ..\bin32\packages\com.dmhelper.app\data
mkdir ..\bin32\packages\com.dmhelper.app\data\bestiary
mkdir ..\bin32\packages\com.dmhelper.app\data\doc
mkdir ..\bin32\packages\com.dmhelper.app\data\pkgconfig
mkdir ..\bin32\packages\com.dmhelper.app\data\plugins
mkdir ..\bin32\packages\com.dmhelper.app\data\resources
mkdir ..\bin32\packages\com.dmhelper.app\data\resources\tables
xcopy /s .\installer\* ..\bin32\*

cd ..

rem Uncomment the following line to skip actually building the SW
rem goto skip_build

rmdir /s /q .\build-32_bit-release
mkdir build-32_bit-release
cd build-32_bit-release
call vcvarsall.bat x86
@echo on
qmake.exe ..\src\DMHelper.pro -spec win32-msvc "CONFIG+=qtquickcompiler" 
jom.exe /d /f Makefile qmake_all
jom.exe /d /f Makefile.Release
goto build_done

:skip_build
cd build-32_bit-release

:build_done

xcopy .\release\DMHelper.exe ..\bin32\packages\com.dmhelper.app\data\
xcopy %QT_DIR%\%QT_VERSION%\msvc%MSVC_VERSION%\bin\Qt5Xml.dll ..\bin32\packages\com.dmhelper.app\data\
xcopy ..\src\binsrc\* ..\bin32\packages\com.dmhelper.app\data\*
xcopy /s ..\src\bestiary\* ..\bin32\packages\com.dmhelper.app\data\resources\*
xcopy /s ..\src\doc\* ..\bin32\packages\com.dmhelper.app\data\doc\*
xcopy /s ..\src\binsrc\pkgconfig\* ..\bin32\packages\com.dmhelper.app\data\pkgconfig\*
xcopy /s ..\src\binsrc\plugins\* ..\bin32\packages\com.dmhelper.app\data\plugins\*
xcopy /s ..\src\resources\* ..\bin32\packages\com.dmhelper.app\data\resources\*

windeployqt --compiler-runtime --no-opengl-sw --no-angle --no-svg ..\bin32\packages\com.dmhelper.app\data

rem Create the installer
cd ..\bin32
binarycreator -c config\config_win32.xml -p packages "DMHelper 32-bit release Installer"
cd ..
move ".\bin32\DMHelper 32-bit release Installer.exe" ".\DMHelper 32-bit release Installer.exe"

rem Create the zip-file distribution
"%SEVENZIP_APP%" a -tzip archive.zip .\bin32\packages\com.dmhelper.app\data\*
del "DMHelper 32-bit release.zip"
rename archive.zip "DMHelper 32-bit release.zip"

:end
pause
