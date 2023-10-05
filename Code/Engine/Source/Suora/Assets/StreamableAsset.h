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

		void PreInitializeAsset(const std::string& str) override;
		void InitializeAsset(const std::string& str) override;
		void Serialize(Yaml::Node& root);

		virtual bool IsAssetReloadRequired() const override;
		virtual void ReloadAsset() override;

		void SetSourceAssetName(const std::string& name);
		std::string GetSourceAssetName() const;
		void SetAssetStreamMode(AssetStreamMode streamMode);
		AssetStreamMode GetAssetStreamMode() const;

	private:
		std::string m_SourceAssetName;
		AssetStreamMode m_StreamMode = AssetStreamMode::AlwaysLoaded;
		std::filesystem::file_time_type m_LastWriteTimeOfSource;
	};

}