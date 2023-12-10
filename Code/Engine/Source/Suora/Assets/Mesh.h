#pragma once
#include "StreamableAsset.h"
#include <vector>
#include <string>
#include <future>
#include "Suora/Common/VectorUtils.h"
#include "Suora/Renderer/Vertex.h"
#include "Suora/Renderer/VertexArray.h"
#include "Material.h"
#include "Mesh.generated.h"

struct aiScene;
namespace Assimp
{
	class Importer;
}

namespace Suora
{
	class Material;
	class MeshNode;
	class Decima;

	struct Cluster
	{
		Ref<Cluster> Child1;
		Ref<Cluster> Child2;
		std::vector<uint32_t> Indices;
		Vec3 LocalPosition = Vec3(0);
		Vec3 Normal = Vec3(1);
		float ClusterRadius = 0;

		Ref<VertexArray> GetVertexArray(MeshBuffer& buffer);
	private:
		Ref<VertexArray> m_VertexArray;
	};

	class Mesh : public StreamableAsset
	{
		SUORA_CLASS(7854672332);
		ASSET_EXTENSION(".mesh");

	public:
		Mesh();
		~Mesh();
		void PreInitializeAsset(Yaml::Node& root) override;
		void InitializeAsset(Yaml::Node& root) override;
		virtual void ReloadAsset() override;
		virtual uint32_t GetAssetFileSize() override;
		VertexArray* GetVertexArray();
		void RebuildMesh();
		static MeshBuffer Decimate(/*COPY*/ MeshBuffer buffer, uint32_t N);
		static void Decimate_Edge(MeshBuffer& buffer, uint32_t N);
		static void Decimate_ClusterEdge(MeshBuffer& buffer);
		static void Decimate_Cluster(const MeshBuffer& meshBuffer, const Ref<Cluster>& cluster);
		Ref<MeshBuffer> Async_LoadMeshBuffer(const String& path, const std::vector<Vertex>& v, const std::vector<uint32_t>& i);
		void Serialize(Yaml::Node& root) override;
		void PostProcessCluster(MeshBuffer& buffer, Ref<Cluster> cluster, int32_t& id);
		void GroupClusters(MeshBuffer& buffer, std::vector<Ref<Cluster>> clusters);
		void Clusterfication(MeshBuffer& buffer);

		Vec3 m_ImportScale = Vec3(1.0f);
		MaterialSlots m_Materials;

		Array<Ref<Mesh>> m_Submeshes;
		Ref<Cluster> m_MainCluster = nullptr;
		std::unordered_map<MeshNode*, Ref<Mesh>> m_DecimaMeshes;

		bool m_FlipNormals = true;
		bool m_IsDecimaMesh = false;
		float m_BoundingSphereRadius = 1.0f;
		float m_NegativeY_Bounds = 0.0f;
		MeshBuffer m_MeshBuffer;
		Ref<VertexArray> m_VertexArray = nullptr;
		Ref<std::future<Ref<MeshBuffer>>> m_AsyncMeshBuffer;

		inline bool IsMasterMesh() const { return m_IsMasterMesh; }
		inline bool IsSubMesh() const { return m_ParentMesh; }
		inline bool IsDecimaMesh() const { return m_IsDecimaMesh; }

		// Primitive Shapes
		static Mesh* Quad;
		static Mesh* Plane;
		static Mesh* Cube;
		static Mesh* Sphere;

		static Array<String> GetSupportedSourceAssetExtensions();

	private:
		bool m_IsMasterMesh = false;
		Mesh* m_ParentMesh = nullptr;
		int m_SubmeshIndex = -1;
		Ref<Assimp::Importer> m_SubmeshImporter;
		const aiScene* m_SubmeshScene = nullptr;


		friend class DetailsPanel;
		friend class Decima;
	};
}