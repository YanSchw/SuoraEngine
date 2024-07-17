#pragma once
#include "CoreMinimal.h"
#include "Suora/GameFramework/GameInstance.h"
#include "MyGameModule.generated.h"

class MyGameModule : public GameModule
{
    SUORA_CLASS(5438795711);
public:
	virtual void Initialize() override;
	virtual void Update(float deltaTime) override;
};