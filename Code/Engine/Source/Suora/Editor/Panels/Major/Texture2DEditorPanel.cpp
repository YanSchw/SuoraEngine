#include "Precompiled.h"
#include "Texture2DEditorPanel.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/Assets/Texture2D.h"
#include "Suora/Editor/Panels/MinorTab.h"

namespace Suora
{

	class TextureViewer : public MinorTab
	{
	public:
		Texture2D* m_Texture2D;
		float m_Zoom = 1.0f;
		Vec2 m_CameraPos = Vec2(0.0f);
	public:
		TextureViewer(MajorTab* majorTab, Texture2D* texture)
			: MinorTab(majorTab), m_Texture2D(texture)
		{
			Name = "Texture Viewer";
		}
		void Render(float deltaTime) override
		{
			const Vec2 center = Vec2(GetWidth() / 2.0f, GetHeight() / 2.0f);
			const Vec2 size = Vec2(m_Texture2D->GetTexture()->GetWidth(), m_Texture2D->GetTexture()->GetHeight());
			if (IsInputValid() && IsInputMode(EditorInputEvent::None))
			{
				m_Zoom += 0.08f * NativeInput::GetMouseScrollDelta();
				m_Zoom = Math::Clamp(m_Zoom, 0.1f, 5.85f);
				if (NativeInput::IsMouseButtonPressed(Mouse::ButtonRight))
				{
					Vec2 delta = NativeInput::GetMouseDelta() * (1.0f / m_Zoom);
					delta.x *= -1;
					m_CameraPos += Vec2(delta.x / size.x, delta.y / size.y);
				}
			}
			m_CameraPos.x = Math::Clamp(m_CameraPos.x, -1.0f, 1.0f);
			m_CameraPos.y = Math::Clamp(m_CameraPos.y, -1.0f, 1.0f);

			EditorUI::DrawRect(0, 0, GetWidth(), GetHeight(), 0.0f, Color(0.25f, 0.25f, 0.25f, 1.0f));
			EditorUI::DrawTexturedRect(m_Texture2D->GetTexture(), center.x - size.x / 2.0f * m_Zoom - m_CameraPos.x * size.x / 2.0f * m_Zoom,
				                                                    center.y - size.y / 2.0f * m_Zoom - m_CameraPos.y * size.y / 2.0f * m_Zoom,
				                                                    size.x * m_Zoom, size.y * m_Zoom, 0.0f, Color(1.0f));


			EditorUI::DrawRect(center.x, center.y, 1, 1, 0.0f, Color(0.85f, 0.25f, 0.25f, 1.0f));
		}
	};

	Texture2DEditorPanel::Texture2DEditorPanel()
	{
		m_AssetClass = Texture2D::StaticClass();
	}
	Texture2DEditorPanel::Texture2DEditorPanel(Texture2D* texture)
	{
		m_AssetClass = Texture2D::StaticClass();
		Init();
	}
	void Texture2DEditorPanel::Init()
	{
		Super::Init();

		m_Name = m_Asset->As<Texture2D>()->GetAssetName();
		m_Texture2D = m_Asset->As<Texture2D>();

		Ref<TextureViewer> t1 = CreateRef<TextureViewer>(this, m_Texture2D);
		Ref<DetailsPanel> t2 = CreateRef<DetailsPanel>(this);
		t2->m_Data = m_Texture2D;

		Ref<DockingSpace> ds1 = CreateRef<DockingSpace>(0, 0, 0.65f, 1, this);					    m_DockspacePanel.m_DockingSpaces.Add(ds1); ds1->m_MinorTabs.Add(t1);
		Ref<DockingSpace> ds2 = CreateRef<DockingSpace>(0.65f, 0.0f, 1.0f, 1.0f, this);			    m_DockspacePanel.m_DockingSpaces.Add(ds2); ds2->m_MinorTabs.Add(t2);

	}

	Texture2DEditorPanel::~Texture2DEditorPanel()
	{

	}

	void Texture2DEditorPanel::Update(float deltaTime)
	{
		Super::Update(deltaTime);

	}

	Texture* Texture2DEditorPanel::GetIconTexture()
	{
		return AssetManager::GetAsset<Texture2D>(SuoraID("26ed62e2-e39b-4028-aca2-7b116c74541f"))->GetTexture();
	}

	void Texture2DEditorPanel::SaveAsset()
	{
		Super::SaveAsset();

		SuoraVerify(false);
	}

}