#include "Precompiled.h"
#include "AssetManager.h"
#include <unordered_map>
#include <future>
#include <thread>
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Suora/Assets/SuoraProject.h"
#include "Suora/Core/Engine.h"
#include "Platform/Platform.h"

#include "Mesh.h"
#include "Material.h"
#include "Font.h"
#include "Texture2D.h"
#include "ShaderGraph.h"

namespace Suora
{

	static std::unordered_map<String, Class> AssetClasses;

	static void PreInitializeAsset(Asset* asset)
	{
		const String str = Platform::ReadFromFile(asset->m_Path.string());
		asset->PreInitializeAsset(str);
	}

	static String GetCorrespondingAssetExtension(const Class& cls)
	{
		return Asset::GetAssetExtensionByClass(cls.GetNativeClassID());
	}
	static Class GetCorrespondingAssetClass(const String& fileExt)
	{
		if (fileExt == ".png" || fileExt == ".jpg")
			return Texture2D::StaticClass();

		if (fileExt == ".obj" || fileExt == ".fbx")
			return Mesh::StaticClass();

		return Class::None;
	}

	Asset* AssetManager::CreateMissingAsset(const Class& cls, const SuoraID& id)
	{
		Asset* asset = Cast<Asset>(New(cls));
		asset->m_UUID = id;
		s_Assets.Add(asset);
		asset->SetFlag(AssetFlags::Missing);
		return asset;
	}

	void AssetManager::Initialize(const FilePath& contentPath)
	{
		ProjectSettings::s_SeekingProjectSettings = true;
		HotReload(contentPath, ProjectSettings::StaticClass());
		ProjectSettings::s_SeekingProjectSettings = false;

		if (ProjectSettings* project = GetFirstAssetOfType<ProjectSettings>())
		{
			s_ProjectAssetPath = contentPath.string();
			if (project->GetEnginePath() == "")
			{
				s_EngineAssetPath = s_ProjectAssetPath;
			}
			else
			{
				s_EngineAssetPath = project->GetEnginePath() + "/Content";
			}
		}
		else
		{
			s_ProjectAssetPath = "";
			s_EngineAssetPath = contentPath.string();
		}

		// Now, actually load all Assets...
		HotReload(s_EngineAssetPath);
		if (s_ProjectAssetPath != "" && s_ProjectAssetPath != s_EngineAssetPath)
		{
			HotReload(s_ProjectAssetPath);
		}

	}

	AssetManager::~AssetManager()
	{
		for (Asset* asset : s_Assets.GetData())
		{
			delete asset;
		}
	}

	void AssetManager::HotReload(const std::filesystem::path& contentPath, const Class& baseClass)
	{
		std::vector<DirectoryEntry> Entries = File::GetAllAbsoluteEntriesOfPath(contentPath);
		
		for (auto& file : Entries)
		{
			if (GetAssetByPath(file)) continue;

			Asset* asset = nullptr;

			const String ext = File::GetFileExtension(file);
			const Class cls = Asset::GetAssetClassByExtension(ext);
			if (!cls.Inherits(baseClass)) continue;
			if (cls != Asset::StaticClass()) asset = Cast<Asset>(New(cls));

			if (asset)
			{
				const FilePath path = file;
				asset->m_Path = path;
				asset->m_Name = path.filename().string();
				s_Assets.Add(asset);
			}

		}

		InitializeAllAssets();

	}

	void AssetManager::InitializeAllAssets()
	{
		for (int i = 0; i < s_Assets.Size(); i++)
		{
			if (!s_Assets[i]->IsFlagSet(AssetFlags::WasPreInitialized) && !s_Assets[i]->IsFlagSet(AssetFlags::Missing))
			{
				const String str = Platform::ReadFromFile(s_Assets[i]->m_Path.string());
				s_Assets[i]->PreInitializeAsset(str);
			}
		}
		std::unordered_map<String, Asset*> UsedUUIDs;
		for (int i = 0; i < s_Assets.Size(); i++)
		{
			if (s_Assets[i]->IsFlagSet(AssetFlags::Missing))
			{
				for (int j = i + 1; j < s_Assets.Size(); j++)
				{
					if (s_Assets[i]->m_UUID == s_Assets[j]->m_UUID && s_Assets[i]->GetClass() == s_Assets[j]->GetClass() && !s_Assets[j]->IsFlagSet(AssetFlags::Missing))
					{
						s_Assets[i]->ClearFlag(AssetFlags::Missing);
						s_Assets[i]->m_Path = s_Assets[j]->m_Path;
						s_Assets[i]->m_Name = s_Assets[j]->m_Name;
						const String str = Platform::ReadFromFile(s_Assets[i]->m_Path.string());
						s_Assets[i]->PreInitializeAsset(str);

						delete s_Assets[j];
						s_Assets.RemoveAt(j);
						break;
					}
				}
			}
			
			if (s_Assets[i]->IsFlagSet(AssetFlags::Missing))
			{
				// In this case, the Missing Asset was not resolved! -> skip
				continue;
			}

			if (UsedUUIDs.find(s_Assets[i]->m_UUID.GetString()) != UsedUUIDs.end())
			{
				SuoraError("Colliding Asset UUIDs: {0}", s_Assets[i]->m_UUID.GetString());
				SuoraError("   -> {0}", s_Assets[i]->m_Name);
				SuoraError("   -> {0}", UsedUUIDs[s_Assets[i]->m_UUID.GetString()]->m_Name);
				SuoraAssert(false);
			}
			else
			{
				UsedUUIDs[s_Assets[i]->m_UUID.GetString()] = s_Assets[i];
			}
		}
		for (int i = 0; i < s_Assets.Size(); i++)
		{
			if (!s_Assets[i]->IsFlagSet(AssetFlags::WasInitialized) && !s_Assets[i]->IsFlagSet(AssetFlags::Missing))
			{
				const String str = Platform::ReadFromFile(s_Assets[i]->m_Path.string());
				s_Assets[i]->InitializeAsset(str);
			}
		}
	}

	void AssetManager::Update(float deltaTime)
	{
		for (int i = s_AssetStreamPool.Size() - 1; i >= 0; i--)
		{
			if (Mesh* mesh = s_AssetStreamPool[i]->As<Mesh>())
			{
				//mesh->GetVertexArray();
			}
			else if (Texture2D* texture = s_AssetStreamPool[i]->As<Texture2D>())
			{
				//texture->GetTexture();
			}
			else
			{
				SuoraError("AssetManager::Update(float): Missing 'StreamAssetClass' implementation. Cannot resolve possible stream blockage!");
			}
		}

		if (s_AssetHotReloading)
		{
			for (int i = 0; i < s_AssetHotReloadingCount; i++)
			{
				s_AssetHotReloadingIteratorIndex %= s_Assets.Size();
				Asset* asset = s_Assets[s_AssetHotReloadingIteratorIndex++];

				if (asset->IsMissing() || !asset->IsFlagSet(AssetFlags::WasPreInitialized))
				{
					continue;
				}

				if (asset->IsAssetReloadRequired())
				{
					asset->ReloadAsset();
				}
			}
		}
	}
	
	void AssetManager::RemoveAsset(Asset* asset)
	{
		if (!asset) return;

		asset->SetFlag(AssetFlags::Missing);
		asset->RemoveAsset();
	}
	void AssetManager::RenameAsset(Asset* asset, const String& name)
	{
		Platform::RenameFile(asset->m_Path, name);

		asset->m_Name = name;
		const String ext = asset->m_Path.extension().string();
		asset->m_Path = asset->m_Path.parent_path() / (name + ext);
	}

	void AssetManager::LoadAsset(const String& path)
	{
		if (GetAssetByPath(path)) return;

		DirectoryEntry file = DirectoryEntry(path);
		Asset* asset = nullptr;

		const String ext = File::GetFileExtension(file);
		const Class cls = Asset::GetAssetClassByExtension(ext);
		if (cls != Asset::StaticClass()) asset = Cast<Asset>(New(cls));

		if (asset)
		{
			const FilePath path = file;
			asset->m_Path = path;
			asset->m_Name = path.filename().string();
			s_Assets.Add(asset);

			asset->PreInitializeAsset(Platform::ReadFromFile(asset->m_Path.string()));
			asset->InitializeAsset(Platform::ReadFromFile(asset->m_Path.string()));
		}
	}

	Array<Asset*> AssetManager::GetAssetsByClass(Class type)
	{
		Array<Asset*> array;
		for (Asset* asset : s_Assets)
		{
			if (Asset* a = Cast(asset, type)) array.Add(a);
		}
		return array;
	}

	Asset* AssetManager::GetAssetByPath(const std::filesystem::path& path)
	{
		for (Asset* asset : s_Assets)
		{
			if (asset->m_Path == path) return asset;
		}
		return nullptr;
	}

	Asset* AssetManager::CreateAsset(const Class& assetClass, const String& name, const String& dir)
	{
		Asset* asset = New(assetClass)->As<Asset>();
		std::vector<String> exts = asset->GetAssetExtensions();
		s_Assets.Add(asset);
		asset->m_Name = name;
		asset->m_Path = dir + "/" + name + (exts.size() > 0 ? exts[0] : ".asset");
		asset->m_UUID = SuoraID::Generate();

		return asset;
	}

	uint32_t AssetManager::GetAssetStreamCountLimit()
	{
		return 2;
	}

}