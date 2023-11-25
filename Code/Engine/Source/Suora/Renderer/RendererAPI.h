#pragma once

#include "Suora/Common/VectorUtils.h"
#include "Suora/Renderer/VertexArray.h"

namespace Suora 
{

	enum class CullingMode : uint32_t;
	enum class AlphaBlendMode : uint32_t;
	class Framebuffer;

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1
		};
	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const Vec4& color) = 0;
		virtual void Clear() = 0;
		virtual void ClearDepth() = 0;

		virtual void SetDepthTest(bool enabled) = 0;
		virtual void SetDepthMask(bool enabled) = 0;
		virtual void SetCullingMode(CullingMode mode) = 0;
		virtual void UIAlphaBlending() = 0;
		virtual void SetAlphaBlending(AlphaBlendMode alpha) = 0;
		virtual void SetWireframeMode(bool b) = 0;
		virtual void SetWireframeThickness(float value) = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawIndexed(VertexArray* vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawInstanced(VertexArray* vertexArray, uint32_t instanceCount) = 0;
		virtual void MultiDraw(const std::vector<Ref<VertexArray>>& arrays) = 0;

		static API GetAPI() { return s_API; }
		static Scope<RendererAPI> Create();
	private:
		static API s_API;
	};

}
