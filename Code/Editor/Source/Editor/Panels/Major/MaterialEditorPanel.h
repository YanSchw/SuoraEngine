#pragma once
#include "../MajorTab.h"
#include "MaterialEditorPanel.generated.h"

namespace Suora
{

	class MaterialEditorPanel : public MajorTab
	{
		SUORA_CLASS(586794347);
	public:
		Ptr<Material> m_Material = nullptr;
		World m_World;


		MaterialEditorPanel();
		MaterialEditorPanel(Material* material);
		~MaterialEditorPanel();

		void Init() override;
		virtual void Update(float deltaTime) override;
		virtual Texture* GetIconTexture() override;
		void SaveAsset() override;
	private:
		Ref<ViewportPanel> m_ViewportPanel = nullptr;
		bool m_InitResetCamera = false;
	};

}