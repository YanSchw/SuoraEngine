#include "Precompiled.h"
#include "ExportProjectDetails.h"

#include "Suora/Editor/Panels/Major/ExportProjectPanel.h"

namespace Suora
{
	
	void ExportProjectDetails::ViewObject(Object* obj, float& y)
	{
		if (obj->IsA<ExportSettings>())
		{
			ViewExportSettings(y, obj->As<ExportSettings>());
		}
	}

	void ExportProjectDetails::ViewExportSettings(float& y, ExportSettings* settings)
	{
		y -= 35.0f;
		if (EditorUI::CategoryShutter(0, "General", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			y -= 34.0f;
			DrawLabel("Output Path", y, 35.0f);
			EditorUI::ButtonParams PathButtonParams;
			PathButtonParams.TextOffsetLeft = 5.0f;
			PathButtonParams.TextOrientation = Vec2(-1.0f, 0.0f);
			if (EditorUI::Button(settings->m_OutputPath.string(), GetSeperator() * GetWidth() + 5.0f, y + 5.0f, (GetWidth() * (1.0f - GetSeperator())) - 45.0f, 25.0f, PathButtonParams))
			{
				std::optional<String> path = Platform::ChoosePathDialog();
				if (path.has_value()) settings->m_OutputPath = path.value();
			}
		}
		y -= 35.0f;
		if (EditorUI::CategoryShutter(1, "Platform Windows", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			y -= 34.0f;
			DrawLabel("MSBuild Path", y, 35.0f);
			EditorUI::ButtonParams PathButtonParams;
			PathButtonParams.TextOffsetLeft = 5.0f;
			PathButtonParams.TextOrientation = Vec2(-1.0f, 0.0f);
			if (EditorUI::Button(settings->m_MSBuildPath.string(), GetSeperator() * GetWidth() + 5.0f, y + 5.0f, (GetWidth() * (1.0f - GetSeperator())) - 45.0f, 25.0f, PathButtonParams))
			{
				std::optional<String> path = Platform::OpenFileDialog({".exe"});
				if (path.has_value()) settings->m_MSBuildPath = path.value();
			}
		}
	}

}