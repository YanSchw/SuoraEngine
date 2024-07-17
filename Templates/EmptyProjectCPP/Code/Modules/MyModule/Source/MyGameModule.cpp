#include "MyGameModule.h"

void MyGameModule::Initialize()
{
	SUORA_LOG(LogCategory::Gameplay, LogLevel::Trace, "Hello from my custom GameModule!");
}

void MyGameModule::Update(float deltaTime)
{
	SUORA_LOG(LogCategory::Gameplay, LogLevel::Trace, "How about some spam");
}