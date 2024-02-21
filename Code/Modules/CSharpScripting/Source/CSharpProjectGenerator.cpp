#include "CSharpProjectGenerator.h"
#include "CSharpCodeGenerator.h"
#include "CoreMinimal.h"
#include "Suora/Platform/Platform.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/SuoraProject.h"
#include "Suora/Common/Random.h"

namespace Suora
{

	static void GenerateSingleCSProj(const std::filesystem::path& dir)
	{
        String compilerSources = "";
        for (auto It : std::filesystem::recursive_directory_iterator(dir))
        {
            if (It.is_directory())
                continue;

            if (It.path().string().ends_with(".cs"))
            {
                compilerSources += "<Compile Include=\"" + It.path().string() + "\" />\n";
            }
        }

		String str = "\
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
  </PropertyGroup>\n\
  <ItemGroup Condition=\" '$(Configuration)|$(Platform)' == 'Debug|AnyCPU' \">\n\
  </ItemGroup>\n\
  <ItemGroup Condition=\" '$(Configuration)|$(Platform)' == 'Release|AnyCPU' \">\n\
  </ItemGroup>\n\
  <ItemGroup>\n\
    " + compilerSources + "\n\
  </ItemGroup>\n\
  <ItemGroup>\n\
    <ProjectReference Include=\"" + AssetManager::GetEngineAssetPath() + "/../Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Managed/Coral.Managed.csproj\">\n\
      <Project>{11178401-7DD7-FF37-4612-49B5B2914414}</Project>\n\
      <Name>Coral.Managed</Name>\n\
    </ProjectReference>\n\
    <ProjectReference Include=\"../../../Build/CSharp/Generated/Suora.Generated.csproj\">\n\
      <Name>Suora.Generated</Name>\n\
    </ProjectReference>\n\
  </ItemGroup>\n\
</Project>\
";

        Platform::WriteToFile((dir / (dir.filename().string() + ".csproj")).string(), str);
	}

    static void GenerateSuora_GeneratedCSProjectFiles()
    {
        std::filesystem::path path = AssetManager::GetProjectAssetPath() + "/../Build/CSharp/Generated";
        std::filesystem::remove_all(path);

        std::filesystem::create_directories(path);

        std::filesystem::path srcPath = AssetManager::GetEngineAssetPath() + "/../Code/Modules/CSharpScripting/ThirdParty/Coral/Suora.Generated/";
        std::filesystem::copy(srcPath, path, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

        // Generate...
        {
            String gen;
            CSharpCodeGenerator::Generate_AllNativeClasses_CS(gen);
            Platform::WriteToFile((path / "Source/AllNativeClasses.cs").string(), gen);
        }

        String SuoraGeneratedPath = path.string() + "/Suora.Generated.csproj";
        String csproj = Platform::ReadFromFile(SuoraGeneratedPath);
        StringUtil::ReplaceSequence(csproj, "PATH_TO_CORAL_MANAGED", "\
<ProjectReference Include=\"" + AssetManager::GetEngineAssetPath() + "/../Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Managed/Coral.Managed.csproj\">\n\
      <Project>{11178401-7DD7-FF37-4612-49B5B2914414}</Project>\n\
      <Name>Coral.Managed</Name>\n\
    </ProjectReference>");
        Platform::WriteToFile(SuoraGeneratedPath, csproj);

    }

	void CSharpProjectGenerator::GenerateCSProjectFiles()
	{
		if (AssetManager::GetProjectAssetPath() == "")
		{
			return;
		}

        GenerateSuora_GeneratedCSProjectFiles();

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

        GenerateVisualStudioSolution();
	}

    static String GenerateVSStudioGUID(Random& rand)
    {
        String guid = "";

        for (int i = 0; i < 8; i++)
        {
            guid += std::to_string(rand.Next() % 10);
        }
        guid += "-";
        for (int i = 0; i < 4; i++)
        {
            guid += std::to_string(rand.Next() % 10);
        }
        guid += "-";
        for (int i = 0; i < 4; i++)
        {
            guid += std::to_string(rand.Next() % 10);
        }
        guid += "-";
        for (int i = 0; i < 4; i++)
        {
            guid += std::to_string(rand.Next() % 10);
        }
        guid += "-";
        for (int i = 0; i < 12; i++)
        {
            guid += std::to_string(rand.Next() % 10);
        }

        return guid;
    }

    void CSharpProjectGenerator::GenerateVisualStudioSolution()
    {
        if (AssetManager::GetProjectAssetPath() == "")
        {
            return;
        }

        Random rand = Random(0);
        String projects = "";
        String configs = "";

        std::filesystem::path path = AssetManager::GetProjectAssetPath() + "/../Code/CSharp";
        if (std::filesystem::exists(path))
        {
            for (auto dir : std::filesystem::directory_iterator(path))
            {
                if (dir.is_directory())
                {
                    String guid = GenerateVSStudioGUID(rand);
                    projects += "\
Project(\"{" + GenerateVSStudioGUID(rand) + "}\") = \"" + dir.path().filename().string() + "\", \"" + (dir.path() / (dir.path().filename().string() + ".csproj")).string() + "\", \"{" + guid + "}\"\n\
EndProject\n";
                    configs = "\
\t\t{" + guid + "}.Debug|Any CPU.ActiveCfg = Debug|Any CPU\n\
\t\t{" + guid + "}.Debug|Any CPU.Build.0 = Debug|Any CPU\n\
\t\t{" + guid + "}.Release|Any CPU.ActiveCfg = Release|Any CPU\n\
\t\t{" + guid + "}.Release|Any CPU.Build.0 = Release|Any CPU\n";
                }
            }
        }
        else
        {
            return;
        }

        String str = "\
Microsoft Visual Studio Solution File, Format Version 12.00\n\
# Visual Studio Version 17\n\
Project(\"{9A19103F-16F7-4668-BE54-9A1E7A4F7556}\") = \"Coral.Managed\", \"" + AssetManager::GetEngineAssetPath() + "/../Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Managed/Coral.Managed.csproj\", \"{11178401-7DD7-FF37-4612-49B5B2914414}\"\n\
EndProject\n\
Project(\"{4A19103F-16F7-4668-BE54-9A1E7A4F7556}\") = \"Suora.Generated\", \"" + AssetManager::GetProjectAssetPath() + "/../Build/CSharp/Generated/Suora.Generated.csproj\", \"{31178401-7DD7-FF37-4612-49B5B2914414}\"\n\
EndProject\n\
Project(\"{2150E333-8FDC-42A3-9474-1A3956D46DE8}\") = \"SuoraEngine-Internal\", \"SuoraEngine-Internal\", \"{65CB7E83-D18B-FAB9-9AC6-433706463F96}\"\n\
EndProject\n\
" + projects + "\
Global\n\
	GlobalSection(SolutionConfigurationPlatforms) = preSolution\n\
		Debug|Any CPU = Debug|Any CPU\n\
		Release|Any CPU = Release|Any CPU\n\
	EndGlobalSection\n\
	GlobalSection(ProjectConfigurationPlatforms) = postSolution\n\
		{11178401-7DD7-FF37-4612-49B5B2914414}.Debug|Any CPU.ActiveCfg = Debug|Any CPU\n\
		{11178401-7DD7-FF37-4612-49B5B2914414}.Debug|Any CPU.Build.0 = Debug|Any CPU\n\
		{11178401-7DD7-FF37-4612-49B5B2914414}.Release|Any CPU.ActiveCfg = Release|Any CPU\n\
		{11178401-7DD7-FF37-4612-49B5B2914414}.Release|Any CPU.Build.0 = Release|Any CPU\n\
		{31178401-7DD7-FF37-4612-49B5B2914414}.Debug|Any CPU.ActiveCfg = Debug|Any CPU\n\
		{31178401-7DD7-FF37-4612-49B5B2914414}.Debug|Any CPU.Build.0 = Debug|Any CPU\n\
		{31178401-7DD7-FF37-4612-49B5B2914414}.Release|Any CPU.ActiveCfg = Release|Any CPU\n\
		{31178401-7DD7-FF37-4612-49B5B2914414}.Release|Any CPU.Build.0 = Release|Any CPU\n\
" + configs + "\
	EndGlobalSection\n\
	GlobalSection(SolutionProperties) = preSolution\n\
		HideSolutionNode = FALSE\n\
	EndGlobalSection\n\
	GlobalSection(NestedProjects) = preSolution\n\
		{11178401-7DD7-FF37-4612-49B5B2914414} = {65CB7E83-D18B-FAB9-9AC6-433706463F96}\n\
		{31178401-7DD7-FF37-4612-49B5B2914414} = {65CB7E83-D18B-FAB9-9AC6-433706463F96}\n\
	EndGlobalSection\n\
EndGlobal\
";

        std::filesystem::path slnPath = AssetManager::GetProjectAssetPath() + "/../";
        slnPath /= ProjectSettings::Get()->GetAssetName() + "-CSharp.sln";

        Platform::WriteToFile(slnPath.string(), str);

    }

}