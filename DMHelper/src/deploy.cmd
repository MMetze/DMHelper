rmdir /s ..\bin
mkdir ..\bin
xcopy ..\build-release\DMHelper.exe ..\bin
xcopy .\binsrc\* ..\bin

C:\Qt\5.8\mingw53_32\bin\windeployqt --compiler-runtime --no-opengl-sw --no-angle --no-svg ..\bin

rem not needed?
rem xcopy C:\Qt\5.8\mingw53_32\bin\libgcc_s_dw2-1.dll ..\bin
rem xcopy C:\Qt\5.8\mingw53_32\bin\libstdc++-6.dll ..\bin
rem xcopy C:\Qt\5.8\mingw53_32\bin\libwinpthread-1.dll ..\bin

@pause
