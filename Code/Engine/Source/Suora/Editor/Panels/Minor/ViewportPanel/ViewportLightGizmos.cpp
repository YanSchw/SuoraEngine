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
			Renderer3D::DrawMesh(camera, tr2.GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("0eb4c94e-e88e-436e-a803-12739b755f0c")), ViewportDebugGizmo::GizmoMaterial(isHandlingMousePick, pickingID, Vec3(1.0f)), isHandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);

			if (isHandlingMousePick)
			{
				(*pickingMap)[*pickingID] = light;
				(*pickingID)++;
			}
		}
	}

}