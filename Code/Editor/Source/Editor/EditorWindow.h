#pragma once
#include "Suora/Core/Window.h"
#include "Suora/Renderer/GraphicsContext.h"

namespace Suora
{
	class Asset;
	class MajorTab;
	class MinorTab;
	class Texture2D;
	class Launcher;

	enum class EditorInputEvent
	{
		None = 0,
		Window_Resizing,
		Window_Dragging,
		Dockspace_Resizing,
		Dockspace_PrepareDragging,
		Dockspace_Dragging,
		EditorUI_DragFloat,
		EditorUI_SliderFloat,
		EditorUI_PopupMenu,
		EditorUI_Overlay,
		EditorUI_ScrollbarVertical,
		DetailsPanel_MoveSperator,
		Viewport_EditorCamera,
		NodeGraph_DragNode,
		NodeGraph_DragWire,
		TransformGizmo,
		ContentBrowser_AssetDrag,
		EditorWindow_Splashscreen
	};

	class EditorWindow
	{
	public:
		EditorWindow();
		~EditorWindow();

		void Update(float deltaTime);
		void Render(float deltaTime);

	private:
		void RegisterMajortab(MajorTab* majorTab);
	public:
		template<class T, class ... Args>
		void AddMajorTab(Args&&... args)
		{
			SelectedMajorTab = Tabs.Size();
			Tabs.Add(Ref<MajorTab>(new T(std::forward<Args>(args)...)));
			Ref<MajorTab> majorTab = Tabs[Tabs.Last()];
			RegisterMajortab(majorTab.get());
		}
		void DelegateChanges(MajorTab* majorTab);
		void OpenAsset(Asset* asset);
		void CloseAsset(Asset* asset);
		void StopPlayInEditor();

		void RenderSelectedMajorTab(float deltaTime);
		void RenderTooltip(float deltaTime);
		void RenderBottomBarAndHeroTool(float deltaTime);
		void RenderTitlebar(float deltaTime);

		Window* GetWindow();
		static EditorWindow* GetCurrent() { return s_Current; }

		void CopyAssetFilesToDirectory(const std::filesystem::path& directory, const std::filesystem::path& toBeCopied);
		void HandleAssetFileDrop(Array<String> paths);
		void ForceOpenContentDrawer();

		EditorInputEvent m_InputEvent = EditorInputEvent::None;
	private:
		Window* m_Window;
		Ref<Launcher> m_Launcher;
		Array<Ref<MajorTab>> Tabs;
		int32_t SelectedMajorTab = -1;
		Ref<MajorTab> m_PrivateMajorTab;

		bool m_WasEditorStartupAssetOpened = false;

		bool m_HeroToolOpened = false;
		float m_HeroToolHeight = 0.0f;
		Array<Ref<MinorTab>> m_HeroTools;
		uint32_t m_SelectedHeroTool = 0;

		uint32_t m_ConsoleDebugs = 0;
		uint32_t m_ConsoleWarnings = 0;
		uint32_t m_ConsoleErrors = 0;

		inline static EditorWindow* s_Current = nullptr;
	};

}