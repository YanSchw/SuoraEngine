#pragma once
#include <CoreMinimal.h>
#include <glm/glm.hpp>
#include "LightNode.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "DirectionalLightNode.generated.h"

namespace Suora
{
	class LightNode;
	class Framebuffer;

	struct ShadowCascade
	{
		Ref<Framebuffer> m_ShadowMapBuffer;
		float m_CascadeSize = 10.0f;
		float m_CascadeForwardOffset = 0.0f;
		Mat4 m_Matrix;
		
		ShadowCascade(uint32_t resolution, float size, float offset);
	};

	class DirectionalLightNode : public LightNode
	{
		SUORA_CLASS(45738744712);
	public:

		MEMBER()
		Color m_Color = Color(1.0f);

		MEMBER()
		float m_Radius = 4.5f;

		MEMBER()
		bool m_SoftShadows = true;

		MEMBER()
		float m_ShadowDistance = 500.0f;

		DirectionalLightNode();
		void Begin() override;
		void WorldUpdate(float deltaTime) override;
		void OnDestroyed();

		void ShadowMap(World& world, CameraNode& camera, RenderingParams& params) override;

	private:
		Array<ShadowCascade> m_Cascades;
		CameraNode m_LightCamera;

		friend class RenderPipeline;
	};

}