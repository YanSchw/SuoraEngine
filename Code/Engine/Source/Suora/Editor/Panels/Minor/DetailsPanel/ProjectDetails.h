#pragma once
#include "DetailsPanel.h"
#include "ProjectDetails.generated.h"

namespace Suora
{
	class Mesh;

	class ProjectDetails : public DetailsPanelImplementation
	{
		SUORA_CLASS(437843883);
	public:

		virtual void ViewObject(Object* obj, float& y) override;
		void ViewProjectSettings(float& y, ProjectSettings* settings);
	};

}