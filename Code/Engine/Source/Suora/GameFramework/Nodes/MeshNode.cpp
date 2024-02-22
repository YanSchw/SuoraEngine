#include "Precompiled.h"
#include "MeshNode.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Suora/Renderer/Renderer3D.h"

namespace Suora
{

	void MeshNode::RenderDeferredSingleInstance(World& world, CameraNode& camera, RenderingParams& params, int32_t ID)
	{
		if (GetMesh() && GetMaterials().Materials.Size() > 0 && GetMaterials().Materials[0] && GetMaterials().Materials[0]->IsDeferred())
		{
			Renderer3D::DrawMeshNode(&camera, this, MaterialType::Material, ID);

		}
	}

	void MeshNode::RenderForwardSingleInstance(World& world, CameraNode& camera, RenderingParams& params, int32_t ID)
	{
		if (GetMesh() && GetMaterials().Materials.Size() > 0 && GetMaterials().Materials[0] && GetMaterials().Materials[0]->GetShaderGraph() && !GetMaterials().Materials[0]->IsDeferred())
		{
			Renderer3D::DrawMeshNode(&camera, this, MaterialType::Material, ID);
		}
	}

	void MeshNode::RenderShadowSingleInstance(World& world, CameraNode& lightCamera, RenderingParams& params, LightNode* light, int32_t ID)
	{
		if (m_CastShadow && GetMesh())
		{
			Renderer3D::DrawMeshNode(&lightCamera, this, MaterialType::Depth, ID);
		}
	}

	float MeshNode::GetBoundingSphereRadius() const
	{
		const Vec3 scale = glm::abs(GetScale());
		float scalar = scale.x;
		scalar = scale.y > scalar ? scale.y : scalar;
		scalar = scale.z > scalar ? scale.z : scalar;
		return m_Mesh->m_BoundingSphereRadius * scalar;
	}
	float MeshNode::GetApproximateScreenPercentage(CameraNode* camera) const
	{
		if (!m_Mesh) return 0.0f;
		return GetBoundingSphereRadius() / (glm::distance(GetPosition(), camera->GetPosition()) * glm::sin(glm::radians(camera->GetPerspectiveVerticalFOV())));
	}

	void MeshNode::SetMesh(Mesh* mesh)
	{
		m_Mesh = mesh;
	}

	Mesh* MeshNode::GetMesh() const
	{
		return m_Mesh;
	}

	MeshNode::MeshNode()
	{
		m_Materials.Owner = this;
	}
	MeshNode::~MeshNode()
	{
	}

	void MeshNode::Begin()
	{
		Super::Begin();
		
	}


}