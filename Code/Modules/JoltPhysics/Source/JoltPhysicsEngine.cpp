#include "JoltPhysicsEngine.h"
#include "JoltPhysicsWorld.h"

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

// STL includes
#include <iostream>
#include <cstdarg>
#include <thread>

namespace Suora
{
	struct SetPhysicsEngineClassStruct
	{
		SetPhysicsEngineClassStruct()
		{
			Physics::PhysicsEngine::s_PhysicsEngineClass = Physics::JoltPhysicsEngine::StaticClass();
		}
	};
	static SetPhysicsEngineClassStruct s_SetPhysicsEngineClassStruct;

}

namespace Suora::Physics
{
	void JoltPhysicsEngine::Initialize()
	{
		m_WasInitialized = true;
		JPH::RegisterDefaultAllocator();

		JPH::Factory::sInstance = new JPH::Factory();

		JPH::RegisterTypes();
	}
	JoltPhysicsEngine::~JoltPhysicsEngine()
	{
		if (!m_WasInitialized) return;

		JPH::UnregisterTypes();

		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;

	}
	Ref<class PhysicsWorld> JoltPhysicsEngine::CreatePhysicsWorld()
	{
		Ref<JoltPhysicsWorld> world = CreateRef<JoltPhysicsWorld>();
		world->Inititalize();
		return world;
	}

}
