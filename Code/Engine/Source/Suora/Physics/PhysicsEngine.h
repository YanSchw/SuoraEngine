#pragma once
#include <unordered_map>
#include "Suora/Common/VectorUtils.h"
#include "Suora/Common/Array.h"
#include "Suora/Common/Math.h"
#include "Suora/Core/Object/Pointer.h"

namespace entt
{
	template<typename Entity>
	class basic_registry;
	enum class entity : uint32_t;
}
namespace Suora::Physics
{
	struct AABB;
}

namespace Suora
{
	class Node;
	class CollisionNode;
	class SphereCollisionNode;
	class PhysicsNode;
	class RigidbodyNode;
	class CharacterNode;
	enum class CollsionNodeType : uint32_t;

	struct HitResult;
	struct RaycastParams;

	struct ConvexShape
	{
		Vec3 Points[512] = { Vec::Zero };
		uint32_t PointCount = 0;

		Vec3 FindFurthestPoint(const glm::mat4& transform, const Vec3& direction) const
		{
			Vec3 maxPoint = Vec3(0);
			float maxDistance = -FLT_MAX;

			for (int i = 0; i < PointCount; i++)
			{
				const Vec3& point = Points[i];
				Vec3 p = Vec3(Vec4(point, 1.0f) * glm::transpose(transform));

				float distance = glm::dot(p, direction);
				if (distance > maxDistance) {
					maxDistance = distance;
					maxPoint = p;
				}
			}

			return maxPoint;
		}

		Physics::AABB GenerateAABB(const glm::mat4& transform);

		inline static ConvexShape MakeBoxShape() 
		{
			ConvexShape shape;
			shape.Points[0] = 0.5f * glm::vec3(-1, -1, -1);
			shape.Points[1] = 0.5f * glm::vec3(-1, 1, -1);
			shape.Points[2] = 0.5f * glm::vec3(1, 1, -1);
			shape.Points[3] = 0.5f * glm::vec3(1, -1, -1);

			shape.Points[7] = 0.5f * glm::vec3(-1, -1, 1);
			shape.Points[6] = 0.5f * glm::vec3(1, -1, 1);
			shape.Points[5] = 0.5f * glm::vec3(1, 1, 1);
			shape.Points[4] = 0.5f * glm::vec3(-1, 1, 1);
			shape.PointCount = 8;

			return shape;
		}
		inline static ConvexShape MakeSphereShape(float radius)
		{
			ConvexShape shape;
			const int64_t n_stacks = 32; const int64_t n_slices = 16;
			// add top vertex
			shape.Points[0] = Vec3(0, 1, 0);

			// generate vertices per stack / slice
			int64_t lastIndex = 0;
			for (int64_t i = 0; i < n_stacks - 1; i++)
			{
				auto phi = Math::PI * double(i + 1) / double(n_stacks);
				for (int j = 0; j < n_slices; j++)
				{
					auto theta = 2.0 * Math::PI * double(j) / double(n_slices);
					auto x = std::sin(phi) * std::cos(theta);
					auto y = std::cos(phi);
					auto z = std::sin(phi) * std::sin(theta);
					shape.Points[i * n_slices + j] = Vec3(x, y, z);
					lastIndex = i * n_slices + j;
				}
			}

			// add bottom vertex
			shape.Points[++lastIndex] = Vec3(0, -1, 0);
			shape.PointCount = ++lastIndex;

			return shape;
		}
		inline static ConvexShape MakeCapsuleShape(float radius, float height)
		{
			ConvexShape shape;
			shape.Points[0] = 0.5f * glm::vec3(-1, -1, -1);
			shape.Points[1] = 0.5f * glm::vec3(-1, 1, -1);
			shape.Points[2] = 0.5f * glm::vec3(1, 1, -1);
			shape.Points[3] = 0.5f * glm::vec3(1, -1, -1);

			shape.Points[7] = 0.5f * glm::vec3(-1, -1, 1);
			shape.Points[6] = 0.5f * glm::vec3(1, -1, 1);
			shape.Points[5] = 0.5f * glm::vec3(1, 1, 1);
			shape.Points[4] = 0.5f * glm::vec3(-1, 1, 1);

			shape.Points[8] = 0.5f * glm::vec3(glm::sqrt(2.0f), -1, 0);
			shape.Points[9] = 0.5f * glm::vec3(-glm::sqrt(2.0f), -1, 0);
			shape.Points[10] = 0.5f * glm::vec3(0, -1, glm::sqrt(2.0f));
			shape.Points[11] = 0.5f * glm::vec3(0, -1, -glm::sqrt(2.0f));

			shape.Points[12] = 0.5f * glm::vec3(0, 1, glm::sqrt(2.0f));
			shape.Points[13] = 0.5f * glm::vec3(0, 1, -glm::sqrt(2.0f));
			shape.Points[14] = 0.5f * glm::vec3(glm::sqrt(2.0f), 1, 0);
			shape.Points[15] = 0.5f * glm::vec3(-glm::sqrt(2.0f), 1, 0);
			//..
			shape.Points[16] = 0.15f * glm::vec3(-1, -(1.0f / 0.15f), -1);
			shape.Points[17] = 0.15f * glm::vec3(-1, (1.0f / 0.15f), -1);
			shape.Points[18] = 0.15f * glm::vec3(1, (1.0f / 0.15f), -1);
			shape.Points[19] = 0.15f * glm::vec3(1, -(1.0f / 0.15f), -1);

			shape.Points[20] = 0.15f * glm::vec3(-1, -(1.0f / 0.15f), 1);
			shape.Points[21] = 0.15f * glm::vec3(1, -(1.0f / 0.15f), 1);
			shape.Points[22] = 0.15f * glm::vec3(1, (1.0f / 0.15f), 1);
			shape.Points[23] = 0.15f * glm::vec3(-1, (1.0f / 0.15f), 1);

			shape.Points[24] = 0.15f * glm::vec3(glm::sqrt(2.0f), -(1.0f / 0.15f), 0);
			shape.Points[25] = 0.15f * glm::vec3(-glm::sqrt(2.0f), -(1.0f / 0.15f), 0);
			shape.Points[26] = 0.15f * glm::vec3(0, -(1.0f / 0.15f), glm::sqrt(2.0f));
			shape.Points[27] = 0.15f * glm::vec3(0, -(1.0f / 0.15f), -glm::sqrt(2.0f));

			shape.Points[28] = 0.15f * glm::vec3(0, (1.0f / 0.15f), glm::sqrt(2.0f));
			shape.Points[29] = 0.15f * glm::vec3(0, (1.0f / 0.15f), -glm::sqrt(2.0f));
			shape.Points[30] = 0.15f * glm::vec3(glm::sqrt(2.0f), (1.0f / 0.15f), 0);
			shape.Points[31] = 0.15f * glm::vec3(-glm::sqrt(2.0f), (1.0f / 0.15f), 0);
			shape.PointCount = 32;

			return shape;

			{
				ConvexShape shape = MakeSphereShape(radius);

				const float extendedHeight = height - radius * 2.0f;
				if (extendedHeight > 0.0f)
				{
					for (int i = 0; i < shape.PointCount; i++)
					{
						shape.Points[i].y += extendedHeight * (shape.Points[i].y > 0.0f ? +1.0f : -1.0f);
					}
				}

				return shape;
			}
		}
	};
}

namespace Suora::Physics
{
	struct ManifoldPoints 
	{
		Vec3d A;
		Vec3d B;
		Vec3d Normal;
		double Depth;
		bool HasCollision;
		PhysicsNode* Other = nullptr;
		CollisionNode* ColliderA = nullptr;
		CollisionNode* ColliderB = nullptr;

		ManifoldPoints()
			: A(0.0f), B(0.0f), Normal(0.0f), Depth(0.0f), HasCollision(false)
		{}


		ManifoldPoints(const Vec3d& a, const Vec3d& b)
			: A(a), B(b), HasCollision(true)
		{
			glm::dvec3 ba = a - b;
			Depth = glm::length(ba);
			if (Depth > 0.00001f)
			{
				Normal = ba / Depth;
			}
			else {
				Normal = glm::dvec3(0, 1, 0);
				Depth = 1;
			}
		}

		ManifoldPoints(const Vec3d& a, const Vec3d& b, const Vec3d& n, float d)
			: A(a), B(b), Normal(n), Depth(d), HasCollision(true)
		{}

		void SwapPoints()
		{
			{
				Vec3d t = A;
				A = B;
				B = t;
			}
			{
				CollisionNode* temp = ColliderA;
				ColliderA = ColliderB;
				ColliderB = temp;
			}
			Normal = -Normal;
		}
	};
	struct AABB
	{
		Vec3 Center = Vec::Zero;
		Vec3 HalfExtends = Vec::One * 0.5f;
		
		AABB(const Vec3& center, const Vec3& halfExtends)
			: Center(center), HalfExtends(halfExtends)
		{
		}
		bool operator!=(const AABB& other) const;
		bool operator==(const AABB& other) const;
		Vec3 Min() const { return Center - HalfExtends; }
		Vec3 Max() const { return Center + HalfExtends; }
	};

	struct Collider
	{
		glm::mat4 Transform = glm::mat4();
		AABB aabb = AABB(Vec::Zero, Vec::One);
		CollisionNode* Node = nullptr;
		PhysicsNode* ParentPhysicsNode = nullptr;
	};
	struct SphereCollider
	{
		float Radius = 1;
	};
	struct BoxCollider
	{
		// Just needs some data....
	private: char _ = -1;
	};
	struct CapsuleCollider
	{
		float Radius = .5f;
		float Height = 2;
	};
	struct MeshCollider
	{
		ConvexShape Shape;
	};

	class PhysicsWorld
	{
	public:
		PhysicsWorld();
		~PhysicsWorld();

		entt::entity CreateCollider(CollisionNode* node);
		void DestroyCollider(CollisionNode* node);
		void TickCollider(CollisionNode* node);

		void Update(float deltaTime);
		void Step(float deltaTime);
		void ResolveCollisions(const Array<ManifoldPoints>& Manifolds, RigidbodyNode* body, CharacterNode* character, RigidbodyNode* otherBody, float isStatic);
		ManifoldPoints CheckCollisions(const Array<entt::entity>& In, const Array<CollisionNode*>& exclude);

		bool Raycast(const Vec3& start, const Vec3& end, HitResult& result, const RaycastParams& params);

		Array<PhysicsNode*> m_PhysicsNodes;

		Scope<entt::basic_registry<entt::entity>> m_Entities;
		Vec3 m_Gravity = Vec3(0, -9.81f, 0);

		float m_Accumulator = 0.0f;
		float m_TimeStep = 1.0f / 60.0f;
		inline static bool s_InPhysicsSimulation = false;

	private:

		struct CollisionEventDispatch
		{
			Ptr<CollisionNode> Collider;
			Ptr<RigidbodyNode> Body;
			Ptr<CollisionNode> Other;
		};

		Array<CollisionEventDispatch> m_CollisionEventDispatches;
		Array<Ptr<CollisionNode>> m_PostPhysicsTicksRequired;


		friend class ::Suora::CollisionNode;
	};

}