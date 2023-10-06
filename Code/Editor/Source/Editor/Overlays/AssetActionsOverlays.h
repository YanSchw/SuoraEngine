#pragma once
#include "DragableOverlay.h"
#include <functional>

namespace Suora
{
	class Asset;

	struct CreateNewFolderOverlay : public DragableOverlay
	{
		CreateNewFolderOverlay(const std::string& directory)
			: DragableOverlay("Create an Empty Folder"), m_Directory(directory)
		{
			s_FolderName = "Empty";
		}

		inline static std::string s_FolderName;
		std::string m_Directory;

		virtual void Render(float deltaTime) override;
		void CreateFolder();
	};

	struct RenameAssetOverlay : public DragableOverlay
	{
		RenameAssetOverlay(Asset* asset, const std::string& name)
			: DragableOverlay("Rename Asset"), m_Asset(asset)
		{
			s_Name = name;
		}

		inline static std::string s_Name;
		Asset* m_Asset = nullptr;

		virtual void Render(float deltaTime) override;
	};

	struct CreateAssetOverlay abstract : public DragableOverlay
	{
		CreateAssetOverlay(const std::string& directory, const std::string& title, const std::string& defaultAssetName)
			: DragableOverlay(title), m_Directory(directory)
		{
			s_AssetName = defaultAssetName;
		}

		inline static std::string s_AssetName;
		std::string m_Directory;

		virtual void Init();
		virtual void Render(float deltaTime) override;
		virtual std::string GetAssetExtension();

		virtual void CreateAsset();
	private:
		bool m_WasInitialized = false;
	};
	struct CreateSimpleAssetOverlay : public CreateAssetOverlay
	{
		CreateSimpleAssetOverlay(const std::string& directory, const std::string& title, const std::string& defaultAssetName, const Class& assetClass, const std::function<void(Asset*)>& lambda)
			: CreateAssetOverlay(directory, title, defaultAssetName), m_AssetClass(assetClass), m_Lambda(lambda)
		{
		}
		Class m_AssetClass = Class::None;
		std::function<void(Asset*)> m_Lambda;

		void Render(float deltaTime) override;
		std::string GetAssetExtension() override;
		void CreateAsset() override;
	};

	struct CreateClassOverlay : public CreateAssetOverlay
	{
		CreateClassOverlay(const std::string& directory)
			: CreateAssetOverlay(directory, "Create a new Blueprint Class", "MyBlueprint")
		{
		}
		EditorUI::SubclassHierarchyBoxParams _Params;
		int m_ClassTab = 0;
		Class m_ParentClass = Node3D::StaticClass();

		void Render(float deltaTime) override;
		void CreateAsset() override;


		virtual std::string GetAssetExtension() override
		{
			return ".node";
		}
	};
	struct CreateLevelOverlay : public CreateAssetOverlay
	{
		CreateLevelOverlay(const std::string& directory)
			: CreateAssetOverlay(directory, "Create a new Level", "MyLevel")
		{
		}
		EditorUI::SubclassHierarchyBoxParams _Params;
		Class m_ParentClass = LevelNode::StaticClass();

		void Render(float deltaTime) override;
		void CreateAsset() override;

		virtual std::string GetAssetExtension() override
		{
			return ".level";
		}
	};

	struct ImportTexture2DOverlay : public CreateAssetOverlay
	{
		ImportTexture2DOverlay(const std::filesystem::path& filePath, const std::string& directory)
			: CreateAssetOverlay(directory, "Import " + filePath.string(), filePath.stem().string())
		{
			m_SourceFile = filePath;
		}

		virtual void Render(float deltaTime) override;
		virtual std::string GetAssetExtension() override;

		virtual void CreateAsset() override;

	private:
		std::filesystem::path m_SourceFile;
		Ref<Texture> m_PreviewTexture = nullptr;
	};
	struct ImportMeshOverlay : public CreateAssetOverlay
	{
		ImportMeshOverlay(const std::filesystem::path& filePath, const std::string& directory)
			: CreateAssetOverlay(directory, "Import " + filePath.string(), filePath.stem().string())
		{
			m_SourceFile = filePath;
		}

		virtual void Render(float deltaTime) override;
		virtual std::string GetAssetExtension() override;

		virtual void CreateAsset() override;

	private:
		std::filesystem::path m_SourceFile;
	};

}