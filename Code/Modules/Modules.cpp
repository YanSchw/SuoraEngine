#include <iostream>
#include "Suora/Core/Log.h"

struct Modules
{
	Modules()
	{
		
	}
};

inline extern void Modules_Init()
{
	SUORA_LOG(Suora::LogCategory::Module, Suora::LogLevel::Info, "Initializing all Engine/Project Modules:");
	extern void MyModule_Init(); MyModule_Init();
}

inline static Modules __Modules__;