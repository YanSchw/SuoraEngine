#pragma once
#include <Suora.h>
#include <glm/glm.hpp>
#include "LightNode.h"
#include "PointLightNode.generated.h"

namespace Suora
{
	class LightNode;

	struct PointLightMatrixStruct
	{
		Mat4 ViewTop = {};
		Mat4 ViewBottom = {};
		Mat4 ViewLeft = {};
		Mat4 ViewRight = {};
		Mat4 ViewForward = {};
		Mat4 ViewBackward = {};
	};

	class PointLightNode : public LightNode
	{
		SUORA_CLASS(6527498345);
	public:

		PROPERTY()
		Color m_Color = Color(1.0f);

		PROPERTY()
		float m_Radius = 4.5f;

		PROPERTY()
		float m_LightCullRange = 85.0f;
		PROPERTY()
		float m_LightCullFalloff = 25.0f;

		PointLightNode();
		void Begin() override;
		void WorldUpdate(float deltaTime) override;
		void OnDestroyed();
		void Capture(World& world, CameraNode& camera, RenderingParams& params, CameraNode& view, const glm::ivec2& rect);
		void ShadowMap(World& world, CameraNode& camera, RenderingParams& params) override;

	private:
		inline static bool s_InitShadowAtlas = false;
		inline static Ref<Framebuffer> s_ShadowAtlas = nullptr;
		inline static Array<PointLightNode*> s_ShadowAtlasContent;

		PointLightMatrixStruct m_ViewMatrix;

		friend class RenderPipeline;
	};

}