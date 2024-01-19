#include "Precompiled.h"
#include "Suora/Renderer/GraphicsContext.h"

#include "Suora/Renderer/RendererAPI.h"
#include "Suora/Platform/OpenGL/OpenGLContext.h"

namespace Suora 
{

	Scope<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}