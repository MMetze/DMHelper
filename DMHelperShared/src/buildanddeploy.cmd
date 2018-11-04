:start
@set choice=
@set /p choice=Are you sure you would like to completely rebuild and redeploy the shared library? (y/n)
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
qmake.exe ..\src\DMHelperShared.pro -spec win32-g++
mingw32-make

:skip_build

xcopy .\release\DMHelperShared.dll ..\bin

cd ..
del src.zip
"C:\Program Files\7-Zip\7z" a -tzip src.zip src\* inc\*

:end
pause
