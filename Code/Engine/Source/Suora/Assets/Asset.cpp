#include "Precompiled.h"
#include "Asset.h"
#include "Suora/Serialization/Yaml.h"
#include "Platform/Platform.h"

namespace Suora
{

	void Asset::PreInitializeAsset(const std::string& str)
	{
		Yaml::Node root;
		Yaml::Parse(root, str);
		m_UUID = root["UUID"].As<std::string>();

		SetFlag(AssetFlags::WasPreInitialized);
	}

	void Asset::InitializeAsset(const std::string& str)
	{
		SetFlag(AssetFlags::WasInitialized);
	}

	void Asset::Serialize(Yaml::Node& root)
	{
		root = Yaml::Node();
		root["UUID"] = m_UUID.GetString();
	}

	void Asset::RemoveAsset()
	{
		Platform::RemoveFile(m_Path);
	}

	Class Asset::GetAssetClassByExtension(const std::string& ext)
	{
		Array<Class> derivatives = Class::GetSubclassesOf(Asset::StaticClass());//Asset::GetClassDerivatives();

		for (Class cls : derivatives)
		{
			Asset* asset = Cast<Asset>(New(cls));
			const std::vector<std::string> assetExtensions = asset->GetAssetExtensions();
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
	std::string Asset::GetAssetExtensionByClass(NativeClassID id)
	{
		Array<Class> derivatives = Class::GetSubclassesOf(Asset::StaticClass());//Asset::GetClassDerivatives();

		for (auto& cls : derivatives)
		{
			Asset* asset = Cast<Asset>(New(cls));
			if (asset)
			{
				if (asset->GetNativeClass().GetNativeClassID() == id)
				{
					const std::string out = asset->GetAssetExtensions()[0];
					delete asset;
					return out;
				}
				delete asset;
			}
		}

		return ".";
	}

}