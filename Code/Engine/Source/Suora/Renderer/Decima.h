#pragma once
#include <future>
#include <unordered_map>
#include <thread>
#include "inttypes.h"
#include <glm/glm.hpp>

namespace Suora
{
	class World;
	class CameraNode;
	class Mesh;
	class MeshNode;
	struct MeshBuffer;
	struct Cluster;
	class VertexArray;

	class Decima
	{
	public:
		inline static uint32_t s_TrianglesPerCluster = 128;
		inline static uint32_t s_TriangleStreamLimit = 48000;
		inline static uint32_t s_PerFrameTriangleBudget = 2400;
		inline static uint32_t s_MaxAsyncMeshes = 8;

		std::unordered_map<MeshNode*, std::future<std::vector<Ref<Cluster>>>> m_Jobs;
		int64_t m_Index = 0;
		std::thread m_Thread;
		bool m_ThreadInit = false;

		std::vector<Ref<Cluster>> Generate(Mesh* mesh, const glm::mat4& transform, const glm::vec3& cameraPos, const glm::vec3& cameraForward, float fov);
		void Run(World* world, CameraNode* camera);

		inline static std::unordered_map<MeshNode*, std::vector<Ref<VertexArray>>> m_DecimaMeshes;

	private:

		int32_t m_OffFrames = 0;
		inline static int32_t s_ActiveClusters = 0;
	};

}