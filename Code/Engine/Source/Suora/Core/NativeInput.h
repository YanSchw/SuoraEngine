#pragma once
#include <unordered_map>
#include "CoreMinimal.h"
#include "Suora/Common/Delegate.h"
#include "Suora/Core/KeyCodes.h"
#include "Suora/Core/MouseCodes.h"

namespace Suora 
{

	/** Not used for Gameplay */
	class NativeInput
	{
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static bool WasLeftMouseButtonPressed()
		{
			return !s_LastFrameLeftMouseButton && IsMouseButtonPressed(Mouse::ButtonLeft);
		}
		static Vec2 GetMousePosition();

		inline static Vec2 s_LastMousePosition = Vec2(), s_DeltaMousePosition = Vec2();
		static Vec2 GetMouseDelta()
		{
			return s_DeltaMousePosition;
		}
		static float GetMouseX();
		static float GetMouseY();
		static float GetMouseScrollDelta()
		{
			return s_ScrollDelta;
		}

		static bool GetKey(KeyCode key)
		{
			return s_KeyEvents[key];
		}
		static bool GetKeyDown(KeyCode key)
		{
			return s_DownKeyEvents[key];
		}
		static bool GetKeyUp(KeyCode key)
		{
			return s_UpKeyEvents[key];
		}
		static bool GetMouseButton(MouseCode button)
		{
			return s_MouseEvents[button];
		}
		static bool GetMouseButtonDown(MouseCode button)
		{
			return s_DownMouseEvents[button];
		}
		static bool GetMouseButtonUp(MouseCode button)
		{
			return s_UpMouseEvents[button];
		}

		inline static Delegate<char> s_CharInputCallback;

		static void ResetMouseDelta()
		{
			s_ResetMouseDelta = true;
			s_DeltaMousePosition = Vec2(0.0f);
			s_LastMousePosition = GetMousePosition();
		}

		static void ConsumeInput()
		{
			ResetMouseDelta();
			for (auto& it : s_KeyEvents) it.second = false;
			for (auto& it : s_DownKeyEvents) it.second = false;
			for (auto& it : s_UpKeyEvents) it.second = false;

			for (auto& it : s_MouseEvents) it.second = false;
			for (auto& it : s_LastMouseEvents) it.second = false;
			for (auto& it : s_DownMouseEvents) it.second = false;
			for (auto& it : s_UpMouseEvents) it.second = false;
		}

	private:
		inline static std::unordered_map<KeyCode, bool> s_KeyEvents, s_LastKeyEvents, s_DownKeyEvents, s_UpKeyEvents;
		inline static std::unordered_map<MouseCode, bool> s_MouseEvents, s_LastMouseEvents, s_DownMouseEvents, s_UpMouseEvents;
		inline static float s_GlobalScrollDelta = 0;
		inline static float s_ScrollDelta = 0;
		inline static bool s_LastFrameLeftMouseButton = false;
		inline static bool s_ResetMouseDelta = false;

		static void Init()
		{
		}
		static void Tick(float deltaTime)
		{
			NativeTick(deltaTime);

			s_DeltaMousePosition = GetMousePosition() - s_LastMousePosition;
			s_LastMousePosition = GetMousePosition();
			if (s_ResetMouseDelta)
			{
				s_ResetMouseDelta = false;
				s_DeltaMousePosition = Vec2(0.0f);
			}
			s_LastFrameLeftMouseButton = IsMouseButtonPressed(Mouse::ButtonLeft);
			s_ScrollDelta = s_GlobalScrollDelta;
			s_GlobalScrollDelta = 0;
		}
		static void NativeTick(float deltaTime);

		friend class Application;
		friend class Engine;
		friend class WindowsWindow;

	};
}