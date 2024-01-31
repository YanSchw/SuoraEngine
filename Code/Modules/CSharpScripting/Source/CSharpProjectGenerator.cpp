#include "CSharpProjectGenerator.h"
#include "CoreMinimal.h"
#include "Suora/Platform/Platform.h"
#include "Suora/Assets/AssetManager.h"


namespace Suora
{

	static void GenerateSingleCSProj(const std::filesystem::path& filePath)
	{
        std::string compilerSources = "";
        for (auto It : std::filesystem::recursive_directory_iterator(filePath))
        {
            if (It.is_directory())
                continue;

            if (It.path().string().ends_with(".cs"))
            {
                compilerSources += "<Compile Include=\"" + It.path().string() + "\" />\n";
            }
        }

		std::string str = "\
<Project Sdk=\"Microsoft.NET.Sdk\">\n\
  <PropertyGroup>\n\
    <OutputType>Library</OutputType>\n\
    <AppDesignerFolder>Properties</AppDesignerFolder>\n\
    <TargetFramework>net8.0</TargetFramework>\n\
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>\n\
    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>\n\
    <AppendTargetFrameworkToOutputPath>false</AppendTargetFrameworkToOutputPath>\n\
    <Nullable>enable</Nullable>\n\
  </PropertyGroup>\n\
  <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Debug|AnyCPU' \">\n\
    <PlatformTarget>AnyCPU</PlatformTarget>\n\
    <DebugType>portable</DebugType>\n\
    <DebugSymbols>true</DebugSymbols>\n\
    <Optimize>false</Optimize>\n\
    <OutputPath>" + AssetManager::GetProjectAssetPath() + "/../Build/CSharp/Debug/</OutputPath>\n\
    <IntermediateOutputPath>" + AssetManager::GetProjectAssetPath() + "/../Build/Intermediate/CSharp/Debug/</IntermediateOutputPath>\n\
    <DefineConstants></DefineConstants>\n\
    <ErrorReport>prompt</ErrorReport>\n\
    <WarningLevel>4</WarningLevel>\n\
    <NoWarn>CS8500</NoWarn>\n\
  </PropertyGroup>\n\
  <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Release|AnyCPU' \">\n\
    <PlatformTarget>AnyCPU</PlatformTarget>\n\
    <DebugType>portable</DebugType>\n\
    <DebugSymbols>true</DebugSymbols>\n\
    <Optimize>false</Optimize>\n\
    <OutputPath>" + AssetManager::GetProjectAssetPath() + "/../Build/CSharp/Release/</OutputPath>\n\
    <IntermediateOutputPath>" + AssetManager::GetProjectAssetPath() + "/../Build/Intermediate/CSharp/Release/</IntermediateOutputPath>\n\
    <DefineConstants></DefineConstants>\n\
    <ErrorReport>prompt</ErrorReport>\n\
    <WarningLevel>4</WarningLevel>\n\
    <NoWarn>CS8500</NoWarn>\n\
  </PropertyGroup>\n\
  <ItemGroup Condition=\" '$(Configuration)|$(Platform)' == 'Debug|AnyCPU' \">\n\
  </ItemGroup>\n\
  <ItemGroup Condition=\" '$(Configuration)|$(Platform)' == 'Release|AnyCPU' \">\n\
  </ItemGroup>\n\
  <ItemGroup>\n\
    " + compilerSources + "\n\
  </ItemGroup>\n\
</Project>\
";

        Platform::WriteToFile((filePath / (filePath.filename().string() + ".csproj")).string(), str);
	}

	void CSharpProjectGenerator::GenerateCSProjectFiles()
	{
		if (AssetManager::GetProjectAssetPath() == "")
		{
			return;
		}

		std::filesystem::path path = AssetManager::GetProjectAssetPath() + "/../Code/CSharp";
		if (std::filesystem::exists(path))
		{
            for (auto dir : std::filesystem::directory_iterator(path))
            {
                if (dir.is_directory())
                {
                    GenerateSingleCSProj(dir.path());
                }
            }
		}

	}

}