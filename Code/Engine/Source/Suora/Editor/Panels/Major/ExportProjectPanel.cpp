#include "Precompiled.h"
#include "ExportProjectPanel.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Platform/Platform.h"

#include <chrono>

namespace Suora
{

	ExportProjectPanel::ExportProjectPanel()
	{
	}
	void ExportProjectPanel::Init()
	{
		Super::Init();

		m_Name = "Export Project";
		m_ExportSettings = CreateRef<ExportSettings>();

		Ref<DetailsPanel> t1 = CreateRef<DetailsPanel>(this);
		t1->m_Data = m_ExportSettings;

		Ref<DockingSpace> ds1 = CreateRef<DockingSpace>(0, 0, 1, 1, this);					    m_DockspacePanel.m_DockingSpaces.Add(ds1); ds1->m_MinorTabs.Add(t1);

	}

	void ExportProjectPanel::Update(float deltaTime)
	{
		Super::Update(deltaTime);

	}

	static void ExportWindowsRuntime(ExportSettings* settings)
	{
		auto runtimePath = std::filesystem::path(AssetManager::GetEngineAssetPath()).parent_path().append("Binaries").append("Runtime").append("RuntimeWindows.exe");
		
		if (!ProjectSettings::Get()->m_IsNativeProject && std::filesystem::exists(runtimePath))
		{
			SUORA_INFO(Log::CustomCategory("Export"), "Using Precompiled Runtime Binaries.");
			std::filesystem::copy_file(runtimePath, settings->m_OutputPath / (ProjectSettings::Get()->GetProjectName() + ".exe"));
		}
		else
		{
			SUORA_ERROR(Log::CustomCategory("Export"), "Native build required!!");
		}
	}

	static void ExportContent(ExportSettings* settings)
	{
		Platform::CreateDirectory(settings->m_OutputPath / "Content");

		Platform::CopyDirectory(AssetManager::GetEngineAssetPath(),  settings->m_OutputPath / "Content");
		Platform::CopyDirectory(AssetManager::GetProjectAssetPath(), settings->m_OutputPath / "Content");

		// Remove Copied .suora File
		std::vector<DirectoryEntry> entries = File::GetAllAbsoluteEntriesOfPath(settings->m_OutputPath / "Content");
		for (auto file : entries)
		{
			const String ext = File::GetFileExtension(file);
			if (ext == ".suora")
			{
				std::filesystem::remove(file);
				break;
			}
		}

		Yaml::Node root;
		const String enginePath = ProjectSettings::Get()->GetEnginePath();
		ProjectSettings::Get()->SetEnginePath("");
		ProjectSettings::Get()->Serialize(root);
		ProjectSettings::Get()->SetEnginePath(enginePath);
		String out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile((settings->m_OutputPath / "Content" / (ProjectSettings::Get()->GetProjectName() + ".suora")).string(), out);

	}

	static void ExportProject(ExportSettings* settings)
	{
		auto begin = std::chrono::high_resolution_clock::now();

		if (std::filesystem::exists(settings->m_OutputPath))
		{
			SUORA_WARN(Log::CustomCategory("Export"), "The Output path already exists. You might want to export into an empty directory!");
		}

		// Create Directory
		Platform::CreateDirectory(settings->m_OutputPath);

		// Executable Runtime
		ExportWindowsRuntime(settings);

		// Content
		ExportContent(settings);

		auto end = std::chrono::high_resolution_clock::now();
		SUORA_INFO(Log::CustomCategory("Export"), "It took {0} seconds to export the project.", std::chrono::duration_cast<std::chrono::seconds>(end - begin).count());
		Platform::ShowInExplorer(settings->m_OutputPath);
	}

	void ExportProjectPanel::DrawToolbar(float& x, float y, float height)
	{
		Super::DrawToolbar(x, y, height);

		x += 25.0f;
		if (EditorUI::Button("Export Project", x, y + height * 0.1f, height + 90.0f, height * 0.8f, EditorUI::ButtonParams::Highlight()))
		{
			ExportProject(m_ExportSettings.get());
		}
		x += height + 100.0f;
	}

	Texture* ExportProjectPanel::GetIconTexture()
	{
		return Icon::Cogwheel;
	}

	void ExportProjectPanel::SaveAsset()
	{
		Super::SaveAsset();

		SuoraVerify(false);
	}

	ExportSettings::ExportSettings()
	{
		m_OutputPath = std::filesystem::path(AssetManager::GetProjectAssetPath()).parent_path().append("Build").append("Distribution");

#ifdef SUORA_PLATFORM_WINDOWS
		m_MSBuildPath = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Msbuild\\Current\\Bin\\MSBuild.exe";
#endif // SUORA_PLATFORM_WINDOWS

	}

}