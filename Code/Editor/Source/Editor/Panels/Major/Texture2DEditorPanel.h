#pragma once
#include "Editor/Panels/MajorTab.h"
#include "Texture2DEditorPanel.generated.h"

namespace Suora
{

	class Texture;
	class Texture2D;

	class Texture2DEditorPanel : public MajorTab
	{
		SUORA_CLASS(9174847113);
	public:
		Ptr<Texture2D> m_Texture2D = nullptr;

		Texture2DEditorPanel();
		Texture2DEditorPanel(Texture2D* texture);
		~Texture2DEditorPanel();

		void Init() override;
		virtual void Update(float deltaTime) override;
		virtual Texture* GetIconTexture() override;
		void SaveAsset() override;
	
	};

}