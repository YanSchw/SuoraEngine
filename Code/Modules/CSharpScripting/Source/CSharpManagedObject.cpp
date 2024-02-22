#include "CSharpManagedObject.h"
#include "CSharpScriptEngine.h"

namespace Suora
{

	ICSharpManagedObject::~ICSharpManagedObject()
	{
		CSharpScriptEngine::Get()->DestroyManagedObject(GetRootObject());
	}

}