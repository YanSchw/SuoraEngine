#pragma once
#include <vector>
#include <chrono>
#include "Suora/GameFramework/Node.h"
#include "Suora/GameFramework/Nodes/RenderableNode3D.h"
#include "Suora/Common/Array.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/Assets/Material.h"
#include "MeshNode.generated.h"

namespace Suora
{
	class Mesh;
	class Material;
	class CameraNode;
	class Decima;

	class MeshNode : public RenderableNode3D
	{
		SUORA_CLASS(844454323);

	private:

		virtual bool IsDeferredRenderable() const override { return true; }
		virtual bool IsForwardRenderable()  const override { return true; }
		virtual bool IsShadowRenderable()   const override { return true; }

		virtual void RenderDeferredSingleInstance(World& world, CameraNode& camera, RenderingParams& params, int32_t ID) override;
		virtual void RenderForwardSingleInstance(World& world, CameraNode& camera, RenderingParams& params, int32_t ID) override;
		virtual void RenderShadowSingleInstance(World& world, CameraNode& lightCamera, RenderingParams& params, LightNode* light, int32_t ID) override;

		uint32_t m_ClusterCount = 0;

	private:
		PROPERTY()
		Mesh* m_Mesh = nullptr;
	public:
		PROPERTY()
		MaterialSlots m_Materials;
		
		PROPERTY()
		bool m_CastShadow = true;

		PROPERTY()
		bool m_ContributeGI = true;

	public:
		float GetBoundingSphereRadius() const;
		float GetApproximateScreenPercentage(CameraNode* camera) const;

		MaterialSlots GetMaterials() const
		{
			return m_Materials.OverwritteMaterials ? m_Materials : (m_Mesh ? m_Mesh->m_Materials : m_Materials);
		}

		void SetMesh(Mesh* mesh);
		Mesh* GetMesh() const;

		MeshNode();
		~MeshNode();
		void Begin() override;

		friend class Decima;
	};
}