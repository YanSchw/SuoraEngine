#pragma once

#include <glm/glm.hpp>
#include "Suora/Common/VectorUtils.h"

namespace Suora
{
	class BufferLayout;

	struct VertexLayout
	{
		static const BufferLayout VertexBufferLayout;
	};

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec4 Color = glm::vec4(1, 1, 1, 1);
		glm::vec2 TexCoord = glm::vec2(0);
		glm::vec3 Normal = glm::vec3(0, 1, 0);
		glm::vec3 Tangent = glm::vec3(1, 0, 0);
		glm::vec3 Bitangent = glm::vec3(0, 0, 1);
		int32_t Cluster = 0;
		float TexIndex = 0;
		float TilingFactor = 1;

		Vertex()
		{
			Position = Vec3();
			Color = glm::vec4(1, 1, 1, 1);
			TexCoord = glm::vec2(0);
			TexIndex = 0;
			TilingFactor = 1;
		}
		Vertex(const Vec3& pos)
		{
			Position = pos;
			Color = glm::vec4(1, 1, 1, 1);
			TexCoord = glm::vec2(0);
			TexIndex = 0;
			TilingFactor = 1;
		}
		Vertex(const Vec3& pos, const glm::vec2& uv)
		{
			Position = pos;
			Color = glm::vec4(1, 1, 1, 1);
			TexCoord = uv;
			TexIndex = 0;
			TilingFactor = 1;
		}
		operator glm::vec3()& { return Position; }
		bool Equals(const Vertex& other) const;
		bool operator == (const Vertex& other) const;
		bool operator != (const Vertex& other) const
		{
			return !operator==(other);
		}
		static Vertex Edge(const Vertex& a, const Vertex& b, float f = 0.5f);
	};

	/** For asynchronous Mesh loading */
	struct MeshBuffer
	{
		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices;
		uint32_t ClusterCount = 0;

		MeshBuffer()
		{
		}

		MeshBuffer(const std::vector<Vertex>& v, const std::vector<uint32_t>& i)
			: Vertices(v), Indices(i)
		{
		}
		void Optimize();

	};

}