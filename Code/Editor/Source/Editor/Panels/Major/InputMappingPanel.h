#pragma once
#include "../MajorTab.h"
#include "../Minor/DetailsPanel.h"
#include "Suora/GameFramework/InputModule.h"
#include "InputMappingPanel.generated.h"

namespace Suora
{
	class InputMappingPanel : public MajorTab
	{
		SUORA_CLASS(453723244);
	public:
		InputMappingPanel();
		InputMappingPanel(InputMapping* input);
		void Init() override;
		void Update(float deltaTime) override
		{
			Super::Update(deltaTime);
		}
		virtual Texture* GetIconTexture() override;
		void DrawToolbar(float& x, float y, float height) override;

		void SaveAsset() override;

	};

}