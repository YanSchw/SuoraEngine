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
		EditorPreferencesMajorTab();
		EditorPreferencesMajorTab(EditorPreferences* settings);
		void Init() override;
		void Update(float deltaTime) override
		{
			Super::Update(deltaTime);
		}
		virtual Texture* GetIconTexture() override;
		void DrawToolbar(float& x, float y, float height) override;

		void SaveAsset() override;

	};
	class ProjectSettingsMajorTab : public MajorTab
	{
		SUORA_CLASS(7865847);
	public:
		ProjectSettingsMajorTab();
		ProjectSettingsMajorTab(ProjectSettings* settings);
		void Init() override;
		void Update(float deltaTime) override;
		virtual Texture* GetIconTexture() override;
		void DrawToolbar(float& x, float y, float height) override;

		void SaveAsset() override;

		Ptr<ProjectSettings> m_ProjectSettings = nullptr;
	};

}