#pragma once
#include <string>

#include "Suora/Core/Base.h"

#include "Suora/Core/Window.h"
#include "Suora/Events/Event.h"
#include "Suora/Events/ApplicationEvent.h"

namespace Suora
{
	int SuoraMain(int argc, char** argv);
	class Engine;

	struct CmdArg
	{
		std::string Arg;
	}; 
	struct CmdArgs
	{
		std::vector<CmdArg> Args;
	};

	/** To be implemented in your Application of choice. E.g. Editor, Runtime, etc. */
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void OnEvent(Event& e);

		Window* CreateAppWindow(const WindowProps& props);
		Window& GetWindow() { return *m_Window; }

		virtual void Close();

		static Application& Get() { return *s_Instance; }
		static bool IsApplicationInitialized();
	private:
		Ref<Engine> m_Engine = nullptr;
		void Run();
		virtual void Update(float deltaTime);
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		bool m_Minimized = false;
	protected:
		CmdArgs m_CommandLineArgs;
	private:
		static Application* s_Instance;

		friend class Engine;
		friend int Suora::SuoraMain(int argc, char** argv);
	};

	// To be defined...
	Application* CreateApplication();

}