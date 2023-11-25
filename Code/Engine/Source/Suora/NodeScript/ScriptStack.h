#pragma once

#include <vector>
#include "Suora/Common/StringUtils.h"
#include <stack>
#include <inttypes.h>
#include "Suora/Common/VectorUtils.h"

namespace Suora
{
	class Object;
	struct DelegateNoParams;

	struct ScriptStack
	{
		std::stack<int64_t> m_Stack;
		inline void Push(int64_t n)
		{
			m_Stack.push(n);
		}
		inline int64_t& Peek()
		{
			return m_Stack.top();
		}
		inline int64_t Pop()
		{
			int64_t n = m_Stack.top();
			m_Stack.pop();
			return n;
		}
		void Invert();

		template<class T>
		void Proccess(T value)
		{
				 if constexpr (std::is_same_v<DelegateNoParams const &, T>) { /*Does nothing!*/ }
			else if constexpr (std::is_same_v<T, int64_t>) { m_Stack.push(ConvertToStack<int64_t>(value)); }
			else if constexpr (std::is_same_v<T, int32_t>) { m_Stack.push(ConvertToStack<int32_t>(value)); }
			else if constexpr (std::is_same_v<T, uint32_t>) { m_Stack.push(ConvertToStack<uint32_t>(value)); }
			else if constexpr (std::is_same_v<T, bool>) { m_Stack.push(ConvertToStack<bool>(value)); }
			else if constexpr (std::is_same_v<T, Object*>) { m_Stack.push(ConvertToStack<Object*>(value)); }
			else if constexpr (std::is_same_v<T, float>) { m_Stack.push(ConvertToStack<float>(value)); }
			else if constexpr (std::is_same_v<T, Vec3>) { m_Stack.push(ConvertToStack<Vec3>(value)); }
			else if constexpr (std::is_same_v<T, const Vec3&>) { m_Stack.push(ConvertToStack<const Vec3&>(value)); }
			else if constexpr (std::is_same_v<T, Quat>) { m_Stack.push(ConvertToStack<Quat>(value)); }
			else if constexpr (std::is_base_of_v<Object, std::remove_pointer_t<T>>) { m_Stack.push(ConvertToStack<Object*>(value)); }
			else if constexpr (std::is_convertible_v<T, Object*>) { m_Stack.push(ConvertToStack<Object*>(value)); }
			else { SuoraVerify(false, "This Implementation should not have been called!"); }
		}
		void Proccess()
		{
		}

		template<class T>
		static int64_t ConvertToStack(T value);

		template<class T>
		static T ConvertFromStack(int64_t value);
	};
}