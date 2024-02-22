#pragma once

#include "CoreMinimal.h"

namespace Suora
{

	class CSharpCodeGenerator
	{
	public:
		static void Generate_AllNativeClasses_CS(String& code);
	private:
		static void GenerateManagedFunctions(String& code, const Class& cls);
	};

}