:start
@set choice=
@set /p choice=Are you sure you would like to completely rebuild and redeploy the application? (y/n)
if '%choice%'=='n' goto end
if not '%choice%'=='y' goto start

set PATH=C:\Qt\5.12.3\mingw73_64\bin;C:\Qt\Tools\mingw730_64\bin;%PATH%

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
qmake.exe ..\src\DMHelper.pro -spec win32-g++
mingw32-make
goto build_done

:skip_build
cd build-64_bit-release

:build_done

xcopy .\release\DMHelper.exe ..\bin64\
xcopy C:\Qt\5.12.3\mingw73_64\bin\Qt5Xml.dll ..\bin64\
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
