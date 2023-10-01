#pragma once
#include "../MinorTab.h"
#include <Suora.h>

namespace Suora
{
	class ContentBrowser : public MinorTab
	{
		struct Folder
		{
			std::string m_FolderName;
			std::string m_TargetPath;
		};
		enum class PathMode : uint32_t
		{
			ProjectPath = 0,
			EnginePath
		};
	public:
		ContentBrowser(MajorTab* majorTab);
		~ContentBrowser();

		virtual void Render(float deltaTime) override;
		void DrawFolder(Folder& folder, float& x, float& y, int& index, const Vec2& size, bool& rightClickPossible);
		void DrawAsset(Asset* asset, float& x, float& y, int& index, const Vec2& size, bool& rightClickPossible);
		void DrawContentPaths();
		void OpenAsset(Asset* asset);

		bool ProcessElementClick(int i, Asset* selectAsset);

		float m_ScaleBrowser = 0.75f;

		inline static Ptr<Asset> s_DraggedAsset = nullptr;
		inline static Ptr<Asset> s_LastSelectedAsset = nullptr;
		inline static Ptr<Asset> s_BrowseToAsset = nullptr;

		std::string GetRootPath();
		std::string GetCurrentDirectory() const;

	private:
		Ref<Texture> m_Checkerboard = Texture::Create(AssetManager::GetAssetRootPath() + "/EngineContent/Textures/Checkerboard.png");
		Ref<Texture> m_FolderIcon = Texture::Create(AssetManager::GetAssetRootPath() + "/EditorContent/Icons/FolderNew.png");
		Ref<Texture> m_ShadowTexture = Texture::Create(AssetManager::GetAssetRootPath() + "/EditorContent/Textures/AssetEntryShadow.png");
		Ref<Texture> m_ArrowRight = Texture::Create(AssetManager::GetAssetRootPath() + "/EditorContent/Icons/ArrowRight.png");
		std::string m_CurrentPath = AssetManager::GetAssetRootPath();
		PathMode m_CurrentMode = PathMode::ProjectPath;
		float m_ScrollY = 0.0f;

		int m_SelectedElement = -1;
		float m_LastElementClick = 0.0f;

		// Asset Drag and Drop
		Vec2 m_BeginMousePos;
		bool m_CancelAssetDragNextFrame = false;
	};
}