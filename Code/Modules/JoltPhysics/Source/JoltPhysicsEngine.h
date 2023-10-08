#pragma once
#include "Suora/Physics/PhysicsEngine.h"
#include "JoltPhysicsEngine.generated.h"

namespace Suora::Physics
{

	class JoltPhysicsEngine : public PhysicsEngine
	{
		SUORA_CLASS(543788322543);
	public:
		void Initialize() override;
		~JoltPhysicsEngine();
		Ref<class PhysicsWorld> CreatePhysicsWorld() override;

	private:
		bool m_WasInitialized = false;
	};

}