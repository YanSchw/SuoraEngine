#pragma once
#include <unordered_map>
#include "Suora/Core/Base.h"
#include "Suora/Common/VectorUtils.h"
#include "Suora/Common/Array.h"
#include "Suora/Common/Math.h"
#include "Suora/Core/Object/Pointer.h"
#include "PhysicsWorld.generated.h"

namespace Suora
{
	class Node;
	class ShapeNode;
	class CharacterNode;
	class CharacterController;
	class BoxShapeNode;
	enum class ShapeType : uint32_t;

	struct HitResult;
	struct RaycastParams;
}

namespace Suora::Physics
{

	class PhysicsWorld : public Object
	{
		SUORA_CLASS(42387984338);
	public:
		virtual ~PhysicsWorld() { }
		virtual void CreateShapeNode(ShapeNode* node) { SuoraVerify(false, "Not implemented!"); }
		virtual void DestroyShapeNode(ShapeNode* node) { SuoraVerify(false, "Not implemented!"); }
		virtual void TickShapeNode(ShapeNode* node) { SuoraVerify(false, "Not implemented!"); }

		virtual Ref<CharacterController> CreateCharacterNode(CharacterNode* node) { SuoraVerify(false, "Not implemented!"); return nullptr; }
		virtual void DestroyCharacterNode(CharacterNode* node) { SuoraVerify(false, "Not implemented!"); }
		virtual void TickCharacterNode(CharacterNode* node) { SuoraVerify(false, "Not implemented!"); }

		void Update(float deltaTime);
		virtual void Step(double timeStep)
		{
		}

		virtual bool Raycast(const Vec3& start, const Vec3& end, HitResult& result, const RaycastParams& params);

		Vec3 m_Gravity = Vec3(0, -9.81f, 0);

		float m_Accumulator = 0.0f;
		float m_TimeStep = 1.0f / 60.0f;
		inline static bool s_InPhysicsSimulation = false;

	private:

		/*struct CollisionEventDispatch
		{
			Ptr<CollisionNode> Collider;
			Ptr<RigidbodyNode> Body;
			Ptr<CollisionNode> Other;
		};

		Array<CollisionEventDispatch> m_CollisionEventDispatches;
		Array<Ptr<CollisionNode>> m_PostPhysicsTicksRequired;*/

		friend class ::Suora::ShapeNode;
	};

}