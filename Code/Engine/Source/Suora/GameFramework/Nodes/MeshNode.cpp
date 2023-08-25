#include "Precompiled.h"
#include "MeshNode.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"

namespace Suora
{
	float MeshNode::GetBoundingSphereRadius() const
	{
		const Vec3 scale = glm::abs(GetScale());
		float scalar = scale.x;
		scalar = scale.y > scalar ? scale.y : scalar;
		scalar = scale.z > scalar ? scale.z : scalar;
		return mesh->m_BoundingSphereRadius * scalar;
	}
	float MeshNode::GetApproximateScreenPercentage(CameraNode* camera) const
	{
		if (!mesh) return 0.0f;
		return GetBoundingSphereRadius() / (glm::distance(GetPosition(), camera->GetPosition()) * glm::sin(glm::radians(camera->GetPerspectiveVerticalFOV())));
	}
	MeshNode::MeshNode()
	{
		materials.Owner = this;
	}
	MeshNode::~MeshNode()
	{
	}

	void MeshNode::Begin()
	{
		Super::Begin();
		
	}


}