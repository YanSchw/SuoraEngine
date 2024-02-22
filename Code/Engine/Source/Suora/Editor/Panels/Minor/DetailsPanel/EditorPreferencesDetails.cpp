#include "Precompiled.h"
#include "EditorPreferencesDetails.h"

#include "Suora/Editor/Util/EditorPreferences.h"

namespace Suora
{
	
	void EditorPreferencesDetails::ViewObject(Object* obj, float& y)
	{
		if (obj->IsA<EditorPreferences>())
		{
			ViewEditorPreferences(y, obj->As<EditorPreferences>());
		}
	}

	void EditorPreferencesDetails::ViewEditorPreferences(float& y, EditorPreferences* settings)
	{
		y -= 35.0f;
		if (EditorUI::CategoryShutter(0, "Theme", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			DrawAsset((Asset**)&Font::Instance, Font::StaticClass(), "Editor Font", y, false);

			DrawVec4(&settings->UiHighlightColor, "HighlightColor", y, false);

			y -= 34.0f;
			DrawLabel("Theme Preset", y, 35.0f);
			if (EditorUI::Button("Default", GetWidth() * GetSeperator() + 5.0f, y + 3.0f, 150.0f, 29.0f))
			{
				settings->UiColor = Color(0.14717647f, 0.14717647f, 0.15217647f, 1.0f);
				settings->UiBackgroundColor = Color(0.101961f, 0.105882f, 0.1198039f, 1.0f);
				settings->UiForgroundColor = Color(0.2f, 0.20392157f, 0.211764706f, 1.0f);
			}
			if (EditorUI::Button("Dark", GetWidth() * GetSeperator() + 5.0f + 160.0f, y + 3.0f, 150.0f, 29.0f))
			{
				settings->UiColor = Color(0.14f, 0.14f, 0.14f, 1.0f);
				settings->UiBackgroundColor = Color(0.1f, 0.1f, 0.1f, 1.0f);
				settings->UiForgroundColor = Color(0.2f, 0.2f, 0.2f, 1.0f);
			}
			if (EditorUI::Button("Light", GetWidth() * GetSeperator() + 5.0f + 320.0f, y + 3.0f, 150.0f, 29.0f))
			{
				settings->UiColor = Color(0.67f, 0.67f, 0.70f, 1.0f);
				settings->UiBackgroundColor = Color(0.62f, 0.62f, 0.66f, 1.0f);
				settings->UiForgroundColor = Color(0.73f, 0.74f, 0.77f, 1.0f);
			}

			DrawVec4(&settings->UiColor, "Main Color", y, false);
			DrawVec4(&settings->UiBackgroundColor, "Background Color", y, false);
			DrawVec4(&settings->UiForgroundColor, "Foreground Color", y, false);
		}
		DrawFloat(&settings->UiScale, "UiScale", y, false);
	}

}