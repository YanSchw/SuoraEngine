#include "Precompiled.h"
#include "PhysicsEngine.h"
#include "PhysicsWorld.h"

namespace Suora::Physics
{

	Ref<PhysicsEngine> PhysicsEngine::Create()
	{
		return Ref<PhysicsEngine>(New(s_PhysicsEngineClass)->As<PhysicsEngine>());
	}

	Ref<class PhysicsWorld> PhysicsEngine::CreatePhysicsWorld()
	{
		SuoraVerify(false, "Not implemented!");
		return nullptr;
	}

}