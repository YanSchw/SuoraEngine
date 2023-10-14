#include "JoltPhysicsWorld.h"
#include "JoltPhysicsEngine.h"
#include "JoltCharacterController.h"

#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/Nodes/CharacterNode.h"

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
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

// STL includes
#include <iostream>
#include <cstdarg>
#include <thread>

#include "JoltTypeConversion.h"

namespace Suora::Physics
{

	struct Physics3DLayer
	{
		static constexpr uint8_t STATIC = 0;
		static constexpr uint8_t DEFAULT = 1;
		static constexpr uint8_t OTHER2 = 2;
		static constexpr uint8_t OTHER3 = 3;
		static constexpr uint8_t OTHER4 = 4;
		static constexpr uint8_t OTHER5 = 5;
		static constexpr uint8_t OTHER6 = 6;
		static constexpr uint8_t OTHER7 = 7;
		static constexpr uint8_t OTHER8 = 8;
		static constexpr uint8_t OTHER9 = 9;
		static constexpr uint8_t OTHER10 = 10;
		static constexpr uint8_t OTHER11 = 11;
		static constexpr uint8_t OTHER12 = 12;
		static constexpr uint8_t OTHER13 = 13;
		static constexpr uint8_t OTHER14 = 14;
		static constexpr uint8_t OTHER15 = 15;

		static constexpr uint8_t NUM_LAYERS = 16;
	};

	namespace BroadPhaseLayers
	{
		static constexpr JPH::BroadPhaseLayer STATIC(0);
		static constexpr JPH::BroadPhaseLayer DEFAULT(1);
		static constexpr JPH::uint NUM_LAYERS = 3;
	};

	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			mObjectToBroadPhase[Physics3DLayer::STATIC] = BroadPhaseLayers::STATIC;
			mObjectToBroadPhase[Physics3DLayer::DEFAULT] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER2] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER3] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER4] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER5] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER6] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER7] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER8] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER9] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER10] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER11] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER12] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER13] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER14] = BroadPhaseLayers::DEFAULT;
			mObjectToBroadPhase[Physics3DLayer::OTHER15] = BroadPhaseLayers::DEFAULT;
		}

		JPH::uint GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			SuoraAssert(inLayer < Physics3DLayer::NUM_LAYERS)
			return mObjectToBroadPhase[inLayer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
		{
			switch ((BroadPhaseLayer::Type)inLayer)
			{
			case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
			case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
			default:													JPH_ASSERT(false); return "INVALID";
			}
		}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:
		JPH::BroadPhaseLayer mObjectToBroadPhase[Physics3DLayer::NUM_LAYERS];
	};

	static bool Physics3DBroadPhaseCanCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2)
	{
		/*if (inLayer1 == Physics3DLayer::STATIC)
			return inLayer2 != BroadPhaseLayers::STATIC;*/

		return true;
	}
	class ObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
		{
			return Physics3DBroadPhaseCanCollide(inLayer1, inLayer2);
		}
	};

	static bool Physics3DObjectCanCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2)
	{
		return true;
		/*const NodeLayer layer1 = BIT(inObject1);
		const NodeLayer layer2 = BIT(inObject2);
		return	(layer1 & World::LayerCollisionMask.at(layer2).Flags) == layer1 &&
				(layer2 & World::LayerCollisionMask.at(layer1).Flags) == layer2;*/
	};
	class ObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
	{
	public:
		virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const
		{
			return Physics3DObjectCanCollide(inLayer1, inLayer2);
		}
	};

	JoltPhysicsWorld::JoltPhysicsWorld()
	{
	}
	JoltPhysicsWorld::~JoltPhysicsWorld()
	{
	}

	void JoltPhysicsWorld::CreateShapeNode(ShapeNode* node)
	{
		SuoraVerify(m_Rigidbody_Body.find(node) == m_Rigidbody_Body.end());
		JPH::BodyInterface& bodyInterface = m_PhysicsSystem->GetBodyInterface();
		
		JPH::Ref<JPH::Shape> shape;

		if (node->GetType() == ShapeType::Box)
		{
			JPH::BoxShapeSettings shapeSettings = JPH::BoxShapeSettings(Convert::ToRVec3(node->As<BoxShapeNode>()->GetBoxExtends()));
			shape = shapeSettings.Create().Get();
		}
		else if (node->GetType() == ShapeType::Sphere)
		{
			JPH::SphereShapeSettings shapeSettings = JPH::SphereShapeSettings(node->As<SphereShapeNode>()->GetSphereRadius());
			shape = shapeSettings.Create().Get();
		}
		else if (node->GetType() == ShapeType::None)
		{
			JPH::MutableCompoundShapeSettings shapeSettings;
			shape = shapeSettings.Create().Get();
		}
		else
		{
			SuoraVerify(false, "Not implemented!");
		}

		// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
		JPH::BodyCreationSettings settings = JPH::BodyCreationSettings(shape, Convert::ToRVec3(node->GetPosition()), Convert::ToJoltQuat(node->GetRotation()), static_cast<JPH::EMotionType>(node->GetBodyType()), 0);

		JPH::MassProperties massProperties;
		massProperties.mMass = glm::max(0.01f, node->m_Mass);
		settings.mMassPropertiesOverride = massProperties;
		settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;

		settings.mAllowSleeping = node->m_AllowSleep;
		settings.mLinearDamping = glm::max(0.0f, node->m_LinearDrag);
		settings.mAngularDamping = glm::max(0.0f, node->m_AngularDrag);
		settings.mMotionQuality = node->m_IsContinuous ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete;
		settings.mGravityFactor = node->m_GravityScale;

		// Create the actual rigid body
		JPH::Body* rigidbody = bodyInterface.CreateBody(settings); // Note that if we run out of bodies this can return nullptr
		SuoraAssert(rigidbody);

		// Add it to the world
		bodyInterface.AddBody(rigidbody->GetID(), JPH::EActivation::Activate);

		m_Body_Rigidbody[rigidbody] = node;
		m_Rigidbody_Body[node] = rigidbody;
	}

	void JoltPhysicsWorld::DestroyShapeNode(ShapeNode* node)
	{
		if (m_Rigidbody_Body.find(node) == m_Rigidbody_Body.end())
		{
			return;
		}
		JPH::Body* body = m_Rigidbody_Body[node];

		JPH::BodyInterface& bodyInterface = m_PhysicsSystem->GetBodyInterface();
		bodyInterface.RemoveBody(body->GetID());
		bodyInterface.DestroyBody(body->GetID());

		m_Rigidbody_Body.erase(node);
		m_Body_Rigidbody.erase(body);
	}

	void JoltPhysicsWorld::TickShapeNode(ShapeNode* node)
	{
		JPH::BodyInterface& body_interface = m_PhysicsSystem->GetBodyInterface();
		body_interface.SetPosition(m_Rigidbody_Body[node]->GetID(), Convert::ToRVec3(node->GetPosition()), JPH::EActivation::DontActivate);
	}

	Ref<CharacterController> JoltPhysicsWorld::CreateCharacterNode(CharacterNode* node)
	{
		Ref<JoltCharacterController> characterController = CreateRef<JoltCharacterController>(node);
		m_CharacterControllers[node] = characterController;
		return characterController;
	}

	void JoltPhysicsWorld::DestroyCharacterNode(CharacterNode* node)
	{
		if (auto it = m_CharacterControllers.find(node); it != m_CharacterControllers.end())
			m_CharacterControllers.erase(it);
	}

	void JoltPhysicsWorld::TickCharacterNode(CharacterNode* node)
	{
		Ref<CharacterController> characterController = m_CharacterControllers.at(node);
		characterController->SetInternalPosition(node->GetPosition());
		characterController->SetInternalRotation(node->GetRotation());
	}

	void JoltPhysicsWorld::Step(double timeStep)
	{
		for (auto& It : m_CharacterControllers)
		{
			It.second->Simulate(timeStep);
		}
		for (auto& It : m_CharacterControllers)
		{
			It.second->PreSimulate(timeStep);
		}

		m_PhysicsSystem->Update(timeStep, 1, m_TempAllocator.get(), m_JobSystem.get());

		for (auto It : m_Body_Rigidbody)
		{
			if (!It.second->IsStatic && It.second->ShouldUpdateInCurrentContext())
			{
				JPH::BodyInterface& body_interface = m_PhysicsSystem->GetBodyInterface();
				It.second->SetPosition(Convert::ToVec3(body_interface.GetPosition(It.first->GetID())));
				It.second->SetRotation(Convert::ToSuoraQuat(body_interface.GetRotation(It.first->GetID())));
			}
		}
		for (auto& It : m_CharacterControllers)
		{
			It.second->Simulate(timeStep);
		}
		for (auto& It : m_CharacterControllers)
		{
			JoltCharacterController* joltCharacterController = (JoltCharacterController*)It.second.get();

			const Vec3 newPos = Convert::ToVec3(joltCharacterController->m_Controller->GetPosition());
			if (glm::distance(newPos, It.first->GetPosition()) <= 0.01f)
			{
				joltCharacterController->m_Controller->SetPosition(Convert::ToRVec3(It.first->GetPosition()));
			}
			else
			{
				It.first->SetPosition(newPos);
			}
			It.first->SetRotation(Convert::ToSuoraQuat(joltCharacterController->m_Controller->GetRotation()));

		}
	}

	void JoltPhysicsWorld::Inititalize()
	{
		m_TempAllocator = Ref<JPH::TempAllocatorImpl>(new JPH::TempAllocatorImpl(20 * 1024 * 1024));
		m_JobSystem = Ref<JPH::JobSystemThreadPool>(new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, static_cast<int>(JPH::thread::hardware_concurrency()) - 1));
		constexpr JPH::uint cMaxBodies = 65536;
		constexpr JPH::uint cNumBodyMutexes = 0;
		constexpr JPH::uint cMaxBodyPairs = 65536;
		constexpr JPH::uint cMaxContactConstraints = 10240;

		m_BPLayerInterface = Ref<BPLayerInterfaceImpl>(new BPLayerInterfaceImpl());
		m_PhysicsSystem = Ref<JPH::PhysicsSystem>(new JPH::PhysicsSystem());

		m_ObjectLayerPairFilter = CreateRef<ObjectLayerPairFilter>();
		m_ObjectVsBroadPhaseLayerFilter = CreateRef<ObjectVsBroadPhaseLayerFilter>();

		m_PhysicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *m_BPLayerInterface, *m_ObjectVsBroadPhaseLayerFilter, *m_ObjectLayerPairFilter);
	}

}