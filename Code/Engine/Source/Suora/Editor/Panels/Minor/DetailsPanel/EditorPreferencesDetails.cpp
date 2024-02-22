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
			DrawVec4(&settings->UiColor, "Main Color", y, false);
			DrawVec4(&settings->UiBackgroundColor, "Background Color", y, false);
			DrawVec4(&settings->UiForgroundColor, "Foreground Color", y, false);
		}
		DrawFloat(&settings->UiScale, "UiScale", y, false);
	}

}