#include "Precompiled.h"
#include "MeshDetails.h"

#include "Suora/Assets/Mesh.h"
#include "Suora/Assets/Material.h"

namespace Suora
{
	
	void MeshDetails::ViewObject(Object* obj, float& y)
	{
		if (obj->IsA<Mesh>())
		{
			ViewMesh(y, obj->As<Mesh>());
		}
	}

	void MeshDetails::ViewMesh(float& y, Mesh* mesh)
	{
		y -= 35.0f;
		if (EditorUI::CategoryShutter(0, "Mesh", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			DrawVec3(&mesh->m_ImportScale, "Import Scale", y, false);
		}
		y -= 35.0f;
		if (EditorUI::CategoryShutter(2, "Materials", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			DrawMaterialSlots(&mesh->m_Materials, y, false);
		}
		y -= 35.0f;
		if (EditorUI::CategoryShutter(3, "Decima", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			DrawBool(&mesh->m_IsDecimaMesh, "IsDecimaMesh", y, false);
			y -= 20;
		}
	}

}