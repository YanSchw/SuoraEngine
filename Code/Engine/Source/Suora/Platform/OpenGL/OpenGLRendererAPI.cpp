#include "Precompiled.h"
#include "Suora/Platform/OpenGL/OpenGLRendererAPI.h"
#include "Suora/Platform/OpenGL/OpenGLVertexArray.h"
#include "Suora/Platform/OpenGL/OpenGLFramebuffer.h"
#include "Suora/Platform/OpenGL/OpenGLBuffer.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Core/Log.h"

#include <glad/glad.h>

namespace Suora 
{
	
	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:		 SUORA_CRITICAL(LogCategory::Rendering, message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:       SUORA_ERROR(LogCategory::Rendering, message); return;
			case GL_DEBUG_SEVERITY_LOW:          SUORA_WARN(LogCategory::Rendering, message); return;
			case GL_DEBUG_SEVERITY_NOTIFICATION: SUORA_TRACE(LogCategory::Rendering, message); return;
		}
		
		SUORA_ASSERT(false, "Unknown severity level!");
	}

	void OpenGLRendererAPI::Init()
	{
	#ifdef SUORA_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	#endif

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);

		glEnable(GL_CULL_FACE);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const Vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glUseProgram(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	void OpenGLRendererAPI::ClearDepth()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetDepthTest(bool enabled)
	{
		if (enabled) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);
	}
	void OpenGLRendererAPI::SetDepthMask(bool enabled)
	{
		if (enabled)
		{
			glDepthMask(1);
			glDepthFunc(GL_ALWAYS);
		}
		else
		{
			glDepthMask(1);
			glDepthFunc(GL_LEQUAL);
		}
	}

	void OpenGLRendererAPI::SetCullingMode(CullingMode mode)
	{
		if (mode == CullingMode::None)
		{
			glDisable(GL_CULL_FACE);
		}
		else if (mode == CullingMode::Backface)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
		else if (mode == CullingMode::Frontface)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
		}
		/*if (enabled) glEnable(GL_CULL_FACE);
		else glDisable(GL_CULL_FACE);*/
	}

	void OpenGLRendererAPI::UIAlphaBlending()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	}

	void OpenGLRendererAPI::SetAlphaBlending(AlphaBlendMode alpha)
	{
		if (alpha != AlphaBlendMode::Disable)
		{
			glEnable(GL_BLEND);
			if (alpha == AlphaBlendMode::Additive)
			{
				//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				glBlendFunc(GL_ONE, GL_ONE);
			}
			if (alpha == AlphaBlendMode::Blend)
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			if (alpha == AlphaBlendMode::Constant)
			{
				glBlendFunc(GL_CONSTANT_COLOR, GL_ONE);
			}
			
		}
		else
		{
			glBlendFunc(GL_ONE, GL_ZERO);
			glDisable(GL_BLEND);
		}
	}
	void OpenGLRendererAPI::SetWireframeMode(bool b)
	{
		glPolygonMode(GL_FRONT_AND_BACK, b ? GL_LINE : GL_FILL);
	}
	void OpenGLRendererAPI::SetWireframeThickness(float value)
	{
		glLineWidth(value);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		//glBindTexture(GL_TEXTURE_2D, 0);
	}
	void OpenGLRendererAPI::DrawIndexed(VertexArray* vertexArray, uint32_t indexCount)
	{
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		//glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLRendererAPI::DrawInstanced(VertexArray* vertexArray, uint32_t instanceCount)
	{
		uint32_t count = vertexArray->GetIndexBuffer()->GetCount();
		glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr, instanceCount);
	}
	struct DrawElementsIndirectCommand
	{
		GLuint vertexCount;
		GLuint instanceCount;
		GLuint firstVertex;
		GLuint baseVertex;
		GLuint baseInstance;
	};

	void OpenGLRendererAPI::MultiDraw(const std::vector<Ref<VertexArray>>& arrays)
	{
		for (Ref<VertexArray> It : arrays)
		{
			It->Bind();

			uint32_t count = It->GetIndexBuffer()->GetCount();
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);

		}

	}

}
