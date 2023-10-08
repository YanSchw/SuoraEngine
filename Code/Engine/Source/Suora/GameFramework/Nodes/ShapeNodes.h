#pragma once
#include <vector>
#include <entt.hpp>
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/Delegate.h"
#include "Suora/Common/Array.h"
#include "ShapeNodes.generated.h"

namespace Suora
{
	
	enum class ShapeType : uint32_t
	{
		None = 0,
		Box,
		Sphere,
		Capsule
	};

	class ShapeNode : public Node3D
	{
		SUORA_CLASS(47638332);

	private:
		enum class BodyType { Static = 0, Kinematic, Dynamic };

		ShapeType Type = ShapeType::None;
		BodyType m_BodyType = BodyType::Dynamic;
		bool m_CollisionNodeDestroyed = false;

	public:
		ShapeNode() { }
		ShapeNode(ShapeType type) : Type(type) { }
		virtual ~ShapeNode();
		void InitializeNode(World& world) override;
		void Begin() override;
		virtual void OnNodeDestroy() override;
		void TickTransform(bool inverseParentTransform) override;

		void SetBodyType(BodyType type);
		BodyType GetBodyType() const;

		MEMBER()
		bool IsTrigger = false;

		MEMBER()
		float m_Mass = 1.0f;

		MEMBER()
		bool m_AllowSleep = true;

		MEMBER()
		float m_LinearDrag = 0.0f;

		MEMBER()
		float m_AngularDrag = 0.05f;

		MEMBER()
		bool m_IsContinuous = false;

		MEMBER()
		float m_GravityScale = 1.0f;

		MEMBER()
		bool IsStatic = true;

		ShapeType GetType() const { return Type; }

		MEMBER() Delegate<ShapeNode*> OnTriggerEnter;
		MEMBER() Delegate<ShapeNode*> OnTriggerStay;
		MEMBER() Delegate<ShapeNode*> OnTriggerExit;

		MEMBER() Delegate<ShapeNode*> OnCollisionEnter;
		MEMBER() Delegate<ShapeNode*> OnCollisionStay;
		MEMBER() Delegate<ShapeNode*> OnCollisionExit;

		friend class Physics::PhysicsWorld;
	};
	
	class BoxShapeNode : public ShapeNode
	{
		SUORA_CLASS(1299861);

	public:
		BoxShapeNode() : ShapeNode(ShapeType::Box) { }

		Vec3 GetBoxExtends() const;

		MEMBER()
		Vec3 m_HalfExtends = Vec::One * 0.5f;
	};
	class SphereShapeNode : public ShapeNode
	{
		SUORA_CLASS(78954322);

	public:
		SphereShapeNode() : ShapeNode(ShapeType::Sphere) { }

		float GetSphereRadius() const;

		MEMBER()
		float m_Radius = 0.5f;
	};
	class CapsuleShapeNode : public ShapeNode
	{
		SUORA_CLASS(7655212);

	public:
		CapsuleShapeNode() : ShapeNode(ShapeType::Capsule) { }

	};

}