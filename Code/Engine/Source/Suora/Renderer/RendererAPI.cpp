#include "Precompiled.h"
#include "Suora/Renderer/RendererAPI.h"

#include "Suora/Platform/OpenGL/OpenGLRendererAPI.h"

namespace Suora 
{

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	Scope<RendererAPI> RendererAPI::Create()
	{
		switch (s_API)
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateScope<OpenGLRendererAPI>();
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}