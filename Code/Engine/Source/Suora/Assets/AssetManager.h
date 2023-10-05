#pragma once
#include <vector>
#include <string>
#include <thread>
#include <filesystem>
#include "Asset.h"
#include "Suora/Core/EngineSubSystem.h"
#include "Suora/Common/Filesystem.h"
#include "Suora/Common/SuoraID.h"
#include "Suora/Common/Array.h"

#define ASSET_STREAM_COUNT_LIMIT 6

namespace Suora
{
	class Mesh;
	class Font;
	class Texture2D;

	class AssetManager : public EngineSubSystem
	{
	private:
		inline static Array<Asset*> s_Assets;
		inline static std::string s_AssetRootPath = "", s_ProjectAssetPath = "";
		inline static uint32_t s_AssetHotReloadingIteratorIndex = 0;
		inline static Array<Asset*> s_AssetStreamPool;

		static Asset* CreateMissingAsset(const Class& cls, const SuoraID& id);

	public:
		inline static bool s_AssetHotReloading = false;
		inline static uint32_t s_AssetHotReloadingCount = 8;

		static void Initialize(const FilePath& contentPath);
		~AssetManager();

		static void HotReload(const std::filesystem::path& contentPath = s_AssetRootPath, const Class& baseClass = Asset::StaticClass());
		static void InitializeAllAssets();

		static void Update(float deltaTime);

		template<class T>
		static void RegisterAsset(T* asset)
		{
			s_Assets.Add(asset);
		}

		static void RemoveAsset(Asset* asset);
		static void RenameAsset(Asset* asset, const std::string& name);
		static void LoadAsset(const std::string& path);

		template<class T>
		static T* GetFirstAssetOfType()
		{
			for (Asset* asset : s_Assets)
			{
				if (T* a = Cast<T>(asset)) return a;
			}
			return nullptr;
		}
		template<class T>
		static T* GetAsset(const SuoraID& id)
		{
			if (id.GetString() == "0") return nullptr;

			Array<T*> assets = GetAssets<T>();
			for (T* asset : assets)
			{
				if (asset->m_UUID == id)
				{
					return asset;
				}
			}
			return Cast<T>(CreateMissingAsset(T::StaticClass(), id));
		}
		template<class T>
		static T* GetAssetByName(const std::string& name)
		{
			for (Asset* asset : s_Assets)
			{
				if (T* a = Cast<T>(asset))
				{
					if (a->m_Name == name) return a;
				}
			}
			return nullptr;
		}
		template<class T>
		static Array<T*> GetAssets()
		{
			Array<T*> array;
			for (Asset* asset : s_Assets)
			{
				if (T* a = Cast<T>(asset)) array.Add(a);
			}
			return array;
		}
		static Array<Asset*> GetAssetsByClass(Class type)
		{
			Array<Asset*> array;
			for (Asset* asset : s_Assets)
			{
				if (Asset* a = Cast(asset, type)) array.Add(a);
			}
			return array;
		}
		static Asset* GetAssetByPath(const std::filesystem::path& path)
		{
			for (Asset* asset : s_Assets)
			{
				if (asset->m_Path == path) return asset;
			}
			return nullptr;
		}
		static Asset* CreateAsset(const Class& assetClass, const std::string& name, const std::string& dir);
		template<class T>
		static T* CreateAsset(const std::string& name, const std::string& dir)
		{
			return CreateAsset(T::StaticClass(), name, dir)->As<T>();
		}

		static std::string GetAssetRootPath()
		{
			return s_AssetRootPath;
		}
		static std::string GetProjectAssetPath()
		{
			return s_ProjectAssetPath;
		}
		static void SetProjectAssetPath(const std::string& path)
		{
			s_ProjectAssetPath = path;
		}

		friend class Engine;
		friend class Mesh;
		friend class Texture2D;
		friend class AssetPreview;
		friend struct EditorUI;
		friend class EditorWindow;
		friend class RenderPipeline;
	};

}