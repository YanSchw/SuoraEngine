#pragma once
#include "Node.h"
#include "UserInterface.generated.h"

namespace Suora
{
	class Shader;
	class VertexArray;
	class Asset;
	class Texture2D;

	class UIRenderable : public UINode
	{
		SUORA_CLASS(5762934515);
	public:
		Ref<VertexArray> GetQuad() const;

	protected:
		virtual void RenderUI(const Mat4& view, class Framebuffer& target);
	private:
		void CreateQuad();

		Ref<VertexArray> m_Quad;

		friend class RenderPipeline;
	};

	class UIImage : public UIRenderable
	{
		SUORA_CLASS(647832325);
	public:
		virtual void RenderUI(const Mat4& view, class Framebuffer& target) override;

		MEMBER()
		Texture2D* m_Texture = nullptr;

		MEMBER()
		Color m_Tint = Color(1.0f);

	private:
		inline static Ref<Shader> s_ImageShader;
	};

}