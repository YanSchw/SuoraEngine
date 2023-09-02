#include "Precompiled.h"
#include "BlueprintNodeGraph.h"

#include "Suora/Serialization/Yaml.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"

#include "Suora/GameFramework/InputModule.h"

#define NODE_ID_EVENT 14
#define NODE_ID_NATIVE_FUNC 15
#define NODE_ID_NATIVE_PURE 16
#define NODE_ID_DELEGATE 17
#define NODE_ID_INPUT_EVENT 18

static std::string ExtractFunctionName(std::string name)
{
	int i = 2;
	while (i > 0)
	{
		if (name[0] == ':') i--;
		name.erase(0, 1);
	}
	while (name[i++] != '(');
	while (i > name.size() - 1) name.erase(i, 1);

	return name;
}

namespace Suora
{
	bool BlueprintCompiler::IsVisualNodePinUsed(VisualNodePin& pin, Ref<VisualNodeGraph> graph)
	{
		for (Ref<VisualNode> node : graph->m_Nodes)
		{
			for (VisualNodePin& it : node->m_InputPins)
			{
				if (it.IsReceivingPin && it.Target == &pin) return true;
			}
			for (VisualNodePin& it : node->m_OutputPins)
			{
				if (it.IsReceivingPin && it.Target == &pin) return true;
			}
		}
		return false;
	}
	bool BlueprintCompiler::IsVisualNodePinLocalVar(VisualNodePin& pin, CompilerCache& cache, int& ID)
	{
		for (int i = 0; i < cache.LocalVars.Size(); i++)
		{
			if (cache.LocalVars[i].pin == &pin && cache.LocalVars[i].node == pin.GetNode())
			{
				ID = i;
				return true;
			}
		}
		return false;
	}
	void BlueprintCompiler::Compile(Blueprint& blueprint)
	{
		blueprint.m_DelegateEventsToBindDuringGameplay.Clear();
		blueprint.m_InputEventsToBeBound.Clear();

		Array<Ref<VisualNodeGraph>> eventGraphs;
		Yaml::Node& graphs = blueprint.m_YamlNode_EditorOnly["Node"]["Graphs"];
		int i = 0;
		while (true)
		{
			Yaml::Node& graph = graphs[std::to_string(i)];
			if (graph.IsNone()) break;
			Ref<BlueprintNodeGraph> bng = CreateRef<BlueprintNodeGraph>(&blueprint);
			bng->DeserializeNodeGraph(blueprint.m_YamlNode_EditorOnly["Node"]["Graphs"][std::to_string(i)]);
			eventGraphs.Add(bng);
			i++;
		}
		Compile(blueprint, eventGraphs);
	}

	void BlueprintCompiler::Compile(Blueprint& blueprint, const Array<Ref<VisualNodeGraph>>& eventGraphs)
	{
		blueprint.m_ScriptClass->m_Functions.clear();
		uint64_t hashCounter = 1;

		for (Ref<VisualNodeGraph> graph : eventGraphs)
		{
			graph->FixNodePins();
			graph->TickAllVisualNodes();
			for (Ref<VisualNode> event : graph->m_Nodes)
			{
				if (event->m_NodeID == NODE_ID_EVENT)
				{
					VisualNodePin* exec = nullptr;

					for (VisualNodePin& pin : event->m_OutputPins)
					{
						if (pin.PinID == 1 && pin.Target) exec = &pin;
					}
					if (exec)
						CompileEvent(blueprint, *event, *exec, graph, std::stoull(event->m_InputPins[0].m_AdditionalData));
				}
				else if (event->m_NodeID == NODE_ID_DELEGATE)
				{
					VisualNodePin* exec = nullptr;

					for (VisualNodePin& pin : event->m_OutputPins)
					{
						if (pin.PinID == 1 && pin.Target) exec = &pin;
					}
					if (exec)
					{
						size_t hash = hashCounter++;
						CompileEvent(blueprint, *event, *exec, graph, hash);
						blueprint.m_DelegateEventsToBindDuringGameplay.Add(Blueprint::DelegateEventBind(event->m_InputPins[0].m_AdditionalData, event->m_InputPins[1].m_AdditionalData, hash));
					}
				}
				else if (event->m_NodeID == NODE_ID_INPUT_EVENT)
				{
					if (event->m_InputPins[2].m_AdditionalData == "Action")
					{
						VisualNodePin* execPressed = nullptr;	if (event->m_OutputPins[0].Target) execPressed = &event->m_OutputPins[0];
						VisualNodePin* execReleased = nullptr;	if (event->m_OutputPins[1].Target) execPressed = &event->m_OutputPins[1];
						VisualNodePin* execHelt = nullptr;		if (event->m_OutputPins[2].Target) execPressed = &event->m_OutputPins[2];

						if (execPressed)
						{
							size_t hash = hashCounter++;
							CompileEvent(blueprint, *event, *execPressed, graph, hash);
							blueprint.m_InputEventsToBeBound.Add(Blueprint::InputEventBind(event->m_InputPins[0].m_AdditionalData + "/" + event->m_InputPins[1].m_AdditionalData, hash, InputScriptEventFlags::ButtonPressed));
						}
						if (execReleased)
						{
							size_t hash = hashCounter++;
							CompileEvent(blueprint, *event, *execReleased, graph, hash);
							blueprint.m_InputEventsToBeBound.Add(Blueprint::InputEventBind(event->m_InputPins[0].m_AdditionalData + "/" + event->m_InputPins[1].m_AdditionalData, hash, InputScriptEventFlags::ButtonReleased));
						}
						if (execHelt)
						{
							size_t hash = hashCounter++;
							CompileEvent(blueprint, *event, *execHelt, graph, hash);
							blueprint.m_InputEventsToBeBound.Add(Blueprint::InputEventBind(event->m_InputPins[0].m_AdditionalData + "/" + event->m_InputPins[1].m_AdditionalData, hash, InputScriptEventFlags::ButtonHelt));
						}
					}
					else
					{
						SuoraVerify(false, "Missing Implementation!");
					}
				}
			}
		}
	}
	void BlueprintCompiler::CompileEvent(Blueprint& blueprint, VisualNode& event, VisualNodePin& exec, Ref<VisualNodeGraph> graph, size_t hash)
	{
		blueprint.m_ScriptClass->m_Functions.push_back(ScriptFunction());
		ScriptFunction& func = blueprint.m_ScriptClass->m_Functions[blueprint.m_ScriptClass->m_Functions.size() - 1];
		func.m_IsEvent = true;
		func.m_Hash = hash;

		CompilerCache cache;
		for (int i = 0; i < event.m_OutputPins.Size(); i++)
		{
			if (event.m_OutputPins[i].PinID == 1) continue;
			if (((ScriptDataType)event.m_OutputPins[i].PinID) == ScriptDataType::None) continue;

			if (IsVisualNodePinUsed(event.m_OutputPins[i], graph))
			{
				cache.LocalVars.Add(LocalVar{ &event, &event.m_OutputPins[i] });
				func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::PushToLocalVar, { cache.LocalVars.Last() }));
			}
			else
			{
				func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::Pop));
			}
		}
		CompileVisualNode(func, blueprint, *exec.Target->GetNode(), exec.Target, cache, graph);
		func.m_LocalVarCount = cache.LocalVars.Size();
	}
	bool BlueprintCompiler::CompileVisualNode(ScriptFunction& func, const Class& cls, VisualNode& node, VisualNodePin* exec, CompilerCache& cache, Ref<VisualNodeGraph> graph)
	{
		/// EXEC might be the InExec Pin, but it could also be a OutputPin of a requested Pure Node
		VisualNodePin* HashPin = (node.m_NodeID == NODE_ID_NATIVE_FUNC || node.m_NodeID == NODE_ID_NATIVE_PURE) ? &node.m_InputPins[0] : nullptr;
		VisualNodePin* FlagPin = (node.m_NodeID == NODE_ID_NATIVE_FUNC || node.m_NodeID == NODE_ID_NATIVE_PURE) ? &node.m_InputPins[1] : nullptr;
		FunctionFlags flags = FlagPin ? (FunctionFlags)std::stoi(FlagPin->m_AdditionalData) : FunctionFlags::None;
		VisualNodePin* InstancePin = (0 == ((int32_t)flags & (int32_t)FunctionFlags::Static)) ? ((node.m_NodeID == NODE_ID_NATIVE_FUNC) ? &node.m_InputPins[3] : ((node.m_NodeID == NODE_ID_NATIVE_PURE) ? &node.m_InputPins[2] : nullptr)) : nullptr;
		int64_t NativeHash = HashPin ? (int64_t)std::stoull(HashPin->m_AdditionalData) : 0;

		for (VisualNodePin& pin : node.m_InputPins)
		{
			if (&pin == HashPin || &pin == FlagPin || &pin == InstancePin) continue;
			if (pin.PinID != 0 && pin.PinID != 1)
			{
				CompileVisualNodePin(func, cls, pin, cache, graph);
			}
		}

		/// Compile actual VisualNode
		if (node.m_NodeID == NODE_ID_NATIVE_FUNC)
		{
			if (InstancePin) CompileVisualNodePin(func, cls, *InstancePin, cache, graph);
			func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::CallNativeFunction, { NativeHash }));

			// Return
			if (node.m_OutputPins.Size() >= 2)
			{
				if (IsVisualNodePinUsed(node.m_OutputPins[1], graph))
				{
					cache.LocalVars.Add(LocalVar{ &node, &node.m_OutputPins[1] });
					func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::PushToLocalVar, { cache.LocalVars.Last() }));
				}
				else
				{
					func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::Pop));
				}
			}

			// Exec
			if (node.m_OutputPins.Size() >= 1 && node.m_OutputPins[0].Target)
			{
				CompileVisualNode(func, cls, *node.m_OutputPins[0].Target->GetNode(), node.m_OutputPins[0].Target, cache, graph);
			}

			return false;
		}
		if (node.m_NodeID == NODE_ID_NATIVE_PURE)
		{
			if (InstancePin) CompileVisualNodePin(func, cls, *InstancePin, cache, graph);
			func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::CallNativeFunction, { NativeHash }));

			// Native Pure Nodes only return one Var, so we don't worry about Stackpolution;
			// Also, Pure Nodes never Push their return values onto the Stack or Cache them as LocalVars.

			return false;
		}

		if (node.m_NodeID == 200)
		{
			func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::Multiply_Vec3_Float));
			return false;
		}

		return true;
	}

	bool BlueprintCompiler::CompileVisualNodePin(ScriptFunction& func, const Class& cls, VisualNodePin& pin, CompilerCache& cache, Ref<VisualNodeGraph> graph)
	{
		if (!pin.Target)
		{
			if (pin.PinID == (int64_t)ScriptDataType::ObjectPtr)
			{
				if (cls.Inherits(Class::FromString(pin.m_AdditionalData)))
				{
					func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::PushSelf));
				}
				else return true;
			}
			if (pin.PinID == (int64_t)ScriptDataType::Float)
			{
				const int64_t f = ScriptStack::ConvertToStack<float>(Util::StringToFloat(pin.m_AdditionalData));
				func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::PushConstant, { f }));
			}
			if (pin.PinID == (int64_t)ScriptDataType::Bool)
			{
				const int64_t b = ScriptStack::ConvertToStack<bool>(pin.m_AdditionalData == "true");
				func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::PushConstant, { b }));
			}
			if (pin.PinID == (int64_t)ScriptDataType::Vec3)
			{
				Vec3 vec = Vec::FromString<Vec3>(pin.m_AdditionalData);
				const int64_t x = ScriptStack::ConvertToStack<float>(vec.x);
				const int64_t y = ScriptStack::ConvertToStack<float>(vec.y);
				const int64_t z = ScriptStack::ConvertToStack<float>(vec.z);
				func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::PushConstant, { x }));
				func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::PushConstant, { y }));
				func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::PushConstant, { z }));
				func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::CallNativeFunction, { (int64_t)std::hash<std::string>{}("NodeScriptLibrary::MakeVector3(float, float, float)") }));
			}
			return true;
		}

		int local = 0;
		if (IsVisualNodePinLocalVar(*pin.Target, cache, local))
		{
			func.m_Instructions.push_back(ScriptInstruction(EScriptInstruction::PushLocalVar, { local }));
			return false;
		}

		// Compile Target Node, must be Pure or Custom
		return CompileVisualNode(func, cls, *pin.Target->GetNode(), pin.Target, cache, graph);
	}



	/**********************************************************************************************/

	BlueprintNodeGraph::BlueprintNodeGraph(Blueprint* blueprint)
	{
		m_BlueprintClass = blueprint;
	}

	void BlueprintNodeGraph::UpdateSupportedNodes()
	{
		VisualNodeGraph::UpdateSupportedNodes();
		{
			Ref<VisualNode> IF = CreateRef<VisualNode>();
			IF->m_Title = "If";
			IF->m_NodeID = 100;
			IF->m_Color = glm::vec4(0.8f, 0.49f, 0.38f, 1.0f);
			IF->m_Size = { 215, 105 };
			IF->AddInputPin("InExec", Color(1.0f), 1, false, 30.0f);
			IF->AddInputPin("Condition", GetScriptDataTypeColor(ScriptDataType::Bool), (int64_t)ScriptDataType::Bool, true);
			IF->AddOutputPin("Then", Color(1.0f), 1, true, 30.0f);
			IF->AddOutputPin("Else", Color(1.0f), 1, true, 30.0f);
			AddSupportedNode(IF);
		}
		{
			Ref<VisualNode> MultiplyVec3Float = CreateRef<VisualNode>();
			MultiplyVec3Float->m_Title = "Vec3 * Float";
			MultiplyVec3Float->m_NodeID = 200;
			MultiplyVec3Float->m_Color = glm::vec4(0.8f, 0.49f, 0.38f, 1.0f);
			MultiplyVec3Float->m_Size = { 215, 105 };
			MultiplyVec3Float->AddInputPin("Vec3", GetScriptDataTypeColor(ScriptDataType::Vec3), (int64_t)ScriptDataType::Vec3, true);
			MultiplyVec3Float->AddInputPin("Float", GetScriptDataTypeColor(ScriptDataType::Float), (int64_t)ScriptDataType::Float, true);
			MultiplyVec3Float->AddOutputPin("Vec3", GetScriptDataTypeColor(ScriptDataType::Vec3), (int64_t)ScriptDataType::Vec3, false);
			AddSupportedNode(MultiplyVec3Float);
		}
		for (NativeFunction* func : NativeFunction::s_NativeFunctions)
		{
			if (func->IsFlagSet(FunctionFlags::NodeEvent) && Class(m_BlueprintClass).Inherits(Class(func->m_ClassID)))
			{
				Ref<VisualNode> event = CreateRef<VisualNode>();
				event->m_Title = "Events/" + ExtractFunctionName(func->m_Label);
				event->m_NodeID = NODE_ID_EVENT;
				event->m_Color = Color(100 / 255.0f, 19 / 255.0f, 13 / 255.0f, 1.0f); // glm::vec4(0.82f, 0.25f, 0.18f, 1.0f);
				event->m_Size = { 215, 105 };
				event->AddInputPin("Hash", Color(1.0f), 0, false, 0.0f); event->m_InputPins[0].m_AdditionalData = std::to_string(func->m_Hash);
				event->AddOutputPin("Exec", Color(1.0f), 1, true, 30.0f);
				for (FunctionParam& param : func->m_Params)
				{
					event->AddOutputPin(Util::SmartToUpperCase(param.m_Name, false), GetScriptDataTypeColor(StringToScriptDataType(param.m_Type)), (int64_t)StringToScriptDataType(param.m_Type), false);
				}

				AddSupportedNode(event);
			}
			if (func->IsFlagSet(FunctionFlags::Callable))
			{
				const bool isPure = func->IsFlagSet(FunctionFlags::Pure);
				Ref<VisualNode> function = CreateRef<VisualNode>();
				function->m_Title = Class(func->m_ClassID).GetClassName() + "/" + ExtractFunctionName(func->m_Label);
				function->m_NodeID = isPure ? NODE_ID_NATIVE_PURE : NODE_ID_NATIVE_FUNC;
				function->m_Color = isPure ? /*glm::vec4(0.18f, 0.62f, 0.23f, 1.0f)*/Color(51.0f / 255.0f, 73.0f / 255.0f, 55.0f / 255.0f, 1.0f) : /*glm::vec4(0.18f, 0.22f, 0.63f, 1.0f)*/Color(38 / 255.0f, 61 / 255.0f, 76 / 255.0f, 1.0f);
				function->m_Size = { 275, 165 };
				function->AddInputPin("Hash", Color(1.0f), 0, false, 0.0f); function->m_InputPins[0].m_AdditionalData = std::to_string(func->m_Hash);
				function->AddInputPin("Flags", Color(1.0f), 0, false, 0.0f); function->m_InputPins[1].m_AdditionalData = std::to_string((int32_t)func->m_Flags);
				if (!isPure)
				{
					function->AddOutputPin("Exec", Color(1.0f), 1, true, 30.0f);
					function->AddInputPin("InExec", Color(1.0f), 1, false, 30.0f);
				}
				else
				{
					function->m_BackgroundColor = glm::vec4(0.05f, 0.055f, 0.05f, 0.9f);
				}
				if (!func->IsFlagSet(FunctionFlags::Static))
				{
					function->AddInputPin("Instance", GetScriptDataTypeColor(ScriptDataType::ObjectPtr) * 0.8f, (int64_t)ScriptDataType::ObjectPtr, true);
					function->m_InputPins[function->m_InputPins.Last()].m_AdditionalData = Class(func->m_ClassID).ToString();
				}
				for (FunctionParam& param : func->m_Params)
				{
					function->AddInputPin(Util::SmartToUpperCase(param.m_Name, false), GetScriptDataTypeColor(StringToScriptDataType(param.m_Type)), (int64_t)StringToScriptDataType(param.m_Type), true);
				}
				if (func->m_ReturnType != "void")
				{
					function->AddOutputPin("Result", GetScriptDataTypeColor(StringToScriptDataType(func->m_ReturnType)), (int64_t)StringToScriptDataType(func->m_ReturnType), false);
				}

				AddSupportedNode(function);
			}
		}

		Ref<InputSettings> inputSettings = ProjectSettings::Get()->m_InputSettings;
		if (inputSettings)
		{
			for (auto& Category : inputSettings->m_Categories)
			{
				for (auto& Action : Category->m_Actions)
				{
					Ref<VisualNode> inputEvent = CreateRef<VisualNode>();
					inputEvent->m_Title = "Input Actions/" + Category->m_CategoryName + "/" + Action->m_ActionName;
					inputEvent->m_NodeID = NODE_ID_INPUT_EVENT;
					inputEvent->m_Color = Color(100 / 255.0f, 19 / 255.0f, 13 / 255.0f, 1.0f);
					inputEvent->m_Size = { 185, 105 };
					inputEvent->AddInputPin("CategoryName", Color(1.0f), 0, false, 0.0f); inputEvent->m_InputPins[0].m_AdditionalData = Category->m_CategoryName;
					inputEvent->AddInputPin("ActionName", Color(1.0f), 0, false, 0.0f); inputEvent->m_InputPins[1].m_AdditionalData = Action->m_ActionName;
					inputEvent->AddInputPin("ActionType", Color(1.0f), 0, false, 0.0f);

					AddSupportedNode(inputEvent);
				}
			}
		}

		Node* node = m_BlueprintClass->CreateInstance(true, true, true)->As<Node>();
		RecursiveNodeDelegates(node, false);
		delete node;
	}

	void BlueprintNodeGraph::TickAllVisualNodes()
	{
		for (Ref<VisualNode> node : m_Nodes)
		{
			if (node->m_NodeID == NODE_ID_DELEGATE)
			{
				Class cls = Class::FromString(node->m_InputPins[2].m_AdditionalData);

				const ClassReflector& refl = cls.GetClassReflector();
				Array<Ref<ClassMember>> members = refl.GetAllClassMember();
				for (Ref<ClassMember> It : members)
				{
					if (It->m_Type == ClassMember::Type::Delegate && It->m_MemberName == node->m_InputPins[1].m_AdditionalData)
					{
						ClassMember_Delegate* delegate = (ClassMember_Delegate*)It.get();
						bool bRefreshPins = false;
						if (node->m_OutputPins.Size() - 1 != delegate->m_Args.Size())
						{
							bRefreshPins = true;
						}
						else
						{
							for (int i = 0; i < delegate->m_Args.Size(); i++)
							{
								if ((int64_t)delegate->m_Args[i] != node->m_OutputPins[i + 1].PinID)
								{
									bRefreshPins = true;
								}
							}
						}

						if (bRefreshPins)
						{
							node->m_OutputPins = Array<VisualNodePin>();
							node->AddOutputPin("Exec", Color(1.0f), 1, true, 30.0f);
							for (int i = 0; i < delegate->m_Args.Size(); i++)
							{
								// Do not add a None-Existing DataType to the Node! e.g. DelegateNoParams
								//if (delegate->m_Args[i] == ScriptDataType::None) continue;

								node->AddOutputPin(Util::SmartToUpperCase(delegate->m_StrArgs[i], false), GetScriptDataTypeColor(delegate->m_Args[i]), (int64_t)delegate->m_Args[i], false);
								VisualNodePin& pin = node->m_OutputPins[node->m_OutputPins.Last()];
								if (pin.PinID == (int64_t)ScriptDataType::ObjectPtr)
								{
									Class cls = Class::None;
									Array<Class> allClasses = Class::GetAllClasses();
									for (const Class& It : allClasses)
									{
										if (It.IsNative() && It.GetClassName() + "*" == delegate->m_StrArgs[i])
										{
											cls = It;
											break;
										}
									}
									pin.m_AdditionalData = cls.ToString();
								}
							}
						}
					}
				}
			}
			else if (node->m_NodeID == NODE_ID_INPUT_EVENT)
			{
				Ref<InputSettings> inputSettings = ProjectSettings::Get()->m_InputSettings;
				if (inputSettings)
				{
					for (auto& Category : inputSettings->m_Categories)
					{
						for (auto& Action : Category->m_Actions)
						{
							if (node->m_InputPins[0].m_AdditionalData == Category->m_CategoryName && node->m_InputPins[1].m_AdditionalData == Action->m_ActionName)
							{
								if (Action->m_ActionType == InputActionType::Action)
								{
									node->m_InputPins[2].m_AdditionalData = "Action";
									bool bRefresh = false;
									if (node->m_OutputPins.Size() != 3)
									{
										bRefresh = true;
									}
									else
									{
										if (node->m_OutputPins[0].PinID != 1) bRefresh = true;
										if (node->m_OutputPins[1].PinID != 1) bRefresh = true;
										if (node->m_OutputPins[2].PinID != 1) bRefresh = true;
									}
									if (bRefresh)
									{
										node->m_OutputPins.Clear();
										node->AddOutputPin("Pressed", Color(1.0f), 1, true, 30.0f);
										node->AddOutputPin("Released", Color(1.0f), 1, true, 30.0f);
										node->AddOutputPin("Helt", Color(1.0f), 1, true, 30.0f);
									}
								}
								else
								{
									SuoraVerify(false, "Missing Implementation!");
								}
							}
						}
					}
				}
			}
		}
	}

	void BlueprintNodeGraph::RecursiveNodeDelegates(Node* node, bool isChild)
	{
		const ClassReflector& refl = node->GetClass().GetClassReflector();

		Array<Ref<ClassMember>> members = refl.GetAllClassMember();
		for (Ref<ClassMember> It : members)
		{
			if (It->m_Type == ClassMember::Type::Delegate)
			{
				Ref<VisualNode> event = CreateRef<VisualNode>();
				if (isChild) event->m_Title = "Children/" + node->GetName() + std::string("/") + node->GetName() + std::string(" "); else event->m_Title = "Events/";
				event->m_Title += It->m_MemberName;

				event->m_NodeID = NODE_ID_DELEGATE;
				event->m_Color = Color(100 / 255.0f, 19 / 255.0f, 13 / 255.0f, 1.0f);
				event->m_Size = { 215, 105 };
				event->AddInputPin("ChildNode", Color(1.0f), 0, false, 20.0f); if (isChild) event->m_InputPins[0].m_AdditionalData = node->GetName();
				event->AddInputPin("DelegateName", Color(1.0f), 0, false, 20.0f); event->m_InputPins[1].m_AdditionalData = It->m_MemberName;
				event->AddInputPin("Class", Color(1.0f), 0, false, 20.0f); event->m_InputPins[2].m_AdditionalData = node->GetClass().ToString();
				event->AddOutputPin("Exec", Color(1.0f), 1, true, 30.0f);

				AddSupportedNode(event);
			}
		}

		for (int i = 0; i < node->GetChildCount(); i++)
		{
			RecursiveNodeDelegates(node->GetChild(i), true);
		}
	}

}