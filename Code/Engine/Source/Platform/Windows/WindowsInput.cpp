#include "Precompiled.h"
#include "Suora/Core/NativeInput.h"

#include "Suora/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Suora 
{

	bool NativeInput::IsKeyPressed(const KeyCode key)
	{
		if (!Window::s_CurrentFocusedWindow) return false;

		auto* window = static_cast<GLFWwindow*>(Window::s_CurrentFocusedWindow->GetNativeWindow());
		auto state = glfwGetKey(window, static_cast<int32_t>(key));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool NativeInput::IsMouseButtonPressed(const MouseCode button)
	{
		if (!Window::s_CurrentFocusedWindow) return false;

		auto* window = static_cast<GLFWwindow*>(Window::s_CurrentFocusedWindow->GetNativeWindow());
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}

	glm::vec2 NativeInput::GetMousePosition()
	{
		if (!Window::s_CurrentFocusedWindow) return glm::vec2(0);

		auto* window = static_cast<GLFWwindow*>(Window::s_CurrentFocusedWindow->GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float NativeInput::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float NativeInput::GetMouseY()
	{
		return GetMousePosition().y;
	}

	void NativeInput::NativeTick(float deltaTime)
	{
		auto* window = static_cast<GLFWwindow*>(Window::s_CurrentFocusedWindow->GetNativeWindow());

		// Fills Event Map (!)
		for (auto& it : s_DownKeyEvents) s_KeyEvents[it.first] = s_KeyEvents[it.first];
		for (auto& it : s_UpKeyEvents) s_KeyEvents[it.first] = s_KeyEvents[it.first];
		for (auto& it : s_LastKeyEvents) s_LastKeyEvents[it.first] = s_KeyEvents[it.first];

		// Update Events
		for (auto& it : s_KeyEvents)
		{
			auto state = glfwGetKey(window, static_cast<int32_t>(it.first));
			it.second = (bool)(state == GLFW_PRESS || state == GLFW_REPEAT);
		}
		for (auto& it : s_DownKeyEvents)
		{
			it.second = (!s_LastKeyEvents[it.first] && s_KeyEvents[it.first]);
		}
		for (auto& it : s_UpKeyEvents)
		{
			it.second = (s_LastKeyEvents[it.first] && !s_KeyEvents[it.first]);
		}


		// Fills Event Map (!)
		for (auto& it : s_DownMouseEvents) s_MouseEvents[it.first] = s_MouseEvents[it.first];
		for (auto& it : s_UpMouseEvents) s_MouseEvents[it.first] = s_MouseEvents[it.first];
		for (auto& it : s_LastMouseEvents) s_LastMouseEvents[it.first] = s_MouseEvents[it.first];

		// Update Events
		for (auto& it : s_MouseEvents)
		{
			auto state = glfwGetMouseButton(window, static_cast<int32_t>(it.first));
			it.second = (bool)(state == GLFW_PRESS || state == GLFW_REPEAT);
		}
		for (auto& it : s_DownMouseEvents)
		{
			it.second = (!s_LastMouseEvents[it.first] && s_MouseEvents[it.first]);
		}
		for (auto& it : s_UpMouseEvents)
		{
			it.second = (s_LastMouseEvents[it.first] && !s_MouseEvents[it.first]);
		}

	}

}