#include "Precompiled.h"
#include "ViewportModules.h"

#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/Nodes/Light/PointLightNode.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/Assets/Material.h"

namespace Suora
{

	void ViewportOriginGizmo::DrawDebugGizmos(World* world, CameraNode* camera, int* pickingID, Map<int, Node*>* pickingMap, bool isHandlingMousePick)
	{
		if (!GetViewport()->m_ShowOrigin)
		{
			return;
		}

		Renderer3D::DrawLine3D(camera, Vec3(-10000, 0, 0), Vec3(+10000, 0, 0), Color(210.0f / 255.0f, 77.0f / 255.0f, 96.0f / 255.0f, 0.65f));
		Renderer3D::DrawLine3D(camera, Vec3(0, -10000, 0), Vec3(0, +10000, 0), Color(134.0f / 255.0f, 205.0f / 255.0f, 13.0f / 255.0f, 0.65f));
		Renderer3D::DrawLine3D(camera, Vec3(0, 0, -10000), Vec3(0, 0, +10000), Color(73.0f / 255.0f, 137.0f / 255.0f, 203.0f / 255.0f, 0.65f));
	}

}