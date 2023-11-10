#include "Precompiled.h"
#include "Blueprint.h"
#include "Suora/Common/Delegate.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/NodeScript/NodeScriptObject.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Suora/Assets/Level.h"
#include "Suora/GameFramework/Node.h"
#include "Platform/Platform.h"
#include "Suora/GameFramework/InputModule.h"

namespace Suora
{

	Blueprint::Blueprint()
	{
		m_ParentClass = Class::None;
		m_ScriptClass = (Ref<ScriptClassInternal>)new ScriptClassInternal();

	}

	void Blueprint::PreInitializeAsset(const String& str)
	{
		Super::PreInitializeAsset(str);

		Yaml::Node root;
		Yaml::Parse(root, str);
		m_UUID = root["UUID"].As<String>();
		m_YamlNode_EditorOnly = root;

		Yaml::Node& node = root["Node"];
		m_ParentClass = Class::FromString(root["NodeComposition"]["ParentClass"].As<String>()); //Class::FromString(node["m_ParentClass"].As<String>());
		if (m_ParentClass == Class::None)
		{
			m_ParentClass = Class::FromString(node["Class"].As<String>());
		}

		m_Composition = root["NodeComposition"];
		
		// Delegate Events
		{
			int i = 0;
			while (true)
			{
				Yaml::Node& delegate = node["m_DelegateEventsToBindDuringGameplay"][std::to_string(i++)];
				if (delegate.IsNone()) break;
				m_DelegateEventsToBindDuringGameplay.Add(DelegateEventBind(delegate["ChildName"].As<String>(), delegate["DelegateName"].As<String>(), delegate["ScriptFunctionHash"].As<size_t>()));
			}
		}

		// Input Events
		{
			int i = 0;
			while (true)
			{
				Yaml::Node& input = node["m_InputEventsToBeBound"][std::to_string(i++)];
				if (input.IsNone()) break;
				m_InputEventsToBeBound.Add(InputEventBind(input["Label"].As<String>(), input["ScriptFunctionHash"].As<size_t>(), (InputScriptEventFlags)input["Flags"].As<uint64_t>()));
			}
		}

		// ScriptClass
		m_ScriptClass->Deserialize(node);
	}

	Class Blueprint::GetNodeParentClass() const
	{
		return m_ParentClass;
	}

	Object* Blueprint::CreateInstance(bool isRootNode)
	{
		Node* node = Node::Deserialize(m_Composition, isRootNode);
		node->SetUpdateFlag(UpdateFlag::WorldUpdate);

		// If needed apply NodeGraph here !
		node->Implement<INodeScriptObject>();
		INodeScriptObject* interface = node->GetInterface<INodeScriptObject>();
		interface->m_Class = this;
		interface->m_ScriptClasses.Add(m_ScriptClass);
		interface->m_BlueprintLinks.Add(this);

		// Bind Delegates
		for (const DelegateEventBind& It : m_DelegateEventsToBindDuringGameplay)
		{
			Node* applicant = It.ChildName == "" ? node : node->GetChildByName(It.ChildName);

			if (applicant == nullptr)
			{
				SuoraError("Cannot bind DelegateEvent to \"{0}\"! Node does not exist!", It.ChildName);
				continue;
			}

			const ClassReflector& refl = applicant->GetClass().GetClassReflector();
			Array<Ref<ClassMember>> members = refl.GetAllClassMember();
			for (Ref<ClassMember> member : members)
			{
				if (member->m_Type == ClassMember::Type::Delegate && member->m_MemberName == It.DelegateName)
				{
					TDelegate* delegate = ClassMember::AccessMember<TDelegate>(applicant, member->m_MemberOffset);
					delegate->Bindings.Add(TDelegate::SciptDelegateBinding(node, It.ScriptFunctionHash));
				}
			}
		}

		return node;
	}

	void Blueprint::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		Yaml::Node& node = root["Node"];

		node["Class"] = m_ParentClass.ToString();

		// Serialize
		root["NodeComposition"] = m_Composition;
		

		// NodeGraphs
		Yaml::Node& graphs = node["Graphs"];
		int i = 0;
		while (true)
		{
			Yaml::Node& graph = m_YamlNode_EditorOnly["Node"]["Graphs"][std::to_string(i)];
			if (graph.IsNone()) break;
			graphs[std::to_string(i)] = graph;
			i++;
		}

		// Delegate Events
		for (int i = 0; i < m_DelegateEventsToBindDuringGameplay.Size(); i++)
		{
			Yaml::Node& delegate = node["m_DelegateEventsToBindDuringGameplay"][std::to_string(i)];
			delegate["ChildName"] = m_DelegateEventsToBindDuringGameplay[i].ChildName;
			delegate["DelegateName"] = m_DelegateEventsToBindDuringGameplay[i].DelegateName;
			delegate["ScriptFunctionHash"] = std::to_string(m_DelegateEventsToBindDuringGameplay[i].ScriptFunctionHash);
		}
		// Input Events
		for (int i = 0; i < m_InputEventsToBeBound.Size(); i++)
		{
			Yaml::Node& input = node["m_InputEventsToBeBound"][std::to_string(i)];
			input["Label"] = m_InputEventsToBeBound[i].Label;
			input["ScriptFunctionHash"] = std::to_string(m_InputEventsToBeBound[i].ScriptFunctionHash);
			input["Flags"] = std::to_string((uint64_t)m_InputEventsToBeBound[i].Flags);
		}

		// ScriptClass
		m_ScriptClass->Serialize(node);
	}
	
	void Blueprint::SetParentClass(const Class& cls)
	{
		m_ParentClass = cls;
	}
	void Blueprint::InitComposition(const Class& cls)
	{
		SetParentClass(cls);
		m_Composition = Yaml::Node();
		m_Composition["ChildCount"] = "0";
		m_Composition["RootEnabled"] = "true";
		m_Composition["RootName"] = GetAssetName();
		m_Composition["RootParentClass"] = GetNodeParentClass().ToString();
	}

}