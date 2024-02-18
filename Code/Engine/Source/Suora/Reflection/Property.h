#pragma once
#include <functional>
#include "Suora/Common/StringUtils.h"
#include "Suora/Reflection/Class.h"

namespace Suora
{
	class Object;

	enum class ScriptDataType : int64_t;
	struct TDelegate;

	enum class PropertyType : uint32_t
	{
		None = 0,

		Char,
		Int8,
		Int16,
		Int32,
		Int64,
		UInt8,
		UInt16,
		UInt32,
		UInt64,
		Bool,
		Float,
		Double,
		Vec2,
		Vec3,
		Vec4,
		Quat,
		String,
		MaterialSlots,
		SuoraID,
		ObjectPtr,
		Class,
		SubclassOf,
		Array,
		Delegate
	};

	struct Property
	{
		virtual PropertyType GetType() const = 0;
	};

	struct PrimitiveProperty : public Property
	{
	};

	struct CharProperty : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct Int8Property : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct Int16Property : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct Int32Property : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct Int64Property : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct UInt8Property : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct UInt16Property : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct UInt32Property : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct UInt64Property : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct Vec2Property : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct Vec3Property : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct Vec4Property : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct QuatProperty : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct StringProperty : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct MaterialSlotsProperty : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct BoolProperty : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct FloatProperty : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct DoubleProperty : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct SuoraIDProperty : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};
	struct ClassProperty : public PrimitiveProperty
	{
		virtual PropertyType GetType() const;
	};

	struct SubclassOfProperty : public Property
	{
		Class m_ObjectClass = Class::None;

		SubclassOfProperty(const Class& objectClass)
		{
			m_ObjectClass = objectClass;
		}
		virtual PropertyType GetType() const;
	};
	struct ArrayProperty : public Property
	{
		Ref<Property> m_InnerProperty;

		ArrayProperty(const Ref<Property>& inner)
		{
			m_InnerProperty = inner;
		}
		virtual PropertyType GetType() const;
	};
	struct ObjectPtrProperty : public Property
	{
		Class m_ObjectClass = Class::None;

		ObjectPtrProperty(const Class& objectClass)
		{
			m_ObjectClass = objectClass;
		}
		virtual PropertyType GetType() const;
	};
	struct DelegateProperty : public Property
	{
		DelegateProperty(const String& args)
		{
			FeedSignature(args);
		}
		void FeedSignature(const String& args);
		virtual PropertyType GetType() const;

		Array<String> m_StrArgs;
		Array<ScriptDataType> m_Args;
	};


	struct ClassMemberProperty
	{
		String m_MemberName = "";
		size_t m_MemberOffset = 0;
		Ref<Property> m_Property = nullptr;

		ClassMemberProperty() = default;
		ClassMemberProperty(const String& memberName, size_t memberOffset, Ref<Property> property)
			: m_MemberName(memberName), m_MemberOffset(memberOffset), m_Property(property)
		{
		}
		template<class T>
		static T* AccessMember(Object* obj, size_t offset)
		{
			return (T*)(((std::uint8_t*)obj) + offset);
		}
	};

}