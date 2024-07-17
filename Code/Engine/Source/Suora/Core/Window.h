#pragma once

#include "Suora/Core/Base.h"
#include "Suora/Events/Event.h"
#include "Suora/Common/Array.h"
#include "Suora/Common/Delegate.h"

namespace Suora 
{
	class Texture2D;

	enum class Cursor
	{
		Default,
		Hand,
		Crosshair,
		IBeam,
		HorizontalResize,
		VerticalResize
	};

	struct WindowProps
	{
		String Title;
		uint32_t Width;
		uint32_t Height;
		bool isDecorated;
		bool hasTitlebar;

		WindowProps(const String& title = "",
			        uint32_t width = 1280,
			        uint32_t height = 720,
					bool _isDecorated = true,
					bool _hasTitlebar = true)
			: Title(title), Width(width), Height(height), isDecorated(_isDecorated), hasTitlebar(_hasTitlebar)
		{
		}
	};

	/** To be used in Desktop-Applications */
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual void SetCursor(Cursor cursor) = 0;
		virtual Cursor GetCursor() = 0;
		virtual void SetCursorLocked(bool locked) = 0;
		virtual bool IsCursorLocked() = 0;
		virtual bool IsVSync() const = 0;
		virtual void SetTitle(const String& title) = 0;
		virtual void SetSize(uint32_t width, uint32_t height) = 0;

		virtual void* GetNativeWindow() const = 0;
		virtual void* GetGraphicsContext() const = 0;
		virtual bool IsUndecorated() const = 0;
		virtual void Minimize() = 0;
		virtual bool IsMaximized() const = 0;
		virtual void Maximize() = 0;
		virtual void Iconify() = 0;
		virtual void RegisterOverTitlebar(bool value) = 0;
		virtual void CenterWindow() = 0;

		virtual void SetFullscreen(bool fullscreen) = 0;
		virtual bool IsFullscreen() = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
		static Window* CreatePtr(const WindowProps& props = WindowProps());
		inline static Array<Window*> s_AllWindows = Array<Window*>();
		inline static Window* s_CurrentFocusedWindow = nullptr;

		Cursor m_CurrentCursorType = Cursor::Default;

		Texture2D* m_WindowIconOverride = nullptr;

		Delegate<Array<String>> m_OnDesktopFilesDropped;
	};


}