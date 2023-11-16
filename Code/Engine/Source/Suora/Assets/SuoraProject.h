#pragma once
#include "Asset.h"
#include "SuoraProject.generated.h"

namespace Suora
{
	class Level;
	class BaseRenderPipeline;
	class GameInstance;
	class Texture2D;

	class ProjectSettings : public Asset
	{
		SUORA_CLASS(923476114);
		ASSET_EXTENSION(".suora");

	public:
		ProjectSettings();

		void PreInitializeAsset(const String& str) override;
		void InitializeAsset(const String& str) override;
		void Serialize(Yaml::Node& root) override;

		String GetEnginePath() const;
		void SetEnginePath(const String& path);

		static ProjectSettings* Get();
		static String GetProjectName();

		float m_TargetFramerate = 60.0f;
		bool m_EnableDeferredRendering = true;
		SubclassOf<GameInstance> m_GameInstanceClass;
		Level* m_DefaultLevel = nullptr;
		Texture2D* m_ProjectIconTexture = nullptr;

		Asset* m_EditorStartupAsset = nullptr;
		bool m_IsNativeProject = true;
	private:
		String m_EnginePath = String();

		inline static bool s_SeekingProjectSettings = false;
		friend class AssetManager;
	};

}