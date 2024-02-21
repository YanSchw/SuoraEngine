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

		switch (stack.PeekType())
		{
		case ScriptDataType::Int32:
		{
			ScriptStackType.InvokeStaticMethod("UploadInt32", stack.PopItem<int32_t>());
			break;
		}
		case ScriptDataType::None:
		default:
			SuoraVerify(false, "");
			break;
		}
	}

	void CSScriptStack::UploadInternalCalls(Coral::ManagedAssembly& assembly)
	{
		assembly.AddInternalCall("Suora.ScriptStack", "PrepareSlotInCPP", reinterpret_cast<void*>(&CSScriptStack::ReceivePrepareSlot));
		assembly.AddInternalCall("Suora.ScriptStack", "SendInt32ToCPP", reinterpret_cast<void*>(&CSScriptStack::ReceiveInt32));
		assembly.UploadInternalCalls();
	}

	void CSScriptStack::ReceivePrepareSlot()
	{
		s_Slot = CreateRef<ScriptStack>();
	}

	void CSScriptStack::ReceiveInt32(int32_t value)
	{
		s_Slot->PushItem<int32_t>(value);
	}

}