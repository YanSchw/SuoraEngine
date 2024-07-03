#include "Precompiled.h"
#include "ProjectDetails.h"

#include "Suora/Assets/SuoraProject.h"
#include "Suora/Assets/Level.h"
#include "Suora/Assets/Texture2D.h"

namespace Suora
{
	
	void ProjectDetails::ViewObject(Object* obj, float& y)
	{
		if (obj->IsA<ProjectSettings>())
		{
			ViewProjectSettings(y, obj->As<ProjectSettings>());
		}
	}

	void ProjectDetails::ViewProjectSettings(float& y, ProjectSettings* settings)
	{
		y -= 35.0f;
		if (EditorUI::CategoryShutter(0, "Rendering", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			DrawFloat(&settings->m_TargetFramerate, "Target Framerate", y, false);
			DrawBool(&settings->m_EnableDeferredRendering, "Enable Deferred Rendering", y, false);
		}
		y -= 35.0f;
		if (EditorUI::CategoryShutter(1, "Game", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			DrawAsset((Asset**)(&settings->m_DefaultLevel), Level::StaticClass(), "DefaultLevel", y, false);
			DrawAsset((Asset**)(&settings->m_ProjectIconTexture), Texture2D::StaticClass(), "ProjectIconTexture", y, false);
		}
		y -= 35.0f;
		if (EditorUI::CategoryShutter(2, "Editor", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			DrawAsset((Asset**)&(settings->m_EditorStartupAsset), Asset::StaticClass(), "Editor Startup Asset", y, false);
		}

	}

}