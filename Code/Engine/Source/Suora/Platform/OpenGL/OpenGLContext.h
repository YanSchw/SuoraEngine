#pragma once

#include "Suora/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Suora 
{

	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;
		virtual void MakeCurrent() override;
	private:
		GLFWwindow* m_WindowHandle;
	};

}