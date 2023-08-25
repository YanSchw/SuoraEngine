#include "Precompiled.h"
#include "ScriptClass.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/NodeScript/NodeScriptObject.h"
#include "Suora/Serialization/CompositionLayer.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Suora/Assets/Level.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/Reflection/New.h"

namespace Suora
{

	ScriptClass::ScriptClass()
	{
		m_ParentClass = Class::None;
		m_ScriptClass = (Ref<ScriptClassInternal>)new ScriptClassInternal();

	}

	void ScriptClass::PreInitializeAsset(const std::string& str)
	{
		//Super::PreInitializeAsset(str);

		/*Yaml::Node root;
		Yaml::Parse(root, str);
		m_UUID = root["UUID"].As<std::string>();

		Yaml::Node& node = root["Node"];
		m_ParentClass = Class::FromString(node["m_ParentClass"].As<std::string>());*/


		// ScriptClass
		//m_ScriptClass->Deserialize(node);
	}

	Class ScriptClass::GetScriptParentClass() const
	{
		return m_ParentClass;
	}

	Object* ScriptClass::CreateInstance(bool includeCompositionData)
	{
		Object* obj = New(m_ParentClass, includeCompositionData, false);
		
		/*if (Node* node = Cast<Node>(obj))
		{
			// Apply Composition Layer in World
			Level temp;
			node->SetUpdateMode(EUpdateMode::WorldUpdate);
		}
		else
		{
			// Apply Composition Layer to Object
		}*/

		// If needed apply NodeGraph here !
		obj->Implement<INodeScriptObject>();
		INodeScriptObject* interface = obj->GetInterface<INodeScriptObject>();
		interface->m_Class = this;
		interface->m_ScriptClasses.Add(m_ScriptClass);

		return obj;
	}

}