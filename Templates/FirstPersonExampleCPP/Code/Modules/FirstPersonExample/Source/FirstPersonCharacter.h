#pragma once
#include "CoreMinimal.h"
#include "Suora/GameFramework/Nodes/CharacterNode.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "FirstPersonCharacter.generated.h"

class FirstPersonCharacter : public CharacterNode
{
    SUORA_CLASS(2645454331);
public:
	FirstPersonCharacter();
	
	void Move(const Vec2& axis2D);
	void Look(const Vec2& axis2D);
	void Jump();
	
	PROPERTY()
	float m_MovementSpeed = 2.0f;

private:
	WeakPtr<CameraNode> m_Camera;
	Vec3 m_EulerRot = Vec::Zero;
};