#include "Launcher.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/SuoraProject.h"
#include "EditorWindow.h"
#include "EditorUI.h"
#include <filesystem>
#include <fstream>
#include <sstream>


#ifdef SUORA_PLATFORM_WINDOWS
#include <shellapi.h>
#endif

namespace Suora
{

	Launcher::Launcher(EditorWindow* editor)
		: m_EditorWindow(editor)
	{
		std::filesystem::path engineContentPath = AssetManager::GetEngineAssetPath();
		std::filesystem::path templateContentPath = engineContentPath.parent_path().append("Templates");

		if (std::filesystem::exists(templateContentPath))
		{
			for (auto project : std::filesystem::directory_iterator(templateContentPath))
			{
				if (project.is_directory())
				{
					Ref<TemplateProject> ref = CreateRef<TemplateProject>();
					m_TemplateProjects.Add(ref);
					ref->m_TemplateProjectPath = project;
					ref->m_Name = project.path().filename().string();
					
					ref->m_IsNative = std::filesystem::exists(std::filesystem::path(project).append("Code"));

					std::filesystem::path info = project.path();
					info.append("LauncherInfo");
					if (std::filesystem::exists(info))
					{
						ref->m_Icon = Texture::Create(info.string() + "/Icon.png");
						ref->m_Preview = Texture::Create(info.string() + "/Preview.png");
						std::filesystem::path name = info.string() + "/Name.txt";
						std::filesystem::path desc = info.string() + "/Description.txt";
						{
							std::ifstream t(name);
							std::stringstream buffer;
							buffer << t.rdbuf();
							ref->m_Name = buffer.str();
						}
						{
							std::ifstream t(desc);
							std::stringstream buffer;
							buffer << t.rdbuf();
							ref->m_Description = buffer.str();
						}
					}

				}
			}
		}

		// If there is no Project, the "Create new Project" Tab is shown by default
		m_SelectedTab = EditorPreferences::Get()->m_AllCachedProjectPaths.Size() >= 1 ? 0 : 1;
	}

	void Launcher::Render(float deltaTime)
	{
		const float ui = EditorPreferences::Get()->UiScale;

		RenderCommand::SetAlphaBlending(true);

		{
			{
				EditorUI::ButtonParams GUIParams;
				GUIParams.ButtonRoundness = 0.0f;
				GUIParams.ButtonColorClicked = Color(0.0f);
				GUIParams.ButtonOutlineColor = EditorPreferences::Get()->UiForgroundColor;
				GUIParams.ButtonColor = EditorPreferences::Get()->UiColor;
				GUIParams.ButtonColorHover = GUIParams.ButtonColor;
				GUIParams.HoverCursor = Cursor::Default;
				EditorUI::Button("", 0, 0, 250.0f * ui, GetWindow()->GetHeight(), GUIParams);
			}
			EditorUI::ButtonParams GUIParams;
			GUIParams.ButtonRoundness = 0.0f;
			GUIParams.ButtonColorClicked = Color(0.0f);
			GUIParams.ButtonOutlineColor = EditorPreferences::Get()->UiForgroundColor;
			GUIParams.ButtonColor = EditorPreferences::Get()->UiColor;
			GUIParams.ButtonColorHover = EditorPreferences::Get()->UiForgroundColor;

			int y = GetWindow()->GetHeight() - (36.0f * ui);

			y -= 25.0f;
			y -= 50.0f * ui;
			if (EditorUI::Button("Projects", 5 * ui, y + 5 * ui, 240.0f * ui, 40.0f * ui - 1, m_SelectedTab == 0 ? EditorUI::ButtonParams::Highlight() : EditorUI::ButtonParams()))
			{
				m_SelectedTab = 0;
			}
			y -= 50.0f * ui;
			if (EditorUI::Button("Create new Project", 5 * ui, y + 5 * ui, 240.0f * ui, 40.0f * ui - 1, m_SelectedTab == 1 ? EditorUI::ButtonParams::Highlight() : EditorUI::ButtonParams()))
			{
				m_SelectedTab = 1;
			}

		}

		if (m_SelectedTab == 0)
		{
			RenderProjects();
		}
		else if (m_SelectedTab == 1)
		{
			RenderCreateNewProjects();
		}

	}

	Window* Launcher::GetWindow()
	{
		return m_EditorWindow->GetWindow();
	}

	void Launcher::OpenProject(const std::string& path, bool isNativeProject)
	{
		if (!std::filesystem::exists(path))
		{
			return;
		}

		/*** Always Update the EnginePath of the ProjectSettings ***/
		{
			const std::string str = Platform::ReadFromFile(path);
			Yaml::Node root;
			Yaml::Parse(root, str);
			Yaml::Node& settings = root["Settings"];
			{
				settings["Engine"]["Path"] = std::filesystem::path(AssetManager::GetEngineAssetPath()).parent_path().string();
			}
			std::string out;
			Yaml::Serialize(root, out);
			Platform::WriteToFile(path, out);
		}
		/**/

		if (isNativeProject)
		{
			// 0. Locate ProjectRootPath
			std::filesystem::path projectRootPath = path;
			while (!std::filesystem::is_directory(projectRootPath.append("Content")))
			{
				projectRootPath = projectRootPath.parent_path().parent_path();
			}
			projectRootPath = projectRootPath.parent_path();
			// 1. Remove and recopy the Scripts folder...
			if (std::filesystem::exists(std::filesystem::path(projectRootPath).append("Scripts")))
			{
				std::filesystem::remove_all(std::filesystem::path(projectRootPath).append("Scripts"));
			}
			std::filesystem::path scriptsPath = std::filesystem::path(Engine::Get()->GetRootPath()).append("Binaries").append("Scripts");
			if (std::filesystem::exists(scriptsPath))
			{
				Platform::CopyDirectory(scriptsPath, std::filesystem::path(projectRootPath).append("Scripts"));
			}

			// 2. Open FileExplorer
			Platform::ShowInExplorer(projectRootPath);

			// 3. Generate stuff...
			{
#ifdef SUORA_PLATFORM_WINDOWS
				std::filesystem::current_path(projectRootPath);
				std::string cmdRunSuoraBuildTool = "call " + projectRootPath.string() + "/Scripts/SuoraBuildTool.exe";
				system(cmdRunSuoraBuildTool.c_str());
				std::string cmdGenerateSolution = "call " + projectRootPath.string() + "/Scripts/GenerateSolution.bat";
				system(cmdGenerateSolution.c_str());

				// open the Solution in VisualStudio
				{
					std::string fullPath = projectRootPath.string();
					while (fullPath.find("\\") != std::string::npos) StringUtil::ReplaceSequence(fullPath, "\\", "/");
					std::vector<std::string> splitPath = StringUtil::SplitString(fullPath, '/');
					std::string projectName = splitPath[splitPath.size() - 1];
					std::string solutionPath = projectRootPath.string() + "/" + projectName + ".sln";
					Platform::OpenFileExternally(solutionPath);
				}
#endif
			}

			// 4. Close the Launcher
			std::exit(0);
		}
		else
		{
			std::filesystem::path p = std::filesystem::path(path).parent_path();
			while (!std::filesystem::is_directory(p.append("Content")))
			{
				p = p.parent_path().parent_path();
			}
			AssetManager::SetProjectAssetPath(p.string());
			AssetManager::HotReload(p.string());
		}
	}

	void Launcher::CreateProject(const std::string& projectName, std::filesystem::path projectPath, Ref<TemplateProject> templateProject)
	{
		if (projectName == "")
		{
			SUORA_LOG(LogCategory::Editor, LogLevel::Error, "Cannot create new Project; ProjectName is empty!");
			return;
		}
		Platform::CreateDirectory(projectPath.append(projectName));
		// projectPath now points into the newly created ProjectDirectory

		std::filesystem::path contentPath = std::filesystem::path(templateProject->m_TemplateProjectPath).append("Content");
		if (std::filesystem::exists(contentPath))
		{
			Platform::CopyDirectory(contentPath, std::filesystem::path(projectPath).append("Content"));
		}
		std::filesystem::path codePath = std::filesystem::path(templateProject->m_TemplateProjectPath).append("Code");
		if (std::filesystem::exists(codePath))
		{
			Platform::CopyDirectory(codePath, std::filesystem::path(projectPath).append("Code"));
		}
		std::filesystem::path scriptsPath = std::filesystem::path(Engine::Get()->GetRootPath()).append("Binaries").append("Scripts");
		if (std::filesystem::exists(scriptsPath))
		{
			Platform::CopyDirectory(scriptsPath, std::filesystem::path(projectPath).append("Scripts"));
		}

		std::filesystem::path projectSettingsPath;
		std::vector<DirectoryEntry> entries = File::GetAllAbsoluteEntriesOfPath(projectPath);
		for (auto file : entries)
		{
			const std::string ext = File::GetFileExtension(file);
			if (ext == ".suora")
			{
				projectSettingsPath = std::filesystem::path(file.path()).parent_path().append(projectName + ".suora");
				std::filesystem::rename(file.path(), projectSettingsPath);
				break;
			}
		}
		/*** Finished Project Creation ***/

		{
			EditorPreferences::Get()->m_AllCachedProjectPaths.Add(projectSettingsPath.string());

			Yaml::Node out;
			std::string str;
			AssetManager::GetFirstAssetOfType<EditorPreferences>()->Serialize(out);
			Yaml::Serialize(out, str);
			Platform::WriteToFile(AssetManager::GetFirstAssetOfType<EditorPreferences>()->m_Path.string(), str);
		}
		
		OpenProject(projectSettingsPath.string(), templateProject->m_IsNative);
	}

	static bool EndsWith(std::string_view str, std::string_view suffix)
	{
		return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
	}
	void Launcher::RenderProjects()
	{
		const float ui = EditorPreferences::Get()->UiScale;
		EditorUI::ButtonParams GUIParams;
		GUIParams.ButtonRoundness = 0.0f;
		GUIParams.ButtonColorClicked = Color(0.0f);
		GUIParams.ButtonOutlineColor = EditorPreferences::Get()->UiForgroundColor;
		GUIParams.ButtonColor = EditorPreferences::Get()->UiColor;
		GUIParams.ButtonColorHover = EditorPreferences::Get()->UiForgroundColor;

		Color BackgroundColor = EditorPreferences::Get()->UiColor * 0.65f; BackgroundColor.a = 1.0f;
		EditorUI::DrawRect(250.0f * ui, 0, GetWindow()->GetWidth() - 250.0f * ui, GetWindow()->GetHeight(), 0, BackgroundColor);

		if (EditorUI::Button("Add Project", GetWindow()->GetWidth() - 175.0f, 35.0f, 125, 33.0f))
		{
			std::optional<std::string> path = Platform::OpenFileDialog({".suora"});
			
			if (path.has_value() && EndsWith(path.value(), ".suora"))
			{
				EditorPreferences::Get()->m_AllCachedProjectPaths.Add(path.value());
			}
			{
				Yaml::Node out;
				std::string str;
				AssetManager::GetFirstAssetOfType<EditorPreferences>()->Serialize(out);
				Yaml::Serialize(out, str);
				Platform::WriteToFile(AssetManager::GetFirstAssetOfType<EditorPreferences>()->m_Path.string(), str);
			}
		}

		float x = 250 * ui;


		int y = GetWindow()->GetHeight() - (36.0f * ui);
		y -= 35.0f * ui;
		EditorUI::Button("", x, y, (GetWindow()->GetWidth() - x) * 0.1f, 35.0f * ui - 1, GUIParams);
		EditorUI::Button("NAME", x + (GetWindow()->GetWidth() - x) * 0.1f, y, (GetWindow()->GetWidth() - x) * 0.25f, 35.0f * ui - 1, GUIParams);
		EditorUI::Button("PATH", x + (GetWindow()->GetWidth() - x) * 0.35f, y, (GetWindow()->GetWidth() - x) * 0.40f, 35.0f * ui - 1, GUIParams);
		EditorUI::Button("VERSION", x + (GetWindow()->GetWidth() - x) * 0.75f, y, (GetWindow()->GetWidth() - x) * 0.10f, 35.0f * ui - 1, GUIParams);
		EditorUI::Button("", x + (GetWindow()->GetWidth() - x) * 0.85f, y, (GetWindow()->GetWidth() - x) * 0.1f, 35.0f * ui - 1, GUIParams);

		y -= 20.0f * ui;
		for (auto& It : EditorPreferences::Get()->m_AllCachedProjectPaths)
		{
			std::filesystem::path projectPath = It;

			if (!std::filesystem::exists(projectPath))
			{
				// This Project does exits anymore. So remove it, lets try again next frame...
				EditorPreferences::Get()->m_AllCachedProjectPaths.Remove((std::string)It);
				return;
			}
			if (m_CachedProjects.find(It) == m_CachedProjects.end())
			{
				AnalyzeProjectOnDisk(It);
			}
			CachedProjectInfo& projectInfo = m_CachedProjects[It];

			y -= 75.0f * ui;

			if (EditorUI::Button("", x + (GetWindow()->GetWidth() - x) * 0.1f, y, (GetWindow()->GetWidth() - x) * 0.75f, 60.0f * ui - 1))
			{
				// Now check, if Project isNative
				const std::string str = Platform::ReadFromFile(It);
				Yaml::Node root;
				Yaml::Parse(root, str);
				Yaml::Node& settings = root["Settings"];
				bool isNative = settings["Engine"]["m_IsNativeProject"].As<std::string>() == "true";

				OpenProject(It, isNative);
			}
			
			EditorUI::Text(projectPath.stem().string(), Font::Instance, x + (GetWindow()->GetWidth() - x) * 0.1f + 20, y, (GetWindow()->GetWidth() - x) * 0.25f - 20, 60.0f * ui - 1, 28, Vec2(-1, 0), Color(1));

			EditorUI::Text(It, Font::Instance, x + (GetWindow()->GetWidth() - x) * 0.35f + 1 + 20, y, (GetWindow()->GetWidth() - x) * 0.40f - 1 - 20, 60.0f * ui - 1, 22, Vec2(-1, 0), Color(0.75f));

			EditorUI::Text(projectInfo.m_Version, Font::Instance, x + (GetWindow()->GetWidth() - x) * 0.75f + 1, y, (GetWindow()->GetWidth() - x) * 0.10f - 1, 60.0f * ui - 1, 28, Vec2(0, 0), Color(1));
		}

		EditorUI::DrawRect(x + (GetWindow()->GetWidth() - x) * 0.35f + 1, 0, 2, GetWindow()->GetHeight() - (72.0f * ui), 0, BackgroundColor);
		EditorUI::DrawRect(x + (GetWindow()->GetWidth() - x) * 0.75f + 1, 0, 2, GetWindow()->GetHeight() - (72.0f * ui), 0, BackgroundColor);
	}

	void Launcher::RenderCreateNewProjects()
	{
		const float ui = EditorPreferences::Get()->UiScale;
		EditorUI::ButtonParams GUIParams;
		GUIParams.ButtonRoundness = 0.0f;
		GUIParams.ButtonColorClicked = Color(0.0f);
		GUIParams.ButtonOutlineColor = EditorPreferences::Get()->UiForgroundColor;
		GUIParams.ButtonColor = EditorPreferences::Get()->UiColor;
		GUIParams.ButtonColorHover = EditorPreferences::Get()->UiForgroundColor;

		Color BackgroundColor = EditorPreferences::Get()->UiColor * 0.65f; BackgroundColor.a = 1.0f;
		EditorUI::DrawRect(250.0f * ui, 0, GetWindow()->GetWidth() - 250.0f * ui, GetWindow()->GetHeight(), 0, BackgroundColor);

		float x = 250 * ui;

		float y = GetWindow()->GetHeight() - (36.0f * ui);
		y -= 55.0f * ui;
		if (EditorUI::Button("C++", x + (GetWindow()->GetWidth() - x) * 0.3f - 225.0f, y, 200.0f, 30.0f, m_SelectedContentOnlyTab ? EditorUI::ButtonParams() : EditorUI::ButtonParams::Outlined()))
		{
			m_SelectedContentOnlyTab = false;
			m_SelectedProject = nullptr;
		}
		if (EditorUI::Button("Content only", x + (GetWindow()->GetWidth() - x) * 0.3f + 025.0f, y, 200.0f, 30.0f, !m_SelectedContentOnlyTab ? EditorUI::ButtonParams() : EditorUI::ButtonParams::Outlined()))
		{
			m_SelectedContentOnlyTab = true;
			m_SelectedProject = nullptr;
		}

		y -= 45.0f * ui;
		for (auto& It : m_TemplateProjects)
		{
			if (It->m_IsNative == m_SelectedContentOnlyTab)
			{
				continue;
			}
			y -= 75.0f * ui;

			EditorUI::ButtonParams ProjectParams = (It == m_SelectedProject) ? EditorUI::ButtonParams::Outlined() : EditorUI::ButtonParams();
			ProjectParams.TextOrientation = Vec2(-1.0f, 0.0f);
			ProjectParams.TextOffsetLeft = 80.0f * ui;
			if (EditorUI::Button(It->m_Name, x + (GetWindow()->GetWidth() - x) * 0.05f, y, (GetWindow()->GetWidth() - x) * 0.50f, 60.0f * ui - 1, ProjectParams))
			{
				m_SelectedProject = It;
			}
			EditorUI::DrawTexturedRect(It->m_Icon, x + (GetWindow()->GetWidth() - x) * 0.05f + 10.0f, y + 10.0f, 40.0f * ui, 40.0f * ui, 0.0f, Color(1.0f));

		}


		if (m_SelectedProject)
		{
			float _y = GetWindow()->GetHeight();
			if (m_SelectedProject->m_Preview)
			{
				const float InvAspect = m_SelectedProject->m_Preview->GetHeight() / (float)m_SelectedProject->m_Preview->GetWidth();
				const float previewHeight = (GetWindow()->GetWidth() - x) * 0.4f * InvAspect;
				_y -= previewHeight;
				EditorUI::DrawTexturedRect(m_SelectedProject->m_Preview, x + (GetWindow()->GetWidth() - x) * 0.6f, _y, (GetWindow()->GetWidth() - x) * 0.4f, previewHeight, 0.0f, Color(1.0f));
			}
			_y -= 10.0f;
			_y -= 35.0f;
			EditorUI::Text(m_SelectedProject->m_Name, Font::Instance, x + (GetWindow()->GetWidth() - x) * 0.6f + 20.0f, _y, (GetWindow()->GetWidth() - x) * 0.4f - 20.0f, 35.0f, 26.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
			_y -= 35.0f;
			EditorUI::Text(m_SelectedProject->m_Description, Font::Instance, x + (GetWindow()->GetWidth() - x) * 0.6f + 20.0f, _y, (GetWindow()->GetWidth() - x) * 0.4f - 20.0f, 35.0f, 26.0f, Vec2(-1.0f, 0.0f), Color(1.0f));

			_y -= 35.0f;
			EditorUI::DrawRect(x + (GetWindow()->GetWidth() - x) * 0.6f, _y, GetWindow()->GetWidth(), 2.0f, 0.0f, EditorPreferences::Get()->UiForgroundColor);

			_y -= 25.0f;
			EditorUI::Text("Project Name", Font::Instance, x + (GetWindow()->GetWidth() - x) * 0.6f + 20.0f, _y + 3.0f, (GetWindow()->GetWidth() - x) * 0.4f - 20.0f, 10.0f, 16.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
			EditorUI::TextField(&m_ProjectName, x + (GetWindow()->GetWidth() - x) * 0.6f + 20.0f, _y - 40.0f, (GetWindow()->GetWidth() - x) * 0.4f - 40.0f, 40.0f);

			_y -= 60.0f;
			EditorUI::Text("Project Location", Font::Instance, x + (GetWindow()->GetWidth() - x) * 0.6f + 20.0f, _y + 3.0f, (GetWindow()->GetWidth() - x) * 0.4f - 20.0f, 10.0f, 16.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
			EditorUI::ButtonParams ProjectPathButtonParams;
			ProjectPathButtonParams.TextOffsetLeft = 5.0f;
			ProjectPathButtonParams.TextOrientation = Vec2(-1.0f, 0.0f);
			if (EditorUI::Button(m_ProjectPath.string(), x + (GetWindow()->GetWidth() - x) * 0.6f + 20.0f, _y - 40.0f, (GetWindow()->GetWidth() - x) * 0.4f - 40.0f, 40.0f, ProjectPathButtonParams))
			{
				std::optional<std::string> path = Platform::ChoosePathDialog();
				if (path.has_value()) m_ProjectPath = path.value();
			}

			EditorUI::ButtonParams CreateParams = EditorUI::ButtonParams::Highlight();
			CreateParams.ButtonRoundness = 10.0f;
			CreateParams.ButtonDropShadow = true;
			if (EditorUI::Button("Create Project", x + (GetWindow()->GetWidth() - x) * 0.8f - 125.0f, 100.0f, 250.0f, 30.0f, CreateParams))
			{
				CreateProject(m_ProjectName, m_ProjectPath, m_SelectedProject);
			}
		}

		EditorUI::DrawRect(x + (GetWindow()->GetWidth() - x) * 0.6f, 0.0f, 2.0f, GetWindow()->GetHeight(), 0.0f, EditorPreferences::Get()->UiForgroundColor);

	}

	void Launcher::AnalyzeProjectOnDisk(const std::string& projectPath)
	{
		m_CachedProjects[projectPath] = CachedProjectInfo();
		Yaml::Node root;
		Yaml::Parse(root, Platform::ReadFromFile(projectPath));
		Yaml::Node& settings = root["Settings"];

		m_CachedProjects[projectPath].m_Version += settings["Engine"]["VersionMajor"].As<std::string>() + ".";
		m_CachedProjects[projectPath].m_Version += settings["Engine"]["VersionMinor"].As<std::string>() + ".";
		m_CachedProjects[projectPath].m_Version += settings["Engine"]["VersionPatch"].As<std::string>();
	}

}