@echo off

pushd %~dp0\
call Code\Dependencies\premake\PremakeBinaries\premake5.exe vs2019
popd
PAUSE
