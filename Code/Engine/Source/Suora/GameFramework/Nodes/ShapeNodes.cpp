#include "Precompiled.h"
#include "ShapeNodes.h"
#include "Suora/GameFramework/World.h"
#include "Suora/Physics/PhysicsEngine.h"
#include "Suora/Physics/PhysicsWorld.h"
#include "Suora/Common/Math.h"

namespace Suora
{


	/*void ShapeNode::HandleCollisionEvent(RigidbodyNode* rb, CollisionNode* other)
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

	void ShapeNode::PostPhysicsTick()
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
	}*/

	ShapeNode::~ShapeNode()
	{
		m_CollisionNodeDestroyed = true;
		
	}

	void ShapeNode::InitializeNode(World& world)
	{
		Super::InitializeNode(world);

	}
	void ShapeNode::Begin()
	{
		Super::Begin();

		if (!Implements<IObjectCompositionData>())
		{
			Implement<IObjectCompositionData>();
		}
		GetInterface<IObjectCompositionData>()->m_IsActorLayer = false;

		if (IsStatic)
		{
			SetBodyType(BodyType::Static);
		}

		SuoraAssert(GetWorld());
		SuoraAssert(GetWorld()->GetPhysicsWorld());
		GetWorld()->GetPhysicsWorld()->CreateShapeNode(this);
	}

	void ShapeNode::OnNodeDestroy()
	{
		Super::OnNodeDestroy();

		SuoraAssert(GetWorld());
		SuoraAssert(GetWorld()->GetPhysicsWorld());
		GetWorld()->GetPhysicsWorld()->DestroyShapeNode(this);
	}

	void ShapeNode::TickTransform(bool inverseParentTransform)
	{
		Super::TickTransform(inverseParentTransform);

		if (ShouldUpdateInCurrentContext() && !Physics::PhysicsWorld::s_InPhysicsSimulation)
		{
			SuoraAssert(GetWorld());
			SuoraAssert(GetWorld()->GetPhysicsWorld());
			GetWorld()->GetPhysicsWorld()->TickShapeNode(this);
		}
	}

	void ShapeNode::SetBodyType(BodyType type)
	{
		m_BodyType = type;
	}

	ShapeNode::BodyType ShapeNode::GetBodyType() const
	{
		return m_BodyType;
	}

	Vec3 BoxShapeNode::GetBoxExtends() const
	{
		const Vec3 scale = glm::abs(GetScale());
		return scale * m_HalfExtends;
	}

	float SphereShapeNode::GetSphereRadius() const
	{
		const Vec3 scale = glm::abs(GetScale());
		return glm::max(glm::max(scale.x, scale.y), scale.z) * m_Radius;
	}

}