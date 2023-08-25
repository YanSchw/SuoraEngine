#include "Precompiled.h"
#include "Suora/Renderer/Buffer.h"

#include "Suora/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Suora 
{

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLVertexBuffer>(size);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLVertexBuffer>(vertices, size);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLIndexBuffer>(indices, size);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create()
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLShaderStorageBuffer>();
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}