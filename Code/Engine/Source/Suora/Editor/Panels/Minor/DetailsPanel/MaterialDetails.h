#pragma once
#include "DetailsPanel.h"
#include "MaterialDetails.generated.h"

namespace Suora
{
	class Material;

	class MaterialDetails : public DetailsPanelImplementation
	{
		SUORA_CLASS(5483798946);
	public:

		virtual void ViewObject(Object* obj, float& y) override;
		void ViewMaterial(float& y, Material* material, bool isShaderGraph);
	};

}