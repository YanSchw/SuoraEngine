#pragma once

#include "Suora/Renderer/RendererAPI.h"

namespace Suora 
{

	enum class CullingMode : uint32_t;
	class Framebuffer;

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const Vec4& color) override;
		virtual void Clear() override;
		virtual void ClearDepth() override;

		virtual void SetDepthTest(bool enabled) override;
		virtual void SetDepthMask(bool enabled) override;
		virtual void SetCullingMode(CullingMode mode) override;
		virtual void UIAlphaBlending() override;
		virtual void SetAlphaBlending(AlphaBlendMode alpha) override;
		virtual void SetWireframeMode(bool b) override;
		virtual void SetWireframeThickness(float value) override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		virtual void DrawIndexed(VertexArray* vertexArray, uint32_t indexCount = 0) override;
		virtual void DrawInstanced(VertexArray* vertexArray, uint32_t instanceCount) override;
		virtual void MultiDraw(const std::vector<Ref<VertexArray>>& arrays) override;
	};


}
