#pragma once
#include <functional>
#include "Suora/Common/StringUtils.h"
#include "Class.h"
#include "Suora/Common/Array.h"
#include "Suora/Common/Map.h"

namespace Suora
{
	class Object;

	enum class ScriptDataType : int64_t;
	struct TDelegate;

	struct ClassMember
	{
		enum class Type
		{
			None = 0,
			Class,
			StackAllocatedObject,
			ObjectPtr,
			ArrayList,
			Integer32,
			Float,
			Bool,
			Vector3,
			MaterialSlots,
			SuoraID,
			Vector4,
			SubclassOf,
			Delegate
		};

		String m_MemberName = "";
		size_t m_MemberOffset = 0;
		Type m_Type = Type::None;

		ClassMember() = default;
		ClassMember(const String& memberName, size_t memberOffset, Type type)
			: m_MemberName(memberName), m_MemberOffset(memberOffset), m_Type(type)
		{
		}
		template<class T>
		static T* AccessMember(Object* obj, size_t offset)
		{
			return (T*)(((std::uint8_t*)obj) + offset);
		}
		template<class T>
		static ClassMember::Type GetPrimitiveTypeEnumByTemplate();
		template<class T>
		static Ref<ClassMember> CreatePrimitive(size_t offset, const String& name)
		{
			return Ref<ClassMember>(new ClassMember(name, offset, ClassMember::GetPrimitiveTypeEnumByTemplate<T>()));
		}
	};

	struct ClassMember_ArrayList : public ClassMember
	{
		Ref<ClassMember> m_ArraySubMember;

		ClassMember_ArrayList(const String& memberName, size_t memberOffset, Type type)
			: ClassMember(memberName, memberOffset, type)
		{
		}
	}; 
	struct ClassMember_ObjectPtr : public ClassMember
	{
		Class m_ObjectClass = Class::None;

		ClassMember_ObjectPtr(const String& memberName, size_t memberOffset, Type type)
			: ClassMember(memberName, memberOffset, type)
		{
		}
	};
	struct ClassMember_Delegate : public ClassMember
	{
		ClassMember_Delegate(const String& memberName, size_t memberOffset, Type type)
			: ClassMember(memberName, memberOffset, type)
		{
		}
		void FeedSignature(const String& args);

		Array<String> m_StrArgs;
		Array<ScriptDataType> m_Args;
	};

//====================================================================================================//

	/** Used internally by the SuoraHeaderTool to properly reflect Members and Functions of SuoraClasses */
	struct ClassReflector
	{
		Array<Ref<ClassMember>> m_ClassMembers;
		String m_ClassName;
		Class m_NativeParentClass = Class::None;
		size_t m_ClassSize = 0;

		template<class T>
		void AddObjectPointer(size_t offset, const String& name)
		{
			ClassMember_ObjectPtr* objectPtr = AddMember<ClassMember_ObjectPtr>(name, offset, ClassMember::Type::ObjectPtr);
			objectPtr->m_ObjectClass = T::StaticClass();
		}
		template<class ... Args>
		void AddDelegate(size_t offset, const String& name, const String& args)
		{
			ClassMember_Delegate* delegate = AddMember<ClassMember_Delegate>(name, offset, ClassMember::Type::Delegate);
			delegate->FeedSignature(args);
		}
		
		template<class T>
		void AddPrimitive(size_t offset, const String& name)
		{
			AddMember<ClassMember>(name, offset, ClassMember::GetPrimitiveTypeEnumByTemplate<T>());
		}

		ClassMember_ArrayList* AddArrayList(size_t offset, const String& name)
		{
			ClassMember_ArrayList* member = AddMember<ClassMember_ArrayList>(name, offset, ClassMember::Type::ArrayList);
			return member;
		}

		template<class T>
		T* AddMember(const String& name, size_t offset, ClassMember::Type type)
		{
			Ref<T> member = Ref<T>(new T(name, offset, type));
			m_ClassMembers.Add(member);
			return member.get();
		}

		void SetClassName(const String& name)
		{
			m_ClassName = name;
		}
		void SetNativeParentClass(const Class& cls)
		{
			m_NativeParentClass = cls;
		}
		void SetClassSize(size_t size)
		{
			m_ClassSize = size;
		}

		Array<Ref<ClassMember>> GetAllClassMember() const;

	private:
		inline static Map<NativeClassID, Ref<ClassReflector>> s_Reflectors;
	public:
		static void Create(const Class& cls, const std::function<void(ClassReflector&)>& reflLambda);
		static const ClassReflector& GetByClass(const Class& cls);
		static String GetClassName(const Class& cls);

		friend struct Class;
	};

}