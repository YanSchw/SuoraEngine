#pragma once
#include <array>
#include <future>
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Editor/Panels/Major/NodeClassEditor.h"
#include "Editor/EditorWindow.h"

namespace Suora
{

	class EditorCamera : public CameraNode
	{
	public:
		MajorTab* m_MajorTab = nullptr;
		EditorWindow* m_Window = nullptr;
		float m_DeltaX = 0, m_DeltaY = 0;
		float m_LastX = 0, m_LastY = 0;
		float m_FocusDistance = 1.0f;
		bool m_IsOrthographic = false;
		Vec3 m_Position, m_Rotation;

		// Saving Clips
		std::array<int, 10> m_ClipsSet = std::array<int, 10>();
		std::array<Vec3, 10> m_ClipsPos = {};
		std::array<Vec3, 10> m_ClipsRot = {};

		EditorCamera(EditorWindow* window, MajorTab* major);
		CameraNode* GetCameraComponent();

		void UpdateCamera(float deltaTime, bool cameraInput);

		void SaveClip(int id);
		void LoadClip(int id);

		void UpdatePerspective(float deltaTime, bool inputPossible);
		void UpdateOrthographic(float deltaTime, bool inputPossible);

	private:
		bool IsOverlayOnTop();
		int32_t m_InitalizedInEditor = 0;
	};
}