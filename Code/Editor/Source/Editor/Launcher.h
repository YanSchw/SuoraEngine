#pragma once
#include <Suora.h>
#include <filesystem>

namespace Suora
{
	class Window;

	class Launcher
	{
		EditorWindow* m_EditorWindow = nullptr;
	public:

		struct TemplateProject
		{
			std::filesystem::path m_TemplateProjectPath;
			String m_Name;
			String m_Description = "No Description available!";
			Ref<Texture> m_Icon;
			Ref<Texture> m_Preview;

			bool m_IsNative = false;
		};

		struct CachedProjectInfo
		{
			String m_Version;
		};

		Launcher(EditorWindow* editor);

		void Render(float deltaTime);
		Window* GetWindow();

		void OpenProject(const String& path, bool isNativeProject);
		void CreateProject(const String& projectName, std::filesystem::path projectPath, Ref<TemplateProject> templateProject);

	private:
		void RenderProjects();
		void RenderCreateNewProjects();

		void AnalyzeProjectOnDisk(const String& projectPath);

	public:
		uint32_t m_SelectedTab = 0;

		std::unordered_map<String, CachedProjectInfo> m_CachedProjects;

		Array<Ref<TemplateProject>> m_TemplateProjects;
		Ref<TemplateProject> m_SelectedProject;

		bool m_SelectedContentOnlyTab = false;
		String m_ProjectName = "MyProject";
		std::filesystem::path m_ProjectPath = std::filesystem::current_path();

		friend class EditorWindow;
	};

}