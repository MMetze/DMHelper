:start
@set choice=
@set /p choice=Are you sure you would like to completely rebuild and redeploy the application? (y/n)
if '%choice%'=='n' goto end
if not '%choice%'=='y' goto start

set PATH=C:\Qt\5.14.2\msvc2017_64\bin;C:\Qt\Tools\QtCreator\bin;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%

rmdir /s /q ..\bin64
mkdir ..\bin64
mkdir ..\bin64\bestiary
mkdir ..\bin64\doc
mkdir ..\bin64\pkgconfig
mkdir ..\bin64\plugins
mkdir ..\bin64\resources
mkdir ..\bin64\resources\tables

cd ..

rem Uncomment the following line to skip actually building the SW
rem goto skip_build

rmdir /s /q .\build-64_bit-release
mkdir build-64_bit-release
cd build-64_bit-release
@call vcvarsall.bat x64
@echo on
qmake.exe ..\src\DMHelper.pro -spec win32-msvc "CONFIG+=qtquickcompiler" 
jom.exe /d /f Makefile qmake_all
jom.exe /d /f Makefile.Release
goto build_done

:skip_build
cd build-64_bit-release

:build_done

xcopy .\release\DMHelper.exe ..\bin64\
xcopy C:\Qt\5.14.2\msvc2017_64\bin\Qt5Xml.dll ..\bin64\
xcopy ..\src\binsrc\* ..\bin64\*
xcopy /s ..\src\bestiary\* ..\bin64\resources\*
xcopy /s ..\src\doc\* ..\bin64\doc\*
xcopy /s ..\src\binsrc\pkgconfig\* ..\bin64\pkgconfig\*
xcopy /s ..\src\binsrc\plugins\* ..\bin64\plugins\*
xcopy /s ..\src\resources\* ..\bin64\resources\*

windeployqt --compiler-runtime --no-opengl-sw --no-angle --no-svg ..\bin64

cd ..\bin64
copy NUL DMHelper.log

rem del vc_redist.x64.exe

"C:\Program Files\7-Zip\7z" a -tzip archive.zip *
cd ..
move bin64\archive.zip .\archive.zip
del "DM Helper 64-bit release.zip"
rename archive.zip "DM Helper 64-bit release.zip"

del src.zip
"C:\Program Files\7-Zip\7z" a -tzip src.zip src\*

:end
pause
