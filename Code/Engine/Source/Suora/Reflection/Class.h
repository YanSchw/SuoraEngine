#pragma once
#include "inttypes.h"
#include "Suora/Common/Array.h"
#include "Suora/Common/StringUtils.h"

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

	private:
		NativeClassID m_NativeClassID = 0;
		Blueprint* m_BlueprintClass = nullptr; // nullptr if this is a native class
		ScriptClass* m_ScriptClass = nullptr;  // nullptr if this is a native class
	public:
		explicit Class(const NativeClassID id);
		Class(Blueprint* node);
		explicit Class(Blueprint& node);
		Class(ScriptClass* script);
		explicit Class(ScriptClass& script);

		Class& operator=(const NativeClassID id);
		Class& operator=(Blueprint* node);
		Class& operator=(ScriptClass* script);

		NativeClassID GetNativeClassID() const;
		Blueprint* GetBlueprintClass() const;
		ScriptClass* GetScriptClass() const;

		bool operator==(const Class& cls) const;
		bool operator==(const NativeClassID id) const;
		bool operator==(const Blueprint* node) const;
		bool operator==(const ScriptClass* script) const;
		bool operator!=(const Class& cls) const;
		bool operator!=(const NativeClassID id) const;
		bool operator!=(Blueprint* node) const;
		bool operator!=(ScriptClass* script) const;

		bool IsNative() const;
		bool IsBlueprintClass() const;
		bool IsScriptClass() const;

		bool Inherits(const Class& base) const;
		Class GetParentClass() const;
		ClassType GetClassType() const;

		static void GenerateNativeClassReflector(const Class& cls);

		Array<Class> GetInheritanceTree() const;

		static Array<Class> GetAllClasses();
		static Array<Class> GetSubclassesOf(const Class& base);
		template<class T> static Array<Class> GetSubclassesOf() { return GetSubclassesOf(T::StaticClass()); }

		std::size_t operator()(const Class& cls) const
		{
			return ((std::hash<NativeClassID>()(cls.m_NativeClassID)
				^ (std::hash<Blueprint*>()(cls.m_BlueprintClass) << 1)) >> 1)
				^ (std::hash<NativeClassID>()(cls.m_NativeClassID + 91282743) << 1); // 91282743 is a random number, replace with ScriptClass once implemented
		}

		String GetNativeClassName() const;
		String GetClassName() const;
		const ClassReflector& GetClassReflector() const;

		String ToString() const;
		static Class FromString(const String& str);

		Object* GetClassDefaultObject(bool clear = false) const;

	};

	class ClassInternal
	{
		inline static std::vector<Class> s_NativeClasses;
		friend struct Class;
	};

}

// For use in std::unordered_map
namespace std
{
	template <>
	struct hash<Suora::Class>
	{
		std::size_t operator()(const Suora::Class& cls) const noexcept
		{
			return ((std::hash<Suora::NativeClassID>()(cls.GetNativeClassID())
				^ (std::hash<Suora::Blueprint*>()(cls.GetBlueprintClass()) << 1)) >> 1)
				^ (std::hash<Suora::NativeClassID>()(cls.GetNativeClassID() + 91282743) << 1); // 91282743 is a random number, replace with ScriptClass once implemented
		}
	};
}