#include "Precompiled.h"
#include "BlueprintDetails.h"

#include "Suora/Editor/Overlays/SelectionOverlay.h"
#include "Suora/Editor/Panels/Major/NodeClassEditor.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Suora/NodeScript/ScriptTypes.h"

namespace Suora
{
	
	void BlueprintDetails::ViewObject(Object* obj, float& y)
	{
		if (obj->IsA<Blueprint>())
		{
			ViewBlueprintClass(y, obj->As<Blueprint>());
		}
	}

	void BlueprintDetails::ViewBlueprintClass(float& y, Blueprint* blueprint)
	{
		if (!blueprint) return;

		y -= 36.0f;
		if (EditorUI::Button("Add", GetDetailWidth() - 101.0f, y + 1.0f, 101.0f, 35.0f, ShutterPanelParams()))
		{
			Yaml::Node& graphs = blueprint->m_YamlNode_EditorOnly["Node"]["Graphs"];
			int i = 0;
			while (true)
			{
				Yaml::Node& graph = graphs[std::to_string(i)];
				if (graph.IsNone()) break;
				i++;
			}
			SuoraVerify(GetMajorTab()->IsA<NodeClassEditor>());
			Ref<NodeGraphEditor> NodeGraph = GetMajorTab()->As<NodeClassEditor>()->CreateNodeClassGraphEditorInstance(blueprint, i);
			NodeGraph->m_Graph->SerializeNodeGraph(graphs[std::to_string(i)]);
			graphs[std::to_string(i)]["Label"] = "New NodeGraph";
		}
		if (EditorUI::CategoryShutter(100, "Graphs", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			Yaml::Node& graphs = blueprint->m_YamlNode_EditorOnly["Node"]["Graphs"];
			int i = 0;
			while (true)
			{
				Yaml::Node& graph = graphs[std::to_string(i)];
				if (graph.IsNone()) break;

				y -= 35.0f;
				DrawLabel(graphs[std::to_string(i)]["Label"].As<String>(), y, 35.0f);
				if (EditorUI::Button("Edit Graph", GetDetailWidth() * GetSeperator() + 50.0f, y + 5.0f, 120.0f, 25.0f, EditorUI::ButtonParams::Highlight()))
				{
					NodeClassEditor* editor = GetMajorTab()->As<NodeClassEditor>();
					editor->OpenNodeGraph(blueprint, i);
				}
				i++;
			}
			y -= 20;
		}

		y -= 36.0f;
		if (EditorUI::Button("Add", GetDetailWidth() - 101.0f, y + 1.0f, 101.0f, 35.0f, ShutterPanelParams()))
		{
			blueprint->m_ScriptClass->m_ScriptVars.push_back(ScriptVar());
		}
		if (EditorUI::CategoryShutter(101, "Variables", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			for (ScriptVar& var : blueprint->m_ScriptClass->m_ScriptVars)
			{
				y -= 35.0f;
				DrawLabel(var.m_VarName, y, 35.0f);
				EditorUI::ButtonParams params = EditorUI::ButtonParams::Invisible();
				params.TextColor = Color(0.0f);
				EditorUI::TextField(&var.m_VarName, 1.0f, y, GetDetailWidth() * GetSeperator() - 2.0f, 35.0f, params);

				EditorUI::ButtonParams typeParams;
				typeParams.ButtonColor = GetScriptDataTypeColor(var.m_Type) * 0.5f;
				typeParams.ButtonColorHover = GetScriptDataTypeColor(var.m_Type) * 0.6f;
				typeParams.ButtonOutlineColor = GetScriptDataTypeColor(var.m_Type) * 0.25f;
				typeParams.ButtonRoundness = 22.5f / 2.0f;
				typeParams.TextDropShadow = true;

				String Label = ScriptDataTypeToLabel(var.m_Type);
				if (var.m_Type == ScriptDataType::ObjectPtr || var.m_Type == ScriptDataType::Class)
					Label = Class::FromString(var.m_VarParams).GetClassName();
				if (EditorUI::Button(Label, GetDetailWidth() * GetSeperator() + 50.0f, y + 5.0f, 120.0f, 25.0f, typeParams))
				{
					SelectionOverlay* overlay = EditorUI::CreateOverlay<SelectionOverlay>(NativeInput::GetMousePosition().x, GetMajorTab()->GetEditorWindow()->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y - 400.0f, 400.0f, 400.0f);
					for (int64_t type = (int64_t)ScriptDataType::None + 1; type < (int64_t)ScriptDataType::COUNT; type++)
					{
						if (type == (int64_t)ScriptDataType::ObjectPtr) continue;
						ScriptVar* VAR = &var;
						overlay->m_Entries.Add(SelectionOverlay::SelectionOverlayEntry(ScriptDataTypeToLabel((ScriptDataType)type), {}, [VAR, type]()
							{
								VAR->m_Type = (ScriptDataType)type;
							}));
					}
					Array<Class> classes = Class::GetAllClasses();
					for (Class cls : classes)
					{
						ScriptVar* VAR = &var;
						overlay->m_Entries.Add(SelectionOverlay::SelectionOverlayEntry(cls.GetClassName() + " Pointer", {}, [VAR, cls]()
							{
								VAR->m_Type = ScriptDataType::ObjectPtr;
								VAR->m_VarParams = cls.ToString();
							}));
						overlay->m_Entries.Add(SelectionOverlay::SelectionOverlayEntry(cls.GetClassName() + " Class", {}, [VAR, cls]()
							{
								VAR->m_Type = ScriptDataType::Class;
								VAR->m_VarParams = cls.ToString();
							}));
					}
				}

			}

			y -= 20;
		}
	}

}