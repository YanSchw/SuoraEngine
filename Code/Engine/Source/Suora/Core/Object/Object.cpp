#include "Precompiled.h"
#include "Object.h"
#include "Interface.h"
#include "Suora/NodeScript/NodeScriptObject.h"
#include "Suora/NodeScript/ScriptStack.h"


namespace Suora
{
	SUORA_REGISTER_CLASS(Object, 1);

	Object::Object()
	{
		
	}

	Object::~Object()
	{
		UnimplementAllInterfaces();
		InternalPtr::Nullify(this);
	}

	Class Object::GetClass()
	{
		if (INodeScriptObject* obj = GetInterface<INodeScriptObject>())
		{
			return obj->m_Class;
		}

		return GetNativeClass();
	}

	void Object::Implement(const Class& cls)
	{
		if (Implements(cls)) return;

		Object* obj = New(cls);
		m_Interfaces.push_back(Ref<Object>(obj));
		if (Interface* interface = Cast<Interface>(obj))
		{
			interface->m_RootObject = this;
			interface->OnImplementation();
		}
	}
	void Object::Unimplement(const Class& cls)
	{
		if (!Implements(cls)) return;

		for (int i = 0; i < m_Interfaces.size(); i++)
		{
			if (m_Interfaces[i]->IsA(cls)) m_Interfaces.erase(m_Interfaces.begin() + i);
		}
	}
	void Object::UnimplementAllInterfaces()
	{
		m_Interfaces.clear();
	}
	bool Object::Implements(const Class& cls) const
	{
		for (auto& it : m_Interfaces)
		{
			if (it->IsA(cls)) return true;
		}

		return false;
	}

	Object* Object::GetInterface(const Class& cls)
	{
		for (auto& it : m_Interfaces)
		{
			if (it->IsA(cls)) return it.get();
		}

		return nullptr;
	}

	bool Object::IsA(const Class& cls) const
	{
		return CastImpl(cls);
	}

	void Object::__NodeEventDispatch(size_t hash, ScriptStack& stack)
	{
		if (INodeScriptObject* obj = GetInterface<INodeScriptObject>()) 
		{
			obj->TryDispatchNodeEvent(hash, stack);
		}
	}
	void Object::__NodeEventDispatch(size_t hash)
	{
		if (INodeScriptObject* obj = GetInterface<INodeScriptObject>())
		{
			ScriptStack stack;
			obj->TryDispatchNodeEvent(hash, stack);
		}
	}


}