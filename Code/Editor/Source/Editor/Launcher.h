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
			std::string m_Name;
			std::string m_Description = "No Description available!";
			Ref<Texture> m_Icon;
			Ref<Texture> m_Preview;

			bool m_IsNative = false;
		};

		Launcher(EditorWindow* editor);

		void Render(float deltaTime);
		Window* GetWindow();

		void OpenProject(const std::string& path, bool isNativeProject);
		void CreateProject(const std::string& projectName, std::filesystem::path projectPath, Ref<TemplateProject> templateProject);

	private:
		void RenderProjects();
		void RenderCreateNewProjects();

	public:
		uint32_t m_SelectedTab = 0;
		Array<Ref<TemplateProject>> m_TemplateProjects;
		Ref<TemplateProject> m_SelectedProject;

		bool m_SelectedContentOnlyTab = false;
		std::string m_ProjectName = "MyProject";
		std::filesystem::path m_ProjectPath = std::filesystem::current_path();

		friend class EditorWindow;
	};

}