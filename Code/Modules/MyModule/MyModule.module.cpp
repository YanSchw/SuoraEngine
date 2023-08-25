#include "Suora/Core/Log.h"

//** Module Includes */
#include "MyNode.h"

inline extern void MyModule_Init()
{
	SUORA_LOG(Suora::LogCategory::Module, Suora::LogLevel::Info, " - MyModule");
}