#pragma once

#include "Suora/Renderer/RendererAPI.h"

namespace Suora 
{

	enum class CullingMode : uint32_t
	{
		None = 0,
		Backface,
		Frontface
	}; 
	enum class AlphaBlendMode : uint32_t
	{
		Disable = 0,
		Additive,
		Blend,
		Constant
	};

	class RenderCommand
	{
	public:
		static void Init()
		{
			s_RendererAPI->Init();
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		static void SetClearColor(const Vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		static void Clear()
		{
			s_RendererAPI->Clear();
		}
		static void ClearDepth()
		{
			s_RendererAPI->ClearDepth();
		}
		static void SetDepthTest(bool enabled)
		{
			s_RendererAPI->SetDepthTest(enabled);
		}
		static void SetDepthMask(bool enabled)
		{
			s_RendererAPI->SetDepthMask(enabled);
		}
		static void SetCullingMode(CullingMode mode)
		{
			s_RendererAPI->SetCullingMode(mode);
		}
		static void UIAlphaBlending()
		{
			s_RendererAPI->UIAlphaBlending();
		}
		static void SetAlphaBlending(AlphaBlendMode alpha)
		{
			s_RendererAPI->SetAlphaBlending(alpha);
		}
		static void SetAlphaBlending(bool b)
		{
			SetAlphaBlending((b ? AlphaBlendMode::Blend : AlphaBlendMode::Disable));
		}
		static void SetWireframeMode(bool b)
		{
			s_RendererAPI->SetWireframeMode(b);
		}
		static void SetWireframeThickness(float value)
		{
			s_RendererAPI->SetWireframeThickness(value);
		}

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, count);
		}
		static void DrawIndexed(VertexArray* vertexArray, uint32_t count = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, count);
		}
		static void DrawInstanced(VertexArray* vertexArray, uint32_t instanceCount)
		{
			s_RendererAPI->DrawInstanced(vertexArray, instanceCount);
		}
		static void MultiDraw(const std::vector<Ref<VertexArray>>& arrays)
		{
			s_RendererAPI->MultiDraw(arrays);
		}
	private:
		static Scope<RendererAPI> s_RendererAPI;
	};

}
