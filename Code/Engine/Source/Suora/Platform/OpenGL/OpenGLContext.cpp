#include "Precompiled.h"
#include "Suora/Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Suora 
{

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		SUORA_ASSERT(windowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		SUORA_ASSERT(status, "Failed to initialize Glad!");

		SUORA_INFO(LogCategory::Rendering, "OpenGL Info:");
		SUORA_INFO(LogCategory::Rendering, "  Vendor: {0}", glGetString(GL_VENDOR));
		SUORA_INFO(LogCategory::Rendering, "  Renderer: {0}", glGetString(GL_RENDERER));
		SUORA_INFO(LogCategory::Rendering, "  Version: {0}", glGetString(GL_VERSION));

		SUORA_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "Suora requires at least OpenGL version 4.5!");
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}

	void OpenGLContext::MakeCurrent()
	{
		glfwMakeContextCurrent(m_WindowHandle);
	}

}
