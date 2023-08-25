#include "Precompiled.h"
#include "ScriptVM.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/Core/Object/Object.h"
#include "Suora/Common/VectorUtils.h"

namespace Suora
{

	static void Multiply_Vec3_Float(ScriptStack& stack)
	{
		float f = ScriptStack::ConvertFromStack<float>(stack.Pop());
		Vec3 vec = ScriptStack::ConvertFromStack<Vec3>(stack.Pop());

		Vec3 result = vec * f;
		stack.Proccess<Vec3>(result);
	}

	ScriptFunction::ScriptFunction()
		: m_Hash(0)
	{
	}

	void ScriptFunction::Call(Object* obj, ScriptStack& stack)
	{
		std::vector<int64_t> LocalVars = std::vector<int64_t>(m_LocalVarCount);
		
		// Switch-Statement PreAllocations
		static Object* CallScriptFunction_Target = nullptr;
		static int64_t CallScriptFunction_TargetScriptClass = 0;
		static int64_t CallScriptFunction_TargetScriptFunc = 0;

		for (int i = 0; i < m_Instructions.size(); i++)
		{
			ScriptInstruction& instruction = m_Instructions[i];

			switch (instruction.m_Instruction)
			{
			case EScriptInstruction::IfNotThenJump:
				if (instruction.m_Args[0] == 0) i = instruction.m_Args[1];
				break;

			case EScriptInstruction::CallNativeFunction:
				NativeFunctionManager::Call(instruction.m_Args[0], stack);
				break;

			case EScriptInstruction::CallScriptFunction:
				SuoraError("ScriptInstruction::CallScriptFunction not implemented!");
				CallScriptFunction_Target = (Object*)stack.Pop();
				CallScriptFunction_TargetScriptClass = instruction.m_Args[0];
				CallScriptFunction_TargetScriptFunc = instruction.m_Args[1];
				break;

			case EScriptInstruction::PushConstant:
				stack.Push(instruction.m_Args[0]);
				break;
			case EScriptInstruction::PushSelf:
				stack.Push((int64_t)obj);
				break;
			case EScriptInstruction::PushToLocalVar:
				LocalVars[instruction.m_Args[0]] = stack.Pop();
				break;
			case EScriptInstruction::PushLocalVar:
				stack.Push(LocalVars[instruction.m_Args[0]]);
				break;
			case EScriptInstruction::Pop:
				stack.Pop();
				break;
			case EScriptInstruction::UpStack:
				stack.Push(stack.Peek());
				break;

			case EScriptInstruction::Multiply_Vec3_Float:
				Multiply_Vec3_Float(stack);
				break;

			case EScriptInstruction::None:
			default:
				SuoraError("Unkown Instruction!");
				break;
			}
		}
	}

	void ScriptFunction::Serialize(Yaml::Node& root)
	{
		root["Hash"] = std::to_string(m_Hash);
		root["LocalVarCount"] = std::to_string(m_LocalVarCount);
		root["IsEvent"] = m_IsEvent ? "true" : "false";

		Yaml::Node& inst = root["Instructions"];
		for (int i = 0; i < m_Instructions.size(); i++)
		{
			Yaml::Node& it = inst[std::to_string(i)];
			it["Instruction"] = std::to_string((uint32_t)m_Instructions[i].m_Instruction);
			it["Args"]["0"] = std::to_string(m_Instructions[i].m_Args[0]);
			it["Args"]["1"] = std::to_string(m_Instructions[i].m_Args[1]);
			it["Args"]["2"] = std::to_string(m_Instructions[i].m_Args[2]);
			it["Args"]["3"] = std::to_string(m_Instructions[i].m_Args[3]);
		}
	}

	ScriptFunction ScriptFunction::Deserialize(Yaml::Node& root)
	{
		ScriptFunction func;

		func.m_Hash = root["Hash"].As<size_t>();
		func.m_LocalVarCount = root["LocalVarCount"].As<uint32_t>();
		func.m_IsEvent = root["IsEvent"].As<std::string>() == "true";

		Yaml::Node& inst = root["Instructions"];
		int i = 0;
		while (true)
		{
			Yaml::Node& it = inst[std::to_string(i++)];
			if (it.IsNone()) break;
			ScriptInstruction Instruction;
			Instruction.m_Instruction = (EScriptInstruction) it["Instruction"].As<uint32_t>();
			Instruction.m_Args[0] = it["Args"]["0"].As<int64_t>();
			Instruction.m_Args[1] = it["Args"]["1"].As<int64_t>();
			Instruction.m_Args[2] = it["Args"]["2"].As<int64_t>();
			Instruction.m_Args[3] = it["Args"]["3"].As<int64_t>();
			func.m_Instructions.push_back(Instruction);
		}

		return func;
	}

	ScriptVar::ScriptVar()
		: m_Type(ScriptDataType::None)
	{
	}

	void ScriptEngine::CleanUp()
	{
		for (void* cache : m_ScriptCache)
		{
			delete cache;
		}
		m_ScriptCache.clear();
	}

	void ScriptClassInternal::Serialize(Yaml::Node& root)
	{
		Yaml::Node& script = root["ScriptClass"];

		script["ClassName"] = m_ClassName;
		Yaml::Node& functions = script["Functions"];

		for (int i = 0; i < m_Functions.size(); i++)
		{
			m_Functions[i].Serialize(functions[std::to_string(i)]);
		}
	}

	void ScriptClassInternal::Deserialize(Yaml::Node& root)
	{
		Yaml::Node& script = root["ScriptClass"];

		m_ClassName = script["ClassName"].As<std::string>();
		Yaml::Node& functions = script["Functions"];

		int i = 0;
		while (true)
		{
			Yaml::Node& it = functions[std::to_string(i++)];
			if (it.IsNone()) break;
			m_Functions.push_back(ScriptFunction::Deserialize(it));
		}
	}

}