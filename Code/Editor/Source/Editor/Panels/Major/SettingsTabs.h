#pragma once
#include "../MajorTab.h"
#include "../Minor/DetailsPanel.h"
#include "Suora/Assets/SuoraProject.h"
#include "SettingsTabs.generated.h"

namespace Suora
{
	class Texture;
	class EditorPreferencesMajorTab : public MajorTab
	{
		SUORA_CLASS(54793863);
	public:
		EditorPreferencesMajorTab()
		{
			m_AssetClass = EditorPreferences::StaticClass();
		}
		EditorPreferencesMajorTab(EditorPreferences* settings)
		{
			m_AssetClass = EditorPreferences::StaticClass();
			Init();
		}
		void Init() override
		{
			Super::Init(); 
			m_Name = "Editor Preferences";

			Ref<DockingSpace> ds1 = CreateRef<DockingSpace>(0, 0, 1, 1, this);
			m_DockspacePanel.m_DockingSpaces.Add(ds1);

			Ref<DetailsPanel> details = CreateRef<DetailsPanel>(this);
			details->m_Data = m_Asset->As<EditorPreferences>();
			ds1->m_MinorTabs.Add(details);
		}
		void Update(float deltaTime) override
		{
			Super::Update(deltaTime);
		}
		virtual Texture* GetIconTexture() override;
		void DrawToolbar(float& x, float y, float height) override
		{
			Super::DrawToolbar(x, y, height);

		}

		void SaveAsset() override
		{
			Super::SaveAsset();

			Yaml::Node root;
			EditorPreferences::Get()->Serialize(root);
			std::string out;
			Yaml::Serialize(root, out);
			Platform::WriteToFile(EditorPreferences::Get()->m_Path.string(), out);
		}

	};
	class ProjectSettingsMajorTab : public MajorTab
	{
		SUORA_CLASS(7865847);
	public:
		ProjectSettingsMajorTab()
		{
			m_AssetClass = ProjectSettings::StaticClass();
		}
		ProjectSettingsMajorTab(ProjectSettings* settings)
		{
			m_AssetClass = ProjectSettings::StaticClass();
			Init();
		}
		void Init() override
		{
			Super::Init(); 
			m_Name = "Project Settings";

			Ref<DockingSpace> ds1 = CreateRef<DockingSpace>(0, 0, 1, 1, this);
			m_DockspacePanel.m_DockingSpaces.Add(ds1);

			Ref<DetailsPanel> details = CreateRef<DetailsPanel>(this);
			details->m_Data = m_Asset->As<ProjectSettings>();
			ds1->m_MinorTabs.Add(details);
			m_ProjectSettings = m_Asset->As<ProjectSettings>();
		}
		void Update(float deltaTime) override
		{
			Super::Update(deltaTime);
		}
		virtual Texture* GetIconTexture() override;
		void DrawToolbar(float& x, float y, float height) override
		{
			Super::DrawToolbar(x, y, height);

		}

		void SaveAsset() override
		{
			Super::SaveAsset();

			Yaml::Node root;
			m_ProjectSettings->Serialize(root);
			std::string out;
			Yaml::Serialize(root, out);
			Platform::WriteToFile(m_ProjectSettings->m_Path.string(), out);
		}

		Ptr<ProjectSettings> m_ProjectSettings = nullptr;
	};

}