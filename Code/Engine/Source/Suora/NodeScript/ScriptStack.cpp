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
	ScriptDataType ScriptStack::GetTypeFromTemplate<int64_t>()
	{
		return ScriptDataType::Int64;
	}

	template<>
	int64_t ScriptStack::ConvertToStack<int32_t>(int32_t value)
	{
		return value;
	}
	template<>
	ScriptDataType ScriptStack::GetTypeFromTemplate<int32_t>()
	{
		return ScriptDataType::Int32;
	}

	template<>
	int64_t ScriptStack::ConvertToStack<uint32_t>(uint32_t value)
	{
		return value;
	}
	template<>
	ScriptDataType ScriptStack::GetTypeFromTemplate<uint32_t>()
	{
		return ScriptDataType::UInt32;
	}

	template<>
	int64_t ScriptStack::ConvertToStack<bool>(bool value)
	{
		return value;
	}
	template<>
	ScriptDataType ScriptStack::GetTypeFromTemplate<bool>()
	{
		return ScriptDataType::Bool;
	}

	template<>
	int64_t ScriptStack::ConvertToStack<Object*>(Object* value)
	{
		return (int64_t)value;
	}
	template<>
	ScriptDataType ScriptStack::GetTypeFromTemplate<Object*>()
	{
		return ScriptDataType::ObjectPtr;
	}

	template<>
	int64_t ScriptStack::ConvertToStack<float>(float value)
	{
		float temp = value;
		return *(int64_t*)(float*)&temp;
	}
	template<>
	ScriptDataType ScriptStack::GetTypeFromTemplate<float>()
	{
		return ScriptDataType::Float;
	}

	template<>
	int64_t ScriptStack::ConvertToStack<Vec2>(Vec2 value)
	{
		Vec2* heap = new Vec2();
		BlueprintScriptEngine::m_ScriptCache.push_back({ heap, ScriptDataType::Vec2 });
		*heap = value;
		return (int64_t)heap;
	}
	template<>
	ScriptDataType ScriptStack::GetTypeFromTemplate<Vec2>()
	{
		return ScriptDataType::Vec2;
	}

	template<>
	int64_t ScriptStack::ConvertToStack<Vec3>(Vec3 value)
	{
		Vec3* heap = new Vec3();
		BlueprintScriptEngine::m_ScriptCache.push_back({ heap, ScriptDataType::Vec3 });
		*heap = value;
		return (int64_t)heap;
	}
	template<>
	ScriptDataType ScriptStack::GetTypeFromTemplate<Vec3>()
	{
		return ScriptDataType::Vec3;
	}

	template<>
	int64_t ScriptStack::ConvertToStack<const Vec3&>(const Vec3& value)
	{
		Vec3* heap = new Vec3();
		BlueprintScriptEngine::m_ScriptCache.push_back({ heap, ScriptDataType::Vec3 });
		*heap = value;
		return (int64_t)heap;
	}
	template<>
	ScriptDataType ScriptStack::GetTypeFromTemplate<const Vec3&>()
	{
		return ScriptDataType::Vec3;
	}

	template<>
	int64_t ScriptStack::ConvertToStack<Quat>(Quat value)
	{
		Quat* heap = new Quat();
		BlueprintScriptEngine::m_ScriptCache.push_back({ heap, ScriptDataType::Quat});
		*heap = value;
		return (int64_t)heap;
	}
	template<>
	ScriptDataType ScriptStack::GetTypeFromTemplate<Quat>()
	{
		return ScriptDataType::Quat;
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


	void ScriptStack::Push(int64_t value, ScriptDataType type)
	{
		m_Stack.push(value);
		m_TypeStack.push(type);
	}
	int64_t& ScriptStack::Peek()
	{
		return m_Stack.top();
	}
	ScriptDataType& ScriptStack::PeekType()
	{
		return m_TypeStack.top();
	}
	int64_t ScriptStack::Pop()
	{
		int64_t n = m_Stack.top();
		m_Stack.pop();
		m_TypeStack.pop();
		return n;
	}
	void ScriptStack::Invert()
	{
		{
			std::stack<int64_t> temp;
			while (!m_Stack.empty())
			{
				temp.push(m_Stack.top());
				m_Stack.pop();
			}

			m_Stack = temp;
		}
		{
			std::stack<ScriptDataType> temp;
			while (!m_TypeStack.empty())
			{
				temp.push(m_TypeStack.top());
				m_TypeStack.pop();
			}

			m_TypeStack = temp;
		}
	}
}