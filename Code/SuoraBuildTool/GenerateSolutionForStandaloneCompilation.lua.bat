@echo off

pushd %~dp0\
call ..\Dependencies\premake\PremakeBinaries\premake5.exe vs2022
popd
PAUSE
