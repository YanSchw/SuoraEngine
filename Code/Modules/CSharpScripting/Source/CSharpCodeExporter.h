#pragma once
#include "Suora/Editor/Panels/Major/ExportProjectPanel.h"
#include "CSharpCodeExporter.generated.h"

namespace Suora
{

	class CSharpCodeExporter : public AdditionalProjectExportTask
	{
		SUORA_CLASS(589743893245);
	public:
		virtual void Execute(ExportSettings* settings) override;
	};
	
}