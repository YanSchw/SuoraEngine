#pragma once
#include "Asset.h"
#include "SuoraProject.generated.h"

namespace Suora
{
	class Level;
	class InputSettings;
	class BaseRenderPipeline;
	class GameInstance;
	class Texture2D;

	class ProjectSettings : public Asset
	{
		SUORA_CLASS(923476114);
		ASSET_EXTENSION(".suora");

	public:
		ProjectSettings();

		void PreInitializeAsset(const std::string& str) override;
		void InitializeAsset(const std::string& str) override;
		void Serialize(Yaml::Node& root) override;

		std::string GetEnginePath() const;
		void SetEnginePath(const std::string& path);

		static ProjectSettings* Get();
		static std::string GetProjectName();

		float m_TargetFramerate = 60.0f;
		SubclassOf<GameInstance> m_GameInstanceClass;
		Level* m_DefaultLevel = nullptr;
		Ref<InputSettings> m_InputSettings = nullptr;
		Texture2D* m_ProjectIconTexture = nullptr;

		Asset* m_EditorStartupAsset = nullptr;
		bool m_IsNativeProject = true;
	private:
		std::string m_EnginePath = std::string();

		inline static bool s_SeekingProjectSettings = false;
		friend class AssetManager;
	};

}