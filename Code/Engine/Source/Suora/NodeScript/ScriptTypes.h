#pragma once
#include <string>
#include <inttypes.h>
#include <glm/glm.hpp>

namespace Suora
{
	using Color = glm::vec4;

	enum class ScriptDataType : int64_t
	{
		None = 10,
		Float,
		Vec2,
		Vec3,
		Vec4,
		Quat,
		ObjectPtr,
		Class,
		Bool,
		Void,

		COUNT
	};

	template<class T>
	ScriptDataType TemplateToScriptDataType()
	{
		// default implmentation assumes an ObjectPtr		
		return std::is_base_of<Object, std::remove_pointer_t<T>>::value ? ScriptDataType::ObjectPtr : ScriptDataType::None;
	}

	static ScriptDataType StringToScriptDataType(std::string str)
	{
		if (str.find("const") != std::string::npos && str.find("&") != std::string::npos)
		{
			str.erase(str.find("const"), 5);
			str.erase(str.find("&"), 1);
		}
		for (int i = str.size() - 1; i >= 0; i--)
		{
			if (str[i] == ' ') str.erase(i, 1);
		}
		if (str == "float") return ScriptDataType::Float;
		if (str == "Vec2") return ScriptDataType::Vec2;
		if (str == "Vec3") return ScriptDataType::Vec3;
		if (str == "Vec4") return ScriptDataType::Vec4;
		if (str == "ObjectPtr" || str[str.size() - 1] == '*') return ScriptDataType::ObjectPtr;
		if (str == "Class") return ScriptDataType::Class;
		if (str == "bool") return ScriptDataType::Bool;
		return ScriptDataType::None;
	}
	static std::string ScriptDataTypeToLabel(ScriptDataType type)
	{
		switch (type)
		{
		case ScriptDataType::Float: return "Float";
		case ScriptDataType::Vec2: return "Vec2";
		case ScriptDataType::Vec3: return "Vec3";
		case ScriptDataType::Vec4: return "Vec4";
		case ScriptDataType::ObjectPtr: return "ObjectPtr";
		case ScriptDataType::Class: return "Class";
		case ScriptDataType::Bool: return "Bool";
		case ScriptDataType::None:
		default:
			return "None";
		}
	}
	static Color GetScriptDataTypeColor(ScriptDataType type)
	{
		switch (type)
		{
		case ScriptDataType::Float: return glm::vec4(0.4f, 0.49f, 0.38f, 1.0f);
		case ScriptDataType::Vec2: return glm::vec4(0.4f, 0.2f, 0.28f, 1.0f);
		case ScriptDataType::Vec3: return glm::vec4(0.62f, 0.59f, 0.28f, 1.0f);
		case ScriptDataType::Vec4: return glm::vec4(0.52f, 0.59f, 0.48f, 1.0f);
		case ScriptDataType::ObjectPtr: return glm::vec4(0.0f, 0.64f, 0.94f, 1.0f);
		case ScriptDataType::Class: return glm::vec4(0.67f, 0.04f, 0.74f, 1.0f);
		case ScriptDataType::Bool: return glm::vec4(0.62f, 0.05f, 0.05f, 1.0f);
		case ScriptDataType::None:
		default:
			return glm::vec4(1.0f);
		}
		return glm::vec4(1.0f);
	}

}