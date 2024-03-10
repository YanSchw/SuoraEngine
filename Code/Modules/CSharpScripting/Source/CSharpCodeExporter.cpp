#include "Precompiled.h"
#include "CSharpCodeExporter.h"

#include "CSharpScriptEngine.h"

namespace Suora
{

	void CSharpCodeExporter::Execute(ExportSettings* settings)
	{
		if (!std::filesystem::exists(AssetManager::GetProjectAssetPath() + "/../Code/CSharp/"))
		{
			return;
		}

		SUORA_INFO(Log::CustomCategory("Export"), "Building C# Binaries.");

		CSharpScriptEngine::Get()->BuildAllCSProjects();

		Platform::CreateDirectory((settings->m_OutputPath / "Binaries" / "CSharp").string());

		for (const auto& file : std::filesystem::directory_iterator(AssetManager::GetProjectAssetPath() + "/../Build/CSharp/Release/"))
		{
			if (file.is_directory())
				continue;

			String filename = file.path().filename().string();
			if (filename.ends_with(".dll"))
			{
				std::filesystem::copy_file(file.path(), settings->m_OutputPath / "Binaries" / "CSharp" / filename);
			}
		}

		// Copy over runtimeconfig.json
		Platform::CopyDirectory(AssetManager::GetEngineAssetPath() + "/../Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Managed/Coral.Managed.runtimeconfig.json",
			                    settings->m_OutputPath / "Binaries" / "CSharp");
	}

}