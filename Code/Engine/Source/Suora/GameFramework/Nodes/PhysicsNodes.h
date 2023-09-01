#pragma once
#include <vector>
#include <entt.hpp>
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/Delegate.h"
#include "Suora/Common/Array.h"
#include "PhysicsNodes.generated.h"

namespace Suora
{
	
	enum class CollsionNodeType : uint32_t
	{
		None = 0,
		Box,
		Sphere,
		Capsule
	};
	class PhysicsNode;
	class CollisionNode;
	class BoxCollisionNode;

	class PhysicsNode : public Node3D
	{
		SUORA_CLASS(54789845);

	public:
		MEMBER()
		float m_Mass = 10.0f;

	public:
		PhysicsNode();
		~PhysicsNode();
		virtual void InitializeNode(World& world) override;
		virtual void OnNodeDestroy() override;
		void TickTransform(bool inverseParentTransform) override;

		friend class Physics::PhysicsWorld;
	};

	class RigidbodyNode : public PhysicsNode
	{
		SUORA_CLASS(54381296);

	private:
		Vec3d Velocity = Vec::Zero;
		Vec3d Force = Vec::Zero;
		Vec3d AngularVelocity = Vec::Zero;
		double VelocityDamping = 0.92f;
		int SolvingIterations = 4;
	public:
		RigidbodyNode();
		~RigidbodyNode();
		virtual void InitializeNode(World& world) override;
		virtual void OnNodeDestroy() override;
		void TickTransform(bool inverseParentTransform) override;

		void SetForce(const Vec3& force);

		friend class CharacterNode;
		friend class Physics::PhysicsWorld;
	};

	class CharacterNode : public RigidbodyNode
	{
		SUORA_CLASS(4768354873);

	private:
		double m_Gravity = -0.2f;
		bool m_GroundedLastFrame = false;
		Vec3d m_MovementVelocity = Vec3(0);
		Vec3d m_MovementDirection = Vec3(0.0f);

		bool Grounded = false;

	public:
		CharacterNode();
	public:
		float m_MovementSpeed = 500.0f;

		FUNCTION(Callable)
		void AddMovementInput(const Vec3& delta);

		FUNCTION(Callable, Pure)
		bool IsGrounded();

		FUNCTION(Callable)
		void Jump(float force);

		void Begin() override;
		void WorldUpdate(float deltaTime) override;

		friend class Physics::PhysicsWorld;
	};

	struct ProxyCollisionEvent
	{

	};
	class CollisionNode : public Node3D
	{
		SUORA_CLASS(47638332);

	private:
		CollsionNodeType Type = CollsionNodeType::None;
		entt::entity m_Index = entt::null;
		bool m_CollisionNodeDestroyed = false;

		std::unordered_map<size_t, bool> m_CollisionEventsThisTick;
		std::unordered_map<size_t, RigidbodyNode*> m_CollisionEventsRigidbodies;
		std::unordered_map<size_t, CollisionNode*> m_CollisionEventsOtherCollider;
		void HandleCollisionEvent(RigidbodyNode* rb, CollisionNode* other);
		void PostPhysicsTick();

	public:
		CollisionNode() { }
		CollisionNode(CollsionNodeType type) : Type(type) { }
		virtual ~CollisionNode();
		void InitializeNode(World& world) override;
		void Begin() override;
		virtual void OnNodeDestroy() override;
		void TickTransform(bool inverseParentTransform) override;

		MEMBER()
		bool IsTrigger = false;

		CollsionNodeType GetType() const { return Type; }

		MEMBER() Delegate<RigidbodyNode*, CollisionNode*> OnTriggerEnter;
		MEMBER() Delegate<RigidbodyNode*, CollisionNode*> OnTriggerStay;
		MEMBER() Delegate<RigidbodyNode*, CollisionNode*> OnTriggerExit;

		MEMBER() Delegate<RigidbodyNode*, CollisionNode*> OnCollisionEnter;
		MEMBER() Delegate<RigidbodyNode*, CollisionNode*> OnCollisionStay;
		MEMBER() Delegate<RigidbodyNode*, CollisionNode*> OnCollisionExit;

		friend class Physics::PhysicsWorld;
	};
	
	class BoxCollisionNode : public CollisionNode
	{
		SUORA_CLASS(1299861);

	public:
		BoxCollisionNode() : CollisionNode(CollsionNodeType::Box) { }

		MEMBER()
		Vec3 m_Extends = Vec::One;
	};
	class SphereCollisionNode : public CollisionNode
	{
		SUORA_CLASS(436789);

	public:
		SphereCollisionNode() : CollisionNode(CollsionNodeType::Sphere) { }

		MEMBER()
		float m_Radius = 0.5f;
	};
	class CapsuleCollisionNode : public CollisionNode
	{
		SUORA_CLASS(47684335);

	public:
		CapsuleCollisionNode() : CollisionNode(CollsionNodeType::Capsule) { }

		MEMBER()
		float m_Height = 2.0f;
		MEMBER()
		float m_Radius = 0.5f;
	};

}