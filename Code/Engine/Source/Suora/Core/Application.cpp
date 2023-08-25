#include "Precompiled.h"
#include "Suora/Core/Application.h"

#include "Suora/Core/Log.h"
#include "Suora/Core/Engine.h"

#include "Suora/Renderer/RendererAPI.h"
#include "Suora/Renderer/GraphicsContext.h"

#include "Suora/Core/NativeInput.h"
#include "Platform/Platform.h"

namespace Suora 
{

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		SUORA_PROFILE_FUNCTION();
		m_Engine = Engine::Create();

		SUORA_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		NativeInput::Init();

	}

	Application::~Application()
	{
		SUORA_PROFILE_FUNCTION();

	}

	Window* Application::CreateAppWindow(const WindowProps& props)
	{
		Window* window = Window::CreatePtr(props);
		window->SetEventCallback(SUORA_BIND_EVENT_FN(Application::OnEvent));
		static bool init = false;
		if (!init)
		{
			init = true;
			RendererAPI::Create();
		}

		return window;
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnEvent(Event& e)
	{
		SUORA_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(SUORA_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(SUORA_BIND_EVENT_FN(Application::OnWindowResize));

	}

	bool Application::IsApplicationInitialized()
	{
		return s_Instance;
	}

	void Application::Run()
	{
		SUORA_PROFILE_FUNCTION();

		while (m_Running)
		{
			Instrumentor::Get().EndFrame();
			SUORA_PROFILE_SCOPE("RunLoop");

			m_Engine->Tick();

			Update(m_Engine->GetDeltaTime());
		}
	}

	void Application::Update(float deltaTime)
	{

	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		SUORA_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;

		return false;
	}

}
