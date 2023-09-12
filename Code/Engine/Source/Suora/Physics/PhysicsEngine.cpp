#include "Precompiled.h"
#include "PhysicsEngine.h"
#include <glm/gtx/matrix_decompose.hpp>

#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Nodes/PhysicsNodes.h"

#include <entt.hpp>
#include "GJK.h"

static Suora::Physics::ManifoldPoints GetMaxPen(std::vector<Suora::Physics::ManifoldPoints>& manifolds)
{
	if (manifolds.size() == 0) return Suora::Physics::ManifoldPoints(); // exit if no collision

	size_t maxNormalIndex = 0;
	float  maxNormalDist = FLT_MIN;

	for (size_t i = 0; i < manifolds.size(); i++) 
	{
		if (manifolds[i].Depth > maxNormalDist) 
		{
			maxNormalDist = manifolds[i].Depth;
			maxNormalIndex = i;
		}
	}

	return manifolds[maxNormalIndex];
}

namespace CollsionTest
{
	static ::Suora::Physics::ManifoldPoints Sphere_Sphere(const glm::mat4& TransformA, const glm::mat4& TransformB, const Suora::Physics::SphereCollider& A, const Suora::Physics::SphereCollider& B)
	{
		{
			Suora::Vec3 scale;
			glm::quat rotation;
			Suora::Vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(TransformB, scale, rotation, translation, skew, perspective);
			if (scale.x != scale.y || scale.y != scale.z || scale.z != scale.x)
			{
				SuoraError("CollsionTest: Sphere_Sphere missing Sphere_MeshSphere due to non-uniform scale!");
			}
		}
		const Suora::Vec3 aCenter = Suora::Vec3(TransformA[3]);
		const Suora::Vec3 bCenter = Suora::Vec3(TransformB[3]);

		Suora::Vec3 ab = bCenter - aCenter;

		float aRadius = A.Radius;
		float bRadius = B.Radius;

		float distance = glm::length(ab);

		if (distance < 0.00001f
			|| distance > aRadius + bRadius)
		{
			return ::Suora::Physics::ManifoldPoints();
		}

		Suora::Vec3 normal = glm::normalize(ab);

		Suora::Vec3 aDeep = aCenter + normal * aRadius;
		Suora::Vec3 bDeep = bCenter - normal * bRadius;

		return ::Suora::Physics::ManifoldPoints(aDeep, bDeep);
	}

	static ::Suora::Physics::ManifoldPoints Mesh_Mesh(const glm::mat4& TransformA, const glm::mat4& TransformB, const Suora::Physics::MeshCollider& A, const Suora::Physics::MeshCollider& B)
	{
		std::vector<::Suora::Physics::ManifoldPoints> manifolds;

		auto [collision, simplex] = ::Suora::Physics::GJK(A.Shape, TransformA, B.Shape, TransformB);
		if (collision)
		{
			manifolds.push_back(::Suora::Physics::EPA(simplex, A.Shape, TransformA, B.Shape, TransformB));
		}

		return GetMaxPen(manifolds);
	}
	static ::Suora::Physics::ManifoldPoints Box_Box(const glm::mat4& TransformA, const glm::mat4& TransformB, const Suora::Physics::BoxCollider& A, const Suora::Physics::BoxCollider& B)
	{
		static ::Suora::ConvexShape BoxShape = ::Suora::ConvexShape::MakeBoxShape();
		std::vector<::Suora::Physics::ManifoldPoints> manifolds;

		auto [collision, simplex] = ::Suora::Physics::GJK(BoxShape, TransformA, BoxShape, TransformB);
		if (collision)
		{
			manifolds.push_back(::Suora::Physics::EPA(simplex, BoxShape, TransformA, BoxShape, TransformB));
		}

		return GetMaxPen(manifolds);
	}
	static ::Suora::Physics::ManifoldPoints Box_Capsule(const glm::mat4& TransformA, const glm::mat4& TransformB, const Suora::Physics::BoxCollider& A, const Suora::Physics::CapsuleCollider& B)
	{
		static ::Suora::ConvexShape BoxShape = ::Suora::ConvexShape::MakeBoxShape();
		static ::Suora::ConvexShape CapsuleShape = ::Suora::ConvexShape::MakeCapsuleShape(B.Radius, B.Height);
		std::vector<::Suora::Physics::ManifoldPoints> manifolds;

		auto [collision, simplex] = ::Suora::Physics::GJK(BoxShape, TransformA, CapsuleShape, TransformB);
		if (collision)
		{
			manifolds.push_back(::Suora::Physics::EPA(simplex, BoxShape, TransformA, CapsuleShape, TransformB));
		}

		return GetMaxPen(manifolds);
	}
}

namespace Suora
{

	Physics::AABB ConvexShape::GenerateAABB(const glm::mat4& transform)
	{
		Physics::AABB aabb = Physics::AABB(Vec3(transform[3]), Vec3(0.0001f));
		
		for (uint32_t i = 0; i < PointCount; i++)
		{
			const Vec3& point = Points[i];
			Vec3 worldPos = transform * Vec4(point, 1.0f);
			Vec3 toWorldPos = worldPos - Vec3(transform[3]);
			if (glm::abs(toWorldPos.x) > aabb.HalfExtends.x) aabb.HalfExtends.x = glm::abs(toWorldPos.x);
			if (glm::abs(toWorldPos.y) > aabb.HalfExtends.y) aabb.HalfExtends.y = glm::abs(toWorldPos.y);
			if (glm::abs(toWorldPos.z) > aabb.HalfExtends.z) aabb.HalfExtends.z = glm::abs(toWorldPos.z);
		}

		return aabb;
	}
}

namespace Suora::Physics
{

	PhysicsWorld::PhysicsWorld()
	{
		m_Entities = CreateScope<entt::registry>();
	}
	PhysicsWorld::~PhysicsWorld()
	{

	}

	static AABB CalcAABB(CollisionNode* node)
	{
		if (SphereCollisionNode* sphere = node->As<SphereCollisionNode>())
		{
			return AABB(sphere->GetPosition(), Vec3(sphere->m_Radius));
		}
		else if (BoxCollisionNode* box = node->As<BoxCollisionNode>())
		{
			static ConvexShape BoxShape = ConvexShape::MakeBoxShape();
			return BoxShape.GenerateAABB(box->GetTransformMatrix());
		}
		else if (CapsuleCollisionNode* capsule = node->As<CapsuleCollisionNode>())
		{
			static ConvexShape CapsuleShape = ConvexShape::MakeCapsuleShape(capsule->m_Radius, capsule->m_Height);
			return CapsuleShape.GenerateAABB(capsule->GetTransformMatrix());
		}
		else
		{
			SuoraError("void PhysicsWorld::TickCollider(CollisionNode*): Implementation missing!");
			return AABB(Vec::Zero, Vec::One);
		}
	}

	entt::entity PhysicsWorld::CreateCollider(CollisionNode* node)
	{
		entt::entity entity = m_Entities->create();
		m_Entities->emplace<Collider>(entity, Collider{ node->GetTransformMatrix(), CalcAABB(node), node, node->GetParentNodeOfClass<PhysicsNode>() });

		if (SphereCollisionNode* sphere = node->As<SphereCollisionNode>())
		{
			m_Entities->emplace<SphereCollider>(entity, SphereCollider{ sphere->m_Radius });
		}
		else if (BoxCollisionNode* box = node->As<BoxCollisionNode>())
		{
			m_Entities->emplace<BoxCollider>(entity, BoxCollider{});
		}
		else if (CapsuleCollisionNode* capsule = node->As<CapsuleCollisionNode>())
		{
			m_Entities->emplace<CapsuleCollider>(entity, CapsuleCollider{});
		}
		else
		{
			SuoraError("uint32_t PhysicsWorld::CreateCollider(CollisionNode*): Implementation missing!");
		}

		return entity;
	}

	void PhysicsWorld::DestroyCollider(CollisionNode* node)
	{
		if (m_Entities->valid(node->m_Index))
		{
			m_Entities->destroy(node->m_Index);
		}
		for (int32_t i = m_PostPhysicsTicksRequired.Last(); i >= 0; i--)
		{
			if (m_PostPhysicsTicksRequired[i] == node)
			{
				m_PostPhysicsTicksRequired.RemoveAt(i);
			}
		}
	}

	void PhysicsWorld::TickCollider(CollisionNode* node)
	{
		Collider& collider = m_Entities->get<Collider>(node->m_Index);
		collider.Transform = node->GetTransformMatrix();
		collider.Node = node;
		collider.ParentPhysicsNode = node->GetParentNodeOfClass<PhysicsNode>();
		collider.aabb = CalcAABB(node);

		if (SphereCollisionNode* sphere = node->As<SphereCollisionNode>())
		{
			SphereCollider& col = m_Entities->get<SphereCollider>(node->m_Index);
			col.Radius = sphere->m_Radius;
		}
		else if (BoxCollisionNode* box = node->As<BoxCollisionNode>())
		{
		}
		else if (CapsuleCollisionNode* capsule = node->As<CapsuleCollisionNode>())
		{
		}
		else
		{
			SuoraError("void PhysicsWorld::TickCollider(CollisionNode*): Implementation missing!");
		}
	}

	void PhysicsWorld::Update(float deltaTime)
	{
		s_InPhysicsSimulation = true;

		m_Accumulator += deltaTime;
		while (m_Accumulator >= m_TimeStep)
		{
			Step(m_TimeStep);
			m_Accumulator -= m_TimeStep;
		}

		s_InPhysicsSimulation = false;

		for (auto& It : m_CollisionEventDispatches)
		{
			if (It.Collider)
			{
				It.Collider->HandleCollisionEvent(It.Body, It.Other);
			}
		}
		m_CollisionEventDispatches.Clear();

		for (int i = m_PostPhysicsTicksRequired.Last(); i >= 0; i--)
		{
			if (m_PostPhysicsTicksRequired[i].Get() == nullptr)
			{
				m_PostPhysicsTicksRequired.RemoveAt(i);
			}
			else
			{
				m_PostPhysicsTicksRequired[i]->PostPhysicsTick();
			}
		}

		/*
		auto& view = m_Entities->view<Rigidbody>();
		for (auto& entity : view)
		{
			auto& rigidbody = view.get<Rigidbody>(entity);
			Vec3& position = *(Vec3*)&rigidbody.Transform[3];

			rigidbody.Force += rigidbody.Mass * m_Gravity;

			rigidbody.Velocity += rigidbody.Force / rigidbody.Mass * deltaTime;
			position += rigidbody.Velocity * deltaTime;

			rigidbody.Force = Vec::Zero;

		}

		for (auto& entity : view)
		{
			// Pushresults
			auto& rigidbody = view.get<Rigidbody>(entity);
			rigidbody.Node->SetTransformMatrix(rigidbody.Transform);
		}*/

	}

	void PhysicsWorld::Step(float deltaTime)
	{
		for (PhysicsNode* node : m_PhysicsNodes)
		{
			if (!node->IsEnabled()) continue;

			RigidbodyNode* rigidbody = node->As<RigidbodyNode>();
			CharacterNode* character = node->As<CharacterNode>();

			if (character)
			{
				character->m_MovementVelocity = Math::Lerp(character->m_MovementVelocity, character->m_MovementDirection * (double)character->m_MovementSpeed, (double)(15.0f * deltaTime));
				if (character->m_MovementVelocity != (Vec3d)Vec::Zero)
				{
					character->m_WorldTransformMatrix[3] += Vec4(character->m_MovementVelocity * (double)deltaTime, 0.0f);
				}
				character->m_MovementDirection = Vec::Zero;
				character->AngularVelocity = Vec::Zero;
				character->Grounded = false;
			}

			if (rigidbody)
			{
				rigidbody->Force += rigidbody->m_Mass * m_Gravity;

				rigidbody->Velocity += rigidbody->Force / (double)rigidbody->m_Mass * (double)deltaTime;
				rigidbody->Velocity *= glm::pow(rigidbody->VelocityDamping, deltaTime);
				rigidbody->AngularVelocity *= glm::pow(rigidbody->VelocityDamping, deltaTime);
				node->m_WorldTransformMatrix[3] += Vec4(rigidbody->Velocity * (double)deltaTime, 0.0f);
				if (rigidbody->AngularVelocity != (Vec3d)Vec::Zero && !character)
				{
					node->m_WorldTransformMatrix = glm::rotate(node->m_WorldTransformMatrix, deltaTime * glm::length((Vec3)rigidbody->AngularVelocity), (Vec3)rigidbody->AngularVelocity);
				}
				node->TickTransform(true);

				rigidbody->Force = Vec::Zero;

				// Collision
				Array<CollisionNode*> collider = rigidbody->GetChildNodesOfClass<CollisionNode>();
				Array<entt::entity> childCollider;
				for (auto& It : collider)
				{
					if (!It->IsTrigger)
					{
						childCollider.Add(It->m_Index);
					}
				}

				SuoraVerify(rigidbody);
				Array<CollisionNode*> excludeFromCollisionCheck;
				for (int i = 0; i < rigidbody->SolvingIterations; i++)
				{
					ManifoldPoints manifold = CheckCollisions(childCollider, excludeFromCollisionCheck);
					bool breakNextIter = false;

					// Response
					if (manifold.HasCollision)
					{
						RigidbodyNode* aBody = rigidbody;
						RigidbodyNode* bBody = manifold.Other->As<RigidbodyNode>();

						float aStatic = (float)(int)(!aBody);
						float bStatic = (float)(int)(!bBody);

						if (aBody)
						{
							if (!manifold.ColliderA->IsTrigger && !manifold.ColliderB->IsTrigger)
							{
								ResolveCollisions(Array<ManifoldPoints>{manifold}, aBody, aBody->As<CharacterNode>(), bBody, aStatic);
							}
							else if (manifold.ColliderB->IsTrigger)
							{
								excludeFromCollisionCheck.Add(manifold.ColliderB);
							}

							// Collision Event
							if (manifold.ColliderB) m_CollisionEventDispatches.Add(CollisionEventDispatch{ manifold.ColliderB , aBody, manifold.ColliderA });
								// ^^^^^ manifold.ColliderB->HandleCollisionEvent(aBody, manifold.ColliderA);
						}
						manifold.SwapPoints();
						if (bBody)
						{
							if (!manifold.ColliderA->IsTrigger && !manifold.ColliderB->IsTrigger)
							{
								ResolveCollisions(Array<ManifoldPoints>{manifold}, bBody, bBody->As<CharacterNode>(), aBody, bStatic);
							}
						}

					}
					else
					{
						breakNextIter = true;
					}
					for (auto& It : collider)
					{
						TickCollider(It);
					}
					if (breakNextIter) break;
				}
			}

		}
	}

	void PhysicsWorld::ResolveCollisions(const Array<ManifoldPoints>& Manifolds, RigidbodyNode* body, CharacterNode* character, RigidbodyNode* otherBody, float isStatic)
	{
		const double percent = character ? 0.95f : 0.8f;
		const double slop = character ? 0.002f : 0.0001f;
		const double InvMassSum = 1.0f; // (1.0f / body->m_Mass) + (otherBody ? (1.0f / otherBody->m_Mass) : 0.0f);
		const Vec3d resolution = Manifolds[0].Normal * Manifolds[0].Depth / glm::max(1.0, 1.0);
		Vec3d correction = Manifolds[0].Normal * percent * fmax(Manifolds[0].Depth - slop, 0.0f) / (InvMassSum);
		const double Iter = (1.0f / double(body->SolvingIterations));

		body->m_WorldTransformMatrix[3] -= Vec4(correction, 0.0f) * (float)(character ? 1.0f : Iter);

		body->Velocity -= correction * Iter;
		body->Force -= ((double)body->m_Mass) * Vec3d(m_Gravity) * Iter;
		if (character)
		{
			if (glm::dot(glm::normalize(-Vec3d(m_Gravity)), glm::normalize(Manifolds[0].Normal)) < -0.85f)
			{
				body->Velocity = Vec::Zero;
				character->Grounded = true;
			}
		}

		//Vec3 a = Manifolds[0].A - Vec3(body->m_WorldTransformMatrix[3]); a = glm::normalize(a);
		//Vec3 b = (Manifolds[0].A + glm::normalize(Manifolds[0].Normal) * glm::pow(glm::length(body->Velocity), 2.f)) - Vec3(body->m_WorldTransformMatrix[3]); b = glm::normalize(b);
		//body->AngularVelocity = glm::eulerAngles(glm::angleAxis(glm::acos(glm::dot(a, b) / glm::length(a) * glm::length(b)), glm::cross(a, b)));
		body->TickTransform(true);
	}

	ManifoldPoints PhysicsWorld::CheckCollisions(const Array<entt::entity>& In, const Array<CollisionNode*>& exclude)
	{
		std::vector<ManifoldPoints> manifolds;
		auto& view = m_Entities->view<Collider>();
		for (entt::entity It : In)
		{
			Collider& collider = view.get<Collider>(It);
			for (entt::entity entity : view)
			{
				if (In.Contains(entity)) continue;
				Collider& other = view.get<Collider>(entity);

				if (exclude.Contains(other.Node)) continue;
				if (It == entity) continue;
				if (collider.aabb != other.aabb) continue;

				// Collision Check
				{
					if (m_Entities->has<SphereCollider>(It) && m_Entities->has<SphereCollider>(entity))
					{
						auto& view = m_Entities->view<Collider, SphereCollider>();
						const float dist = glm::distance(Vec3(collider.Transform[3]), Vec3(other.Transform[3]));
						if (dist < view.get<SphereCollider>(It).Radius + view.get<SphereCollider>(entity).Radius)
						{
							ManifoldPoints manifold = CollsionTest::Sphere_Sphere(collider.Transform, other.Transform, view.get<SphereCollider>(It), view.get<SphereCollider>(entity));
							manifold.Other = other.ParentPhysicsNode;
							manifold.ColliderA = collider.Node;
							manifold.ColliderB = other.Node;

							manifolds.push_back(manifold);
						}
					}
					else if (m_Entities->has<BoxCollider>(It) && m_Entities->has<BoxCollider>(entity))
					{
						auto& view = m_Entities->view<Collider, BoxCollider>();
						ManifoldPoints manifold = CollsionTest::Box_Box(collider.Transform, other.Transform, view.get<BoxCollider>(It), view.get<BoxCollider>(entity));
						manifold.Other = other.ParentPhysicsNode;
						manifold.ColliderA = collider.Node;
						manifold.ColliderB = other.Node;

						manifolds.push_back(manifold);
					}
					else if (m_Entities->has<BoxCollider>(It) && m_Entities->has<CapsuleCollider>(entity))
					{
						auto& view = m_Entities->view<Collider, BoxCollider>();
						auto& view2 = m_Entities->view<Collider, CapsuleCollider>();
						ManifoldPoints manifold = CollsionTest::Box_Capsule(collider.Transform, other.Transform, view.get<BoxCollider>(It), view2.get<CapsuleCollider>(entity));
						manifold.Other = other.ParentPhysicsNode;
						manifold.ColliderA = collider.Node;
						manifold.ColliderB = other.Node;

						manifolds.push_back(manifold);
					}
					else if (m_Entities->has<CapsuleCollider>(It) && m_Entities->has<BoxCollider>(entity))
					{
						auto& view = m_Entities->view<Collider, BoxCollider>();
						auto& view2 = m_Entities->view<Collider, CapsuleCollider>();
						ManifoldPoints manifold = CollsionTest::Box_Capsule(collider.Transform, other.Transform, view.get<BoxCollider>(entity), view2.get<CapsuleCollider>(It));
						manifold.Other = other.ParentPhysicsNode;
						manifold.ColliderA = collider.Node;
						manifold.ColliderB = other.Node;

						manifolds.push_back(manifold);
					}
					else
					{
						SuoraError("CollisionResponseRequest PhysicsWorld::CheckCollisions(const ArrayList<entt::entity>&): Missing an Implementation!");
					}
				}
			}
		}

		return GetMaxPen(manifolds);
	}

	static bool RayAABBIntersection(const AABB& aabb, const Vec3& rayPos, const Vec3& rayDir) 
	{
		float tx1 = (aabb.Min().x - rayPos.x) * (1.0f / rayDir).x;
		float tx2 = (aabb.Max().x - rayPos.x) * (1.0f / rayDir).x;

		float tmin = glm::min(tx1, tx2);
		float tmax = glm::max(tx1, tx2);

		float ty1 = (aabb.Min().y - rayPos.y) * (1.0f / rayDir).y;
		float ty2 = (aabb.Max().y - rayPos.y) * (1.0f / rayDir).y;

		tmin = glm::max(tmin, glm::min(ty1, ty2));
		tmax = glm::min(tmax, glm::max(ty1, ty2));

		return tmax >= tmin;
	}

	static bool RayTriangleIntersect(const glm::vec3& orig, const glm::vec3& dir, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t)
	{
		float u, v = 0.0f;
		Vec3 v0v1 = v1 - v0;
		Vec3 v0v2 = v2 - v0;
		Vec3 pvec = glm::cross(dir, v0v2);
		float det = glm::dot(v0v1, pvec);

		// ray and triangle are parallel if det is close to 0
		if (fabs(det) < glm::epsilon<float>()) return false;
		float invDet = 1 / det;

		Vec3 tvec = orig - v0;
		u = glm::dot(tvec, pvec) * invDet;
		if (u < 0 || u > 1) return false;

		Vec3 qvec = glm::cross(tvec, v0v1);
		v = glm::dot(dir, qvec) * invDet;
		if (v < 0 || u + v > 1) return false;

		t = glm::dot(v0v2, qvec) * invDet;

		return true;
	}

	static Vec3 Ray_ConvexShapeHitPoint(const Vec3& start, const Vec3& end, ConvexShape& shape, const glm::mat4& transform)
	{
		const Vec3 direction = glm::normalize(end - start);

		Array<Vec3> projectedPoints;
		Array<float> projectedPointTimes;
		for (uint32_t i = 0; i < shape.PointCount; i++)
		{
			const Vec3 p = Vec3(Vec4(shape.Points[i], 1.0f) * glm::transpose(transform));
			projectedPoints.Add(p);
			const float t = glm::dot(p - start, direction) / glm::dot(direction, direction);
			projectedPointTimes.Add(t);
		}

		bool bDone = false;
		while (!bDone)
		{
			bDone = true;
			for (uint32_t i = 1; i < shape.PointCount; i++)
			{
				if (projectedPointTimes[i] < projectedPointTimes[i - 1])
				{
					projectedPoints.SwapElements(i, i - 1);
					projectedPointTimes.SwapElements(i, i - 1); 
					bDone = false;
				}
			}

		}

		for (uint32_t i = 0; i < shape.PointCount; i++)
		{
			for (uint32_t j = 1; j < shape.PointCount; j++)
			{
				for (uint32_t k = 2; k < shape.PointCount; k++)
				{
					if (i == j || j == k || k == i) continue;

					float t = 0.0f;
					if (RayTriangleIntersect(start, direction, projectedPoints[i], projectedPoints[j], projectedPoints[k], t))
					{
						const Vec3 hitPoint = start + t * direction;
						return hitPoint;
					}
				}
			}
		}

		return Vec::Zero;
	}

	bool PhysicsWorld::Raycast(const Vec3& start, const Vec3& end, HitResult& result, const RaycastParams& params)
	{
		bool bRayCastSuccess = false;
		ConvexShape rayShape;
		rayShape.Points[0] = start;
		rayShape.Points[1] = end;
		rayShape.PointCount = 2;
		static glm::mat4 identityMat4 = glm::mat4(1);

		auto& view = m_Entities->view<Collider>();
		for (entt::entity It : view)
		{
			Collider& collider = view.get<Collider>(It);
			if (!RayAABBIntersection(collider.aabb, start, end - start)) continue;
			if (params.IgnoredCollisionNodes.Contains(collider.Node)) continue;

			// Collision Check
			{
				if (m_Entities->has<BoxCollider>(It))
				{
					static ::Suora::ConvexShape BoxShape = ::Suora::ConvexShape::MakeBoxShape();
					auto [collision, simplex] = ::Suora::Physics::GJK(rayShape, identityMat4, BoxShape, collider.Transform);
					if (collision)
					{
						ManifoldPoints manifold = ::Suora::Physics::EPA(simplex, rayShape, identityMat4, BoxShape, collider.Transform);
						if (manifold.HasCollision)
						{
							manifold.A = Ray_ConvexShapeHitPoint(start, end, BoxShape, collider.Transform);
							if (!bRayCastSuccess || glm::distance(start, Vec3(manifold.A)) < glm::distance(start, result.Point))
							{
								result.Point = manifold.A;
								result.Normal = manifold.Normal;
								result.Collider = collider.Node;
								result.PhysicsBody = collider.ParentPhysicsNode;
							}
							bRayCastSuccess = true;
						}
						
					}
				}
				else
				{
					SUORA_ASSERT(false, "PhysicsWorld::Raycast(): Missing Implementation!");
					SuoraError("PhysicsWorld::Raycast(): Missing Implementation!");
				}
			}

		}


		return bRayCastSuccess;
	}

	bool AABB::operator!=(const AABB& other) const
	{
		return !(operator==(other));
	}

	bool AABB::operator==(const AABB& other) const
	{
#define AABB_MIN(aabb, comp) ((##aabb).Center.##comp - (##aabb).HalfExtends.##comp)
#define AABB_MAX(aabb, comp) ((##aabb).Center.##comp + (##aabb).HalfExtends.##comp)
		return  (AABB_MIN(*this, x) <= AABB_MAX(other, x) && AABB_MAX(*this, x) >= AABB_MIN(other, x)) &&
				(AABB_MIN(*this, y) <= AABB_MAX(other, y) && AABB_MAX(*this, y) >= AABB_MIN(other, y)) &&
				(AABB_MIN(*this, z) <= AABB_MAX(other, z) && AABB_MAX(*this, z) >= AABB_MIN(other, z));
	}

}