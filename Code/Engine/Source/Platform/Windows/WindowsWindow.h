#pragma once

#include "Suora/Core/Window.h"
#include "Suora/Renderer/GraphicsContext.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Suora 
{

	enum class Cursor;

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		unsigned int GetWidth() const override { return m_Data.Width; }
		unsigned int GetHeight() const override { return m_Data.Height; }

		// Window attributes
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
		bool IsMaximized() const override;
		void SetCursor(Cursor cursor) override;
		Cursor GetCursor() override;
		void SetCursorLocked(bool locked) override;
		bool IsWindowResizing();

		virtual void* GetNativeWindow() const { return m_Window; }
		virtual void* GetGraphicsContext() const { return m_Context.get(); }
		virtual bool IsUndecorated() const;
		virtual void Maximize() override;
		virtual void Iconify() override;
		virtual void CenterWindow() override;

		virtual void SetFullscreen(bool fullscreen) override;
		virtual bool IsFullscreen() override;
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;
		GLFWcursor* m_Cursor = nullptr;
		Ref<GraphicsContext> m_Context;
		WindowProps m_Props;

		class Texture2D* m_CurrentIconTexture = nullptr;
		void LoadIconTexture(Texture2D* icon);

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;
			Window* window = nullptr;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;


	};

}