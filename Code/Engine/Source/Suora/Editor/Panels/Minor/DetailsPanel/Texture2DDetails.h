#pragma once
#include "DetailsPanel.h"
#include "Texture2DDetails.generated.h"

namespace Suora
{
	class Texture2D;

	class Texture2DDetails : public DetailsPanelImplementation
	{
		SUORA_CLASS(78645789);
	public:

		virtual void ViewObject(Object* obj, float& y) override;
		void ViewTexture2D(float& y, Texture2D* texture);
	};

}