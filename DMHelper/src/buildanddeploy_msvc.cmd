:start
@set choice=
@set /p choice=Are you sure you would like to completely rebuild and redeploy the application? (y/n)
if '%choice%'=='n' goto end
if not '%choice%'=='y' goto start

set PATH=C:\Qt\5.13.0\msvc2017_64\bin;C:\Qt\Tools\QtCreator\bin;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build;%PATH%
@rem set PATH=C:\Qt\5.13.0\msvc2017_64\bin;C:\Qt\Tools\QtCreator\bin;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.16.27023\bin\HostX64\x64;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\VC\VCPackages;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\TestWindow;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\bin\Roslyn;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Team Tools\Performance Tools\x64;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Team Tools\Performance Tools;C:\Program Files (x86)\Microsoft Visual Studio\Shared\Common\VSPerfCollectionTools\x64;C:\Program Files (x86)\Microsoft Visual Studio\Shared\Common\VSPerfCollectionTools\;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\bin;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%

rmdir /s /q ..\bin64
mkdir ..\bin64
mkdir ..\bin64\bestiary
mkdir ..\bin64\doc

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
xcopy C:\Qt\5.13.0\msvc2017_64\bin\Qt5Xml.dll ..\bin64\
xcopy ..\src\binsrc\* ..\bin64\*
xcopy /s ..\src\bestiary\* ..\bin64\bestiary\*
xcopy /s ..\src\doc\* ..\bin64\doc\*

windeployqt --compiler-runtime --no-opengl-sw --no-angle --no-svg ..\bin64

cd ..\bin64
copy NUL DMHelper.log
"C:\Program Files\7-Zip\7z" a -tzip archive.zip *
cd ..
move bin64\archive.zip .\archive.zip
del "DM Helper 64-bit release.zip"
rename archive.zip "DM Helper 64-bit release.zip"

del src.zip
"C:\Program Files\7-Zip\7z" a -tzip src.zip src\*

:end
pause
