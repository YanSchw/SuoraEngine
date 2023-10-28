#pragma once
#include "Suora/Physics/CharacterController.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>

namespace Suora
{
	class CharacterNode;
}

namespace Suora::Physics
{
	class JoltCharacterController : public CharacterController
	{
	public:
		JoltCharacterController(CharacterNode* node);
		~JoltCharacterController();

		virtual void SetGravityEnabled(bool enableGravity) override { m_HasGravity = enableGravity; }
		virtual bool IsGravityEnabled() const override { return m_HasGravity; }

		virtual void SetSlopeLimit(float slopeLimitDeg) override;
		virtual void SetStepOffset(float stepOffset) override;

		virtual bool IsGrounded() const override;

		virtual ECollisionFlags GetCollisionFlags() const override { return m_CollisionFlags; }

		virtual void SetInternalPosition(const Vec3& position) override;
		virtual Vec3 GetInternalPosition() override;
		virtual void SetInternalRotation(const Quat& rotation) override;
		virtual Quat GetInternalRotation() override;
		virtual void Move(const Vec3& movementInput) override;
		virtual void Jump(float jumpPower) override;

		virtual float GetSpeedDown() const override { return m_Controller->GetLinearVelocity().GetY(); }

	private:
		virtual void PreSimulate(float deltaTime) override;
		virtual void Simulate(float deltaTime) override;

		void UpdateStairWalking(float deltaTime, const JPH::Vec3& oldPosition);

		void Create();

		JPH::PhysicsSystem* GetSystem();
		JPH::TempAllocator* GetTempAllocator();

		class Listener : public JPH::CharacterContactListener
		{
		public:
			Listener(JoltCharacterController* characterController)
				: m_CharacterController(characterController) {}

			void OnContactAdded(const JPH::CharacterVirtual* inCharacter, const JPH::BodyID& inBodyID2, const JPH::SubShapeID& inSubShapeID2, JPH::Vec3Arg inContactPosition, JPH::Vec3Arg inContactNormal, JPH::CharacterContactSettings& ioSettings);

		private:
			JoltCharacterController* m_CharacterController;
		};

	private:
		CharacterNode* m_CharacterNode;
		Listener m_Listener;

		JPH::Ref<JPH::CharacterVirtual> m_Controller;

		float m_JumpForce = 0.0f;
		Vec3 m_MovementInput = Vec3(0.0f);
		Vec3 m_DesiredVelocity = Vec3(0.0f);
		bool m_HasGravity = true;
		bool m_AllowSliding = false;
		float m_StepOffset = 0.0f;

		ECollisionFlags m_CollisionFlags = ECollisionFlags::None;

	private:
		friend class JoltPhysicsWorld;
	};
}