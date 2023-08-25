#pragma once
#include <vector>
#include <chrono>
#include "Suora/GameFramework/Node.h"
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

	class MeshNode : public Node3D
	{
		SUORA_CLASS(844454323);

	private:
		uint32_t m_ClusterCount = 0;

	public:
		MEMBER() 
			Mesh* mesh = nullptr;
		MEMBER() 
			MaterialSlots materials;
		
		MEMBER()
		bool m_CastShadow = true;

		MEMBER()
		bool m_ContributeGI = true;

		float GetBoundingSphereRadius() const;
		float GetApproximateScreenPercentage(CameraNode* camera) const;

		MaterialSlots GetMaterials() const
		{
			return materials.OverwritteMaterials ? materials : (mesh ? mesh->m_Materials : materials);
		}

		MeshNode();
		~MeshNode();
		void Begin() override;

		friend class Decima;
	};
}