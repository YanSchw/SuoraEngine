#pragma once
#include "Suora/Core/Base.h"

namespace Suora
{
	int SuoraMain(int argc, char** argv)
	{
		Suora::Log::Init();

		Suora::CmdArgs args;
		for (int i = 0; i < argc; i++)
		{
			args.Args.push_back({ String(argv[i]) });
		}

		auto app = Suora::CreateApplication();

		if (app)
		{
			app->m_CommandLineArgs = args;

			app->Run();

			delete app;
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
