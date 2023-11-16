#pragma once
#include "InputMappingPanel.h"

namespace Suora
{
	InputMappingPanel::InputMappingPanel()
	{
		m_AssetClass = InputMapping::StaticClass();
	}
	InputMappingPanel::InputMappingPanel(InputMapping* input)
	{
		m_AssetClass = InputMapping::StaticClass();
		Init();
	}
	void InputMappingPanel::Init()
	{
		Super::Init();
		m_Name = "InputMapping";

		Ref<DockingSpace> ds1 = CreateRef<DockingSpace>(0, 0, 1, 1, this);
		m_DockspacePanel.m_DockingSpaces.Add(ds1);

		Ref<DetailsPanel> details = CreateRef<DetailsPanel>(this);
		details->m_Data = m_Asset->As<InputMapping>();
		ds1->m_MinorTabs.Add(details);
	}
	Texture* InputMappingPanel::GetIconTexture()
	{
		return AssetManager::GetAsset<Texture2D>(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"))->GetTexture();
	}
	void InputMappingPanel::DrawToolbar(float& x, float y, float height)
	{
		Super::DrawToolbar(x, y, height);

	}
	void InputMappingPanel::SaveAsset()
	{
		Super::SaveAsset();

		Yaml::Node root;
		m_Asset->As<InputMapping>()->Serialize(root);
		String out;
		Yaml::Serialize(root, out);
		Platform::WriteToFile(m_Asset->m_Path.string(), out);
	}

}