#include "Precompiled.h"
#include "Suora/Renderer/VertexArray.h"

#include "Suora/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Suora/Renderer/Vertex.h"
#include "Buffer.h"

namespace Suora 
{

	Ref<VertexArray> VertexArray::Create()
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLVertexArray>();
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	VertexArray* VertexArray::Create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		VertexArray* vao = nullptr;

		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  vao = new OpenGLVertexArray();
		}

		if (vao)
		{
			Ref<VertexBuffer> vbo = VertexBuffer::Create(sizeof(Vertex) * vertices.size());
			vbo->SetLayout(VertexLayout::VertexBufferLayout);

			vbo->SetData(&(vertices.front()), vertices.size() * sizeof(Vertex));
			vao->AddVertexBuffer(vbo);

			Ref<IndexBuffer> ib = IndexBuffer::Create(&(indices.front()), indices.size());
			vao->SetIndexBuffer(ib);

			return vao;
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	VertexArray* VertexArray::Create(const Ref<MeshBuffer>& buffer)
	{
		return VertexArray::Create(buffer->Vertices, buffer->Indices);
	}
	VertexArray* VertexArray::Create(const MeshBuffer& buffer)
	{
		return VertexArray::Create(buffer.Vertices, buffer.Indices);
	}

}