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

		static ScriptStack Get();

	private:
		static void ReceivePrepareSlot();
		static void ReceiveInt32(int32_t value);
		static void ReceiveUInt32(uint32_t value);
		static void ReceiveBool(bool value);
		static void ReceiveFloat(float value);
		static void ReceiveVec3(float x, float y, float z);
		static void ReceiveObjectPtr(void* ptr);

		friend class CSharpScriptEngine;
	};

}