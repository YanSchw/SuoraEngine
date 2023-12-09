#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include "Asset.h"
#include "Suora/Core/Object/Object.h"
#include "Suora/Core/EngineSubSystem.h"
#include "Suora/Common/Filesystem.h"
#include "Suora/Common/SuoraID.h"
#include "Suora/Common/Array.h"
#include "Suora/Common/Map.h"

namespace Suora
{
	class Mesh;
	class Font;
	class Texture2D;

	/* The AssetManager class is responsible for managing assets in the Suora Engine. */
	class AssetManager : public EngineSubSystem
	{
	private:
		// Static members
		inline static Array<Asset*> s_Assets;
		inline static String s_EngineAssetPath = "", s_ProjectAssetPath = "";
		inline static uint32_t s_AssetHotReloadingIteratorIndex = 0;
		inline static Array<Asset*> s_AssetStreamPool;

		// Private Function to create a missing Asset of a specified Class and ID.
		static Asset* CreateMissingAsset(const Class& cls, const SuoraID& id);

	public:
		// Static public members
		inline static bool s_AssetHotReloading = false;
		inline static uint32_t s_AssetHotReloadingCount = 8;

		// Initializes the AssetManager with the provided content path.
		static void Initialize(const FilePath& contentPath);
		~AssetManager();

		/* Hot reloads assets with optional parameters for content path and base class. 
		 * Only derivatives of baseClass are reloaded.                                  */
		static void HotReload(const std::filesystem::path& contentPath = s_EngineAssetPath, const Class& baseClass = Asset::StaticClass());
		static void InitializeAllAssets();

		static void Update(float deltaTime);

		/* Registers an asset of type T to be managed by the AssetManager. */
		template<class T>
		static void RegisterAsset(T* asset)
		{
			s_Assets.Add(asset);
		}

		/* Removes the specified asset from the AssetManager.
		 * The Asset will remain in Memory and will be flagged as 'Missing' */
		static void RemoveAsset(Asset* asset);
		static void RenameAsset(Asset* asset, const String& name);
		static void LoadAsset(const String& path);

		template<class T>
		static T* GetFirstAssetOfType()
		{
			for (Asset* asset : s_Assets)
			{
				if (T* a = Cast<T>(asset)) return a;
			}
			return nullptr;
		}
		static Asset* GetAsset(const Class& assetClass, const SuoraID& id);
		template<class T>
		static T* GetAsset(const SuoraID& id)
		{
			return GetAsset(T::StaticClass(), id)->As<T>();
		}
		template<class T>
		static T* GetAssetByName(const String& name)
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
		static Array<Asset*> GetAssetsByClass(Class type);
		static Asset* GetAssetByPath(const std::filesystem::path& path);
		static Asset* CreateAsset(const Class& assetClass, const String& name, const String& dir);

		template<class T>
		static T* CreateAsset(const String& name, const String& dir)
		{
			return dynamic_cast<T*>(CreateAsset(T::StaticClass(), name, dir));
		}

		static uint32_t GetAssetStreamCountLimit();

		static String GetEngineAssetPath()
		{
			return s_EngineAssetPath;
		}
		static String GetProjectAssetPath()
		{
			return s_ProjectAssetPath;
		}
		static void SetProjectAssetPath(const String& path)
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