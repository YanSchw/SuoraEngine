#pragma once
#include <vector>
#include <string>
#include <stack>
#include "inttypes.h"
#include "Suora/NodeScript/ScriptStack.h"
#include "Suora/NodeScript/ScriptTypes.h"

namespace Yaml { class Node; }

namespace Suora
{
	class Object;

	struct ScriptVar
	{
		String m_VarName = "Var";
		ScriptDataType m_Type;
		int64_t m_Value = 0;
		String m_VarParams = "";
		
		ScriptVar();
	};

	enum class EScriptInstruction : uint32_t
	{
		None = 0,
		IfNotThenJump,
		CallNativeFunction,
		CallScriptFunction,
		PushConstant,
		PushSelf,
		PushToLocalVar,
		PushLocalVar,
		Pop,
		UpStack,

		Multiply_Vec3_Float
	};

	struct ScriptInstruction
	{
		EScriptInstruction m_Instruction = EScriptInstruction::None;
		int64_t m_Args[4] = {0};
		ScriptInstruction() {  }
		ScriptInstruction(EScriptInstruction inst) : m_Instruction(inst) {  }
		ScriptInstruction(EScriptInstruction inst, const std::vector<int64_t>& args) : m_Instruction(inst) 
		{ 
			for (int i = 0; i < args.size() && i < 4; i++)
				m_Args[i] = args[i]; 
		}
	};

	struct ScriptFunction
	{
		std::vector<ScriptInstruction> m_Instructions;
		size_t m_Hash;
		uint32_t m_LocalVarCount = 0;
		bool m_IsEvent = false;

		ScriptFunction();
		void Call(Object* obj, ScriptStack& stack);

		void Serialize(Yaml::Node& root);
		static ScriptFunction Deserialize(Yaml::Node& root);
	};

	struct ScriptClassInternal
	{
		String m_ClassName;
		std::vector<ScriptFunction> m_Functions;
		std::vector<ScriptVar> m_ScriptVars = std::vector<ScriptVar>();

		void Serialize(Yaml::Node& root);
		void Deserialize(Yaml::Node& root);
	};


	struct ScriptGarbage
	{
		void* Data = nullptr;
		ScriptDataType Type = ScriptDataType::None;
	};
	struct ScriptEngine
	{
		inline static std::vector<ScriptGarbage> m_ScriptCache;

		static void CleanUp();
	};

}