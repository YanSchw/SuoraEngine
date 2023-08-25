#include "EditorCamera.h"

namespace Suora
{

	EditorCamera::EditorCamera(EditorWindow* window, MajorTab* major)
	{
		m_Window = window;
		m_MajorTab = major;
		m_Position = GetPosition();
		m_Rotation = GetEulerRotation();
		//SaveClip(0);
	}

	CameraNode* EditorCamera::GetCameraComponent()
	{
		return this;
	}

	void EditorCamera::UpdateCamera(float deltaTime, bool cameraInput)
	{
		if (m_InitalizedInEditor++ < 3)
		{
			m_Position = GetPosition();
		}

		m_DeltaX = NativeInput::GetMouseDelta().x;
		m_DeltaY = NativeInput::GetMouseDelta().y;

		if (!m_IsOrthographic) UpdatePerspective(deltaTime, cameraInput);
		else				   UpdateOrthographic(deltaTime, cameraInput);

		m_LastX = NativeInput::GetMousePosition().x;
		m_LastY = NativeInput::GetMousePosition().y;


		// Clips
		{
			bool save = NativeInput::IsKeyPressed(Key::LeftControl);
			if (NativeInput::IsKeyPressed(Key::D1)) if (save) SaveClip(0); else if (m_ClipsSet[0] == 1) LoadClip(0);
			if (NativeInput::IsKeyPressed(Key::D2)) if (save) SaveClip(1); else if (m_ClipsSet[1] == 1) LoadClip(1);
			if (NativeInput::IsKeyPressed(Key::D3)) if (save) SaveClip(2); else if (m_ClipsSet[2] == 1) LoadClip(2);
			if (NativeInput::IsKeyPressed(Key::D4)) if (save) SaveClip(3); else if (m_ClipsSet[3] == 1) LoadClip(3);
			if (NativeInput::IsKeyPressed(Key::D5)) if (save) SaveClip(4); else if (m_ClipsSet[4] == 1) LoadClip(4);
			if (NativeInput::IsKeyPressed(Key::D6)) if (save) SaveClip(5); else if (m_ClipsSet[5] == 1) LoadClip(5);
			if (NativeInput::IsKeyPressed(Key::D7)) if (save) SaveClip(6); else if (m_ClipsSet[6] == 1) LoadClip(6);
			if (NativeInput::IsKeyPressed(Key::D8)) if (save) SaveClip(7); else if (m_ClipsSet[7] == 1) LoadClip(7);
			if (NativeInput::IsKeyPressed(Key::D9)) if (save) SaveClip(8); else if (m_ClipsSet[8] == 1) LoadClip(8);
			if (NativeInput::IsKeyPressed(Key::D0)) if (save) SaveClip(9); else if (m_ClipsSet[9] == 1) LoadClip(9);
		}

		if (!cameraInput) return;

		// Focus on Actor
		if (NativeInput::GetKeyDown(Key::F) && m_MajorTab->IsA<NodeClassEditor>() && m_MajorTab->As<NodeClassEditor>()->m_SelectedObject && m_MajorTab->As<NodeClassEditor>()->m_SelectedObject->IsA<Node3D>())
		{
			Node3D* node3D = m_MajorTab->As<NodeClassEditor>()->m_SelectedObject->As<Node3D>();
			m_Position = node3D->GetPosition();
			m_Position -= GetForwardVector() * 10.0f;
			m_FocusDistance = 10;
		}

		// Orthographics Mode
		if (NativeInput::GetKeyDown(Key::KP5))
		{
			m_IsOrthographic = !m_IsOrthographic;
			if (m_IsOrthographic)
			{
				SetProjectionType(CameraNode::ProjectionType::Orthographic);
				SetOrthographic(10.0f, 0.0f, 1000.0f);
				int clampRotX = Math::RoundToInt(m_Rotation.x / 90.0f); m_Rotation.x = (float)clampRotX * 90.0f;
				int clampRotY = Math::RoundToInt(m_Rotation.y / 90.0f); m_Rotation.y = (float)clampRotY * 90.0f;
			}
			else
			{
				SetProjectionType(CameraNode::ProjectionType::Perspective);
			}
		}
		if (NativeInput::GetKeyDown(Key::KP8)) m_Rotation.x += 90;
		if (NativeInput::GetKeyDown(Key::KP2)) m_Rotation.x -= 90;
		if (NativeInput::GetKeyDown(Key::KP4)) m_Rotation.y += 90;
		if (NativeInput::GetKeyDown(Key::KP6)) m_Rotation.y -= 90;

		// Update Position and Rotation
		SetPosition(Math::Lerp(GetPosition(), m_Position, 50.0f * deltaTime));
		SetEulerRotation(m_Rotation);
		//SetEulerRotation(Math::Lerp(GetEulerRotation(), m_Rotation, 50.0f * deltaTime));
	}

	void EditorCamera::SaveClip(int id)
	{
		m_ClipsSet[id] = 1;
		m_ClipsPos[id] = m_Position;
		m_ClipsRot[id] = m_Rotation;
	}
	void EditorCamera::LoadClip(int id)
	{
		m_Position = m_ClipsPos[id];
		m_Rotation = m_ClipsRot[id];
	}

	void EditorCamera::UpdatePerspective(float deltaTime, bool inputPossible)
	{
		if (NativeInput::IsMouseButtonPressed(Mouse::Button0) && inputPossible && !IsOverlayOnTop())
		{
			m_Window->GetWindow()->SetCursorLocked(true);
			m_Window->m_InputEvent = EditorInputEvent::Viewport_EditorCamera;
			NativeInput::GetMouseDelta();

			if (NativeInput::GetKey(Key::LeftAlt))
			{
				Vec3 center = m_Position + GetForwardVector() * m_FocusDistance;

				m_Rotation += Vec::Up * m_DeltaX * 0.5f;
				m_Rotation += Vec::Right * m_DeltaY * 0.5f;
				SetEulerRotation(m_Rotation);
				m_Position = center - GetForwardVector() * m_FocusDistance;
				SetPosition(m_Position);
			}
			else
			{
				m_Rotation += Vec::Up * m_DeltaX * 0.5f;
				Vec3 forward = GetForwardVector(); forward.y = 0; Vec::Normalize(forward);
				m_Position -= forward * 0.1f * m_DeltaY;
			}
		}
		else if (NativeInput::IsMouseButtonPressed(Mouse::Button1) && inputPossible && !IsOverlayOnTop())
		{
			m_Window->GetWindow()->SetCursorLocked(true);
			m_Window->m_InputEvent = EditorInputEvent::Viewport_EditorCamera;
			NativeInput::GetMouseDelta();

			m_Rotation += Vec::Up * m_DeltaX * 0.5f;
			m_Rotation += Vec::Right * m_DeltaY * 0.5f;

			m_Rotation.x = Math::Clamp(m_Rotation.x, -89.0f, 89.0f);

			const float SPEED = (NativeInput::IsKeyPressed(Key::LeftControl) ? 1.0f : (NativeInput::IsKeyPressed(Key::LeftShift) ? 12.0f : 6.0f));
			if (NativeInput::IsKeyPressed(Key::W)) m_Position += GetForwardVector() * SPEED * deltaTime;
			if (NativeInput::IsKeyPressed(Key::S)) m_Position -= GetForwardVector() * SPEED * deltaTime;
			if (NativeInput::IsKeyPressed(Key::A)) m_Position -= GetRightVector() * SPEED * deltaTime;
			if (NativeInput::IsKeyPressed(Key::D)) m_Position += GetRightVector() * SPEED * deltaTime;

			if (NativeInput::IsKeyPressed(Key::E)) m_Position += Vec::Up * SPEED * deltaTime;
			if (NativeInput::IsKeyPressed(Key::Q)) m_Position -= Vec::Up * SPEED * deltaTime;


		}
		else if (NativeInput::IsMouseButtonPressed(Mouse::ButtonMiddle) && inputPossible && !IsOverlayOnTop())
		{
			m_Window->GetWindow()->SetCursorLocked(true);
			m_Window->m_InputEvent = EditorInputEvent::Viewport_EditorCamera;

			m_Position += GetRightVector() * 0.1f * m_DeltaX;
			m_Position -= GetUpVector() * 0.1f * m_DeltaY;
		}
		else if (inputPossible && !IsOverlayOnTop())
		{
			m_Window->GetWindow()->SetCursorLocked(false);
			m_Window->m_InputEvent = EditorInputEvent::None;
		}

		if (inputPossible)
		{
			m_Position += GetForwardVector() * NativeInput::GetMouseScrollDelta();
			m_FocusDistance -= NativeInput::GetMouseScrollDelta();
		}

		SetPerspectiveFarClip(1000.0f);
	}


	void EditorCamera::UpdateOrthographic(float deltaTime, bool inputPossible)
	{
		if (NativeInput::IsMouseButtonPressed(Mouse::Button1) && inputPossible)
		{
			//m_Window->GetWindow()->SetCursorLocked(true);
			m_Window->m_InputEvent = EditorInputEvent::Viewport_EditorCamera;

			m_Position -= GetRightVector() * 0.1f * m_DeltaX;
			m_Position += GetUpVector() * 0.1f * m_DeltaY;
		}
		else if (inputPossible)
		{
			m_Window->m_InputEvent = EditorInputEvent::None;
		}

		float orthoSize = GetOrthographicSize();
		SetOrthographicSize(orthoSize - NativeInput::GetMouseScrollDelta());

	}

	bool EditorCamera::IsOverlayOnTop()
	{
		return EditorUI::_GetOverlays().Size() > 0;
	}
}