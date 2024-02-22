#pragma once
#include "DetailsPanel.h"
#include "BlueprintDetails.generated.h"

namespace Suora
{
	class Blueprint;

	class BlueprintDetails : public DetailsPanelImplementation
	{
		SUORA_CLASS(4578678647);
	public:

		virtual void ViewObject(Object* obj, float& y) override;
		void ViewBlueprintClass(float& y, Blueprint* blueprint);
	};

}