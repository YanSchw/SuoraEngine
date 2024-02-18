#pragma once
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/VectorUtils.h"
#include "LightNode.generated.h"

namespace Suora
{
	class World;
	class CameraNode;
	struct RenderingParams;

	class LightNode : public Node3D
	{
		SUORA_CLASS(712234764);
	public:

		LightNode();
		void Begin() override;
		void WorldUpdate(float deltaTime) override;
		void OnDestroyed();

		virtual void ShadowMap(World& world, CameraNode& camera, RenderingParams& params);

		PROPERTY()
		float m_Intensity = 1.0f;

		PROPERTY()
		bool m_ShadowMap = false;

	};

}