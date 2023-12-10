#include "Precompiled.h"
#include "NativeFunctionManager.h"

namespace Suora
{

	void NativeFunctionManager::RegisterNativeFunction(size_t hash, const NativeFunctionPtr& func)
	{
		s_NativeFunctions[hash] = func;
	}
	void NativeFunctionManager::UnregisterNativeFunction(size_t hash)
	{
		s_NativeFunctions.erase(hash);
	}
	void NativeFunctionManager::Call(size_t hash, ScriptStack& stack)
	{
		SUORA_ASSERT(s_NativeFunctions.find(hash) != s_NativeFunctions.end(), "Cannot find NativeFunction with given Hash.");
		s_NativeFunctions[hash](stack);
	}

	NativeFunction::NativeFunction(const String& label, const NativeFunctionPtr& func, uint64_t id, const std::vector<FunctionParam>& params, const String& returnType, FunctionFlags flags)
	{
		m_Hash = std::hash<String>{}(label);
		m_Label = label;
		m_ClassID = id;
		m_Flags = flags;
		m_Params = params;
		m_ReturnType = returnType;
		NativeFunctionManager::RegisterNativeFunction(m_Hash, func);
		s_NativeFunctions.push_back(this);
	}
	NativeFunction::~NativeFunction()
	{
		NativeFunctionManager::UnregisterNativeFunction(m_Hash);
		s_NativeFunctions.erase(std::find(s_NativeFunctions.begin(), s_NativeFunctions.end(), this));
	}
	bool NativeFunction::IsFlagSet(FunctionFlags flag) const
	{
		return 0 != ((int32_t)m_Flags & (int32_t)flag);
	}

}
