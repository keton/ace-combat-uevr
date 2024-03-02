@echo off

REM change path to script direcotry
%~d0
cd %~dp0

rmdir /S /Q build

"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 & cmake -S . -B build ./build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release & cmake --build ./build --clean-first --config Release --target pacific_drive_plugin
