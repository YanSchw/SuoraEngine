#include "JoltCharacterController.h"
#include "Suora/Core/Base.h"
#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Nodes/CharacterNode.h"
#include "Suora/Physics/PhysicsWorld.h"
#include "JoltPhysicsWorld.h"
#include "JoltTypeConversion.h"
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace Suora::Physics
{

	JoltCharacterController::JoltCharacterController(CharacterNode* node)
		: m_CharacterNode(node), m_Listener(this)
	{
		Create();
	}

	JoltCharacterController::~JoltCharacterController()
	{
		m_Controller->Release();
	}

	void JoltCharacterController::SetSlopeLimit(float slopeLimitDeg)
	{
		m_Controller->SetMaxSlopeAngle(glm::radians(slopeLimitDeg));
	}

	void JoltCharacterController::SetStepOffset(float stepOffset)
	{
		m_StepOffset = stepOffset;
	}

	bool JoltCharacterController::IsGrounded() const
	{
		return m_Controller->GetGroundState() != JPH::CharacterBase::EGroundState::InAir;
	}

	void JoltCharacterController::SetInternalPosition(const Vec3& position)
	{
		m_Controller->SetPosition(Convert::ToRVec3(position));
	}

	Vec3 JoltCharacterController::GetInternalPosition()
	{
		return Convert::ToVec3(m_Controller->GetPosition());
	}
	void JoltCharacterController::SetInternalRotation(const Quat& rotation)
	{
		m_Controller->SetRotation(Convert::ToJoltQuat(rotation));
	}

	Quat JoltCharacterController::GetInternalRotation()
	{
		return Convert::ToSuoraQuat(m_Controller->GetRotation());
	}

	void JoltCharacterController::Move(const Vec3& movementInput)
	{
		m_MovementInput += movementInput;
	}

	void JoltCharacterController::Jump(float jumpForce)
	{
		m_JumpForce = jumpForce;
	}

	void JoltCharacterController::PreSimulate(float deltaTime)
	{
		m_DesiredVelocity = m_MovementInput / deltaTime;
		m_MovementInput = Vec3(0.0f, 0.0f, 0.0f);

		// Cancel movement in opposite direction of normal when sliding
		JPH::CharacterVirtual::EGroundState groundState = m_Controller->GetGroundState();
		JPH::Vec3 desiredVelocity = Convert::ToRVec3(m_DesiredVelocity);
		if (groundState == JPH::CharacterVirtual::EGroundState::OnSteepGround)
		{
			JPH::Vec3 normal = m_Controller->GetGroundNormal();
			normal.SetY(0.0f);
			float dot = normal.Dot(desiredVelocity);
			if (dot < 0.0f)
				desiredVelocity -= (dot * normal) / normal.LengthSq();
		}

		const JPH::Vec3 currentVerticalVelocity = JPH::Vec3(0, m_Controller->GetLinearVelocity().GetY(), 0);

		const JPH::Vec3 groundVelocity = m_Controller->GetGroundVelocity();

		const bool isGrounded = groundState == JPH::CharacterVirtual::EGroundState::OnGround;
		const bool notJumping = (currentVerticalVelocity.GetY() - groundVelocity.GetY()) < 0.1f;

		JPH::Vec3 newVelocity;
		if (isGrounded && notJumping)
		{
			// Assume velocity of ground when on ground
			newVelocity = groundVelocity;

			// Apply JumpForce
			if (m_JumpForce > 0.0f)
			{
				newVelocity += JPH::Vec3(0, m_JumpForce, 0);
				m_JumpForce = 0.0f;
			}
		}
		else
		{
			newVelocity = currentVerticalVelocity;
		}

		// ApplyGravity
		newVelocity += Convert::ToRVec3(Vec3(0, -9.81f, 0)) * m_CharacterNode->m_GravityScale * deltaTime;

		newVelocity += desiredVelocity;

		m_Controller->SetLinearVelocity(newVelocity);
	}

	void JoltCharacterController::Simulate(float deltaTime)
	{
		JPH::Vec3 oldPosition = m_Controller->GetPosition();

		bool groundToAir = m_Controller->GetGroundState() != JPH::CharacterBase::EGroundState::InAir;
		JPH::Vec3 gravity = Convert::ToRVec3(Vec3(0, -9.81f, 0));

		auto broadPhaseLayerFilter = GetSystem()->GetDefaultBroadPhaseLayerFilter(JPH::ObjectLayer(m_CharacterNode->LayerID));
		auto layerFilter = GetSystem()->GetDefaultLayerFilter(JPH::ObjectLayer(m_CharacterNode->LayerID));
		JPH::TempAllocator* tempAllocator = GetTempAllocator();

		m_Controller->Update(deltaTime, gravity, broadPhaseLayerFilter, layerFilter, {}, {}, *tempAllocator);

		if (m_Controller->GetGroundState() != JPH::CharacterBase::EGroundState::InAir)
			groundToAir = false;

		UpdateStairWalking(deltaTime, oldPosition);
	}

	void JoltCharacterController::UpdateStairWalking(float deltaTime, const JPH::Vec3& oldPosition)
	{
		float stepLength = glm::length(m_MovementInput);

		if (stepLength <= 0.0f)
			return;

		JPH::Vec3 achievedHorizontalStep = m_Controller->GetPosition() - oldPosition;
		achievedHorizontalStep.SetY(0.0f);

		JPH::Vec3 stepForwardNormalized = Convert::ToRVec3(m_MovementInput) / stepLength;
		achievedHorizontalStep = glm::max(0.0f, achievedHorizontalStep.Dot(stepForwardNormalized)) * stepForwardNormalized;
		float achievedHorizontalStepLength = achievedHorizontalStep.Length();

		// Cannot move ahead and slope is too steep
		if (achievedHorizontalStepLength + 1.0e-4f < stepLength && m_Controller->CanWalkStairs(Convert::ToRVec3(m_DesiredVelocity)))
		{
			JPH::Vec3 stepForward = stepForwardNormalized * glm::max(0.02f, stepLength - achievedHorizontalStepLength);
			JPH::Vec3 stepForwardTest = stepForwardNormalized * 0.15f;

			auto broadPhaseLayerFilter = GetSystem()->GetDefaultBroadPhaseLayerFilter(JPH::ObjectLayer(m_CharacterNode->LayerID));
			auto layerFilter = GetSystem()->GetDefaultLayerFilter(JPH::ObjectLayer(m_CharacterNode->LayerID));
			JPH::TempAllocator* tempAllocator = GetTempAllocator();

			m_Controller->WalkStairs(deltaTime, JPH::Vec3(0.0f, m_StepOffset, 0.0f), stepForward, stepForwardTest, JPH::Vec3::sZero(), broadPhaseLayerFilter, layerFilter, { }, { }, *tempAllocator);
		}
	}

	void JoltCharacterController::Create()
	{
		JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
		settings->mMaxSlopeAngle = glm::max(0.0f, glm::cos(glm::radians(m_CharacterNode->SlopeLimitDeg)));
		m_StepOffset = m_CharacterNode->StepOffset;

		// CapsuleShape
		{
			float radiusScale = glm::max(m_CharacterNode->GetScale().x, m_CharacterNode->GetScale().z);

			JPH::Ref<JPH::CapsuleShapeSettings> capsuleSettings = new JPH::CapsuleShapeSettings();
			capsuleSettings->mRadius = m_CharacterNode->m_CapsuleRadius * radiusScale;
			capsuleSettings->mHalfHeightOfCylinder = m_CharacterNode->m_CapsuleHalfHeight * m_CharacterNode->GetScale().y;
			capsuleSettings->mMaterial = nullptr;

			auto result = capsuleSettings->Create();

			if (result.HasError())
			{
				SuoraError("Failed to create CharacterController. Error: {}", result.GetError());
				return;
			}

			settings->mShape = result.Get();
		}

		m_Controller = new JPH::CharacterVirtual(settings, Convert::ToRVec3(m_CharacterNode->GetPosition()), Convert::ToJoltQuat(m_CharacterNode->GetRotation()), GetSystem());
		m_Controller->SetListener(&m_Listener);
		m_HasGravity = !m_CharacterNode->DisableGravity;
	}

	JPH::PhysicsSystem* JoltCharacterController::GetSystem()
	{
		return m_CharacterNode->GetWorld()->GetPhysicsWorld()->As<JoltPhysicsWorld>()->m_PhysicsSystem.get();
	}

	JPH::TempAllocator* JoltCharacterController::GetTempAllocator()
	{
		return m_CharacterNode->GetWorld()->GetPhysicsWorld()->As<JoltPhysicsWorld>()->m_TempAllocator.get();
	}

	void JoltCharacterController::Listener::OnContactAdded(const JPH::CharacterVirtual* inCharacter, const JPH::BodyID& inBodyID2, const JPH::SubShapeID& inSubShapeID2, JPH::Vec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::CharacterContactSettings& ioSettings)
	{
		m_CharacterController->m_CollisionFlags = ECollisionFlags::None;

		if (inContactNormal.GetY() < 0.0f)
			m_CharacterController->m_CollisionFlags |= ECollisionFlags::Below;
		else if (inContactNormal.GetY() > 0.0f)
			m_CharacterController->m_CollisionFlags |= ECollisionFlags::Above;

		if (inContactNormal.GetX() != 0.0f || inContactNormal.GetZ() != 0.0f)
			m_CharacterController->m_CollisionFlags |= ECollisionFlags::Sides;
	}
}