#include "PropertyGenerator.h"
#include "Common/Log.h"
#include "Common/Filesystem.h"
#include "Common/StringUtils.h"
#include "Common/Platform.h"
#include <iostream>
#include <fstream>

namespace Suora::Tools
{

	std::string PropertyGenerator::GeneratePropertyRef(std::string memberType)
	{
		// Preprocess
		if (memberType.find("&") != std::string::npos && memberType.find("const") != std::string::npos)
		{
			memberType.erase(memberType.find("&"), 1);
		}
		if (memberType.find("const") != std::string::npos)
		{
			memberType.erase(memberType.find("const"), 5);
		}
		while (memberType.find(" ") != std::string::npos)
		{
			memberType.erase(memberType.find(" "), 1);
		}

		// Create PropertyRef
		if (memberType == "char")
		{
			return "CreateRef<CharProperty>()";
		}
		if (memberType == "std::int8_t" || memberType == "int8_t")
		{
			return "CreateRef<Int8Property>()";
		}
		if (memberType == "std::int16_t" || memberType == "int16_t")
		{
			return "CreateRef<Int16Property>()";
		}
		if (memberType == "int" || memberType == "std::int32_t" || memberType == "int32_t")
		{
			return "CreateRef<Int32Property>()";
		}
		if (memberType == "std::int64_t" || memberType == "int64_t")
		{
			return "CreateRef<Int64Property>()";
		}
		if (memberType == "std::uint8_t" || memberType == "uint8_t")
		{
			return "CreateRef<UInt8Property>()";
		}
		if (memberType == "std::uint16_t" || memberType == "uint16_t")
		{
			return "CreateRef<UInt16Property>()";
		}
		if (memberType == "std::uint32_t" || memberType == "uint32_t")
		{
			return "CreateRef<UInt32Property>()";
		}
		if (memberType == "std::uint64_t" || memberType == "uint64_t")
		{
			return "CreateRef<UInt64Property>()";
		}
		if (memberType == "bool")
		{
			return "CreateRef<BoolProperty>()";
		}
		if (memberType == "float")
		{
			return "CreateRef<FloatProperty>()";
		}
		if (memberType == "double")
		{
			return "CreateRef<DoubleProperty>()";
		}
		if (memberType == "Vec2")
		{
			return "CreateRef<Vec2Property>()";
		}
		if (memberType == "Vec3")
		{
			return "CreateRef<Vec3Property>()";
		}
		if (memberType == "Vec4" || memberType == "Color")
		{
			return "CreateRef<Vec4Property>()";
		}
		if (memberType == "Quat")
		{
			return "CreateRef<QuatProperty>()";
		}
		if (memberType == "String" || memberType == "std::string")
		{
			return "CreateRef<StringProperty>()";
		}
		if (memberType == "MaterialSlots")
		{
			return "CreateRef<MaterialSlotsProperty>()";
		}
		if (memberType == "SuoraID")
		{
			return "CreateRef<SuoraIDProperty>()";
		}
		if (memberType == "Class")
		{
			return "CreateRef<ClassProperty>()";
		}

		if (memberType.substr(0, 11) == "SubclassOf<")
		{
			memberType.erase(0, 11);
			memberType.erase(memberType.size() - 1, 1);
			return "CreateRef<SubclassOfProperty>(Class::FromTemplate<" + memberType + ">())";
		}
		if (memberType.substr(0, 8) == "Delegate")
		{
			return "CreateRef<DelegateProperty>(\"" + memberType + "\")";
		}
		if (memberType.substr(0, 6) == "Array<")
		{
			return "CreateRef<ArrayProperty>(" + GeneratePropertyRef(memberType.substr(6, memberType.size() - 7)) + ")";
		}

		// ObjectPtr
		if (memberType[memberType.size() - 1] == '*') // if (memberType.find("*") != std::string::npos)
		{
			std::string type = memberType.substr(0, memberType.size() - 1);
			return "CreateRef<ObjectPtrProperty>(Class::FromTemplate<" + type + ">())";
		}

		BUILD_ERROR("Could not Create PropertyType!");

		return "";
	}

}