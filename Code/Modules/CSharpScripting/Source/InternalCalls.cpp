#include "Precompiled.h"
#include "InternalCalls.h"

#include "Suora/GameFramework/Node.h"

namespace Suora
{

	void Node_SetUpdateFlag(void* obj, int flag)
	{
		((Node*)obj)->SetUpdateFlag((UpdateFlag)flag);
	}

}