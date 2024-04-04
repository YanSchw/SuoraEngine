#include "Precompiled.h"
#include "ViewportModules.h"

#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/Assets/Material.h"

namespace Suora
{

	void ViewportCameraGizmo::DrawDebugGizmos(World* world, CameraNode* camera, int* pickingID, Map<int, Node*>* pickingMap, bool isHandlingMousePick)
	{
		Array<CameraNode*> cameras = world->FindNodesByClass<CameraNode>();
		for (CameraNode* cam : cameras)
		{
			cam->SetAspectRatio(static_cast<float>(GetViewport()->GetWidth()) / static_cast<float>(GetViewport()->GetHeight()));
			cam->RecalculateProjection();
			Node3D tr;
			tr.SetPosition(cam->GetPosition());
			tr.SetRotation(cam->GetRotation());
			tr.SetScale(Vec3(0.1f));
			Renderer3D::DrawMesh(camera, cam->GetTransformMatrix(), *AssetManager::GetAsset<Mesh>(SuoraID("c37609b9-9067-4e3a-ac04-c493ed2d8009")), GizmoMaterial(isHandlingMousePick, pickingID, Vec3(1.0f), SuoraID("317cf1ef-ac75-46d8-a62f-184891456960")), isHandlingMousePick ? MaterialType::ObjectID : MaterialType::Material);

			Vec3 corners[4];
			{
				Vec4 pos = glm::inverse(cam->GetViewProjectionMatrix()) * Vec4(-1.0f, 1.0f, 1.0f, 1.0f);
				pos.x /= pos.w;
				pos.y /= pos.w;
				pos.z /= pos.w;
				Vec3 end = glm::normalize(((Vec3)pos) - cam->GetPosition()) * 15.0f + cam->GetPosition();
				corners[0] = end;
				Renderer3D::DrawLine3D(camera, cam->GetPosition(), end, Color(0.66f, 1.0f, 0.66f, 0.5f));
			}
			{
				Vec4 pos = glm::inverse(cam->GetViewProjectionMatrix()) * Vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pos.x /= pos.w;
				pos.y /= pos.w;
				pos.z /= pos.w;
				Vec3 end = glm::normalize(((Vec3)pos) - cam->GetPosition()) * 15.0f + cam->GetPosition();
				corners[1] = end;
				Renderer3D::DrawLine3D(camera, cam->GetPosition(), end, Color(0.66f, 1.0f, 0.66f, 0.5f));
			}
			{
				Vec4 pos = glm::inverse(cam->GetViewProjectionMatrix()) * Vec4(-1.0f, -1.0f, 1.0f, 1.0f);
				pos.x /= pos.w;
				pos.y /= pos.w;
				pos.z /= pos.w;
				Vec3 end = glm::normalize(((Vec3)pos) - cam->GetPosition()) * 15.0f + cam->GetPosition();
				corners[2] = end;
				Renderer3D::DrawLine3D(camera, cam->GetPosition(), end, Color(0.66f, 1.0f, 0.66f, 0.5f));
			}
			{
				Vec4 pos = glm::inverse(cam->GetViewProjectionMatrix()) * Vec4(1.0f, -1.0f, 1.0f, 1.0f);
				pos.x /= pos.w;
				pos.y /= pos.w;
				pos.z /= pos.w;
				Vec3 end = glm::normalize(((Vec3)pos) - cam->GetPosition()) * 15.0f + cam->GetPosition();
				corners[3] = end;
				Renderer3D::DrawLine3D(camera, cam->GetPosition(), end, Color(0.66f, 1.0f, 0.66f, 0.5f));
			}
			Renderer3D::DrawLine3D(camera, corners[0], corners[1], Color(0.66f, 1.0f, 0.66f, 0.5f));
			Renderer3D::DrawLine3D(camera, corners[1], corners[3], Color(0.66f, 1.0f, 0.66f, 0.5f));
			Renderer3D::DrawLine3D(camera, corners[2], corners[0], Color(0.66f, 1.0f, 0.66f, 0.5f));
			Renderer3D::DrawLine3D(camera, corners[3], corners[2], Color(0.66f, 1.0f, 0.66f, 0.5f));

			if (isHandlingMousePick)
			{
				(*pickingMap)[*pickingID] = cam;
				(*pickingID)++;
			}
		}
	}

}