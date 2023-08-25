#include <Precompiled.h>
#include "Delegate.h"
#include "Suora/GameFramework/Node.h"

namespace Suora
{

	void TDelegate::DispatchToScriptEngine(Node* node, ScriptStack& stack, size_t hash)
	{
		node->__NodeEventDispatch(hash, stack);
	}

}