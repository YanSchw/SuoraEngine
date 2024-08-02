#pragma once
#include "Suora/Core/Base.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/Delegate.h"
#include "Suora/Common/Array.h"
#include "CharacterNode.generated.h"

namespace Suora
{

	class CharacterNode : public Node3D
	{
		SUORA_CLASS(875943291347);
	public:
		virtual ~CharacterNode();
		void Begin() override;
		void TickTransform(bool inverseParentTransform) override;

		FUNCTION(Callable)
		void AddMovementInput(const Vec3& movementInput);

		FUNCTION(Callable)
		void Jump(float jumpForce);

		FUNCTION(Callable, Pure)
		bool IsGrounded();

		float GetCapsuleHeight() const;
		float GetCapsuleRadius() const;

		PROPERTY()
		float m_CapsuleRadius = 0.5f;
		PROPERTY()
		float m_CapsuleHalfHeight = 1.0f;

		PROPERTY()
		float SlopeLimitDeg = 45.5f;
		PROPERTY()
		float StepOffset = 0.15f;

		uint32_t LayerID = 0;

		PROPERTY()
		bool DisableGravity = false;
		PROPERTY()
		float m_GravityScale = 1.0f;

	protected:
		class CharacterController* GetCharacterController() const;
	private:
		Ref<class CharacterController> m_Controller;
	};

}