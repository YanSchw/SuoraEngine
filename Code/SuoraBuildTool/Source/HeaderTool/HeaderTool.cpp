#include "HeaderTool.h"

#include "Common/Log.h"
#include "Common/Filesystem.h"
#include "Common/StringUtils.h"
#include "Common/Platform.h"

#include <iostream>
#include <fstream>

static std::mutex PrintMutex;
static std::mutex AccessHeaderMutex;
static std::mutex GenerateClassMutex;

namespace Suora::Tools
{
	void HeaderTool::FetchHeaders(const std::string& path)
	{
		m_InitialTime = Platform::GetTime();
		BUILD_INFO("Parsing Headers in: {0}", path);
		std::vector<std::filesystem::directory_entry> entries = File::GetAllAbsoluteEntriesOfPath(path);
		for (auto& file : entries)
		{
			const std::string ext = File::GetFileExtension(file);
			if (ext == ".h")
			{
				m_HeaderPaths.push_back(file);
			}
		}
	}
	void HeaderTool::ParseHeaders(const std::filesystem::path& outPath, bool cacheWriteTime)
	{
		std::vector<std::future<void>> future;

		Ref<Header> obj = Ref<Header>(new Header(this));
		obj->m_ClassID = "1";
		obj->m_ClassName = "Object";
		AddHeader(obj);

		for (auto& path : m_HeaderPaths)
		{
			future.push_back(std::async(std::launch::async, &HeaderTool::ParseSingleHeader, this, path));
		}
		for (auto& fut : future)
		{
			fut.get();
		}
		future.clear();

		ParseAllHeadersParentData();

		for (Ref<Header> header : m_Headers)
		{
			if (header->m_ClassID == "1") continue;
			future.push_back(std::async(std::launch::async, &HeaderTool::GenerateClassSymbols, this, header.get(), &m_Output));
		}
		for (auto& fut : future)
		{
			fut.get();
		}
		future.clear();

		bool bWasAnyFileWritten = false;
		for (auto& it : m_Output)
		{
			if (!cacheWriteTime || !std::filesystem::exists(it.first.m_GeneratedHeaderPath)
				|| std::filesystem::last_write_time(it.first.m_OriginalHeaderPath) > std::filesystem::last_write_time(it.first.m_GeneratedHeaderPath))
			{
				bWasAnyFileWritten = true;
				const std::string str = "#pragma once\n";
				Platform::WriteToFile(it.first.m_GeneratedHeaderPath, str + it.second + "\n\n\n");
				BUILD_DEBUG("Generated: {0}", it.first.m_GeneratedHeaderPath);
			}
		}

		if (!bWasAnyFileWritten)
		{
			BUILD_INFO("No new Headers were generated!");
		}

		BUILD_INFO("Done in {0} seconds.", Platform::GetTime() - m_InitialTime);
	}

	void HeaderTool::ParseSingleHeader(const std::filesystem::path& path)
	{
		std::string str = Platform::ReadFromFile(path.string());

		// Check if further proccessing is necessary
		if (str.find("SUORA_CLASS") == std::string::npos)
		{
			return;
		}

		StringUtil::RemoveCommentsFromString(str);
		// TODO: Remove Comments from file. AND Remove \n and \t etc.
		if (str.find("SUORA_CLASS") != std::string::npos && path.stem() != "Object")
		{
			size_t offset = 0;
			while (true)
			{
				offset = str.find("SUORA_CLASS", offset + 1);
				if (offset == std::string::npos) break;
				Ref<Header> header = Ref<Header>(new Header(this));
				AddHeader(header);
				header->str = str;
				header->m_HeaderPath = path.string();

				size_t classBodyBegin = offset;
				{ int bracketCount = 1; while (bracketCount > 0) { classBodyBegin--; bracketCount += (str[classBodyBegin] == '{') ? -1 : (str[classBodyBegin] == '}') ? +1 : 0; } }
				size_t classBodyEnd = offset;
				{ int bracketCount = 1; while (bracketCount > 0) { classBodyEnd++; bracketCount += (str[classBodyEnd] == '}') ? -1 : (str[classBodyEnd] == '{') ? +1 : 0; } }
				header->m_ClassBodyBegin = classBodyBegin; header->m_ClassBodyEnd = classBodyEnd;

				// SUORA_CLASS  -> (...) <- 
				std::string classID = "";
				{ size_t iter = offset; while (str[iter] != ')' && str[iter] != ',') { iter++; if (str[iter] >= '0' && str[iter] <= '9') classID += str[iter]; } }
				header->m_ClassID = classID;

				// class/struct [ClassName] -> : <- private/protected/public [ParentClass]
				size_t colonOffset = classBodyBegin;
				while (str[colonOffset] != ':') colonOffset--;

				// ClassName
				std::string className = ""; std::string classDeclarator = "";
				{
					size_t iter = colonOffset;
					bool parsingName = true;
					while (classDeclarator != "class" && classDeclarator != "struct")
					{
						iter--;
						if (str[iter] != ' ')
						{
							if (parsingName) className.insert(0, 1, str[iter]);
							else classDeclarator.insert(0, 1, str[iter]);
						}
						else
						{
							if (parsingName && className.size() > 0) { parsingName = false; continue; }
							else if (!parsingName && classDeclarator.size() > 0) { break; }
						}
					}
				}
				header->m_ClassName = className;
				header->m_IsStruct = (classDeclarator == "struct");

				// ClassParent
				std::vector<std::string> classParentData; classParentData.push_back("");
				{
					size_t iter = colonOffset;
					while (iter < classBodyBegin)
					{
						iter++;
						if (str[iter] != ' ' && str[iter] != '\n' && str[iter] != '\t' && str[iter] != '{')
						{
							classParentData[classParentData.size() - 1] += str[iter];
						}
						else if (classParentData[classParentData.size() - 1].size() > 0)
						{
							classParentData.push_back("");
						}
					}
				}
				if (classParentData[0] != "private" && classParentData[0] != "protected" && classParentData[0] != "public")
					header->m_ParentClass = classParentData[0];
				else	header->m_ParentClass = classParentData[1];
			}
		}
	}

	// TODO: Remove
	void HeaderTool::ParseAllHeadersParentData()
	{
		for (Ref<Header> header : m_Headers)
		{
			header->m_ParentNames = header->GetAllParentNames();
			for (const std::string& parentName : header->m_ParentNames)
			{
				std::string id = GetHeaderByClassName(parentName)->m_ClassID;
				header->m_ParentIDs.push_back(id);
			}
		}
	}

	void HeaderTool::GenerateClassSymbols(HeaderTool::Header* header, std::map<HeaderOutput, std::string>* output)
	{
		std::string generated = "/* Auto Generated! */ \n\n";
		generated += "private:\n\n";

		generated += "using Super = " + header->m_ParentClass + ";\n";

		// Function Reflection
		{
			size_t offset = header->m_ClassBodyBegin;
			std::string helperFunctions, functionRegistors;
			while (true)
			{
				offset = header->str.find("FUNCTION", offset + 1);
				if (offset == std::string::npos || offset > header->m_ClassBodyEnd) break;
				std::string meta, func;
				while (header->str[offset] != '(') { offset++; }
				int bracketCount = 1;
				while (bracketCount > 0)
				{
					offset++;
					if (header->str[offset] != '\n' && header->str[offset] != '\t') meta += header->str[offset];
					bracketCount += (header->str[offset] == ')') ? -1 : (header->str[offset] == '(') ? +1 : 0;
				}
				while (header->str[offset] != ';' && header->str[offset] != '{')
				{
					offset++;
					if (header->str[offset] != '\n' && header->str[offset] != '\t') func += header->str[offset];
				}
				FunctionMeta functionMeta = GenerateFunctionReflection(header->m_ClassName, header->m_ClassID, meta, func);
				helperFunctions += functionMeta.m_Helper + "\n";
				functionRegistors += functionMeta.m_Registor + "\n";
			}
			generated +=
"struct InternalAlloc;\n\
struct Internal\n\
{\n\
	friend struct InternalAlloc;\n\
private:\n\
	Internal()\n\
	{\n\
	\tClass::GenerateNativeClassReflector(StaticClass());\n\
	}\n" + helperFunctions + "\n\n" + functionRegistors +
"};\n\
struct InternalAlloc\n\
{\n\
private:\n\
	InternalAlloc() {}\n\
	inline static ObjectFactory::RegisterSuoraClass<" + header->m_ClassName + "> _ClassAllocator_" + header->m_ClassName + ";\n\
	inline static Internal Internal_Instance = Internal();\n\
};\n";
		}
		
		generated += "\npublic:\n\n";

		// Static Class
		generated += "static Class StaticClass() { return Class(" + header->m_ClassID + "); }\n";
		// Dynamic Class
		generated += "virtual Class GetNativeClass() override { return Class(" + header->m_ClassID + "); }\n";

		// CastImpl
		generated += "virtual bool CastImpl(const Class& cls) const override \n{";
		generated += "\n\treturn cls.GetNativeClassID() == " + header->m_ClassID + " ? true : Super::CastImpl(cls);\n}\n";
		
		// Member Reflection
		{
			generated += "virtual void ReflClass(struct ClassReflector& desc) override \n{\n\t/*Super::ReflClass(desc);*/\n\tdesc.SetClassName(\"" + header->m_ClassName + "\");\n\tdesc.SetNativeParentClass(" + header->m_ParentClass + "::StaticClass());\n\tdesc.SetClassSize(sizeof(" + header->m_ClassName + "));\n";
			size_t offset = header->m_ClassBodyBegin;
			while (true)
			{
				offset = header->str.find("MEMBER", offset + 1);
				if (offset == std::string::npos || offset > header->m_ClassBodyEnd) break;
				std::string meta, member;
				while (header->str[offset] != '(') { offset++; }
				int bracketCount = 1; 
				while (bracketCount > 0) 
				{ 
					offset++; 
					if (header->str[offset] != '\n') meta += header->str[offset];
					bracketCount += (header->str[offset] == ')') ? -1 : (header->str[offset] == '(') ? +1 : 0; 
				}
				while (header->str[offset] != ';')
				{
					offset++;
					if (header->str[offset] != '\n' && header->str[offset] != '\t') member += header->str[offset];
				}
				GenerateMemberReflection(generated, meta, member);
			}
			generated += "}\n";
		}

		if (!header->m_IsStruct) generated += "\nprivate:";
		if (false) generated += "#error The Suora Header-Tool could not parse Objectheader <<<" + header->m_ClassID + ">>> (!)";

		std::string macro = generated;
		for (int i = macro.size() - 1; i >= 0; i--)
		{
			if (macro[i] == '\n') macro.insert(i, 1, '\\');
		}
		macro.insert(0, " \\\n");
		macro.insert(0, header->m_ClassID);
		macro.insert(0, "\n\n#define _GENERATED_BODY_");
		std::string generatedPath = header->m_HeaderPath;
		generatedPath.erase(generatedPath.size() - 2, 2);
		generatedPath.insert(generatedPath.size(), ".generated.h");
		HeaderOutput key;
		key.m_OriginalHeaderPath = header->m_HeaderPath;
		key.m_GeneratedHeaderPath = generatedPath;
		std::lock_guard<std::mutex> lock(GenerateClassMutex);
		(*output)[key] += macro;
	}

	void HeaderTool::GenerateTemplateInnerMember(std::string& str, std::string inner, const std::string& index)
	{
		if (inner == "float" || inner == "bool" || inner == "Class")
		{
			str += "\t\tRef<ClassMember> " + index + " = Ref<ClassMember>(new ClassMember(\"Inner\", 0, sizeof(" + inner + "), ClassMember::GetPrimitiveTypeEnumByTemplate<" + inner + ">()));\n";
		}
		else if (inner.find("ArrayList") == 0)
		{
			std::string inner_str = inner.substr(10, inner.size() - 11);
			GenerateTemplateInnerMember(str, inner_str, index + "_ArrayInner");
			str += "\t\tRef<ClassMember_ArrayList> " + index + " = Ref<ClassMember_ArrayList>(new ClassMember_ArrayList(\"InnerArray\", 0, sizeof(" + inner + "), ClassMember::Type::ArrayList));\n";
			str += "\t\t" + index + "->m_ArraySubMember = " + (index + "_ArrayInner") + ";\n";
		}
		else if (inner[inner.size() - 1] == '*')
		{
			std::string type = inner.substr(0, inner.size() - 1);
			if (DoesHeaderExist(type))
			{
				Header* header = GetHeaderByClassName(type);
				if (header->Inherits(GetHeaderByClassName("Asset")))
				{
					// Asset
					str += "\t\tRef<ClassMember_AssetPtr> " + index + " = Ref<ClassMember_AssetPtr>(new ClassMember_AssetPtr(\"Inner\", 0, sizeof(Asset*), ClassMember::Type::AssetPtr));\n";
					str += "\t\t" + index + "->m_AssetClass = Class(" + header->m_ClassID + ");\n";
				}
				else
				{
					// Object
					str += "\t\tRef<ClassMember> " + index + " = Ref<ClassMember>(new ClassMember(\"Inner\", 0, sizeof(Object*), ClassMember::Type::ObjectPtr));\n";
				}
			}
			else
			{
				str += "#error The Suora Header-Tool could not parse the InnerTemplateMember   [...] < " + inner + " > (!)\n";
			}
		}
		else
		{
			str += "#error The Suora Header-Tool could not parse the InnerTemplateMember   [...] < "+inner+" > (!)\n";
		}
		str += "\n";
	}

	void HeaderTool::GenerateMemberReflection(std::string& str, const std::string& meta, std::string member)
	{
		// Receive Member Info
		bool erase = false;
		for (size_t i = 0; i < member.size(); i += (erase) ? 0 : 1)
		{
			if (member[i] == '=' || member[i] == ';') erase = true;
			if (erase)
			{
				member.erase(i);
			}
		}
		std::string memberName, memberType;
		for (int64_t i = member.size() - 1; i >= 0; i--)
		{
			if ((member[i] == ' ' || member[i] == '*') && memberName.size() > 0)
			{
				memberType = member.substr(0, i + 1);
				for (int j = memberType.size() - 1; j >= 0; j--) { if (memberType[j] == ' ') { memberType.erase(j, 1); } }
				break;
			}
			else if (member[i] != ' ')
			{
				memberName.insert(0, 1, member[i]);
			}
		}

		// Out
		str += "\t/* " + member + "*/\n";
		str += "\t{\n";

		if (memberType == "int" || memberType == "float" || memberType == "bool" || memberType == "Vec3" || memberType == "Color" || memberType == "Class")
		{
			str += "\t\tdesc.AddPrimitive<" + memberType + ">(this, &" + memberName + ", \"" + memberName + "\");\n";
		}
		else if (memberType.substr(0, 11) == "SubclassOf<")
		{
			str += "\t\tdesc.AddMember<ClassMember>(\"" + memberName + "\", ClassMember::OffsetOf(this, &" + memberName + "), sizeof(" + memberType + "), ClassMember::Type::SubclassOf);\n";
		}
		else if (memberType == "MaterialSlots")
		{
			str += "\t\tdesc.AddMember<ClassMember>(\"" + memberName + "\", ClassMember::OffsetOf(this, &" + memberName + "), sizeof(" + memberType + "), ClassMember::Type::MaterialSlots);\n";
		}
		else if (memberType.substr(0, 8) == "Delegate")
		{
			str += "\t\tdesc.AddDelegate(this, " + memberName + ".GetInternalDelegate(), \"" + memberName + "\", \"" + memberType + "\");\n";
		}
		else if (memberType.find("ArrayList") == 0)
		{
			std::string inner = memberType.substr(10, memberType.size() - 11);
			GenerateTemplateInnerMember(str, inner, "_ArrayInner");
			str += "\t\tClassMember_ArrayList* array = desc.AddArrayList(this, &" + memberName + ", sizeof(" + memberName + "), \"" + memberName + "\");\n";
			str += "\t\tarray->m_ArraySubMember = _ArrayInner;\n";
		}
		else if (memberType[memberType.size() - 1] == '*') // if (memberType.find("*") != std::string::npos)
		{
			std::string type = memberType.substr(0, memberType.size() - 1);
			if (DoesHeaderExist(type))
			{
				Header* header = GetHeaderByClassName(type);
				if (header->Inherits(GetHeaderByClassName("Asset")))
				{
					// Asset
					str += "\t\tdesc.AddAssetPointer(this, (Asset**) &" + memberName + ", \"" + memberName + "\", " + header->m_ClassID + ");\n";
				}
				else
				{
					// Object
					str += "\t\tdesc.AddObjectPointer(this, (Object**) &" + memberName + ", \"" + memberName + "\");\n";
				}
			}
			else
			{
				str += "#error The Suora Header-Tool could not parse Member of Type   [ " + memberType + " ] (!)\n";
			}
			
		}
		else
		{
			str += "#error The Suora Header-Tool could not parse Member of Type   [ " + memberType + " ] (!)\n";
		}

		str += "\t}\n";
	}

	// FUNCTION(...meta...)
	// ... func ...
	FunctionMeta HeaderTool::GenerateFunctionReflection(const std::string& className, const std::string& classID, const std::string& meta, const std::string& func)
	{
		std::vector<std::string> Signature, Parameters, ParameterNames;
		Signature.push_back("");
		Parameters.push_back("");
		ParameterNames.push_back("");
		// Fetch Signature and Parameters from func
		{
			size_t offset = 0;
			while (func[offset] != '(')
			{
				if (func[offset] != ' ')
				{
					Signature[Signature.size() - 1] += func[offset];
				}
				else if (Signature[Signature.size() - 1].size() > 0)
				{
					Signature.push_back("");
				}
				offset++;
			}
			if (Signature[Signature.size() - 1].size() == 0)
			{
				Signature.erase(Signature.begin() + Signature.size() - 1);
			}
			///
			int bracketCount = 1;
			while (bracketCount > 0)
			{
				offset++;
				if (func[offset] != ',' && func[offset] != '=' && func[offset] != ')')
				{
					if (func[offset] == '\n' || func[offset] == '\t') Parameters[Parameters.size() - 1] += " ";
					else Parameters[Parameters.size() - 1] += func[offset];
				}
				else
				{
					if (func[offset] == '=')
					{
						// TODO: Fix: it could also be: ..... = ButtonParams(arg0, arg1, ...), ...
						while (func[++offset] != ',' && func[offset] != ')');
					}
					// At this line <offset> always refers to a comma or the ending bracket

					{
						bool removingParameterName = false;
						std::string& str = Parameters[Parameters.size() - 1];
						for (int i = str.size() - 1; i >= 0; i--)
						{
							if ((str[i] == ' ' || str[i] == '*' || str[i] == '&') && removingParameterName) break;
							else if (str[i] != ' ') removingParameterName = true;
							if (str[i] != ' ') ParameterNames[ParameterNames.size() - 1].insert(0, 1, str[i]);
							str.erase(i, 1);
						}

						while (str[0] == ' ') str.erase(0, 1);
						while (str.size() > 0 && str[str.size() - 1] == ' ') str.erase(str.size() - 1, 1);

						if (str.size() > 0 && (str[str.size() - 1] == '*' || str[str.size() - 1] == '&'))
						{
							for (int i = str.size() - 2; i >= 0; i--)
							{
								if (str[i] != ' ') break;
								str.erase(i, 1);
							}
						}
						// TODO: if last char is a '&' then check if it begins with "const", else fail the Header parse, as the ScriptRuntime cannot pass References (maybe?)
					}

					Parameters.push_back("");
					ParameterNames.push_back("");
				}
				bracketCount += (func[offset] == ')') ? -1 : (func[offset] == '(') ? +1 : 0;
			}
			// Ignore last Parameter, as it is empty
			Parameters.erase(Parameters.begin() + Parameters.size() - 1);
			ParameterNames.erase(ParameterNames.begin() + ParameterNames.size() - 1);
			///
		}

		const std::string functionName = Signature[Signature.size() - 1];
		const std::string returnType = Signature[Signature.size() - 2];
		bool isStatic = false; for (std::string& str : Signature) { if (str == "static") isStatic = true; }

		// FUNCTION(meta)
		std::string functionFlags = "FunctionFlags::None";
		const bool isCallable = meta.find("Callable") != std::string::npos;				if (isCallable)		functionFlags += " | FunctionFlags::Callable";
		const bool isPure = meta.find("Pure") != std::string::npos;						if (isPure)			functionFlags += " | FunctionFlags::Pure";
		const bool isNodeEvent = meta.find("NodeEvent") != std::string::npos;			if (isNodeEvent)	functionFlags += " | FunctionFlags::NodeEvent";
		const bool isNodeFunction = meta.find("NodeFunction") != std::string::npos;		if (isNodeFunction) functionFlags += " | FunctionFlags::NodeFunction";
																						if (isStatic)		functionFlags += " | FunctionFlags::Static";

		std::string label = className + "::" + functionName + "(";
		for (int i = 0; i < Parameters.size(); i++)
		{
			label += Parameters[i] + ((i == Parameters.size() - 1) ? ")" : ", ");
		}

		std::string registor_Params = "{ ";
		for (int i = 0; i < Parameters.size(); i++)
		{
			if (Parameters[i].size() > 0) 
				registor_Params += "FunctionParam(\"" + Parameters[i] + "\", \"" + ParameterNames[i] + "\")";
			registor_Params += ((i == Parameters.size() - 1) ? " }" : ", ");
		}
		const std::string registor = "inline static NativeFunction Function_" + functionName + " = NativeFunction(\"" + label + "\", &" + functionName + ", (NativeClassID)" + classID + ", " + registor_Params + ", \"" + returnType + "\", " + functionFlags + ");";

		std::string Function = "static void " + functionName + "(ScriptStack& stack)\n{\n";
		if (!isStatic) Function += className + "* self = (" + className + "*) stack.Pop();\n";

		if (Parameters[0] != "")
		{
			for (int i = Parameters.size() - 1; i >= 0; i--)
			{
				Function += "const auto arg" + std::to_string(i) + " = (" + Parameters[i] + ") ScriptStack::ConvertFromStack<" + ((Parameters[i][Parameters[i].size() - 1] == '*') ? "Object*" : Parameters[i]) + ">(stack.Pop());\n";
			}
		}

		if (returnType != "void") Function += "const auto ret = ";
		if (!isStatic) Function += "self->" + functionName + "(";
		else Function += className + "::" + functionName + "(";
		
		if (Parameters[0] != "")
		{
			for (int i = 0; i < Parameters.size(); i++)
			{
				Function += "arg" + std::to_string(i) + (i < Parameters.size() - 1 ? ", " : "");
			}
		}
		Function += ");\n";
		if (returnType != "void") Function += "stack.Push(ScriptStack::ConvertToStack<" + ((returnType[returnType.size() - 1] == '*') ? "Object*" : returnType) + ">("+ ((returnType[returnType.size() - 1] == '*') ? "(Object*)" : "") +"ret));\n";

		Function += "\n}";

		/*static void SetParent(ScriptStack& stack) { ... }
		inline static NativeFunction Function_SetParent = NativeFunction("Transform::SetParent(Transform*)", &SetParent);*/

		return FunctionMeta(Function, registor);
	}

	void HeaderTool::AddHeader(Ref<Header> ref)
	{
		std::lock_guard<std::mutex> lock(AccessHeaderMutex);
		m_Headers.push_back(ref);
	}

	HeaderTool::Header* HeaderTool::GetHeaderByClassName(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(AccessHeaderMutex);
		for (auto& header : m_Headers)
		{
			if (header->m_ClassName == name) return header.get();
		}
		return nullptr;
	}

	bool HeaderTool::DoesHeaderExist(const std::string& name)
	{
		return GetHeaderByClassName(name);
	}

	std::vector<std::string> HeaderTool::Header::GetAllParentNames()
	{
		if (this == nullptr) return std::vector<std::string>(); // For "None" (Parent of Object)

		std::vector<std::string> parents = m_HeaderTool->GetHeaderByClassName(m_ParentClass)->GetAllParentNames();
		if (m_ParentClass != "None") parents.push_back(m_ParentClass);

		return parents;
	}
	bool Suora::Tools::HeaderTool::Header::Inherits(Header* other)
	{
		std::lock_guard<std::mutex> lock(AccessHeaderMutex);
		for (std::string& parent : m_ParentNames)
		{
			if (parent == other->m_ClassName) return true;
		}

		return false;
	}
}
