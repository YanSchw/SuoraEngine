@echo off
setlocal enableextensions

REM Generate Project Files for SuoraBuildTool
call "Code\SuoraBuildTool\GenerateSolutionForStandaloneCompilation.bat"

REM Make Directory for SuoraBuildTool.exe
mkdir "\Binaries\Scripts\"

set MSBUILD_EXE="C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\MSBuild.exe"

%MSBUILD_EXE% "Code\SuoraBuildTool\SuoraBuildTool.vcxproj" /p:configuration=Dist /p:platform=x64

copy "Code\SuoraBuildTool\Build\Dist-windows-x86_64\SuoraBuildTool\SuoraBuildTool.exe" "Binaries\Scripts\SuoraBuildTool.exe"

call "Binaries\Scripts\SuoraBuildTool.exe"

pushd %~dp0\
call Code\Dependencies\premake\PremakeBinaries\premake5.exe vs2022
popd
PAUSE
