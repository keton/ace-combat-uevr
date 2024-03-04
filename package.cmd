@echo off

REM change path to script direcotry
%~d0
cd %~dp0

REM mkdir is mkdir -p equivalent with extensions
setlocal enableextensions

mkdir build\Release\profile\plugins
copy profile_template\* build\Release\profile
copy build\Release\ace_combat_plugin.dll build\Release\profile\plugins

del /q Ace7Game.zip

7z a Ace7Game.zip .\build\Release\profile\*
