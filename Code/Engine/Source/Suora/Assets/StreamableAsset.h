#pragma once
#include <filesystem>
#include "Asset.h"
#include "StreamableAsset.generated.h"

namespace Suora
{
	enum class AssetStreamMode : int32_t
	{
		AlwaysLoaded = 0,
		StreamOnDemand
	};

	class StreamableAsset : public Asset
	{
		SUORA_CLASS(175815436);
	public:
		std::filesystem::path GetSourceAssetPath() const;
		bool IsSourceAssetPathValid() const;

		void PreInitializeAsset(const String& str) override;
		void InitializeAsset(const String& str) override;
		void Serialize(Yaml::Node& root);

		virtual bool IsAssetReloadRequired() const override;
		virtual void ReloadAsset() override;

		void SetSourceAssetName(const String& name);
		String GetSourceAssetName() const;
		void SetAssetStreamMode(AssetStreamMode streamMode);
		AssetStreamMode GetAssetStreamMode() const;

	private:
		String m_SourceAssetName;
		AssetStreamMode m_StreamMode = AssetStreamMode::AlwaysLoaded;
		std::filesystem::file_time_type m_LastWriteTimeOfSource;
	};

}