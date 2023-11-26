#include "Precompiled.h"
#include "Decima.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/Renderer/Vertex.h"
#include "Suora/Renderer/VertexArray.h"
#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Suora/Core/Threading.h"
#include "Suora/Common/Math.h"
#include "Suora/Common/VectorUtils.h"
#include <mutex>

namespace Suora
{

	static float GetAbsScale(const Mat4& transform)
	{
		Vec3 scale;
		scale[0] = glm::length(Vec3(transform[0]));
		scale[1] = glm::length(Vec3(transform[1]));
		scale[2] = glm::length(Vec3(transform[2]));
		scale = glm::abs(scale);

		float scalar = scale.x;
		scalar = scale.y > scalar ? scale.y : scalar;
		scalar = scale.z > scalar ? scale.z : scalar;
		return scalar;
	}

	static float Falloff(float distance)
	{
		//const float dist = glm::distance(camera->GetPosition(), node->GetPosition());
		if (distance >= 100.0f) return 0.25f;
		if (distance >= 50.0f) return Math::Remap(distance, 50.0f, 100.0f, 0.5f, 0.25f);
		return Math::Remap(distance, 0.0f, 50.0f, 1.0f, 0.5f);
	}

	static float GetApproximateScreenPercentage(float fov, const Mat4& transform, const Vec3& cameraPos, Cluster* cluster)
	{
		Mat4 tr = (transform * glm::translate(Mat4(), cluster->LocalPosition));
		Vec3 translation = tr[3];
		
		return (GetAbsScale(transform) * 1.25f * cluster->ClusterRadius) / (glm::distance(translation, cameraPos) * glm::sin(glm::radians(fov)));
	}

	std::vector<Ref<Cluster>> Decima::Generate(Mesh* mesh, const Mat4& transform, const Vec3& cameraPos, const Vec3& cameraForward, float fov)
	{
		std::vector<Ref<Cluster>> Clusters;
		Clusters.push_back(mesh->m_MainCluster);

		int Budget = s_TriangleStreamLimit;

		{
			bool bDone = true;
			do
			{
				bDone = true;
				for (int i = 0; i < Clusters.size(); i++)
				{
					if (GetApproximateScreenPercentage(fov, transform, cameraPos, Clusters[i].get()) 
						* Falloff(glm::distance(cameraPos, (Vec3)transform[3])) 
						* ((glm::dot(cameraForward, Clusters[i]->Normal) < -0.5f) ? 1.0f : Math::Remap(glm::dot(cameraForward, Clusters[i]->Normal), -0.5f, -1.0f, 0.5f, 0.05f))
						
						>= 0.0045f) // 0.0035f
					{
						if (Clusters[i]->Child1 && Clusters[i]->Child2)
						{
							Budget += Clusters[i]->Indices.size() / 3; Budget -= Clusters[i]->Child1->Indices.size() / 3; Budget -= Clusters[i]->Child2->Indices.size() / 3;
							Clusters.push_back(Clusters[i]->Child1);
							Clusters.push_back(Clusters[i]->Child2);
							Clusters.erase(Clusters.begin() + i);
							bDone = false;
							break;
						}
					}
				}
			} while (!bDone && Budget > 0);
		}


		return Clusters;
	}

	void Decima::Run(World* world, CameraNode* camera)
	{
		Array<MeshNode*> meshNodes = world->FindNodesByClass<MeshNode>();

		if (meshNodes.Size() > 0)
		{
			for (int64_t k = 0; k < s_MaxAsyncMeshes && m_Jobs.size() < s_MaxAsyncMeshes; k++)
			{
				m_Index %= meshNodes.Size();
				MeshNode* meshNode = meshNodes[m_Index];

				if (meshNode->m_Mesh && meshNode->m_Mesh->IsDecimaMesh() && meshNode->m_Mesh->m_MainCluster && meshNode->m_Mesh->GetVertexArray())
				{
					if (meshNode->m_Mesh->m_DecimaMeshes.find(meshNode) == meshNode->m_Mesh->m_DecimaMeshes.end())
					{
						meshNode->m_Mesh->m_DecimaMeshes[meshNode] = CreateRef<Mesh>();
					}

					if (m_Jobs.find(meshNode) == m_Jobs.end())
					{
						m_Jobs[meshNode] = std::async(std::launch::async, &Decima::Generate, this, meshNode->m_Mesh, meshNode->GetTransformMatrix(), camera->GetPosition(), camera->GetForwardVector(), camera->GetPerspectiveVerticalFOV());
					}

				}
				m_Index++;
			}
		}

		std::vector<MeshNode*> JobsPendingKill;
		for (auto& It : m_Jobs)
		{
			if (/*m_OffFrames <= 0 && */IsFutureReady(It.second))
			{
				JobsPendingKill.push_back(It.first);

				std::vector<Ref<Cluster>> Clusters = It.second.get();
				std::vector<Ref<VertexArray>> vao;
				for (auto& clusters : Clusters)
				{
					vao.push_back(clusters->GetVertexArray(It.first->m_Mesh->m_MeshBuffer));
				}

				s_ActiveClusters -= m_DecimaMeshes[It.first].size();
				m_DecimaMeshes[It.first] = vao;
				s_ActiveClusters += m_DecimaMeshes[It.first].size();

			}
		}
		for (auto It : JobsPendingKill)
		{
			m_Jobs.erase(It);
		}

		m_OffFrames--;

	}


}