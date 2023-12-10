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
		PrintSuoraEngineAsciiArt();

		m_Engine = Engine::Create();

		SUORA_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		NativeInput::Init();

	}

	Application::~Application()
	{

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
		while (m_Running)
		{
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

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;

		return false;
	}

	void Application::PrintSuoraEngineAsciiArt()
	{
		SUORA_LOG(LogCategory::Core, LogLevel::Trace, "==================================================================================");

		SUORA_LOG(LogCategory::Core, LogLevel::Trace, "________                               __________              _____             ");
		SUORA_LOG(LogCategory::Core, LogLevel::Trace, "__  ___/___  ____________________ _    ___  ____/_____________ ___(_)___________ ");
		SUORA_LOG(LogCategory::Core, LogLevel::Trace, "_____ \\_  / / /  __ \\_  ___/  __ `/    __  __/  __  __ \\_  __ `/_  /__  __ \\  _ \\");
		SUORA_LOG(LogCategory::Core, LogLevel::Trace, "____/ // /_/ // /_/ /  /   / /_/ /     _  /___  _  / / /  /_/ /_  / _  / / /  __/");
		SUORA_LOG(LogCategory::Core, LogLevel::Trace, "/____/ \\__,_/ \\____//_/    \\__,_/      /_____/  /_/ /_/_\\__, / /_/  /_/ /_/\\___/ ");
		SUORA_LOG(LogCategory::Core, LogLevel::Trace, "                                                       /____/                    ");

		SUORA_LOG(LogCategory::Core, LogLevel::Trace, "== Version {0}.{1}.{2} =================================================================", SUORA_VERSION_MAJOR, SUORA_VERSION_MINOR, SUORA_VERSION_PATCH);
	}

}
