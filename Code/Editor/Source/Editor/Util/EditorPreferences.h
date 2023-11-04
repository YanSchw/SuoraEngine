#pragma once
#include "Suora.h"
#include "EditorPreferences.generated.h"

#include <glm/glm.hpp>
#include <random>
#include <unordered_map>

namespace Suora
{
	using Color = glm::vec4;
	using NativeClassID = uint64_t;

	class EditorPreferences : public Asset
	{
		SUORA_CLASS(54319524);
		ASSET_EXTENSION(".edpref");
	public:

		static Color GetAssetClassColor(NativeClassID AssetClassID);

		static EditorPreferences* Get();

		void PreInitializeAsset(const std::string& str) override;
		void InitializeAsset(const std::string& str) override;
		void Serialize(Yaml::Node& root) override;

		uint32_t GetAssetFileSize() override;

		EditorPreferences();

		Array<std::string> m_AllCachedProjectPaths;
		float UiScale = 1;
		Color UiColor = Color(0.14717647f, 0.14717647f, 0.15217647f, 1.0f); //Color(0.2f, 0.2f, 0.21f, 1.0f); //Color(0.26f, 0.27f, 0.29f, 1);
		Color UiForgroundColor = Color(0.2f, 0.20392157f, 0.211764706f, 1.0f); // Color(0.18531372f, 0.18531372f, 0.18531372f, 1.0f);
		Color UiBackgroundColor = Color(0.101961f, 0.105882f, 0.1198039f, 1.0f); // Color(0.10216078f, 0.10216078f, 0.10216078f, 1.0f);
		Color UiHighlightColor = Color(0.4f, 0.64f, 1.0f, 1.0f);// = Color(.87f, .64f, .04f, 1.0f); // = Color(.87f, .64f, .04f, 1.0f); for Orange
		Color UiTitlebarColor = Color(0.05882f, 0.05882f, 0.05882f, 1.0f);
		Color UiInputColor = Color(0.05882f, 0.05882f, 0.05882f, 1.0f);
		Color UiTextColor = Color(0.75f, 0.75f, 0.75f, 1.0f);

		bool m_AutoImportTextures = true;
	};

}