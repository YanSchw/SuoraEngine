#include "Precompiled.h"
#include "ScriptStack.h"
#include "Suora/Common/VectorUtils.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Suora/Common/Delegate.h"

namespace Suora
{
	class Object;

	template<>
	int64_t ScriptStack::ConvertToStack<int64_t>(int64_t value)
	{
		return value;
	}
	template<>
	int64_t ScriptStack::ConvertToStack<int32_t>(int32_t value)
	{
		return value;
	}
	template<>
	int64_t ScriptStack::ConvertToStack<uint32_t>(uint32_t value)
	{
		return value;
	}
	template<>
	int64_t ScriptStack::ConvertToStack<bool>(bool value)
	{
		return value;
	}
	template<>
	int64_t ScriptStack::ConvertToStack<Object*>(Object* value)
	{
		return (int64_t)value;
	}
	template<>
	int64_t ScriptStack::ConvertToStack<float>(float value)
	{
		float temp = value;
		return *(int64_t*)(float*)&temp;
	}
	template<>
	int64_t ScriptStack::ConvertToStack<Vec2>(Vec2 value)
	{
		Vec2* heap = new Vec2();
		ScriptEngine::m_ScriptCache.push_back(heap);
		*heap = value;
		return (int64_t)heap;
	}
	template<>
	int64_t ScriptStack::ConvertToStack<Vec3>(Vec3 value)
	{
		Vec3* heap = new Vec3();
		ScriptEngine::m_ScriptCache.push_back(heap);
		*heap = value;
		return (int64_t)heap;
	}
	template<>
	int64_t ScriptStack::ConvertToStack<const Vec3&>(const Vec3& value)
	{
		Vec3* heap = new Vec3();
		ScriptEngine::m_ScriptCache.push_back(heap);
		*heap = value;
		return (int64_t)heap;
	}
	template<>
	int64_t ScriptStack::ConvertToStack<Quat>(Quat value)
	{
		Quat* heap = new Quat();
		ScriptEngine::m_ScriptCache.push_back(heap);
		*heap = value;
		return (int64_t)heap;
	}

	/**/

	template<>
	int64_t ScriptStack::ConvertFromStack<int64_t>(int64_t value)
	{
		return value;
	}
	template<>
	int32_t ScriptStack::ConvertFromStack<int32_t>(int64_t value)
	{
		return value;
	}
	template<>
	uint32_t ScriptStack::ConvertFromStack<uint32_t>(int64_t value)
	{
		return value;
	}
	template<>
	bool ScriptStack::ConvertFromStack<bool>(int64_t value)
	{
		return value;
	}
	template<>
	Object* ScriptStack::ConvertFromStack<Object*>(int64_t value)
	{
		return (Object*)value;
	}
	template<>
	Object& ScriptStack::ConvertFromStack<Object&>(int64_t value)
	{
		return *(Object*)value;
	}
	template<>
	float ScriptStack::ConvertFromStack<float>(int64_t value)
	{
		int64_t temp = value;
		return *(float*)(int64_t*)&temp;
	}
	template<>
	Vec3 ScriptStack::ConvertFromStack<Vec3>(int64_t value)
	{
		return *(Vec3*)value;
	}
	template<>
	const Vec3& ScriptStack::ConvertFromStack<const Vec3&>(int64_t value)
	{
		return *(Vec3*)value;
	}
	template<>
	Quat ScriptStack::ConvertFromStack<Quat>(int64_t value)
	{
		return *(Quat*)value;
	}


	void ScriptStack::Invert()
	{
		std::stack<int64_t> temp;
		while (!m_Stack.empty())
		{
			temp.push(m_Stack.top());
			m_Stack.pop();
		}

		m_Stack = temp;
	}
}