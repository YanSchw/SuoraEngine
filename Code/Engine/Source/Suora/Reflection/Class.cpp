#include "Precompiled.h"
#include "Class.h"
#include "ClassReflector.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/Assets/ScriptClass.h"
#include "Suora/Common/Array.h"
#include "Suora/Common/Random.h"

namespace Suora
{
	const Class Class::None = Class((NativeClassID) 0);

	bool Class::Inherits(const Class& base) const
	{
		if ((*this) == base) return true;
		return (m_NativeClassID == 1) ? false : (GetParentClass() == base ? true : (GetParentClass() != Class::None ? GetParentClass().Inherits(base) : false));
	}

	Class Class::GetParentClass() const
	{
		switch (GetClassType())
		{
		case ClassType::Native:
			return ClassReflector::GetByClass(*this).m_NativeParentClass;
		case ClassType::BlueprintClass:
			return m_BlueprintClass->GetNodeParentClass();
		case ClassType::ScriptClass:
			return m_ScriptClass->GetScriptParentClass();
		case ClassType::None:
		default:
			SuoraError("Class::GetParentClass() implementation missing!");
			return Class::None;
		}
		return Class::None;
	}

	ClassType Class::GetClassType() const
	{
		if (IsNative() && m_NativeClassID != 0) return ClassType::Native;
		if (IsScriptClass()) return ClassType::ScriptClass;
		if (IsBlueprintClass()) return ClassType::BlueprintClass;

		return ClassType::None;
	}

	void Class::GenerateNativeClassReflector(const Class& cls)
	{
		ClassReflector::GetByClass(cls);
		ClassInternal::s_NativeClasses.push_back(cls);
	}

	Array<Class> Class::GetInheritanceTree() const
	{
		Array<Class> out;
		Class level = *this;

		while (level != Class::None)
		{
			out.Insert(0, level);
			level = level.GetParentClass();
		}

		return out;
	}

	Array<Class> Class::GetAllClasses()
	{
		Array<Class> classes = ClassInternal::s_NativeClasses;
		classes.Add(Object::StaticClass());

		const Array<Blueprint*> blueprints = AssetManager::GetAssets<Blueprint>();
		for (Blueprint* bp : blueprints)
		{
			classes.Add(Class(bp));
		}

		const Array<ScriptClass*> sclasses = AssetManager::GetAssets<ScriptClass>();
		for (ScriptClass* sclass : sclasses)
		{
			classes.Add(Class(sclass));
		}

		return classes;
	}

	Array<Class> Class::GetSubclassesOf(const Class& base)
	{
		Array<Class> classes = GetAllClasses();
		Array<Class> subClasses;

		int i = 0;
		for (Class& cls : classes)
		{
			if (cls != Class::None && cls.Inherits(base))
			{
				subClasses.Add(cls);
			}
		}

		return subClasses;
	}

	String Class::GetNativeClassName() const
	{
		return ClassReflector::GetClassName(*this);
	}

	String Class::GetClassName() const
	{
		switch (GetClassType())
		{
		case ClassType::Native:
			return GetNativeClassName();
		case ClassType::BlueprintClass:
			return m_BlueprintClass->GetAssetName();
		case ClassType::ScriptClass:
		case ClassType::None:
			return "None";
		default:
			SuoraError("Class::GetParentClass() implementation missing!");
			return "";
		}
		return "";
	}

	const ClassReflector& Class::GetClassReflector() const
	{
		return ClassReflector::GetByClass(*this);
	}

	String Class::ToString() const
	{
		String str = IsNative() ? "Native$" : "Node$";

		if (IsNative())
		{
			str.append(std::to_string(GetNativeClassID()));
		}
		else
		{
			str.append(GetBlueprintClass()->m_UUID.GetString());
		}

		return str;
	}

	Class Class::FromString(const String& str)
	{
		if (str.find("Native$") != String::npos)
		{
			const String substr = str.substr(7, str.size() - 7);
			const NativeClassID id = std::stoll(substr);
			return Class(id);
		}
		if (str.find("Node$") != String::npos)
		{
			const String uid = str.substr(5, str.size() - 5);
			return Class(AssetManager::GetAsset<Blueprint>(uid));
		}

		return Class::None;
	}

	Object* Class::GetClassDefaultObject(bool clear) const
	{
		static std::unordered_map<Class, Ref<Object>, std::hash<Suora::Class>> s_ClassDefaultObjects;
		if (clear) s_ClassDefaultObjects.clear();

		if (*this == Class::None) return nullptr;

		if (s_ClassDefaultObjects.find(*this) == s_ClassDefaultObjects.end())
		{
			Ref<Object> obj = Ref<Object>(New(*this));

			s_ClassDefaultObjects[*this] = obj;

			return s_ClassDefaultObjects[*this].get();
		}
		else
		{
			return s_ClassDefaultObjects[*this].get();
		}
	}

	Class::Class(const NativeClassID id)
	{
		m_NativeClassID = id;
	}
	Class::Class(Blueprint* node)
	{
		m_BlueprintClass = node;
	}
	Class::Class(Blueprint& node)
	{
		m_BlueprintClass = &node;
	}
	Class::Class(ScriptClass* script)
	{
		m_ScriptClass = script;
	}
	Class::Class(ScriptClass& script)
	{
		m_ScriptClass = &script;
	}
	Class& Class::operator=(const NativeClassID id)
	{
		m_NativeClassID = id;
		m_BlueprintClass = nullptr;
		m_ScriptClass = nullptr;
		return *this;
	}
	Class& Class::operator=(Blueprint* node)
	{
		m_NativeClassID = 0;
		m_BlueprintClass = node;
		m_ScriptClass = nullptr;
		return *this;
	}
	Class& Class::operator=(ScriptClass* script)
	{
		m_NativeClassID = 0;
		m_BlueprintClass = nullptr;
		m_ScriptClass = script;
		return *this;
	}
	NativeClassID Class::GetNativeClassID() const
	{
		return m_NativeClassID;
	}
	Blueprint* Class::GetBlueprintClass() const
	{
		return m_BlueprintClass;
	}
	ScriptClass* Class::GetScriptClass() const
	{
		return m_ScriptClass;
	}
	bool Class::operator==(const Class& cls) const
	{
		return IsNative() ? (*this == cls.m_NativeClassID) : (IsScriptClass() ? (*this == cls.m_ScriptClass) : (*this == cls.m_BlueprintClass));
	}
	bool Class::operator==(const NativeClassID id) const
	{
		return id == m_NativeClassID;
	}
	bool Class::operator==(const Blueprint* node) const
	{
		return node == m_BlueprintClass;
	}
	bool Class::operator==(const ScriptClass* script) const
	{
		return script == m_ScriptClass;
	}
	bool Class::operator!=(const Class& cls) const
	{
		return !operator==(cls);
	}
	bool Class::operator!=(const NativeClassID id) const
	{
		return !operator==(id);
	}
	bool Class::operator!=(Blueprint* node) const
	{
		return !operator==(node);
	}
	bool Class::operator!=(ScriptClass* script) const
	{
		return !operator==(script);
	}
	bool Class::IsNative() const
	{
		return !m_BlueprintClass && !m_ScriptClass;
	}
	bool Class::IsBlueprintClass() const
	{
		return m_BlueprintClass;
	}
	bool Class::IsScriptClass() const
	{
		return m_ScriptClass;
	}
}