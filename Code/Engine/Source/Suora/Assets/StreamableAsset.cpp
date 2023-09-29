#include "Precompiled.h"
#include "StreamableAsset.h"

namespace Suora
{
	std::filesystem::path StreamableAsset::GetSourceAssetPath() const
	{
		std::filesystem::path p = m_Path;
		return p.replace_filename(m_SourceAssetName);
	}

	bool StreamableAsset::IsSourceAssetPathValid() const
	{
		return std::filesystem::exists(GetSourceAssetPath());
	}

	void StreamableAsset::PreInitializeAsset(const std::string& str)
	{
		Super::PreInitializeAsset(str);

		Yaml::Node root;
		Yaml::Parse(root, str);
		Yaml::Node& streamable = root["StreamableAsset"];

		m_SourceAssetName = streamable["m_SourceAssetName"].As<std::string>();
		m_StreamMode = (AssetStreamMode)std::stoi(streamable["m_StreamMode"].As<std::string>());
	}
	void StreamableAsset::InitializeAsset(const std::string& str)
	{
		Super::InitializeAsset(str);
	}

	void StreamableAsset::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		Yaml::Node& streamable = root["StreamableAsset"];

		streamable["m_SourceAssetName"] = m_SourceAssetName;
		streamable["m_StreamMode"] = std::to_string((int32_t)m_StreamMode);
	}

	void StreamableAsset::SetSourceAssetName(const std::string& name)
	{
		m_SourceAssetName = name;
	}
	std::string StreamableAsset::GetSourceAssetName() const
	{
		return m_SourceAssetName;
	}
	void StreamableAsset::SetAssetStreamMode(AssetStreamMode streamMode)
	{
		m_StreamMode = streamMode;
	}
	AssetStreamMode StreamableAsset::GetAssetStreamMode() const
	{
		return m_StreamMode;
	}
}