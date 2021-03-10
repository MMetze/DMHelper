:start
@set choice=
@set /p choice=Are you sure you would like to completely rebuild and redeploy the CLIENT application? (y/n)
if '%choice%'=='n' goto end
if not '%choice%'=='y' goto start

set QT_DIR=C:\Qt
set QT_VERSION=5.15.2
set QT_INSTALLER_VERSION=4.0
set MSVC_VERSION=2019
set SEVENZIP_APP=C:\Program Files\7-Zip\7z

set PATH=%QT_DIR%\%QT_VERSION%\msvc%MSVC_VERSION%_64\bin;%QT_DIR%\Tools\QtInstallerFramework\%QT_INSTALLER_VERSION%\bin;%QT_DIR%\Tools\QtCreator\bin;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%

rmdir /s /q ..\bin64-client
mkdir ..\bin64-client
mkdir ..\bin64-client\config
mkdir ..\bin64-client\packages
mkdir ..\bin64-client\packages\com.dmhelperclient.app
mkdir ..\bin64-client\packages\com.dmhelperclient.app\meta
mkdir ..\bin64-client\packages\com.dmhelperclient.app\data
mkdir ..\bin64-client\packages\com.dmhelperclient.app\data\bestiary
mkdir ..\bin64-client\packages\com.dmhelperclient.app\data\doc
mkdir ..\bin64-client\packages\com.dmhelperclient.app\data\pkgconfig
mkdir ..\bin64-client\packages\com.dmhelperclient.app\data\plugins
mkdir ..\bin64-client\packages\com.dmhelperclient.app\data\resources
mkdir ..\bin64-client\packages\com.dmhelperclient.app\data\resources\tables
xcopy /s .\installer-client\* ..\bin64-client\*

cd ..

rem Uncomment the following line to skip actually building the SW
rem goto skip_build

rmdir /s /q .\build-client-64_bit-release
mkdir build-client-64_bit-release
cd build-client-64_bit-release
call vcvarsall.bat x64
@echo on
qmake.exe ..\src\DMHelperClient.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
jom.exe /d /f Makefile qmake_all
jom.exe /d /f Makefile.Release
goto build_done

:skip_build
cd build-client-64_bit-release

:build_done

xcopy .\release\DMHelperClient.exe ..\bin64-client\packages\com.dmhelperclient.app\data\
xcopy %QT_DIR%\%QT_VERSION%\msvc%MSVC_VERSION%_64\bin\Qt5Xml.dll ..\bin64-client\packages\com.dmhelperclient.app\data\
xcopy ..\src\binsrc\* ..\bin64-client\packages\com.dmhelperclient.app\data\*
rem Uncomment this when we have documentation for the client app!
rem xcopy /s ..\src\doc\* ..\bin64\packages\com.dmhelper.app\data\doc\*
xcopy /s ..\src\binsrc\pkgconfig\* ..\bin64-client\packages\com.dmhelperclient.app\data\pkgconfig\*
xcopy /s ..\src\binsrc\plugins\* ..\bin64-client\packages\com.dmhelperclient.app\data\plugins\*

windeployqt --compiler-runtime --no-opengl-sw --no-angle --no-svg ..\bin64-client\packages\com.dmhelperclient.app\data

rem Create the installer
cd ..\bin64-client
binarycreator -c config\config_win64.xml -p packages "DMHelper Client 64-bit release Installer"
cd ..
move ".\bin64-client\DMHelper Client 64-bit release Installer.exe" ".\DMHelper Client 64-bit release Installer.exe"

rem Create the zip-file distribution
"%SEVENZIP_APP%" a -tzip archive.zip .\bin64\packages\com.dmhelperclient.app\data\*
del "DMHelper Client 64-bit release.zip"
rename archive.zip "DMHelper Client 64-bit release.zip"

:end
pause
