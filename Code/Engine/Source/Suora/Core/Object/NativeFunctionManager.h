#pragma once
#include <unordered_map>
#include <functional>
#include <string>
#include "Suora/Core/Log.h"

namespace Suora
{
	struct ScriptStack;
	typedef void (*NativeFunctionPtr) (ScriptStack&);

	/** Used internally by all SuoraClasses to register reflected Funtions. Use FUNTION()-macro then use SuoraBuildTool */
	class NativeFunctionManager
	{
		inline static std::unordered_map<size_t, NativeFunctionPtr> s_NativeFunctions;
	public:
		NativeFunctionManager() = delete;
		NativeFunctionManager(NativeFunctionManager&&) = delete;
		NativeFunctionManager(const NativeFunctionManager&) = delete;

		static void RegisterNativeFunction(size_t hash, const NativeFunctionPtr& func);
		static void UnregisterNativeFunction(size_t hash);
		static void Call(size_t hash, ScriptStack& stack);
	};

	struct FunctionParam
	{
		String m_Type;
		String m_Name;
		FunctionParam(const String& type, const String& name)
			: m_Type(type), m_Name(name)
		{
		}
	};

	enum class FunctionFlags : int32_t
	{
		None = 0,
		Callable = 1,
		Pure = 2,
		NodeEvent = 4,
		NodeFunction = 8,
		Static = 16
	};

	inline FunctionFlags operator|(FunctionFlags a, FunctionFlags b)
	{
		return static_cast<FunctionFlags>(static_cast<int32_t>(a) | static_cast<int32_t>(b));
	}

	struct NativeFunction
	{
		NativeFunction(const String& label, const NativeFunctionPtr& func, uint64_t id, const std::vector<FunctionParam>& params, const String& returnType, FunctionFlags flags = FunctionFlags::None);
		~NativeFunction();
		bool IsFlagSet(FunctionFlags flag) const;

		inline static std::vector<NativeFunction*> s_NativeFunctions;
	//private:
		String m_Label;
		size_t m_Hash;
		std::vector<FunctionParam> m_Params;
		String m_ReturnType = "void";
		uint64_t m_ClassID;
		FunctionFlags m_Flags;
	};

	struct NativeFunctionHashCheck
	{
		NativeFunctionHashCheck(const String& str)
		{
			size_t hash = std::hash<String>{}(str);
			for (auto& It : NativeFunction::s_NativeFunctions)
			{
				if (It->m_Hash == hash)
				{
					return;
				}
			}

			SuoraError("NativeFunctionHashCheck failed. You are using NODESCRIPT_EVENT_DISPATCH with an unregistered Hash: \"{0}\"", str);
			SUORA_ASSERT(false);
		}
	};

}
