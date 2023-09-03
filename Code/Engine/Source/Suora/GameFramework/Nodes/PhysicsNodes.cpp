#include "Precompiled.h"
#include "PhysicsNodes.h"
#include "Suora/GameFramework/World.h"
#include "Suora/Physics/PhysicsEngine.h"
#include "Suora/Common/Math.h"

namespace Suora
{
	PhysicsNode::PhysicsNode()
	{
	}
	PhysicsNode::~PhysicsNode()
	{
		if (GetWorld())
		{
			if (GetWorld()->GetPhysicsWorld()->m_PhysicsNodes.Contains(this))
			{
				GetWorld()->GetPhysicsWorld()->m_PhysicsNodes.Remove(this);
			}
		}
	}

	void PhysicsNode::InitializeNode(World& world)
	{
		world.GetPhysicsWorld()->m_PhysicsNodes.Add(this);

		Super::InitializeNode(world);
	}

	void PhysicsNode::OnNodeDestroy()
	{
		Super::OnNodeDestroy();
		
		if (GetWorld())
		{
			GetWorld()->GetPhysicsWorld()->m_PhysicsNodes.Remove(this);
		}
	}

	void PhysicsNode::TickTransform(bool inverseParentTransform)
	{
		Super::TickTransform(inverseParentTransform);

		
	}

	RigidbodyNode::RigidbodyNode()
	{
	}
	RigidbodyNode::~RigidbodyNode()
	{
	}

	void RigidbodyNode::InitializeNode(World& world)
	{
		Super::InitializeNode(world);
	}

	void RigidbodyNode::OnNodeDestroy()
	{
		Super::OnNodeDestroy();

	}

	void RigidbodyNode::TickTransform(bool inverseParentTransform)
	{
		Super::TickTransform(inverseParentTransform);

	}
	void RigidbodyNode::SetForce(const Vec3& force)
	{
		Force = force;
	}

	CharacterNode::CharacterNode()
	{
		m_Mass = 100.0f;
		SolvingIterations = 16;
		SetUpdateFlag(UpdateFlag::WorldUpdate);
	}
	void CharacterNode::AddMovementInput(const Vec3& delta)
	{
		m_MovementDirection += delta;

	}

	bool CharacterNode::IsGrounded()
	{
		return Grounded;
		//return m_GroundedLastFrame && IsVelocityGrounded();
	}
	void CharacterNode::Jump(float force)
	{
		AddWorldOffset(glm::normalize(GetWorld()->GetPhysicsWorld()->m_Gravity * -1.0f) * 0.2f);
		Force = glm::normalize(GetWorld()->GetPhysicsWorld()->m_Gravity * -1.0f) * force;
	}
	void CharacterNode::Begin()
	{
		Super::Begin();

	}

	void CharacterNode::WorldUpdate(float deltaTime)
	{
		Super::WorldUpdate(deltaTime);

	}


	void CollisionNode::HandleCollisionEvent(RigidbodyNode* rb, CollisionNode* other)
	{
		size_t hash = ((size_t)rb) ^ ((size_t)other);
		
		if (m_CollisionEventsThisTick.find(hash) == m_CollisionEventsThisTick.end())
		{
			m_CollisionEventsThisTick[hash] = true;
			if (IsTrigger) OnTriggerEnter(rb, other); else OnCollisionEnter(rb, other);
			if (m_CollisionNodeDestroyed)
			{
				return;
			}
			GetWorld()->GetPhysicsWorld()->m_PostPhysicsTicksRequired.Add(this);
		}
		else if (!m_CollisionEventsThisTick[hash])
		{
			m_CollisionEventsThisTick[hash] = true;
			if (IsTrigger) OnTriggerStay(rb, other); else OnCollisionStay(rb, other);
			if (m_CollisionNodeDestroyed)
			{
				Array<Ptr<CollisionNode>>& arr = GetWorld()->GetPhysicsWorld()->m_PostPhysicsTicksRequired;
				for (int i = 0; i < arr.Size(); i++)
				{
					if (arr[i].Get() == this)
					{
						arr[i] = nullptr;
						break;
					}
				}
			}
		}

		m_CollisionEventsRigidbodies[hash] = rb;
		m_CollisionEventsOtherCollider[hash] = other;
	}

	void CollisionNode::PostPhysicsTick()
	{
		Array<size_t> removeThese;
		for (auto& It : m_CollisionEventsThisTick)
		{
			if (It.second)
			{
				It.second = false;
			}
			else
			{
				removeThese.Add(It.first);
			}
		}
		for (auto It : removeThese)
		{
			m_CollisionEventsThisTick.erase(It);
			m_CollisionEventsRigidbodies.erase(It);
			m_CollisionEventsOtherCollider.erase(It);
			if (IsTrigger) OnTriggerExit(m_CollisionEventsRigidbodies[It], m_CollisionEventsOtherCollider[It]); else OnCollisionExit(m_CollisionEventsRigidbodies[It], m_CollisionEventsOtherCollider[It]);
		}

		if (m_CollisionNodeDestroyed || m_CollisionEventsThisTick.empty())
		{
			Array<Ptr<CollisionNode>>& arr = GetWorld()->GetPhysicsWorld()->m_PostPhysicsTicksRequired;
			for (int i = 0; i < arr.Size(); i++)
			{
				if (arr[i].Get() == this)
				{
					arr[i] = nullptr;
					break;
				}
			}
		}
	}

	CollisionNode::~CollisionNode()
	{
		m_CollisionNodeDestroyed = true;
		if (GetWorld())
		{
			GetWorld()->GetPhysicsWorld()->DestroyCollider(this);
		}
	}

	void CollisionNode::InitializeNode(World& world)
	{
		Super::InitializeNode(world);

		m_Index = world.GetPhysicsWorld()->CreateCollider(this);
		world.GetPhysicsWorld()->TickCollider(this);
	}
	void CollisionNode::Begin()
	{
		Super::Begin();

		if (!Implements<IObjectCompositionData>())
		{
			Implement<IObjectCompositionData>();
		}
		GetInterface<IObjectCompositionData>()->m_IsActorLayer = false;
	}

	void CollisionNode::OnNodeDestroy()
	{
		Super::OnNodeDestroy();

		if (GetWorld())
		{
			GetWorld()->GetPhysicsWorld()->DestroyCollider(this);
		}
	}

	void CollisionNode::TickTransform(bool inverseParentTransform)
	{
		Super::TickTransform(inverseParentTransform);

		if (GetWorld())
		{
			if (!Physics::PhysicsWorld::s_InPhysicsSimulation)
			{
				GetWorld()->GetPhysicsWorld()->TickCollider(this);
			}
		}
	}

}