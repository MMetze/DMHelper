:start
@set choice=
@set /p choice=Are you sure you would like to completely rebuild and redeploy the application? (y/n)
if '%choice%'=='n' goto end
if not '%choice%'=='y' goto start

set PATH=C:\Qt\5.11.1\mingw53_32\bin;C:\Qt\Tools\mingw530_32\bin;%PATH%

rmdir /s /q ..\bin
mkdir ..\bin

cd ..

rem Uncomment the following line to skip actually building the SW
rem goto skip_build

rmdir /s /q .\build-release
mkdir build-release
cd build-release
qmake.exe ..\src\DMHelper.pro -spec win32-g++
mingw32-make
goto build_done

:skip_build
cd build-release

:build_done

xcopy .\release\DMHelper.exe ..\bin\
xcopy ..\..\DMHelperShared\bin\DMHelperShared.dll ..\bin\
xcopy C:\Qt\5.11.1\mingw53_32\bin\Qt5Xml.dll ..\bin\
xcopy ..\src\binsrc\* ..\bin\*

windeployqt --compiler-runtime --no-opengl-sw --no-angle --no-svg ..\bin

cd ..\bin
copy NUL DMHelper.log
rem NOT NEEDED rename "DMHelper.exe" "DMHelper.__exe"
"C:\Program Files\7-Zip\7z" a -tzip archive.zip *
cd ..
move bin\archive.zip .\archive.zip
del "DM Helper release.zip"
rename archive.zip "DM Helper release.zip"

del src.zip
"C:\Program Files\7-Zip\7z" a -tzip src.zip src\*

:end
pause
