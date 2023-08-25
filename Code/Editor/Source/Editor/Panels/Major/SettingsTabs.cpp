#pragma once
#include "SettingsTabs.h"

namespace Suora
{
	Texture* EditorPreferencesMajorTab::GetIconTexture()
	{
		return AssetManager::GetAsset<Texture2D>(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"))->GetTexture();
	}
	Texture* ProjectSettingsMajorTab::GetIconTexture()
	{
		return AssetManager::GetAsset<Texture2D>(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"))->GetTexture();
	}
}