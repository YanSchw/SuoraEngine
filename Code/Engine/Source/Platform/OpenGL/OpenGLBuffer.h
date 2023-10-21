#pragma once

#include "Suora/Renderer/Buffer.h"

namespace Suora 
{

	struct Vertex;

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		
		virtual void SetData(const void* data, uint32_t size) override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

		int32_t GetVertexCount() const override;

	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
		int32_t m_Count = 0;
		friend class OpenGLRendererAPI;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const uint32_t* indices, uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const override { return m_Count; }
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
	class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
	{
	public:
		OpenGLShaderStorageBuffer();
		virtual ~OpenGLShaderStorageBuffer();

		virtual void Bind() const;
		virtual void Write(size_t size, void* data);
		virtual void Read(size_t size, void* data);

	private:
		uint32_t m_SSBO;
	};


}