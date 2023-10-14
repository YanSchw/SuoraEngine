#pragma once
#include "Suora/Common/VectorUtils.h"

namespace Suora
{
	enum class ECollisionFlags : uint8_t
	{
		None,
		Sides,
		Above,
		Below,
		ScopedBitmaskMark
	};
	inline ECollisionFlags operator|(ECollisionFlags a, ECollisionFlags b)
	{
		return static_cast<ECollisionFlags>(static_cast<int32_t>(a) | static_cast<int32_t>(b));
	}
	inline ECollisionFlags operator|=(ECollisionFlags a, ECollisionFlags b)
	{
		return static_cast<ECollisionFlags>(static_cast<int32_t>(a) | static_cast<int32_t>(b));
	}

	class CharacterController abstract
	{
	public:
		virtual ~CharacterController() = default;

		virtual void SetGravityEnabled(bool enableGravity) = 0;
		virtual bool IsGravityEnabled() const = 0;

		virtual void SetSlopeLimit(float slopeLimitDeg) = 0;
		virtual void SetStepOffset(float stepOffset) = 0;

		virtual bool IsGrounded() const = 0;

		virtual ECollisionFlags GetCollisionFlags() const = 0;

		virtual void SetInternalPosition(const Vec3& position) = 0;
		virtual Vec3 GetInternalPosition() = 0;
		virtual void SetInternalRotation(const Quat& rotation) = 0;
		virtual Quat GetInternalRotation() = 0;
		virtual void Move(const Vec3& movementInput) = 0;
		virtual void Jump(float jumpForce) = 0;

		virtual float GetSpeedDown() const = 0;

		virtual void PreSimulate(float deltaTime) = 0;
		virtual void Simulate(float deltaTime) = 0;
	};
}