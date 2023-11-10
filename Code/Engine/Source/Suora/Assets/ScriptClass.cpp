#include "Precompiled.h"
#include "ScriptClass.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/NodeScript/NodeScriptObject.h"
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

	void ScriptClass::PreInitializeAsset(const String& str)
	{
		//Super::PreInitializeAsset(str);

		/*Yaml::Node root;
		Yaml::Parse(root, str);
		m_UUID = root["UUID"].As<String>();

		Yaml::Node& node = root["Node"];
		m_ParentClass = Class::FromString(node["m_ParentClass"].As<String>());*/


		// ScriptClass
		//m_ScriptClass->Deserialize(node);
	}

	Class ScriptClass::GetScriptParentClass() const
	{
		return m_ParentClass;
	}

	Object* ScriptClass::CreateInstance(bool isRootNode)
	{
		Object* obj = New(m_ParentClass, isRootNode);

		// If needed apply NodeGraph here !
		obj->Implement<INodeScriptObject>();
		INodeScriptObject* interface = obj->GetInterface<INodeScriptObject>();
		interface->m_Class = this;
		interface->m_ScriptClasses.Add(m_ScriptClass);

		return obj;
	}

}