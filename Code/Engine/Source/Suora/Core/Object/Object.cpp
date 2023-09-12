#include "Precompiled.h"
#include "Object.h"
#include "MemoryManager.h"
#include "Interface.h"
#include "Suora/NodeScript/NodeScriptObject.h"
#include "Suora/NodeScript/ScriptStack.h"


namespace Suora
{
	REGISTER_CLASS(Object, 1);

	Object::Object()
	{
		
	}

	Object::~Object()
	{
		//MemoryManager::s_MemoryFootprint -= (uint32_t)sizeof(this);
		MemoryManager::Instance()->NullifyPtrTo((Object*)this);
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

	void Object::ReflClass(ClassReflector& desc)
	{
		desc.SetClassName("Object");
		desc.SetClassSize(sizeof(Object));
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

		//ImplementUsingRootObject(cls, this);
	}
	void Object::ImplementUsingRootObject(const Class& cls, Object* root)
	{
		/*if (!m_Interface)
		{
			m_Interface = New(cls);
			if (Interface* interface = Cast<Interface>(m_Interface))
			{
				interface->m_RootObject = root;
				interface->OnImplementation();
			}
		}
		else
		{
			m_Interface->ImplementUsingRootObject(cls, root);
		}*/
	}
	void Object::Unimplement(const Class& cls)
	{
		if (!Implements(cls)) return;

		for (int i = 0; i < m_Interfaces.size(); i++)
		{
			if (m_Interfaces[i]->IsA(cls)) m_Interfaces.erase(m_Interfaces.begin() + i);
		}

		/*if (Cast(m_Interface, cls))
		{
			Object* new_Interface = m_Interface->m_Interface;
			m_Interface->m_Interface = nullptr;
			delete m_Interface;
			m_Interface = new_Interface;
		}
		else m_Interface->Unimplement(cls);*/
	}
	void Object::UnimplementAllInterfaces()
	{
		m_Interfaces.clear();
		/*if (m_Interface)
		{
			m_Interface->UnimplementAllInterfaces();
			delete m_Interface;
			m_Interface = nullptr;
		}
		m_Interface = nullptr;*/
	}
	bool Object::Implements(const Class& cls) const
	{
		for (auto& it : m_Interfaces)
		{
			if (it->IsA(cls)) return true;
		}

		return false;
		/*if (!m_Interface) return false;
		return Cast(m_Interface, cls) ? true : m_Interface->Implements(cls);*/
	}

	Object* Object::GetInterface(const Class& cls)
	{
		for (auto& it : m_Interfaces)
		{
			if (it->IsA(cls)) return it.get();
		}

		return nullptr;
		//return (m_Interface != nullptr) ? (Cast(m_Interface, cls) ? m_Interface : m_Interface->GetInterface(cls)) : nullptr;
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