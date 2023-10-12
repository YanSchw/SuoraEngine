#include "Precompiled.h"
#include "CharacterNode.h"
#include "Suora/GameFramework/World.h"
#include "Suora/Physics/CharacterController.h"
#include "Suora/Physics/PhysicsEngine.h"
#include "Suora/Physics/PhysicsWorld.h"

namespace Suora
{
	CharacterNode::~CharacterNode()
	{
		m_Controller = nullptr;

		if (IsPendingKill())
		{
			SuoraAssert(GetWorld());
			SuoraAssert(GetWorld()->GetPhysicsWorld());
			GetWorld()->GetPhysicsWorld()->DestroyCharacterNode(this);
		}
	}

	void CharacterNode::Begin()
	{
		Super::Begin();

		SuoraAssert(GetWorld());
		SuoraAssert(GetWorld()->GetPhysicsWorld());
		m_Controller = GetWorld()->GetPhysicsWorld()->CreateCharacterNode(this);
	}

	void CharacterNode::TickTransform(bool inverseParentTransform)
	{
		Super::TickTransform(inverseParentTransform);

		if (ShouldUpdateInCurrentContext() && !Physics::PhysicsWorld::s_InPhysicsSimulation)
		{
			SuoraAssert(GetWorld());
			SuoraAssert(GetWorld()->GetPhysicsWorld());
			GetWorld()->GetPhysicsWorld()->TickCharacterNode(this);
		}
	}

	void CharacterNode::AddMovementInput(const Vec3& movementInput)
	{
		GetCharacterController()->Move(movementInput);
	}

	void CharacterNode::Jump(float jumpForce)
	{
		GetCharacterController()->Jump(jumpForce);
	}

	bool CharacterNode::IsGrounded()
	{
		return GetCharacterController()->IsGrounded();
	}

	CharacterController* CharacterNode::GetCharacterController() const
	{
		return m_Controller.get();
	}
}