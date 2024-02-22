#include "Precompiled.h"
#include "SettingsTabs.h"

namespace Suora
{
	EditorPreferencesMajorTab::EditorPreferencesMajorTab()
	{
		m_AssetClass = EditorPreferences::StaticClass();
	}
	EditorPreferencesMajorTab::EditorPreferencesMajorTab(EditorPreferences* settings)
	{
		m_AssetClass = EditorPreferences::StaticClass();
		Init();
	}
	void EditorPreferencesMajorTab::Init()
	{
		Super::Init();
		m_Name = "Editor Preferences";

		Ref<DockingSpace> ds1 = CreateRef<DockingSpace>(0, 0, 1, 1, this);
		m_DockspacePanel.m_DockingSpaces.Add(ds1);

		Ref<DetailsPanel> details = CreateRef<DetailsPanel>(this);
		details->m_Data = m_Asset->As<EditorPreferences>();
		ds1->m_MinorTabs.Add(details);
	}
	Texture* EditorPreferencesMajorTab::GetIconTexture()
	{
		return AssetManager::GetAsset<Texture2D>(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"))->GetTexture();
	}
	void EditorPreferencesMajorTab::DrawToolbar(float& x, float y, float height)
	{
		Super::DrawToolbar(x, y, height);

	}
	void EditorPreferencesMajorTab::SaveAsset()
	{
		Super::SaveAsset();

		Yaml::Node root;
		EditorPreferences::Get()->Serialize(root);
		String out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(EditorPreferences::Get()->m_Path.string(), out);
	}


	ProjectSettingsMajorTab::ProjectSettingsMajorTab()
	{
		m_AssetClass = ProjectSettings::StaticClass();
	}

	ProjectSettingsMajorTab::ProjectSettingsMajorTab(ProjectSettings* settings)
	{
		m_AssetClass = ProjectSettings::StaticClass();
		Init();
	}

	void ProjectSettingsMajorTab::Init()
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

	void ProjectSettingsMajorTab::Update(float deltaTime)
	{
		Super::Update(deltaTime);
	}

	Texture* ProjectSettingsMajorTab::GetIconTexture()
	{
		return AssetManager::GetAsset<Texture2D>(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"))->GetTexture();
	}
	void ProjectSettingsMajorTab::DrawToolbar(float& x, float y, float height)
	{
		Super::DrawToolbar(x, y, height);

	}
	void ProjectSettingsMajorTab::SaveAsset()
	{
		Super::SaveAsset();

		Yaml::Node root;
		m_ProjectSettings->Serialize(root);
		String out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(m_ProjectSettings->m_Path.string(), out);
	}
}