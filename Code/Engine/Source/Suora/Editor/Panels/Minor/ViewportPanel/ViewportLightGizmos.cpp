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

	void ViewportPointLightGizmo::DrawDebugGizmos(World* world, CameraNode* camera, int* pickingID, Map<int, Node*>* pickingMap, bool isHandlingMousePick)
	{
		Array<PointLightNode*> plights = world->FindNodesByClass<PointLightNode>();
		for (PointLightNode* light : plights)
		{
			Node3D tr;
			tr.SetPosition(light->GetPosition());
			tr.SetRotation(camera->GetRotation());
			Material* gizmoMat = ViewportDebugGizmo::GizmoMaterial(isHandlingMousePick, pickingID, Vec3(light->m_Color));
			const UniformSlot uniform = gizmoMat->m_UniformSlots[0];
			gizmoMat->m_UniformSlots[0].m_Texture2D = AssetManager::GetAsset<Texture2D>(SuoraID("f789d2bf-dcda-4e30-b2d9-3db979b7c6da"));
			Renderer3D::DrawMesh(camera, tr.GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("75f466f7-baec-4c5a-a23b-a5e3dc3d22bc")), gizmoMat, isHandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);
			gizmoMat->m_UniformSlots[0] = uniform;

			if (isHandlingMousePick)
			{
				(*pickingMap)[*pickingID] = light;
				(*pickingID)++;
			}
		}
	}

	static void Draw3DArrow(CameraNode* camera, DirectionalLightNode* node)
	{
		Array<std::pair<Vec3, Vec3>> lines;

		lines.Add({ Vec3(-0.35f, 0, 0), Vec3(+0.35f, 0, 0) });
		lines.Add({ Vec3(-0.35f, 0, 0), Vec3(-0.35f, 0, 1) });
		lines.Add({ Vec3(+0.35f, 0, 0), Vec3(+0.35f, 0, 1) });
		lines.Add({ Vec3(-0.35f, 0, 1), Vec3(-0.65f, 0, 1) });
		lines.Add({ Vec3(+0.35f, 0, 1), Vec3(+0.65f, 0, 1) });
		lines.Add({ Vec3(-0.65f, 0, 1), Vec3(0, 0, 2) });
		lines.Add({ Vec3(+0.65f, 0, 1), Vec3(0, 0, 2) });


		lines.Add({ Vec3(0, -0.35f, 0), Vec3(0, +0.35f, 0) });
		lines.Add({ Vec3(0, -0.35f, 0), Vec3(0, -0.35f, 1) });
		lines.Add({ Vec3(0, +0.35f, 0), Vec3(0, +0.35f, 1) });
		lines.Add({ Vec3(0, -0.35f, 1), Vec3(0, -0.65f, 1) });
		lines.Add({ Vec3(0, +0.35f, 1), Vec3(0, +0.65f, 1) });
		lines.Add({ Vec3(0, -0.65f, 1), Vec3(0, 0, 2) });
		lines.Add({ Vec3(0, +0.65f, 1), Vec3(0, 0, 2) });

		Node3D proxy;
		proxy.SetPosition(node->GetPosition());
		proxy.SetRotation(node->GetRotation());

		for (const std::pair<Vec3, Vec3>& line : lines)
		{
			const Vec4 a = proxy.GetTransformMatrix() * Vec4(line.first, 1);
			const Vec4 b = proxy.GetTransformMatrix() * Vec4(line.second, 1);
			Renderer3D::DrawLine3D(camera, a, b, node->m_Color);
		}
	}

	void ViewportDirectionalLightGizmo::DrawDebugGizmos(World* world, CameraNode* camera, int* pickingID, Map<int, Node*>* pickingMap, bool isHandlingMousePick)
	{
		Array<DirectionalLightNode*> dlights = world->FindNodesByClass<DirectionalLightNode>();
		for (DirectionalLightNode* light : dlights)
		{
			Node3D tr;
			tr.SetPosition(light->GetPosition());
			tr.SetRotation(camera->GetRotation());
			Material* gizmoMat = ViewportDebugGizmo::GizmoMaterial(isHandlingMousePick, pickingID, Vec3(1.0f));
			const UniformSlot uniform = gizmoMat->m_UniformSlots[0];
			gizmoMat->m_UniformSlots[0].m_Texture2D = AssetManager::GetAsset<Texture2D>(SuoraID("64738d74-08a9-4383-8659-620808d5269a"));
			Renderer3D::DrawMesh(camera, tr.GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("75f466f7-baec-4c5a-a23b-a5e3dc3d22bc")), gizmoMat, isHandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);
			gizmoMat->m_UniformSlots[0] = uniform;
			Node3D tr2;
			tr2.SetPosition(light->GetPosition());
			tr2.SetRotation(light->GetRotation());
			tr2.SetScale(Vec3(0.25f));

			Draw3DArrow(camera, light);

			if (isHandlingMousePick)
			{
				(*pickingMap)[*pickingID] = light;
				(*pickingID)++;
			}
		}
	}

}