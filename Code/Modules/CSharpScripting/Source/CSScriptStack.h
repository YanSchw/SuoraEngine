#pragma once
#include "Suora/NodeScript/ScriptStack.h"

#include "Type.hpp"

namespace Coral
{
	class ManagedAssembly;
}

namespace Suora
{

	class CSScriptStack
	{
		inline static Coral::Type ScriptStackType;
		inline static Ref<ScriptStack> s_Slot = nullptr;

	public:
		static void UploadScriptStack(ScriptStack stack);

		static void UploadInternalCalls(Coral::ManagedAssembly& assembly);

	private:
		static void ReceivePrepareSlot();
		static void ReceiveInt32(int32_t value);

		friend class CSharpScriptEngine;
	};

}