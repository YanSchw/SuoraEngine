#include "CSScriptStack.h"
#include "CSharpScriptEngine.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/NodeScript/ScriptStack.h"
#include "inttypes.h"

#include "Assembly.hpp"

namespace Suora
{

	void CSScriptStack::UploadScriptStack(ScriptStack stack)
	{
		ScriptStackType.InvokeStaticMethod("PrepareSlot");

		stack.Invert();

		while (!stack.IsEmpty())
		{
			switch (stack.PeekType())
			{
			case ScriptDataType::Int32: ScriptStackType.InvokeStaticMethod("UploadInt32", stack.PopItem<int32_t>()); break;
			case ScriptDataType::UInt32: ScriptStackType.InvokeStaticMethod("UploadUInt32", stack.PopItem<uint32_t>()); break;
			case ScriptDataType::Bool: ScriptStackType.InvokeStaticMethod("UploadBool", stack.PopItem<bool>()); break;
			case ScriptDataType::Float: ScriptStackType.InvokeStaticMethod("UploadFloat", stack.PopItem<float>()); break;
			case ScriptDataType::Vec3:
			{
				Vec3 vec = stack.PopItem<Vec3>();
				ScriptStackType.InvokeStaticMethod("UploadVec3", vec.x, vec.y, vec.z);
				break;
			}
			case ScriptDataType::ObjectPtr: ScriptStackType.InvokeStaticMethod("UploadObjectPtr", (void*)stack.PopItem<Object*>()); break;
			case ScriptDataType::None:
			default:
				SuoraVerify(false, "Missing Implementation!");
				break;
			}
		}
	}

	void CSScriptStack::UploadInternalCalls(Coral::ManagedAssembly& assembly)
	{
		assembly.AddInternalCall("Suora.ScriptStack", "PrepareSlotInCPP", reinterpret_cast<void*>(&CSScriptStack::ReceivePrepareSlot));
		assembly.AddInternalCall("Suora.ScriptStack", "SendInt32ToCPP", reinterpret_cast<void*>(&CSScriptStack::ReceiveInt32));
		assembly.AddInternalCall("Suora.ScriptStack", "SendUInt32ToCPP", reinterpret_cast<void*>(&CSScriptStack::ReceiveUInt32));
		assembly.AddInternalCall("Suora.ScriptStack", "SendFloatToCPP", reinterpret_cast<void*>(&CSScriptStack::ReceiveFloat));
		assembly.AddInternalCall("Suora.ScriptStack", "SendVec3ToCPP", reinterpret_cast<void*>(&CSScriptStack::ReceiveVec3));
		assembly.AddInternalCall("Suora.ScriptStack", "SendObjectPtrToCPP", reinterpret_cast<void*>(&CSScriptStack::ReceiveObjectPtr));
		assembly.UploadInternalCalls();
	}

	ScriptStack CSScriptStack::Get()
	{
		s_Slot->Invert();
		const ScriptStack stack = *s_Slot;

		s_Slot = nullptr;

		return stack;
	}

	void CSScriptStack::ReceivePrepareSlot()
	{
		s_Slot = CreateRef<ScriptStack>();
	}

	void CSScriptStack::ReceiveInt32(int32_t value)
	{
		s_Slot->PushItem<int32_t>(value);
	}
	void CSScriptStack::ReceiveUInt32(uint32_t value)
	{
		s_Slot->PushItem<uint32_t>(value);
	}

	void CSScriptStack::ReceiveBool(bool value)
	{
		s_Slot->PushItem<bool>(value);
	}

	void CSScriptStack::ReceiveFloat(float value)
	{
		s_Slot->PushItem<float>(value);
	}

	void CSScriptStack::ReceiveVec3(float x, float y, float z)
	{
		s_Slot->PushItem<Vec3>(Vec3(x, y, z));
	}

	void CSScriptStack::ReceiveObjectPtr(void* ptr)
	{
		s_Slot->PushItem<Object*>((Object*)ptr);
	}

}