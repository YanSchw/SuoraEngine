#include "Precompiled.h"
#include "PhysicsWorld.h"
#include <glm/gtx/matrix_decompose.hpp>

#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Nodes/PhysicsNodes.h"

namespace Suora::Physics
{

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

		/*for (auto& It : m_CollisionEventDispatches)
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
		}*/

	}

	bool PhysicsWorld::Raycast(const Vec3& start, const Vec3& end, HitResult& result, const RaycastParams& params)
	{
		SuoraVerify(false, "Not implemented!");
		return false;
	}


}