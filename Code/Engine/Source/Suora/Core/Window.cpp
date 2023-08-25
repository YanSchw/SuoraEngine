#include "Precompiled.h"
#include "Window.h"

#ifdef SUORA_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsWindow.h"
#endif

namespace Suora
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
	#ifdef SUORA_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
	#else
		SUORA_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
	#endif
	}

	Window* Window::CreatePtr(const WindowProps& props)
	{
#ifdef SUORA_PLATFORM_WINDOWS
		return new WindowsWindow(props);
#else
		SUORA_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}

//	const bool Window::IsEditor() const
//	{
//#ifdef SUORA_EDITOR
//		return true;
//#endif // SUORA_EDITOR
//		return false;
//	}

}