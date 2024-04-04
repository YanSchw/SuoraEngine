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

	void ViewportGridGizmo::DrawDebugGizmos(World* world, CameraNode* camera, int* pickingID, Map<int, Node*>* pickingMap, bool isHandlingMousePick)
	{
		const int32_t x = static_cast<int32_t>(camera->GetPosition().x);
		const int32_t z = static_cast<int32_t>(camera->GetPosition().z);

		const float absPos = glm::abs(camera->GetPosition().y);
		const Color color = absPos <= 2.0f ? Color(0.5f) : Color(1.0f) / absPos;

		for (int32_t i = x - 50; i <= x + 50; i++)
		{
			Renderer3D::DrawLine3D(camera, Vec3(i, 0, z - 50), Vec3(i, 0, z + 50), color);
		}
		for (int32_t i = z - 50; i <= z + 50; i++)
		{
			Renderer3D::DrawLine3D(camera, Vec3(x - 50, 0, i), Vec3(x + 50, 0, i), color);
		}
	}

}