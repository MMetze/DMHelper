@echo off
REM Launch PowerShell build script with temporary execution policy
REM powershell -NoProfile -ExecutionPolicy Bypass -File "./buildanddeploy_msvc64_cmake.ps1"
powershell -NoProfile -ExecutionPolicy Bypass -File "./buildanddeploy_msvc64_cmake.ps1" -SkipBuild
