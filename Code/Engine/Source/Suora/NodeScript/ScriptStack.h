#pragma once

#include <vector>
#include <stack>
#include <inttypes.h>
#include "Suora/Core/Base.h"
#include "Suora/Common/StringUtils.h"
#include "Suora/Common/Array.h"
#include "Suora/Common/VectorUtils.h"

namespace Suora
{
	class Object;
	struct DelegateNoParams;
	enum class ScriptDataType : int64_t;

	struct ScriptStack
	{
		void Push(int64_t value, ScriptDataType type);
		int64_t& Peek();
		ScriptDataType& PeekType();
		int64_t Pop();
		void Invert();

		bool IsEmpty() const;

		template<class T>
		void Proccess(T value)
		{
				 if constexpr (std::is_same_v<DelegateNoParams const &, T>)         { /*Does nothing!*/ }
			else if constexpr (std::is_same_v<T, int64_t>)                          { PushItem<int64_t>(value); }
			else if constexpr (std::is_same_v<T, int32_t>)                          { PushItem<int32_t>(value); }
			else if constexpr (std::is_same_v<T, uint32_t>)                         { PushItem<uint32_t>(value); }
			else if constexpr (std::is_same_v<T, bool>)                             { PushItem<bool>(value); }
			else if constexpr (std::is_same_v<T, Object*>)                          { PushItem<Object*>(value); }
			else if constexpr (std::is_same_v<T, float>)                            { PushItem<float>(value); }
			else if constexpr (std::is_same_v<T, Vec3>)                             { PushItem<Vec3>(value); }
			else if constexpr (std::is_same_v<T, const Vec3&>)                      { PushItem<const Vec3&>(value); }
			else if constexpr (std::is_same_v<T, Quat>)                             { PushItem<Quat>(value); }
			else if constexpr (std::is_base_of_v<Object, std::remove_pointer_t<T>>) { PushItem<Object*>(value); }
			else if constexpr (std::is_convertible_v<T, Object*>)                   { PushItem<Object*>(value); }
			else { SuoraVerify(false, "This Implementation should not have been called!"); }
		}
		void Proccess()
		{
		}

		template<class T>
		void PushItem(T item)
		{
			Push(ConvertToStack<T>(item), GetTypeFromTemplate<T>());
		}
		template<class T>
		T PopItem()
		{
			return ConvertFromStack<T>(Pop());
		}

		template<class T>
		static int64_t ConvertToStack(T value);

		template<class T>
		static T ConvertFromStack(int64_t value);

		template<class T>
		static ScriptDataType GetTypeFromTemplate();

	private:
		std::stack<int64_t> m_Stack;
		std::stack<ScriptDataType> m_TypeStack;
	};
}