#pragma once
#include "Suora/Core/Base.h"
#include <iostream>

namespace Suora
{
	int SuoraMain(int argc, char** argv)
	{
		Suora::Log::Init();

		Suora::CmdArgs args;
		for (int i = 0; i < argc; i++)
		{
			args.Args.push_back({ std::string(argv[i]) });
		}

		SUORA_PROFILE_BEGIN_SESSION("Startup", "SuoraProfile-Startup.json");
		auto app = Suora::CreateApplication();
		SUORA_PROFILE_END_SESSION();

		if (app)
		{
			app->m_CommandLineArgs = args;

			SUORA_PROFILE_BEGIN_SESSION("Runtime", "SuoraProfile-Runtime.json");
			app->Run();
			SUORA_PROFILE_END_SESSION();

			SUORA_PROFILE_BEGIN_SESSION("Shutdown", "SuoraProfile-Shutdown.json");
			delete app;
			SUORA_PROFILE_END_SESSION();
		}
		return 0;
	}
}

#ifdef SUORA_PLATFORM_WINDOWS

extern Suora::Application* Suora::CreateApplication();

int main(int argc, char** argv)
{
	return Suora::SuoraMain(argc, argv);
}
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return Suora::SuoraMain(0, nullptr);
}

#endif
