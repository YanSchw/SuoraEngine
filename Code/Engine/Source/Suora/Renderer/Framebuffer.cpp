#include "Precompiled.h"
#include "Suora/Renderer/Framebuffer.h"

#include "Suora/Renderer/RendererAPI.h"

#include "Suora/Platform/OpenGL/OpenGLFramebuffer.h"

namespace Suora 
{
	
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLFramebuffer>(spec);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}

