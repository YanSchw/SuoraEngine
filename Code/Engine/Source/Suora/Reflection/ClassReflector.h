#pragma once
#include "Suora/Common/StringUtils.h"
#include "Class.h"
#include "Suora/Common/Array.h"

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
			AssetPtr,
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
			return (T*)(((char*)obj) + offset);
		}
		inline static size_t OffsetOf(void* obj, void* ptr)
		{
			return size_t(ptr) - size_t(obj);
		}
		template<class T>
		static ClassMember::Type GetPrimitiveTypeEnumByTemplate();
		template<class T>
		static Ref<ClassMember> CreatePrimitive(class Object* obj, T* ptr, const String& name)
		{
			return Ref<ClassMember>(new ClassMember(name, ClassMember::OffsetOf(obj, ptr), ClassMember::GetPrimitiveTypeEnumByTemplate<T>()));
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
	struct ClassMember_AssetPtr : public ClassMember
	{
		Class m_AssetClass = Class::None;

		ClassMember_AssetPtr(const String& memberName, size_t memberOffset, Type type)
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

		void AddObjectPointer(class Object* obj, class Object** ptr, const String& name)
		{
			AddMember<ClassMember>(name, ClassMember::OffsetOf(obj, ptr), ClassMember::Type::ObjectPtr);
		}
		void AddAssetPointer(class Object* obj, class Asset** ptr, const String& name, NativeClassID assetClassID)
		{
			ClassMember_AssetPtr* assetPtr = AddMember<ClassMember_AssetPtr>(name, ClassMember::OffsetOf(obj, ptr), ClassMember::Type::AssetPtr);
			assetPtr->m_AssetClass = Class(assetClassID);
		}
		template<class ... Args>
		void AddDelegate(class Object* obj, TDelegate* d, const String& name, const String& args)
		{
			ClassMember_Delegate* delegate = AddMember<ClassMember_Delegate>(name, ClassMember::OffsetOf(obj, d), ClassMember::Type::Delegate);
			delegate->FeedSignature(args);
		}
		
		template<class T>
		void AddPrimitive(class Object* obj, T* ptr, const String& name)
		{
			AddMember<ClassMember>(name, ClassMember::OffsetOf(obj, ptr), ClassMember::GetPrimitiveTypeEnumByTemplate<T>());
		}

		ClassMember_ArrayList* AddArrayList(class Object* obj, void* array, const String& name)
		{
			if (obj)
			{
				size_t offset = size_t(array) - size_t(obj);
				ClassMember_ArrayList* member = AddMember<ClassMember_ArrayList>(name, offset, ClassMember::Type::ArrayList);
				return member;
			}
			return nullptr;
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
		inline static std::unordered_map<NativeClassID, ClassReflector*> m_Reflectors;
	public:
		static const ClassReflector& GetByClass(const Class& cls);
		static String GetClassName(const Class& cls);

		friend struct Class;
	};

}