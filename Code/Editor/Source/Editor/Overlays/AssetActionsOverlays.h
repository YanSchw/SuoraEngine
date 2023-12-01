#pragma once
#include "DragableOverlay.h"
#include "Suora/GameFramework/Node.h"
#include <functional>

namespace Suora
{
	class Asset;

	struct CreateNewFolderOverlay : public DragableOverlay
	{
		CreateNewFolderOverlay(const String& directory)
			: DragableOverlay("Create an Empty Folder"), m_Directory(directory)
		{
			s_FolderName = "Empty";
		}

		inline static String s_FolderName;
		String m_Directory;

		virtual void Render(float deltaTime) override;
		void CreateFolder();
	};

	struct RenameAssetOverlay : public DragableOverlay
	{
		RenameAssetOverlay(Asset* asset, const String& name)
			: DragableOverlay("Rename Asset"), m_Asset(asset)
		{
			s_Name = name;
		}

		inline static String s_Name;
		Asset* m_Asset = nullptr;

		virtual void Render(float deltaTime) override;
	};

	struct CreateAssetOverlay : public DragableOverlay
	{
		CreateAssetOverlay(const String& directory, const String& title, const String& defaultAssetName)
			: DragableOverlay(title), m_Directory(directory)
		{
			s_AssetName = defaultAssetName;
		}

		inline static String s_AssetName;
		String m_Directory;

		virtual void Init();
		virtual void Render(float deltaTime) override;
		virtual String GetAssetExtension();

		virtual void CreateAsset();
	private:
		bool m_WasInitialized = false;
	};
	struct CreateSimpleAssetOverlay : public CreateAssetOverlay
	{
		CreateSimpleAssetOverlay(const String& directory, const String& title, const String& defaultAssetName, const Class& assetClass, const std::function<void(Asset*)>& lambda)
			: CreateAssetOverlay(directory, title, defaultAssetName), m_AssetClass(assetClass), m_Lambda(lambda)
		{
		}
		Class m_AssetClass = Class::None;
		std::function<void(Asset*)> m_Lambda;

		void Render(float deltaTime) override;
		String GetAssetExtension() override;
		void CreateAsset() override;
	};

	struct CreateClassOverlay : public CreateAssetOverlay
	{
		CreateClassOverlay(const String& directory)
			: CreateAssetOverlay(directory, "Create a new Blueprint Class", "MyBlueprint")
		{
		}
		EditorUI::SubclassHierarchyBoxParams _Params;
		int m_ClassTab = 0;
		Class m_ParentClass = Node3D::StaticClass();

		void Render(float deltaTime) override;
		void CreateAsset() override;


		virtual String GetAssetExtension() override
		{
			return ".node";
		}
	};
	struct CreateLevelOverlay : public CreateAssetOverlay
	{
		CreateLevelOverlay(const String& directory)
			: CreateAssetOverlay(directory, "Create a new Level", "MyLevel")
		{
		}
		EditorUI::SubclassHierarchyBoxParams _Params;
		Class m_ParentClass = LevelNode::StaticClass();

		void Render(float deltaTime) override;
		void CreateAsset() override;

		virtual String GetAssetExtension() override
		{
			return ".level";
		}
	};

	struct ImportTexture2DOverlay : public CreateAssetOverlay
	{
		ImportTexture2DOverlay(const std::filesystem::path& filePath, const String& directory)
			: CreateAssetOverlay(directory, "Import " + filePath.string(), filePath.stem().string())
		{
			m_SourceFile = filePath;
		}

		virtual void Render(float deltaTime) override;
		virtual String GetAssetExtension() override;

		virtual void CreateAsset() override;

	private:
		std::filesystem::path m_SourceFile;
		Ref<Texture> m_PreviewTexture = nullptr;
	};
	struct ImportMeshOverlay : public CreateAssetOverlay
	{
		ImportMeshOverlay(const std::filesystem::path& filePath, const String& directory)
			: CreateAssetOverlay(directory, "Import " + filePath.string(), filePath.stem().string())
		{
			m_SourceFile = filePath;
		}

		virtual void Render(float deltaTime) override;
		virtual String GetAssetExtension() override;

		virtual void CreateAsset() override;

		class ShaderGraph* m_Shadergraph = nullptr;
		bool m_CreateMaterials = false;

	private:
		std::filesystem::path m_SourceFile;
	};

}