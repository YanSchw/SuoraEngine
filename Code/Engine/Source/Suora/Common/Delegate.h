#pragma once

#include <algorithm>
#include <vector>
#include <functional>
#include "Suora/Common/Array.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/Core/Object/Pointer.h"
#include "Suora/NodeScript/ScriptStack.h"

namespace Suora
{
	enum class ScriptDataType : int64_t;
	class Node;

	struct DelegateNoParams
	{
	private:
		uint8_t _padding;
	};

	struct TDelegate
	{
		struct SciptDelegateBinding
		{
			Ptr<Node> NodeBinding;
			size_t ScriptFunctionHash;
			SciptDelegateBinding(Node* node, size_t funcHash)
				: NodeBinding(node), ScriptFunctionHash(funcHash)
			{
			}
		};

		void DispatchToScriptEngine(Node* node, ScriptStack& stack, size_t hash);

		Array<SciptDelegateBinding> Bindings;

		// Use this instead of 'void' when declaring a Delegate that does not use parameters. -> Delegate<TDelegate::NoParams> m_MyDelegate
		/*struct NoParams : DelegateNoParams
		{
		};*/
		static constexpr DelegateNoParams NoParam = DelegateNoParams();
	};

	template<class ... Args>
	class Delegate
	{
	public:
		Delegate()
		{
		}
		/*std::function<void(Args...)> Func;*/

		/*Delegate(void(*Function)(Args...))
		{
			Func = std::bind(Function, Args...);
		}*/

		/*void operator()(Args... args)
		{
			Func(args);
		}*/
	private:
		TDelegate m_Delegate;
		std::vector<std::function<void(Args...)>> Functions;

		/*void Bind(void* Object, void(*Function)(Args... args))
		{
			Register([Object](auto&& ...args) { return Function(args...); });
		}*/

	public:
		// Bind Member-Functions
		template<typename R, typename T, typename U, typename... Args>
		void Attach(R(T::* f)(Args...), U p)
		{
			std::function<R(Args...)> function = [p, f](Args... args)->R { return (p->*f)(args...); };
			Register(function);
		};

		template<class T>
		void Bind(T* Object, void(T::* Function)(Args...))
		{
			//auto x = sizeof...(Args);
			//std::cout << x << std::endl;
			auto f = std::bind(Function, Object, std::make_integer_sequence< sizeof...(Args) >{});
			Register(f);
		}

		void Register(std::function<void(Args...)> Callback)
		{
			Functions.push_back(Callback);
		}

		void UnbindAll()
		{
			Functions.clear();
		}

		// For the ScriptStack
		template<class T>
		void AddArg(ScriptStack& stack, const T& arg)
		{
			stack.Proccess<T>(arg);
		}

		template<class T, class ... Ts>
		void FeedArgs(ScriptStack& stack, const T& arg, const Ts&... args)
		{
			AddArg<T>(stack, arg);
			if constexpr (sizeof...(Ts) > 0)
			{
				// this line will only be instantiated if there are further
				// arguments. if pack... is empty, there will be no call to
				FeedArgs<Ts...>(stack, args...);
			}
		}


		template <class... Args>
		void Execute(Args&&... args)
		{
			for (auto i = 0; i < Functions.size(); i++)
			{
				Functions[i](std::forward<Args>(args)...);
			}
			for (int i = m_Delegate.Bindings.Last(); i >= 0; i--)
			{
				if (m_Delegate.Bindings[i].NodeBinding)
				{
					ScriptStack stack;
					FeedArgs<Args...>(stack, args...);
					stack.Invert();
					m_Delegate.DispatchToScriptEngine(m_Delegate.Bindings[i].NodeBinding, stack, m_Delegate.Bindings[i].ScriptFunctionHash);
				}
				else
				{
					m_Delegate.Bindings.RemoveAt(i);
				}
			}
		}
		template <class... Args>
		void operator()(Args&&... args)
		{
			Execute(std::forward<Args>(args)...);
		}

		bool IsBound() const
		{
			return Functions.size() > 0;
		}

		TDelegate* GetInternalDelegate()
		{
			return &m_Delegate;
		}

		friend class Blueprint;
		friend struct ClassReflector;
	};

}