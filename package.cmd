@echo off

REM change path to script direcotry
%~d0
cd %~dp0

REM mkdir is mkdir -p equivalent with extensions
setlocal enableextensions

mkdir build\Release\profile\plugins
copy profile_template\* build\Release\profile
copy build\Release\pacific_drive_plugin.dll build\Release\profile\plugins

del /q PenDriverPro-Win64-Shipping.zip

7z a PenDriverPro-Win64-Shipping.zip .\build\Release\profile\*
