#pragma once
#include "Suora/Core/Base.h"
#include "Suora/Common/VectorUtils.h"
#include "Suora/Common/Array.h"
#include "Suora/Common/Math.h"
#include "Suora/Core/Object/Pointer.h"
#include "Suora/Core/Object/Object.h"
#include "PhysicsEngine.generated.h"

namespace Suora::Physics
{

	class PhysicsEngine : public Object
	{
		SUORA_CLASS(547893438543);
	public:
		static Ref<PhysicsEngine> Create();

		virtual void Initialize() = 0;

		virtual Ref<class PhysicsWorld> CreatePhysicsWorld() = 0;

	public:
		inline static SubclassOf<PhysicsEngine> s_PhysicsEngineClass = PhysicsEngine::StaticClass();

	};

}