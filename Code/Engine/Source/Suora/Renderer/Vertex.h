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
		Vec3 Position;
		Vec4 Color = Vec4(1, 1, 1, 1);
		Vec2 TexCoord = Vec2(0);
		Vec3 Normal = Vec3(0, 1, 0);
		Vec3 Tangent = Vec3(1, 0, 0);
		Vec3 Bitangent = Vec3(0, 0, 1);
		int32_t Cluster = 0;
		float TexIndex = 0;
		float TilingFactor = 1;

		Vertex()
		{
			Position = Vec3();
			Color = Vec4(1, 1, 1, 1);
			TexCoord = Vec2(0);
			TexIndex = 0;
			TilingFactor = 1;
		}
		Vertex(const Vec3& pos)
		{
			Position = pos;
			Color = Vec4(1, 1, 1, 1);
			TexCoord = Vec2(0);
			TexIndex = 0;
			TilingFactor = 1;
		}
		Vertex(const Vec3& pos, const Vec2& uv)
		{
			Position = pos;
			Color = Vec4(1, 1, 1, 1);
			TexCoord = uv;
			TexIndex = 0;
			TilingFactor = 1;
		}
		operator Vec3()& { return Position; }
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