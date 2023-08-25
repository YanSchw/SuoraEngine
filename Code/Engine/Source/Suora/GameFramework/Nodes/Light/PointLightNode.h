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
		glm::mat4 ViewTop = {};
		glm::mat4 ViewBottom = {};
		glm::mat4 ViewLeft = {};
		glm::mat4 ViewRight = {};
		glm::mat4 ViewForward = {};
		glm::mat4 ViewBackward = {};
	};

	class PointLightNode : public LightNode
	{
		SUORA_CLASS(6527498345);
	public:

		MEMBER()
		float m_Radius = 4.5f;

		MEMBER()
		float m_LightCullRange = 85.0f;
		MEMBER()
		float m_LightCullFalloff = 25.0f;

		PointLightNode();
		void Begin() override;
		void WorldUpdate(float deltaTime) override;
		void OnDestroyed();
		void Capture(World& world, CameraNode& camera, CameraNode& view, const glm::ivec2& rect);
		void ShadowMap(World& world, CameraNode& camera) override;

	private:
		inline static bool s_InitShadowAtlas = false;
		inline static Ref<Framebuffer> s_ShadowAtlas = nullptr;
		inline static Array<PointLightNode*> s_ShadowAtlasContent;

		PointLightMatrixStruct m_ViewMatrix;

		friend class RenderPipeline;
	};

}