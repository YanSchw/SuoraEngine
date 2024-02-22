#pragma once
#include "DetailsPanel.h"
#include "MeshDetails.generated.h"

namespace Suora
{
	class Mesh;

	class MeshDetails : public DetailsPanelImplementation
	{
		SUORA_CLASS(57468894);
	public:

		virtual void ViewObject(Object* obj, float& y) override;
		void ViewMesh(float& y, Mesh* mesh);
	};

}