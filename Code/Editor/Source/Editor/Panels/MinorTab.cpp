#include "Precompiled.h"
#include "MinorTab.h"
#include "MajorTab.h"
#include "Editor/EditorWindow.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/Framebuffer.h"

namespace Suora
{
	MinorTab::MinorTab(MajorTab* majorTab)
	{
		SetMajorTab(majorTab);

		FramebufferSpecification spec;
		spec.Width = 100;
		spec.Height = 100;
		spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGBA8);
		spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
		m_Framebuffer = Framebuffer::Create(spec);

		m_IconTexture = AssetManager::GetAsset<Texture2D>(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"));
	}
	void MinorTab::Update(float deltaTime, uint32_t width, uint32_t height)
	{
		bool resize = (GetWidth() != width || GetHeight() != height);
		if (resize)
		{
			Width = width;
			Height = height;
			m_Framebuffer->Resize(width, height);
		}

		if (EditorUI::GetInput().x < 0.0f || EditorUI::GetInput().y < 0.0f || EditorUI::GetInput().x > width || EditorUI::GetInput().y > height)
		{
			if (IsInputMode(EditorInputEvent::None))
			{
				EditorUI::PushInput(-100000.0f, -100000.0f, 0.0f, 0.0f);
			}
		}
	}

	int MinorTab::GetWidth() const
	{
		return (int)Width;
	}
	int MinorTab::GetHeight() const
	{
		return (int)Height;
	}

	Vec2 MinorTab::GetMinorMousePos() const
	{
		return EditorUI::GetInput();
	}

	MajorTab* MinorTab::GetMajorTab() const
	{
		return m_MajorTab;
	}

	void MinorTab::SetMajorTab(MajorTab* majorTab)
	{
		m_MajorTab = majorTab;
	}

	bool MinorTab::IsInputValid()
	{
		const Vec2& pos = EditorUI::GetInput();
		return pos.x >= 0 && pos.x <= GetWidth() && pos.y >= 0 && pos.y <= GetHeight();
	}

	bool MinorTab::IsInputMode(EditorInputEvent event)
	{
		return GetMajorTab()->GetEditorWindow()->m_InputEvent == event;
	}
	void MinorTab::SetInputMode(EditorInputEvent event)
	{
		GetMajorTab()->GetEditorWindow()->m_InputEvent = event;
	}

	void MinorTab::Subframe(MinorTab& subframe, float x, float y, float width, float height)
	{
		Vec2 input = EditorUI::GetInput();
		Vec2 offset = EditorUI::GetInputOffset();

		subframe.Render(0.0f);
		RenderPipeline::RenderFramebufferIntoFramebuffer(*subframe.m_Framebuffer, *m_Framebuffer, *RenderPipeline::GetFullscreenPassShaderStatic(), glm::ivec4(x, y, width, height), "u_Texture", 0, false);

	}
}