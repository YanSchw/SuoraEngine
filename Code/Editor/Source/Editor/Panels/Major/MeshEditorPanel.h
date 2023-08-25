#pragma once
#include "../MajorTab.h"
#include "MeshEditorPanel.generated.h"

namespace Suora
{

	class Mesh;

	class MeshEditorPanel : public MajorTab
	{
		SUORA_CLASS(5487392221);
	public:
		Ptr<Mesh> m_Mesh = nullptr;

		MeshEditorPanel();
		MeshEditorPanel(Mesh* mesh);
		~MeshEditorPanel();

		void Init() override;
		virtual void Update(float deltaTime) override;
		virtual Texture* GetIconTexture() override;
		void DrawToolbar(float& x, float y, float height) override;

		void SaveAsset() override;

	private:
		Ref<ViewportPanel> m_ViewportPanel = nullptr;
		bool m_InitResetCamera = false;
		World m_World;
	};

}