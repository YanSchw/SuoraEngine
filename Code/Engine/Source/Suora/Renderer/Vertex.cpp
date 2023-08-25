#include "Precompiled.h"
#include "Vertex.h"
#include "Buffer.h"
#include "Suora/Common/Math.h"

namespace Suora
{

	const BufferLayout VertexLayout::VertexBufferLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tangent" },
			{ ShaderDataType::Float3, "a_Bitangent" },
			{ ShaderDataType::Int, "a_Cluster" },
			{ ShaderDataType::Float, "a_TexIndex" },
			{ ShaderDataType::Float, "a_TilingFactor" }
	};
	bool Vertex::Equals(const Vertex & other) const
	{
		return glm::distance(Position, other.Position) <= 0.05f;
	}
	bool Vertex::operator==(const Vertex& other) const
	{
		return glm::distance(Position, other.Position) <= 0.0005f;
	}
	Vertex Vertex::Edge(const Vertex& a, const Vertex& b, float f)
	{
		Vertex v;
		v.Position = Math::Lerp(a.Position, b.Position, f);
		v.Color = Math::Lerp(a.Color, b.Color, f);
		v.TexCoord = Math::Lerp(a.TexCoord, b.TexCoord, f);
		v.Normal = Math::Lerp(a.Normal, b.Normal, f);
		v.Tangent = Math::Lerp(a.Tangent, b.Tangent, f);
		v.Bitangent = Math::Lerp(a.Bitangent, b.Bitangent, f);
		v.Cluster = 1;
		v.TexIndex = Math::Lerp(a.TexIndex, b.TexIndex, f);
		v.TilingFactor = Math::Lerp(a.TilingFactor, b.TilingFactor, f);
		return v;
	}

	void MeshBuffer::Optimize()
	{
		/*                 ORIGINAL  NEW  */
		std::unordered_map<uint32_t, uint32_t> V;
		int index = 0;

		for (int i = 0; i < Indices.size(); i++)
		{
			if (V.find(Indices[i]) == V.end()) V[Indices[i]] = index++;
			Indices[i] = V[Indices[i]];
		}
		std::vector<Vertex> NewVertices = std::vector<Vertex>(V.size());
		for (auto It : V)
		{
			NewVertices[It.second] = Vertices[It.first];
		}
		Vertices = NewVertices;
	}

}