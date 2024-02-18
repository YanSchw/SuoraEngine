#include "Precompiled.h"
#include "Property.h"
#include "Suora/Core/Object/Object.h"
#include "Suora/NodeScript/ScriptTypes.h"

namespace Suora
{

	PropertyType CharProperty::GetType() const
	{
		return PropertyType::Char;
	}

	PropertyType Int8Property::GetType() const
	{
		return PropertyType::Int8;
	}
	PropertyType Int16Property::GetType() const
	{
		return PropertyType::Int16;
	}
	PropertyType Int32Property::GetType() const
	{
		return PropertyType::Int32;
	}
	PropertyType Int64Property::GetType() const
	{
		return PropertyType::Int64;
	}

	PropertyType UInt8Property::GetType() const
	{
		return PropertyType::UInt8;
	}
	PropertyType UInt16Property::GetType() const
	{
		return PropertyType::UInt16;
	}
	PropertyType UInt32Property::GetType() const
	{
		return PropertyType::UInt32;
	}
	PropertyType UInt64Property::GetType() const
	{
		return PropertyType::UInt64;
	}

	PropertyType Vec2Property::GetType() const
	{
		return PropertyType::Vec2;
	}
	PropertyType Vec3Property::GetType() const
	{
		return PropertyType::Vec3;
	}
	PropertyType Vec4Property::GetType() const
	{
		return PropertyType::Vec4;
	}
	PropertyType QuatProperty::GetType() const
	{
		return PropertyType::Quat;
	}
	PropertyType StringProperty::GetType() const
	{
		return PropertyType::String;
	}
	PropertyType MaterialSlotsProperty::GetType() const
	{
		return PropertyType::MaterialSlots;
	}
	PropertyType BoolProperty::GetType() const
	{
		return PropertyType::Bool;
	}
	PropertyType FloatProperty::GetType() const
	{
		return PropertyType::Float;
	}
	PropertyType DoubleProperty::GetType() const
	{
		return PropertyType::Double;
	}
	PropertyType SuoraIDProperty::GetType() const
	{
		return PropertyType::SuoraID;
	}
	PropertyType ClassProperty::GetType() const
	{
		return PropertyType::Class;
	}
	PropertyType SubclassOfProperty::GetType() const
	{
		return PropertyType::SubclassOf;
	}
	PropertyType ArrayProperty::GetType() const
	{
		return PropertyType::Array;
	}
	PropertyType ObjectPtrProperty::GetType() const
	{
		return PropertyType::ObjectPtr;
	}
	PropertyType DelegateProperty::GetType() const
	{
		return PropertyType::Delegate;
	}

	void DelegateProperty::FeedSignature(const String& args)
	{
		String sub = args.substr(9, args.size() - 10);
		std::vector<String> arg = StringUtil::SplitString(sub, ',');
		for (auto& It : arg)
		{
			if (It != "TDelegate::NoParams")
			{
				m_StrArgs.Add(It);
				m_Args.Add(StringToScriptDataType(It));
			}
		}
	}


}