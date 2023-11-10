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

	void StreamableAsset::PreInitializeAsset(const String& str)
	{
		Super::PreInitializeAsset(str);

		Yaml::Node root;
		Yaml::Parse(root, str);
		Yaml::Node& streamable = root["StreamableAsset"];

		m_SourceAssetName = streamable["m_SourceAssetName"].As<String>();
		m_StreamMode = (AssetStreamMode)std::stoi(streamable["m_StreamMode"].As<String>());

		m_LastWriteTimeOfSource = std::filesystem::last_write_time(GetSourceAssetPath());
	}
	void StreamableAsset::InitializeAsset(const String& str)
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

	bool StreamableAsset::IsAssetReloadRequired() const
	{
		if (Super::IsAssetReloadRequired())
		{
			return true;
		}

		return m_LastWriteTimeOfSource != std::filesystem::last_write_time(GetSourceAssetPath());
	}

	void StreamableAsset::ReloadAsset()
	{
		Super::ReloadAsset();

		m_LastWriteTimeOfSource = std::filesystem::last_write_time(GetSourceAssetPath());
	}

	void StreamableAsset::SetSourceAssetName(const String& name)
	{
		m_SourceAssetName = name;
	}
	String StreamableAsset::GetSourceAssetName() const
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