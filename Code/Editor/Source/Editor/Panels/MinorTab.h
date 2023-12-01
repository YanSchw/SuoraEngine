#pragma once
#include <string>
#include "Suora/Core/Base.h"
#include "Suora/Common/Array.h"
#include "Editor/EditorUI.h"

namespace Suora
{
	class MajorTab;
	struct DockingSpace;
	class Texture2D;
	enum class EditorInputEvent;

	class MinorTab
	{
		Ptr<MajorTab> m_MajorTab;
	public:
		String Name = "MinorTab";
		Ref<Framebuffer> m_Framebuffer;
		uint32_t Width = 100, Height = 100;
		Texture2D* m_IconTexture = nullptr;

		MinorTab(MajorTab* majorTab);

		void Update(float deltaTime, uint32_t width, uint32_t height);
		virtual void Render(float deltaTime) = 0;
		int GetWidth() const;
		int GetHeight() const; 
		Vec2 GetMinorMousePos() const;
		MajorTab* GetMajorTab() const;
	private:
		void SetMajorTab(MajorTab* majorTab);
	public:

		bool IsInputValid();
		bool IsInputMode(EditorInputEvent event);
		void SetInputMode(EditorInputEvent event);

		// Editor UI
		void Subframe(MinorTab& subframe, float x, float y, float width, float height);

		friend class MajorTab;
		friend class EditorWindow;
	};
}