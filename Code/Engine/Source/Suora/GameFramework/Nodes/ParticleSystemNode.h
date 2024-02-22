#pragma once
#include <vector>
#include "Suora/GameFramework/Node.h"
#include "Suora/GameFramework/Nodes/RenderableNode3D.h"
#include "Suora/Common/Array.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/Assets/Material.h"
#include "ParticleSystemNode.generated.h"

namespace Suora
{
	class Mesh;
	class Material;
	class CameraNode;
	class ShaderStorageBuffer;

	constexpr size_t PARTICLE_STRUCT_SIZE = 64;
	struct alignas(PARTICLE_STRUCT_SIZE) Particle
	{
		Vec4 Position = Vec4();
		Vec4 Scale = Vec4(1, 1, 1, 0);
		Vec4 Velocity = Vec4();
		float LifeTime = 0.0f;
	};

	class ParticleSystemNode : public RenderableNode3D
	{
		SUORA_CLASS(8754984118);
	public:

		ParticleSystemNode();
		~ParticleSystemNode();
		void Begin() override;

		void SimulateSystem(float deltaTime);

		Material* GetMaterial() const
		{
			return m_Material;
		}

		void SetMesh(Mesh* mesh);
		Mesh* GetMesh() const;

		PROPERTY()
		Material* m_Material = nullptr;

	private:
		virtual bool IsForwardRenderable()  const override { return true; }
		virtual bool IsShadowRenderable()   const override { return true; }

		virtual void RenderForwardSingleInstance(World& world, CameraNode& camera, RenderingParams& params, int32_t ID) override;
		virtual void RenderShadowSingleInstance(World& world, CameraNode& lightCamera, RenderingParams& params, LightNode* light, int32_t ID) override;

		PROPERTY()
		Mesh* m_Mesh = nullptr;

		PROPERTY()
		float m_LifeTime = 5.0f;

		Ref<ShaderStorageBuffer> m_SSBO;
		Array<Particle> m_Particles;

		friend class Decima;
	};
}