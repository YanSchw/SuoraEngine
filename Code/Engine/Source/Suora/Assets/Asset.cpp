#include "Precompiled.h"
#include "Asset.h"
#include "Suora/Serialization/Yaml.h"
#include "Platform/Platform.h"

namespace Suora
{

	void Asset::PreInitializeAsset(Yaml::Node& root)
	{
		m_UUID = SuoraID(root["UUID"].As<String>());
		m_LastWriteTime = std::filesystem::last_write_time(m_Path);

		SetFlag(AssetFlags::WasPreInitialized);
	}

	void Asset::InitializeAsset(Yaml::Node& root)
	{
		SetFlag(AssetFlags::WasInitialized);
	}

	void Asset::Serialize(Yaml::Node& root)
	{
		root = Yaml::Node();
		root["UUID"] = m_UUID.GetString();
	}

	bool Asset::IsAssetReloadRequired() const
	{
		return m_LastWriteTime != std::filesystem::last_write_time(m_Path);
	}

	void Asset::ReloadAsset()
	{
		m_LastWriteTime = std::filesystem::last_write_time(m_Path);
	}

	void Asset::RemoveAsset()
	{
		Platform::RemoveFile(m_Path);
	}

	Class Asset::GetAssetClassByExtension(const String& ext)
	{
		Array<Class> derivatives = Class::GetSubclassesOf(Asset::StaticClass());

		for (Class cls : derivatives)
		{
			Asset* asset = Cast<Asset>(New(cls));
			const std::vector<String> assetExtensions = asset->GetAssetExtensions();
			for (auto& str : assetExtensions)
			{
				if (str == ext)
				{
					delete asset;
					return cls;
				}
			}
			delete asset;
		}

		return Asset::StaticClass();
	}
	String Asset::GetAssetExtensionByClass(NativeClassID id)
	{
		Array<Class> derivatives = Class::GetSubclassesOf(Asset::StaticClass());

		for (auto& cls : derivatives)
		{
			Asset* asset = Cast<Asset>(New(cls));
			if (asset)
			{
				if (asset->GetNativeClass().GetNativeClassID() == id)
				{
					const String out = asset->GetAssetExtensions()[0];
					delete asset;
					return out;
				}
				delete asset;
			}
		}

		return ".";
	}

}