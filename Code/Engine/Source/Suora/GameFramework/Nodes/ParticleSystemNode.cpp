#include "Precompiled.h"
#include "ParticleSystemNode.h"
#include "Suora/Core/Engine.h"
#include "Suora/Common/Random.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/Buffer.h"

#include "Suora/Core/NativeInput.h"

namespace Suora
{

	void ParticleSystemNode::RenderForwardSingleInstance(World& world, CameraNode& camera, RenderingParams& params, int32_t ID)
	{
		SimulateSystem(Engine::Get()->GetDeltaTime());

		if (m_Particles.Size() <= 0)
		{
			return;
		}

		if (m_SSBO == nullptr)
		{
			m_SSBO = ShaderStorageBuffer::Create();
		}

		m_SSBO->Write((sizeof(Particle)) * m_Particles.Size(), &m_Particles[0]);
		m_SSBO->Bind();

		if (GetMesh() && GetMaterial())
		{
			if (!GetMaterial()->GetShaderGraph()) return;
			if (!GetMaterial()->GetShaderGraph()->GetShaderViaType(MaterialType::Material)) return;

			VertexArray* vao = GetMesh()->GetVertexArray();
			if (!vao) return;

			RenderCommand::SetDepthTest(GetMaterial()->m_DepthTest);
			RenderCommand::SetCullingMode(GetMaterial()->m_BackfaceCulling ? CullingMode::Backface : CullingMode::None);

			GetMaterial()->ApplyUniforms(MaterialType::Material);
			GetMaterial()->GetShaderGraph()->GetShaderViaType(MaterialType::Material)->SetMat4("u_Transform", GetTransformMatrix());

			GetMaterial()->GetShaderGraph()->GetShaderViaType(MaterialType::Material)->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

			vao->Bind();
			RenderCommand::DrawInstanced(vao, m_Particles.Size());
		}
	}

	void ParticleSystemNode::RenderShadowSingleInstance(World& world, CameraNode& lightCamera, RenderingParams& params, LightNode* light, int32_t ID)
	{
	}

	void ParticleSystemNode::SetMesh(Mesh* mesh)
	{
		m_Mesh = mesh;
	}

	Mesh* ParticleSystemNode::GetMesh() const
	{
		return m_Mesh;
	}

	ParticleSystemNode::ParticleSystemNode()
	{
	}
	ParticleSystemNode::~ParticleSystemNode()
	{
	}

	void ParticleSystemNode::Begin()
	{
		Super::Begin();
		
	}

	void ParticleSystemNode::SimulateSystem(float deltaTime)
	{

		// Spawn a Particle every Frame...
		m_Particles.Add(Particle());
		m_Particles[m_Particles.Last()].Velocity = Vec4(-0.5f + (float)Random().NextDouble(), -0.5f + (float)Random().NextDouble(), -0.5f + (float)Random().NextDouble(), 0.0f);

		Array<uint32_t> KillIndicies;

		uint32_t idx = 0;
		for (Particle& It : m_Particles)
		{
			It.Position += It.Velocity * deltaTime;
			It.LifeTime += deltaTime;

			if (It.LifeTime >= m_LifeTime)
			{
				KillIndicies.Add(idx);
			}
			idx++;
		}

		for (int i = KillIndicies.Size() - 1; i >= 0; i--)
		{
			m_Particles.RemoveAt(i);
		}
	}


}