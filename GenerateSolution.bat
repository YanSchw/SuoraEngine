@echo off

pushd %~dp0\
call Code\Dependencies\premake\PremakeBinaries\premake5.exe vs2022
popd
PAUSE
