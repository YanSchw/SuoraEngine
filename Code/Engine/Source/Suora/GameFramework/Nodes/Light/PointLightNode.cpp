#include "Precompiled.h"
#include "PointLightNode.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"

#define POINT_LIGHT_SHADOW_RESOLUTION 512

namespace Suora
{

	PointLightNode::PointLightNode()
	{
	}
	void PointLightNode::Begin()
	{
		Super::Begin();
	}
	void PointLightNode::WorldUpdate(float deltaTime)
	{
		Super::WorldUpdate(deltaTime);
	}
	void PointLightNode::OnDestroyed()
	{
		if (s_ShadowAtlasContent.Contains(this))
		{
			s_ShadowAtlasContent[s_ShadowAtlasContent.IndexOf(this)] = nullptr;
		}
	}

	void PointLightNode::Capture(World& world, CameraNode& camera, CameraNode& view, const glm::ivec2& rect)
	{
		s_ShadowAtlas->Bind();
		//RenderCommand::SetViewport(rect.x, rect.y, rect.z, rect.w);
		RenderPipeline::ClearDepth(rect.x, rect.y, POINT_LIGHT_SHADOW_RESOLUTION, POINT_LIGHT_SHADOW_RESOLUTION);
		RenderCommand::SetViewport(rect.x, rect.y, POINT_LIGHT_SHADOW_RESOLUTION, POINT_LIGHT_SHADOW_RESOLUTION);
		RenderCommand::SetDepthTest(true);
		RenderCommand::SetAlphaBlending(false);

		Array<MeshNode*> meshes = world.FindNodesByClass<MeshNode>();
		for (MeshNode* meshNode : meshes)
		{
			if (meshNode->mesh)
			{
				if (meshNode->m_CastShadow)
				{
					Renderer3D::DrawMeshNode(&view, meshNode, MaterialType::Depth, 0);
				}
			}
		}
	}

	void PointLightNode::ShadowMap(World& world, CameraNode& camera)
	{
		if (!s_ShadowAtlasContent.Contains(this))
		{
			int index = s_ShadowAtlasContent.IndexOf(nullptr);
			if (index == -1)
			{
				s_ShadowAtlasContent.Add(this);
			}
			else
			{
				s_ShadowAtlasContent[index] = this;
			}
		}
		for (int i = 0; i < s_ShadowAtlasContent.Size(); i++)
		{
			// TODO: Fix
			/*if (s_ShadowAtlasContent[i] != this && !s_ShadowAtlasContent[i]->m_ShadowMap)
			{
				s_ShadowAtlasContent[i] = nullptr;
			}*/
		}

		if (s_InitShadowAtlas && s_ShadowAtlas->GetSize().x / POINT_LIGHT_SHADOW_RESOLUTION != s_ShadowAtlasContent.Size())
		{
			//s_InitShadowAtlas = false;
			s_ShadowAtlas->Resize(POINT_LIGHT_SHADOW_RESOLUTION * s_ShadowAtlasContent.Size(), POINT_LIGHT_SHADOW_RESOLUTION * 6);
		}
		if (!s_InitShadowAtlas)
		{
			s_InitShadowAtlas = true;

			FramebufferSpecification spec;
			spec.Width = POINT_LIGHT_SHADOW_RESOLUTION * s_ShadowAtlasContent.Size();
			spec.Height = POINT_LIGHT_SHADOW_RESOLUTION * 6;
			spec.Attachments.Attachments.push_back({ FramebufferTextureFormat::DEPTH32F_STENCIL8, FramebufferTextureFilter::Linear });
			s_ShadowAtlas = Framebuffer::Create(spec);
		}

		s_ShadowAtlas->Bind();
		//RenderCommand::ClearDepth();

		// Actual Shadowmapping Code....
		CameraNode View;
		View.SetPosition(GetPosition());
		View.SetPerspective(90, 0.1f, 45.0f);
		View.SetAspectRatio(1);

		const int32_t INDEX = s_ShadowAtlasContent.IndexOf(this);

	//TOP:
		View.SetEulerRotation(Vec3(-90, 0, 0));
		View.RecalculateProjection();
		m_ViewMatrix.ViewTop = View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix());
		Capture(world, camera, View, glm::ivec2(POINT_LIGHT_SHADOW_RESOLUTION * INDEX, POINT_LIGHT_SHADOW_RESOLUTION * 2));
	//BOTTOM:
		View.SetEulerRotation(Vec3(90, 0, 0));
		View.RecalculateProjection();
		m_ViewMatrix.ViewBottom = View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix());
		Capture(world, camera, View, glm::ivec2(POINT_LIGHT_SHADOW_RESOLUTION * INDEX, POINT_LIGHT_SHADOW_RESOLUTION * 3));
	//LEFT:
		View.SetEulerRotation(Vec3(0, -90, 0));
		View.RecalculateProjection();
		m_ViewMatrix.ViewLeft = View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix());
		Capture(world, camera, View, glm::ivec2(POINT_LIGHT_SHADOW_RESOLUTION * INDEX, POINT_LIGHT_SHADOW_RESOLUTION * 1));
	//RIGHT:
		View.SetEulerRotation(Vec3(0, 90, 0));
		View.RecalculateProjection();
		m_ViewMatrix.ViewRight = View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix());
		Capture(world, camera, View, glm::ivec2(POINT_LIGHT_SHADOW_RESOLUTION * INDEX, POINT_LIGHT_SHADOW_RESOLUTION * 0));
	//FORWARD:
		View.SetEulerRotation(Vec3(0, 0, 0));
		View.RecalculateProjection();
		m_ViewMatrix.ViewForward = View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix());
		Capture(world, camera, View, glm::ivec2(POINT_LIGHT_SHADOW_RESOLUTION * INDEX, POINT_LIGHT_SHADOW_RESOLUTION * 4));
	//BACKWARD:
		View.SetEulerRotation(Vec3(0, 180, 0));
		View.RecalculateProjection();
		m_ViewMatrix.ViewBackward = View.GetProjectionMatrix() * glm::inverse(View.GetTransformMatrix());
		Capture(world, camera, View, glm::ivec2(POINT_LIGHT_SHADOW_RESOLUTION * INDEX, POINT_LIGHT_SHADOW_RESOLUTION * 5));
	}

}