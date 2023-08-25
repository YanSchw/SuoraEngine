#pragma once
#include "inttypes.h"
#include <unordered_map>

template<class T> class Array;

namespace Suora
{
	class Object;
	class Blueprint;
	class ScriptClass;
	struct ClassReflector;

	using NativeClassID = uint64_t;

	enum class ClassType : uint32_t
	{
		None = 0,
		Native,
		BlueprintClass,
		ScriptClass
	};

	/** Representation of any SuoraClasses; Regardless of ClassType (Native C++, Blueprint Asset or external ScriptClass) */
	struct Class
	{
		static const Class None;
		friend struct Class;

	private:
		NativeClassID m_NativeClassID = 0;
		Blueprint* m_BlueprintClass = nullptr; // nullptr if this is a native class
		ScriptClass* m_ScriptClass = nullptr;  // nullptr if this is a native class
	public:
		Class(NativeClassID id)
		{
			m_NativeClassID = id;
		}
		Class(Blueprint* node)
		{
			m_BlueprintClass = node;
		}
		Class(Blueprint& node)
		{
			m_BlueprintClass = &node;
		}
		Class(ScriptClass* script)
		{
			m_ScriptClass = script;
		}
		Class(ScriptClass& script)
		{
			m_ScriptClass = &script;
		}

		Class& operator=(NativeClassID id)
		{
			m_NativeClassID = id;
			m_BlueprintClass = nullptr;
			m_ScriptClass = nullptr;
			return *this;
		}
		Class& operator=(Blueprint* node)
		{
			m_NativeClassID = 0;
			m_BlueprintClass = node;
			m_ScriptClass = nullptr;
			return *this;
		}
		Class& operator=(ScriptClass* script)
		{
			m_NativeClassID = 0;
			m_BlueprintClass = nullptr;
			m_ScriptClass = script;
			return *this;
		}

		NativeClassID GetNativeClassID() const
		{
			return m_NativeClassID;
		}
		Blueprint* GetBlueprintClass() const
		{
			return m_BlueprintClass;
		}
		ScriptClass* GetScriptClass() const
		{
			return m_ScriptClass;
		}

		bool operator==(const Class& cls) const
		{
			return IsNative() ? (*this == cls.m_NativeClassID) : ( IsScriptClass() ? (*this == cls.m_ScriptClass) : (*this == cls.m_BlueprintClass));
		}
		bool operator==(NativeClassID id) const
		{
			return id == m_NativeClassID;
		}
		bool operator==(Blueprint* node) const
		{
			return node == m_BlueprintClass;
		}
		bool operator==(ScriptClass* script) const
		{
			return script == m_ScriptClass;
		}
		bool operator!=(const Class& cls) const
		{
			return !operator==(cls);
		}
		bool operator!=(NativeClassID id) const
		{
			return !operator==(id);
		}
		bool operator!=(Blueprint* node) const
		{
			return !operator==(node);
		}
		bool operator!=(ScriptClass* script) const
		{
			return !operator==(script);
		}

		bool IsNative() const
		{
			return !m_BlueprintClass && !m_ScriptClass;
		}
		bool IsBlueprintClass() const
		{
			return m_BlueprintClass;
		}
		bool IsScriptClass() const
		{
			return m_ScriptClass;
		}

		bool Inherits(const Class& base) const
		{
			if ((*this) == base) return true;
			return (m_NativeClassID == 1) ? false : (GetParentClass() == base ? true : (GetParentClass() != Class::None ? GetParentClass().Inherits(base) : false));
		}
		Class GetParentClass() const;
		ClassType GetClassType() const;

		static void GenerateNativeClassReflector(const Class& cls);

		Array<Class> GetInheritanceTree();

		static Array<Class> GetAllClasses();
		static Array<Class> GetSubclassesOf(const Class& base);
		template<class T> static Array<Class> GetSubclassesOf() { return GetSubclassesOf(T::StaticClass()); }

		std::size_t operator()(const Class& cls) const
		{
			return ((std::hash<NativeClassID>()(cls.m_NativeClassID)
				^ (std::hash<Blueprint*>()(cls.m_BlueprintClass) << 1)) >> 1)
				^ (std::hash<NativeClassID>()(cls.m_NativeClassID + 91282743) << 1); // 91282743 is a random number, replace with ScriptClass once implemented
		}

		std::string GetNativeClassName() const;
		std::string GetClassName() const;
		const ClassReflector& GetClassReflector() const;

		std::string ToString() const;
		static Class FromString(const std::string& str);

		Object* GetClassDefaultObject(bool clear = false) const;

		inline static std::vector<Class> s_NativeClasses;
	};

}

// For use in std::unordered_map
namespace std
{
	template <>
	struct hash<Suora::Class>
	{
		std::size_t operator()(const Suora::Class& cls) const
		{
			return ((std::hash<Suora::NativeClassID>()(cls.GetNativeClassID())
				^ (std::hash<Suora::Blueprint*>()(cls.GetBlueprintClass()) << 1)) >> 1)
				^ (std::hash<Suora::NativeClassID>()(cls.GetNativeClassID() + 91282743) << 1); // 91282743 is a random number, replace with ScriptClass once implemented
		}
	};
}