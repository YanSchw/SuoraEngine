#include "Precompiled.h"
#include "UserInterface.h"

#include "Suora/Assets/Texture2D.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Renderer/Vertex.h"
#include "Suora/Renderer/Texture.h"
#include "Suora/Renderer/Buffer.h"
#include "Suora/Renderer/VertexArray.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/Shader.h"

namespace Suora
{

	Ref<VertexArray> UIRenderable::GetQuad() const
	{
		return m_Quad;
	}
	void UIRenderable::RenderUI(const glm::mat4& view, Framebuffer& target)
    {
		CreateQuad();
    }

    void UIRenderable::CreateQuad()
    {
		UINode::RectTransform transform = GetRectTransform();

		m_Quad = VertexArray::Create();

		Vertex vertices[4] = { Vertex(transform.UpperRight + transform.GetDown(), Vec2(1, 0)),
							   Vertex(transform.UpperLeft,                        Vec2(0, 1)),
							   Vertex(transform.UpperRight,                       Vec2(1, 1)),
							   Vertex(transform.BottomLeft,                       Vec2(0, 0)) };
		uint32_t indices[6] = { 2, 1, 0, 0, 1, 3 };

		Ref<VertexBuffer> QuadVBO = VertexBuffer::Create(sizeof(Vertex) * 4);
		QuadVBO->SetLayout(VertexLayout::VertexBufferLayout);

		Ref<IndexBuffer> QuadIB = IndexBuffer::Create(&indices[0], sizeof(uint32_t) * 6);
		m_Quad->SetIndexBuffer(QuadIB);

		QuadVBO->SetData(&vertices[0], 4 * sizeof(Vertex));
		m_Quad->AddVertexBuffer(QuadVBO);
    }

    void UIImage::RenderUI(const glm::mat4& view, Framebuffer& target)
    {
		if (!m_Texture)
		{
			return;
		}
		if (!s_ImageShader)
		{
			s_ImageShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/InGameUI/UIImage.glsl");
		}

        Super::RenderUI(view, target);

		s_ImageShader->Bind();
		s_ImageShader->SetInt("u_Texture", 0); m_Texture->GetTexture()->Bind(0);
		s_ImageShader->SetFloat4("u_Tint", m_Tint);
		GetQuad()->Bind();
		RenderCommand::DrawIndexed(GetQuad());
    }


}