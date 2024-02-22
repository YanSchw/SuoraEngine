#include "Precompiled.h"
#include "MaterialDetails.h"

#include "Suora/Assets/Material.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/Texture2D.h"

namespace Suora
{
	
	void MaterialDetails::ViewObject(Object* obj, float& y)
	{
		if (obj->GetNativeClass().Inherits(Material::StaticClass()))
		{
			ViewMaterial(y, obj->As<Material>(), obj->GetNativeClass().Inherits(ShaderGraph::StaticClass()));
		}
	}

	void MaterialDetails::ViewMaterial(float& y, Material* material, bool isShaderGraph)
	{
		if (!isShaderGraph)
		{
			y -= 36.0f;
			if (EditorUI::CategoryShutter(0, "ShaderGraph", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
			{
				const ShaderGraph* temp = material->m_ShaderGraph;
				DrawAsset((Asset**)&material->m_ShaderGraph, ShaderGraph::StaticClass(), "ShaderGraph", y, false);

				if (temp != material->m_ShaderGraph && material->m_ShaderGraph)
				{
					material->m_UniformSlots = material->m_ShaderGraph->m_UniformSlots;
				}
				y -= 20;
			}
		}
		else
		{
			y -= 36.0f;
			if (EditorUI::CategoryShutter(0, "Shader", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
			{
				int i = 0;
				std::vector<std::pair<String, std::function<void(void)>>> options;
				std::filesystem::path directory = AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/ShadergraphBase/";
				std::vector<DirectoryEntry> entries = File::GetAllAbsoluteEntriesOfPath(directory);
				for (int j = 0; j < entries.size(); j++)
				{
					const String name = entries[j].path().filename().string();
					if (name == material->As<ShaderGraph>()->m_BaseShader) i = j;
					options.push_back({ name, [name, material]() { material->As<ShaderGraph>()->m_BaseShader = name; } });
				}

				DrawDropDown("Shader", options, i, y);
			}
		}

		y -= 35.0f;
		if (EditorUI::CategoryShutter(1, "Uniforms", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			for (UniformSlot& slot : material->m_UniformSlots)
			{
				if (slot.m_Type == ShaderGraphDataType::Float)
				{
					DrawFloat(&slot.m_Float, slot.m_Label, y, false);
				}
				else if (slot.m_Type == ShaderGraphDataType::Vec3)
				{
					DrawVec3(&slot.m_Vec3, slot.m_Label, y, false);
				}
				else if (slot.m_Type == ShaderGraphDataType::Texture2D)
				{
					DrawAsset((Asset**)&slot.m_Texture2D, Texture2D::StaticClass(), slot.m_Label, y, false);
				}
				else
				{
					SuoraError("Unkown Type!");
				}
			}
			y -= 20;
		}

		y -= 35.0f;
		if (EditorUI::CategoryShutter(2, "Material", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			DrawBool(&material->m_BackfaceCulling, "BackfaceCulling", y, false);
			y -= 20;
		}
	}

}