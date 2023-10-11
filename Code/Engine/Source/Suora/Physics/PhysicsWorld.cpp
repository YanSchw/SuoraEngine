#include "Precompiled.h"
#include "PhysicsWorld.h"
#include <glm/gtx/matrix_decompose.hpp>

#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"

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

	}

	bool PhysicsWorld::Raycast(const Vec3& start, const Vec3& end, HitResult& result, const RaycastParams& params)
	{
		SuoraVerify(false, "Not implemented!");
		return false;
	}


}