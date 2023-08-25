#pragma once

#include <iostream>
#include <typeinfo>
#include <inttypes.h>
#include <vector>
#include <string>

#include "Suora/Core/Object/NativeFunctionManager.h"
#include "Suora/Core/Object/ObjectFactory.h"
#include "Suora/Core/Object/Pointer.h"
#include "Suora/Reflection/Class.h"
#include "Suora/Reflection/ClassReflector.h"
#include "Suora/NodeScript/ScriptStack.h"

namespace Suora
{
	struct ScriptStack;
	class INodeScriptObject;

	/** Base Class for all SuoraClasses */
	class Object
	{
	private:
		std::vector<Ref<Object>> m_Interfaces;
	public:
		Object();
		virtual ~Object();

		static Class StaticClass() { return (Class) 1; }
		virtual Class GetNativeClass() { return (Class) 1; };
		Class GetClass();
		
		virtual bool CastImpl(const Class& cls) const
		{
			return (cls.GetNativeClassID() == 1);
		}
		virtual void ReflClass(struct ClassReflector& desc);

		// Interfaces
		void Implement(const Class& cls);
	private:
		void ImplementUsingRootObject(const Class& cls, Object* root);
	public:
		void Unimplement(const Class& cls);
		void UnimplementAllInterfaces();
		bool Implements(const Class& cls) const;
		Object* GetInterface(const Class& cls);
		template<class T>
		void Implement()
		{
			Implement(T::StaticClass());
		}
		template<class T>
		void Unimplement()
		{
			Unimplement(T::StaticClass());
		}
		template<class T>
		bool Implements() const
		{
			return Implements(T::StaticClass());
		}
		template<class T>
		T* GetInterface()
		{
			return (T*) GetInterface(T::StaticClass());
		}

		bool IsA(const Class& cls) const;
		template<class T>
		bool IsA() const
		{
			return IsA(T::StaticClass());
		}
		template<class T>
		T* As() const
		{
			return Cast<T>(this);
		}

		void __NodeEventDispatch(size_t hash, ScriptStack& stack);
		void __NodeEventDispatch(size_t hash);

	};
}

/////////////////////////////////////////////////
/////////////  TYPE REFLECTION  /////////////////
/////////////////////////////////////////////////

#define GENERATED_BODY(ID) _GENERATED_BODY_##ID

#define SUORA_CLASS(ID) GENERATED_BODY(ID)
//#define SuoraClass(ID) SUORA_CLASS(ID)

#define ENUM(ID) _ENUM_BODY_##ID enum class

#define FUNCTION(...)
enum
{
	Callable,

	Pure,

	// With 'Return' you can specify how the result of the function is named in Nodes. The default is -> Return = "Result" 
	Return,

	// With 'NodeEvent' you can implement this function as an Event in the NodeGraph. Use the NODEEVENT_DISPATCH(); macro in the functions native implementation
	NodeEvent,

	// With 'NodeFunction' you can override this function as a Function in the NodeGraph. Use the NODEFUNCTION_RETURN(); macro in the functions native implementation
	NodeFunction,

	// TypeAccessors auto connect this Function with ScriptNodes that do operations on the returned type.
	//    - TypeAccessors only work with ObjectPointers
	//    - There may not be multiple Functions marked as 'TypeAccessor' that return an ObjectPointer of the same Class
	//    - The returned ObjectClass must differ from the Instace ObjectClass
	TypeAccessor
};
#define NODEEVENT_DISPATCH() NodeEvent::Dispatch(this, __FUNCTION__)
#define NODEFUNCTION_RETURN() if (isNodeFunctionImplementedInNodeGrap) return NodeFunction::Yield<TheReturnTypeOfThisNativeFunction>(this, __FUNCTION__)

#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2
#define EXPAND(x) CONCATENATE(x,)

#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define VA_NARGS(...) CONCATENATE(VA_NARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1),)
#define VA_NARGS2(...) ((int)(sizeof((int[]){ __VA_ARGS__ })/sizeof(int)))

#define PUSH_VAR_TO_STACK_1(var) __stack.Proccess(var)

#define PUSH_VAR_TO_STACK_2(var, ...)    \
    __stack.Proccess(var);	\
    PUSH_VAR_TO_STACK_1(__VA_ARGS__);

#define PUSH_VAR_TO_STACK_3(var, ...)    \
    __stack.Proccess(var);	\
    PUSH_VAR_TO_STACK_2(__VA_ARGS__);

#define PUSH_VAR_TO_STACK_4(var, ...)    \
    __stack.Proccess(var);	\
    PUSH_VAR_TO_STACK_3(__VA_ARGS__);

#define PUSH_VAR_TO_STACK_5(var, ...)    \
    __stack.Proccess(var);	\
    PUSH_VAR_TO_STACK_4(__VA_ARGS__);

#define PUSH_VAR_TO_STACK_6(var, ...)    \
	__stack.Proccess(var);	\
  PUSH_VAR_TO_STACK_5(__VA_ARGS__);

#define PUSH_VAR_TO_STACK_7(var, ...)    \
    __stack.Proccess(var);	\
    PUSH_VAR_TO_STACK_6(__VA_ARGS__);

#define PUSH_VAR_TO_STACK_8(var, ...)    \
    __stack.Proccess(var);	\
    PUSH_VAR_TO_STACK_7(__VA_ARGS__);

#define NODESCRIPT_EVENT_DISPATCH(HASH, ...) { \
	static size_t __function_hash = std::hash<std::string>{}(HASH);\
	static Suora::NativeFunctionHashCheck _check_func = Suora::NativeFunctionHashCheck(HASH);\
	ScriptStack __stack;\
	CONCATENATE(PUSH_VAR_TO_STACK_,VA_NARGS(__VA_ARGS__))(__VA_ARGS__);\
	__NodeEventDispatch(__function_hash, __stack);\
}

#define MEMBER(...)
enum
{
	VisibleOnly,

	// only for Floats and Integers. example: Range = (0.0f, 1.0f)
	Range,

	// only applies for StackAllocatedObjects. Data and functionality of subclasses are injected into the Reflection System.
	Polymorphic
};

/////////////////////////////////////////////////
//////////////////  CASTING  ////////////////////
/////////////////////////////////////////////////

namespace Suora
{
	// Dynamically cast an object type-safely.
	template <class To, class From>
	static To* Cast(From* Src)
	{
		return Src ? (Src->CastImpl(To::StaticClass()) ? (To*)Src : nullptr) : nullptr;
	}

	template <class From>
	static From* Cast(From* Src, const Class& To)
	{
		return Src ? (Src->CastImpl(To) ? Src : nullptr) : nullptr;
	}
}

#include "Suora/NodeScript/ScriptStack.h"
#include "Suora/Reflection/SubclassOf.h"
#include "Suora/Reflection/New.h"