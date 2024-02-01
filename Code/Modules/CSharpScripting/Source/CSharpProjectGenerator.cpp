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

        Platform::WriteToFile((filePath / (filePath.filename().string() + ".csproj")).string(), str);
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
            String gen = "using System;\nusing Suora;\n\nnamespace Suora\n{\n";

            Array<Class> allClasses = Class::GetAllClasses();
            for (const Class& cls : allClasses)
            {
                if (!cls.IsNative())
                    continue;
                if (cls.GetNativeClassID() == 1)
                    continue;

                gen += "\t[NativeSuoraClass(NativeID = " + std::to_string(cls.GetNativeClassID()) + ")]\n";

                String parentNativeClass = (cls.GetParentClass() == Object::StaticClass()) ? "Suora.SuoraObject" : "Suora." + cls.GetParentClass().GetClassName();
                gen += "\tpublic class " + cls.GetClassName() + " : " + parentNativeClass + "\n\t{\n";

                gen += "\t}\n\n";

            }

            gen += "}";
            Platform::WriteToFile((path / "Source/AllNativeClasses.cs").string(), gen);
        }

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
	}

}