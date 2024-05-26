#pragma once
#include "DetailsPanel.h"
#include "ExportProjectDetails.generated.h"

namespace Suora
{
	class ExportSettings;

	class ExportProjectDetails : public DetailsPanelImplementation
	{
		SUORA_CLASS(56487893331);
	public:

		virtual void ViewObject(Object* obj, float& y) override;
		void ViewExportSettings(float& y, ExportSettings* settings);
	};

}