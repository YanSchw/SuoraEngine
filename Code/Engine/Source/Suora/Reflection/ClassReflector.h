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
	struct ClassMemberProperty;
	struct Property;

	/** Used internally by the SuoraHeaderTool to properly reflect Members and Functions of SuoraClasses */
	struct ClassReflector
	{
		Array<Ref<ClassMemberProperty>> m_ClassProperties;
		String m_ClassName;
		Class m_NativeParentClass = Class::None;
		size_t m_ClassSize = 0;

		void AddClassProperty(const String& name, size_t offset, const Ref<Property>& property)
		{
			m_ClassProperties.Add(CreateRef<ClassMemberProperty>(name, offset, property));
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

		Array<Ref<ClassMemberProperty>> GetAllClassMemberProperties() const;

	private:
		inline static Map<NativeClassID, Ref<ClassReflector>> s_Reflectors;
	public:
		static void Create(const Class& cls, const std::function<void(ClassReflector&)>& reflLambda);
		static const ClassReflector& GetByClass(const Class& cls);
		static String GetClassName(const Class& cls);

		friend struct Class;
	};

}