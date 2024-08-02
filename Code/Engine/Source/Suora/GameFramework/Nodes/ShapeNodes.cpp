#include "Precompiled.h"
#include "ShapeNodes.h"
#include "Suora/GameFramework/World.h"
#include "Suora/Physics/PhysicsEngine.h"
#include "Suora/Physics/PhysicsWorld.h"
#include "Suora/Common/Math.h"

namespace Suora
{

	ShapeNode::~ShapeNode()
	{
		m_CollisionNodeDestroyed = true;
		
		if (IsPendingKill())
		{
			SuoraAssert(GetWorld());
			SuoraAssert(GetWorld()->GetPhysicsWorld());
			GetWorld()->GetPhysicsWorld()->DestroyShapeNode(this);
		}
	}

	void ShapeNode::InitializeNode(World& world)
	{
		Super::InitializeNode(world);

	}
	void ShapeNode::Begin()
	{
		Super::Begin();

		if (IsStatic())
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

	}

	void ShapeNode::TickTransform(bool inverseParentTransform)
	{
		Super::TickTransform(inverseParentTransform);

		TickShapeNode();
	}

	void ShapeNode::SetBodyType(BodyType type)
	{
		m_BodyType = type;
	}

	ShapeNode::BodyType ShapeNode::GetBodyType() const
	{
		return m_BodyType;
	}

	void ShapeNode::SetIsTrigger(bool InIsTrigger)
	{
		m_IsTrigger = InIsTrigger;
		TickShapeNode();
	}

	bool ShapeNode::IsTrigger() const
	{
		return m_IsTrigger;
	}

	void ShapeNode::SetIsStatic(bool InIsStatic)
	{
		m_IsStatic = InIsStatic;
		TickShapeNode();
	}

	bool ShapeNode::IsStatic() const
	{
		return m_IsStatic;
	}

	void ShapeNode::TickShapeNode()
	{
		if (ShouldUpdateInCurrentContext() && !Physics::PhysicsWorld::s_InPhysicsSimulation)
		{
			SuoraAssert(GetWorld());
			SuoraAssert(GetWorld()->GetPhysicsWorld());
			GetWorld()->GetPhysicsWorld()->TickShapeNode(this);
		}
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