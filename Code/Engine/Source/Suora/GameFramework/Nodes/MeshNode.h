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

	private:
		MEMBER() 
		Mesh* m_Mesh = nullptr;
	public:
		MEMBER() 
		MaterialSlots m_Materials;
		
		MEMBER()
		bool m_CastShadow = true;

		MEMBER()
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