#pragma once
#include <iostream>
#include <stdint.h>
#include "Suora/Common/Filesystem.h"
#include "Suora/Common/SuoraID.h"
#include "Suora/Core/Object/Object.h"
#include "Suora/Serialization/Yaml.h"
#include "Asset.generated.h"

namespace Suora
{
	
	enum class AssetFlags : uint32_t
	{
		None = 0,
		Missing = 1,
		WasPreInitialized = 2,
		WasInitialized = 4,
	};

	inline AssetFlags operator|(AssetFlags a, AssetFlags b)
	{
		return static_cast<AssetFlags>(static_cast<int32_t>(a) | static_cast<int32_t>(b));
	}
	inline AssetFlags operator&(AssetFlags a, AssetFlags b)
	{
		return static_cast<AssetFlags>(static_cast<int32_t>(a) & static_cast<int32_t>(b));
	}

	/** Baseclass for all Assets */
	class Asset : public Object
	{
		SUORA_CLASS(43786347);
	public:
		std::string m_Name = "Missing Asset";
		SuoraID m_UUID;
		FilePath m_Path;
		AssetFlags m_Flags = AssetFlags::None;

		virtual void PreInitializeAsset(const std::string& str);
		virtual void InitializeAsset(const std::string& str);
		virtual void Serialize(Yaml::Node& root);

		virtual bool IsLoaded() const
		{
			return false;
		}

		std::string GetAssetName() const
		{
			return m_Path.stem().string();
		}

		virtual uint32_t GetAssetFileSize()
		{
			return std::filesystem::exists(m_Path) ? std::filesystem::file_size(m_Path) : 0;
		}

		virtual void RemoveAsset();

		inline bool IsFlagSet(AssetFlags flag) const
		{
			return 0 != ((int32_t)m_Flags & (int32_t)flag);
		}
		inline void SetFlag(AssetFlags flag)
		{
			m_Flags = m_Flags | flag;
		}
		inline void ClearFlag(AssetFlags flag)
		{
			m_Flags = m_Flags & (AssetFlags)(~((uint32_t)flag));
		}

		inline bool IsMissing() const
		{
			return IsFlagSet(AssetFlags::Missing);
		}

		virtual std::vector<std::string> GetAssetExtensions() { return {}; }
		static Class GetAssetClassByExtension(const std::string& ext);
		static std::string GetAssetExtensionByClass(NativeClassID id);
	};

}

#define ASSET_EXTENSION(...) \
virtual std::vector<std::string> GetAssetExtensions() override { return {__VA_ARGS__}; }
