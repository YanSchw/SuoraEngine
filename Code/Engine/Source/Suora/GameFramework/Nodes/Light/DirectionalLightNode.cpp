#include "Precompiled.h"
#include "DirectionalLightNode.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Suora/Core/Application.h"
#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/RenderCommand.h"

namespace Suora
{

	ShadowCascade::ShadowCascade(uint32_t resolution, float size, float offset)
		: m_CascadeSize(size), m_CascadeForwardOffset(offset), m_Matrix()
	{
		FramebufferSpecification spec;
		spec.Width = resolution;
		spec.Height = resolution;
		spec.Attachments.Attachments.push_back(FramebufferTextureFormat::DEPTH32F_STENCIL8);
		m_ShadowMapBuffer = Framebuffer::Create(spec);
	}

	DirectionalLightNode::DirectionalLightNode()
	{
		if (!Application::IsApplicationInitialized()) return;
		m_ShadowMap = true; 
		
		m_Cascades.Add(ShadowCascade( 8192 / 4, 5.0f,	0.65f ));
		m_Cascades.Add(ShadowCascade( 8192 / 4, 25.0f,	2.2f ));
		m_Cascades.Add(ShadowCascade( 8192 / 4, 125.0f,	9.0f ));
		m_Cascades.Add(ShadowCascade( 8192 / 4, 500.0f,	65.0f ));
	}
	void DirectionalLightNode::Begin()
	{
		Super::Begin();
	}
	void DirectionalLightNode::WorldUpdate(float deltaTime)
	{
		Super::WorldUpdate(deltaTime);
	}
	void DirectionalLightNode::OnDestroyed()
	{
	}

	void DirectionalLightNode::ShadowMap(World& world, CameraNode& camera)
	{
		if (!IsEnabled()) return;
		RenderCommand::SetClearColor(glm::vec4(0.0f));
		RenderCommand::SetDepthTest(true);
		RenderCommand::SetAlphaBlending(false);

		for (ShadowCascade& cascade : m_Cascades)
		{
			cascade.m_ShadowMapBuffer->Bind();
			RenderCommand::Clear();
		}
		
		m_LightCamera.GetTransform()->SetPosition(camera.GetTransform()->GetPosition()/* - GetTransform()->GetForwardVector() * 500.0f*/);
		m_LightCamera.GetTransform()->SetRotation(GetTransform()->GetRotation());
		m_LightCamera.GetTransform()->SetScale(Vec3(1.0f));
		m_LightCamera.SetOrthographic(5.0f, -500.0f, 500.0f);

		Array<MeshNode*> meshes = world.FindNodesByClass<MeshNode>();
		
		for (ShadowCascade& cascade : m_Cascades)
		{
			m_LightCamera.SetPosition(camera.GetPosition() + camera.GetForwardVector() * 0.0f);
			m_LightCamera.SetOrthographic(cascade.m_CascadeSize, -m_ShadowDistance, m_ShadowDistance);
			cascade.m_ShadowMapBuffer->Bind();
			cascade.m_Matrix = m_LightCamera.GetProjectionMatrix();
			for (MeshNode* meshNode : meshes)
			{
				if (meshNode->m_CastShadow)
				{
					Renderer3D::DrawMeshNode(&m_LightCamera, meshNode, MaterialType::Depth, 0);
				}
			}
		}
	}

}