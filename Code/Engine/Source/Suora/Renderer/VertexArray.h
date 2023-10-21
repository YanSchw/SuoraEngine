#pragma once

#include <memory>
#include <vector>
#include "Suora/Renderer/Buffer.h"

namespace Suora 
{

	struct MeshBuffer;

	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

		static Ref<VertexArray> Create();
		static VertexArray* Create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		static VertexArray* Create(const Ref<MeshBuffer>& buffer);
		static VertexArray* Create(const MeshBuffer& buffer);
	};

}
