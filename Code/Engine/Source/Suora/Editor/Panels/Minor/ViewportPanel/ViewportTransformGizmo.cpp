#include "Precompiled.h"
#include "ViewportPanel.h"
#include "Suora/Editor/Panels/MajorTab.h"
#include "Suora/Editor/Util/EditorCamera.h"
#include "Suora/Editor/Util/EditorPreferences.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Core/Engine.h"
#include "Suora/GameFramework/GameInstance.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/Nodes/Light/PointLightNode.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/InputModule.h"

namespace Suora
{
	static bool IntersectRaySphere(const Vec3& pos, const Vec3& dir, const Vec3& spherePos, float sphereRadius, float& t, Vec3& q)
	{
		Vec3 oc = pos - spherePos;
		float a = glm::dot(dir, dir);
		float b = 2.0 * glm::dot(oc, dir);
		float c = dot(oc, oc) - sphereRadius * sphereRadius;
		float discriminant = b * b - 4 * a * c;
		return (discriminant > 0);
	}

	static float RayRayTime(const Vec3& pos1, const Vec3& dir1, const Vec3& pos2, const Vec3& dir2, float& t, float& u, Vec3& p1, Vec3& p2)
	{
		const float a = glm::dot(dir1, dir1);
		const float b = glm::dot(dir1, dir2);
		const float c = glm::dot(dir2, dir2);
		const float d = glm::dot(dir1, (pos2 - pos1));
		const float e = glm::dot(dir2, (pos2 - pos1));
		const float DD = a * c - b * b;

		if (DD == 0.0f) return 0.0f;

		t = (b * e - c * d) / DD;
		u = (a * e - b * d) / DD;

		p1 = (pos1 - t * dir1);
		p2 = (pos2 - u * dir2);
		return glm::distance(p1, p2);
	}

	// Source: https://stackoverflow.com/questions/23975555/how-to-do-ray-plane-intersection
	static bool RayPlane(const Vec3& normal, const Vec3& planeCenter, const Vec3& rayDir, const Vec3& rayPos, float& T)
	{
		float denom = glm::dot(normal, rayDir);
		if (abs(denom) > 0.0001f) // your favorite epsilon
		{
			float t = glm::dot((planeCenter - rayPos), normal) / denom;
			T = t;
			if (t >= 0) return true; // you might want to allow an epsilon here too
		}
		return false;
	}
	static Vec2 RayPlane(const Vec3& planeDirA, const Vec3& planeDirB, const Vec3& planeCenter, const Vec3& rayDir, const Vec3& rayPos, float& T)
	{
		Vec2 UV = Vec2();
		{
			float u;
			Vec3 p1, p2;
			RayRayTime(planeCenter, planeDirA, rayPos, rayDir, UV.x, u, p1, p2);
			T = u;
		}
		{
			float u;
			Vec3 p1, p2;
			RayRayTime(planeCenter, planeDirB, rayPos, rayDir, UV.y, u, p1, p2);
			T = (u > T) ? T : u;
		}
		return UV;
	}

	
	static float lastFrameTranlate_T = 0.0f, lastFrameTranlate_T2 = 0.0f;
	static Vec3 translateDir = Vec3();
	static Vec3 translateSecondDir = Vec3();
	static int translatePlane = 0;
	static Vec3 TransformGizmo_TranslateBegin = Vec3();
	static Texture2D* TransformGizmo_SelectionTexture = nullptr;

	static void DrawTransformGizmoCenter(CameraNode* camera, const Vec3& pos)
	{
		Node3D anchor;
		anchor.SetPosition(pos);
		anchor.SetScale(Vec3(0.0035f));
		Material* gizmoMat = AssetManager::GetAsset<Material>(SuoraID("9bc003a3-7ceb-4433-b39d-81537ecdd5c5"));
		gizmoMat->m_UniformSlots[0].m_Texture2D = AssetManager::GetAsset<Texture2D>(SuoraID("f867b6a1-6b04-42fe-a4b2-6e2e54c71eab"));
		Renderer3D::DrawMesh(camera, anchor.GetTransformMatrix(), *Mesh::Sphere, gizmoMat);
	}

	bool ViewportPanel::TransformGizmo_Translate(Node3D* node)
	{
		m_GizmoBuffer->Bind();
		const Vec3 rayPos = m_EditorCamera->GetPosition();
		const Vec3 rayDir = glm::normalize(m_EditorCamera->GetCameraComponent()->ScreenPosToWorldDirection(EditorUI::GetInput(), GetWidth(), GetHeight()));

		Node3D tr1, tr2, tr3;
		tr1.SetPosition(m_EditorCamera->GetPosition() + Vec::Normalized((node->GetPosition() - m_EditorCamera->GetPosition())) * 0.1f);
		tr2.SetPosition(m_EditorCamera->GetPosition() + Vec::Normalized((node->GetPosition() - m_EditorCamera->GetPosition())) * 0.1f);
		tr3.SetPosition(m_EditorCamera->GetPosition() + Vec::Normalized((node->GetPosition() - m_EditorCamera->GetPosition())) * 0.1f);

		const Vec3 dir1 = TransformGizmo_Local ? node->GetRightVector() : Vec3(1.0f, 0.0f, 0.0f);
		const Vec3 upDir1 = TransformGizmo_Local ? node->GetUpVector() : Vec3(0.0f, 1.0f, 0.0f);
		tr1.SetLookDirection(dir1, upDir1);
		tr1.SetScale(Vec3(0.0025f));
		const Vec3 dir2 = TransformGizmo_Local ? node->GetUpVector() : Vec3(0.0f, 1.0f, 0.0f);
		const Vec3 upDir2 = TransformGizmo_Local ? node->GetRightVector() : Vec3(1.0f, 0.0f, 0.0f);
		tr2.SetLookDirection(dir2, upDir2);
		tr2.SetScale(Vec3(0.0025f));
		const Vec3 dir3 = TransformGizmo_Local ? node->GetForwardVector() : Vec3(0.0f, 0.0f, 1.0f);
		const Vec3 upDir3 = TransformGizmo_Local ? node->GetRightVector() : Vec3(1.0f, 0.0f, 0.0f);
		tr3.SetLookDirection(dir3, upDir3);
		tr3.SetScale(Vec3(0.0025f));

		Vec3 p1, p2;
		float tt = 0, u = 0;
		float dist = RayRayTime(tr1.GetPosition(), dir1, rayPos, rayDir, tt, u, p1, p2);
		const bool intersect1 = (dist < 0.001f && tt <= 0.0f && tt >= -0.02f);
		dist = RayRayTime(tr2.GetPosition(), dir2, rayPos, rayDir, tt, u, p1, p2);
		const bool intersect2 = (dist < 0.001f && tt <= 0.0f && tt >= -0.02f);
		dist = RayRayTime(tr3.GetPosition(), dir3, rayPos, rayDir, tt, u, p1, p2);
		const bool intersect3 = (dist < 0.001f && tt <= 0.0f && tt >= -0.02f);

		float ttt = 0.0f;
		RayRayTime(tr2.GetPosition(), dir2, rayPos, rayDir, tt, u, p1, p2);
		RayRayTime(tr3.GetPosition(), dir3, rayPos, rayDir, ttt, u, p1, p2);
		const bool planeX = !intersect1 && !intersect2 && !intersect3 && tt <= -0.008f && tt >= -0.0175f && ttt <= -0.008f && ttt >= -0.0175f;
		RayRayTime(tr3.GetPosition(), dir3, rayPos, rayDir, tt, u, p1, p2);
		RayRayTime(tr1.GetPosition(), dir1, rayPos, rayDir, ttt, u, p1, p2);
		const bool planeY = !intersect1 && !intersect2 && !intersect3 && tt <= -0.008f && tt >= -0.0175f && ttt <= -0.008f && ttt >= -0.0175f;
		RayRayTime(tr1.GetPosition(), dir1, rayPos, rayDir, tt, u, p1, p2);
		RayRayTime(tr2.GetPosition(), dir2, rayPos, rayDir, ttt, u, p1, p2);
		const bool planeZ = !intersect1 && !intersect2 && !intersect3 && tt <= -0.008f && tt >= -0.0175f && ttt <= -0.008f && ttt >= -0.0175f;

		DrawTransformGizmoCenter(m_EditorCamera->GetCameraComponent(), tr1.GetPosition());

		RenderCommand::SetWireframeMode(false);
		RenderCommand::SetWireframeThickness(1.0f);
		Mesh* arrow = AssetManager::GetAsset<Mesh>(SuoraID("29d7f100-5d30-4437-aa6b-00ec9a9ebf49"));
		{
			Material* gizmoMat = AssetManager::GetAsset<Material>(SuoraID("9bc003a3-7ceb-4433-b39d-81537ecdd5c5"));
			gizmoMat->m_UniformSlots[0].m_Texture2D = intersect1 ? TransformGizmo_SelectionTexture : AssetManager::GetAsset<Texture2D>(SuoraID("0ab83375-a7e4-4df1-b4b4-2d0b811e61b9"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), tr1.GetTransformMatrix(), *arrow, gizmoMat);
		}
		{
			Material* gizmoMat = AssetManager::GetAsset<Material>(SuoraID("9bc003a3-7ceb-4433-b39d-81537ecdd5c5"));
			gizmoMat->m_UniformSlots[0].m_Texture2D = intersect2 ? TransformGizmo_SelectionTexture : AssetManager::GetAsset<Texture2D>(SuoraID("401106d8-738a-4c92-b00d-dabba0401984"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), tr2.GetTransformMatrix(), *arrow, gizmoMat);
		}
		{
			Material* gizmoMat = AssetManager::GetAsset<Material>(SuoraID("9bc003a3-7ceb-4433-b39d-81537ecdd5c5"));
			gizmoMat->m_UniformSlots[0].m_Texture2D = intersect3 ? TransformGizmo_SelectionTexture : AssetManager::GetAsset<Texture2D>(SuoraID("6d1941c6-2fb0-4aa3-a789-75c1052f0954"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), tr3.GetTransformMatrix(), *arrow, gizmoMat);
		}
		RenderCommand::SetWireframeMode(false);
		RenderCommand::SetWireframeThickness(1.0f);
		// Planes
		{
			Node3D qTr;
			qTr.SetPosition(tr1.GetPosition());
			qTr.SetLookDirection(-dir1, upDir1);
			qTr.SetScale(Vec3(0.0025f));
			Mesh* quad = AssetManager::GetAsset<Mesh>(SuoraID("f959df3f-16d8-44cc-a6da-5f1c5b8ecc71"));
			Material* gizmoMat = AssetManager::GetAsset<Material>(SuoraID("9bc003a3-7ceb-4433-b39d-81537ecdd5c5"));
			gizmoMat->m_UniformSlots[0].m_Texture2D = !planeX ? AssetManager::GetAsset<Texture2D>(SuoraID("816454cc-04c3-473b-b5b5-2df728d1e8d7")) : AssetManager::GetAsset<Texture2D>(SuoraID("9e442432-fffd-4eec-9cbc-f40ef3c806c2"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), qTr.GetTransformMatrix(), *quad, gizmoMat);
		}
		{
			Node3D qTr;
			qTr.SetPosition(tr1.GetPosition());
			qTr.SetLookDirection(dir2, upDir2);
			qTr.SetScale(Vec3(0.0025f));
			Mesh* quad = AssetManager::GetAsset<Mesh>(SuoraID("f959df3f-16d8-44cc-a6da-5f1c5b8ecc71"));
			Material* gizmoMat = AssetManager::GetAsset<Material>(SuoraID("9bc003a3-7ceb-4433-b39d-81537ecdd5c5"));
			gizmoMat->m_UniformSlots[0].m_Texture2D = !planeY ? AssetManager::GetAsset<Texture2D>(SuoraID("56fd0f4b-c6ff-4c86-9438-37f753397084")) : AssetManager::GetAsset<Texture2D>(SuoraID("9e442432-fffd-4eec-9cbc-f40ef3c806c2"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), qTr.GetTransformMatrix(), *quad, gizmoMat);
		}
		{
			Node3D qTr;
			qTr.SetPosition(tr1.GetPosition());
			qTr.SetLookDirection(-dir3, upDir3);
			qTr.SetScale(Vec3(0.0025f));
			Mesh* quad = AssetManager::GetAsset<Mesh>(SuoraID("f959df3f-16d8-44cc-a6da-5f1c5b8ecc71"));
			Material* gizmoMat = AssetManager::GetAsset<Material>(SuoraID("9bc003a3-7ceb-4433-b39d-81537ecdd5c5"));
			gizmoMat->m_UniformSlots[0].m_Texture2D = !planeZ ? AssetManager::GetAsset<Texture2D>(SuoraID("98d5cbc3-9672-4379-aa9b-947223cb5103")) : AssetManager::GetAsset<Texture2D>(SuoraID("9e442432-fffd-4eec-9cbc-f40ef3c806c2"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), qTr.GetTransformMatrix(), *quad, gizmoMat);
		}

		Vec3 dir = intersect1 ? dir1 : (intersect2 ? dir2 : (intersect3 ? dir3 : Vec::Zero));
		bool planeTranslate = planeX || planeY || planeZ;
		if ((dir != Vec::Zero || planeTranslate) && NativeInput::GetMouseButtonDown(Mouse::ButtonLeft))
		{
			if (NativeInput::GetKey(Key::LeftAlt) || NativeInput::GetKey(Key::RightAlt))
			{
				node->Duplicate();
			}
			SetInputMode(EditorInputEvent::TransformGizmo);
			RayRayTime(node->GetPosition(), dir, rayPos, rayDir, lastFrameTranlate_T, u, p1, p2);
			translateDir = dir;
			translatePlane = planeTranslate ? (planeX ? 1 : planeY ? 2 : planeZ ? 3 : -1) : 0;
			if (planeTranslate)
			{
				if (planeX) { translateDir = dir2; translateSecondDir = dir3; }
				if (planeZ) { translateDir = dir1; translateSecondDir = dir2; }
				if (planeY) { translateDir = dir3; translateSecondDir = dir1; }
				TransformGizmo_SnapTranslate = false;
				RayRayTime(node->GetPosition(), translateDir, rayPos, rayDir, lastFrameTranlate_T, u, p1, p2);
				RayRayTime(node->GetPosition(), translateSecondDir, rayPos, rayDir, lastFrameTranlate_T2, u, p1, p2);
			}
			TransformGizmo_TranslateBegin = node->GetPosition();
		}
		if (IsInputMode(EditorInputEvent::TransformGizmo) && TransformGizmo_Tool == 1)
		{
			dir = translateDir;
			float currentT = 0.0f;
			RayRayTime(node->GetPosition(), dir, rayPos, rayDir, currentT, u, p1, p2);
			float delta = lastFrameTranlate_T - currentT;

			Vec3 worldOffset = Vec::Zero;
			if (!TransformGizmo_SnapTranslate)
			{
				worldOffset = dir * delta;
				node->AddWorldOffset(worldOffset);
				RayRayTime(node->GetPosition(), dir, rayPos, rayDir, lastFrameTranlate_T, u, p1, p2);
			}
			else if (glm::length(dir * delta) >= TransformGizmo_SnapTranslateGrid)
			{
				worldOffset = dir * TransformGizmo_SnapTranslateGrid * ((float)(int)(delta / TransformGizmo_SnapTranslateGrid));
				node->AddWorldOffset(worldOffset);
				RayRayTime(node->GetPosition(), dir, rayPos, rayDir, lastFrameTranlate_T, u, p1, p2);
			}
			if (NativeInput::GetKey(Key::LeftShift))
			{
				GetEditorCamera()->AddWorldOffset(worldOffset);
			}

			// For Plane Translation (Redundant!)
			if (translatePlane != 0)
			{
				dir = translateSecondDir;
				currentT = 0.0f;
				RayRayTime(node->GetPosition(), dir, rayPos, rayDir, currentT, u, p1, p2);
				delta = lastFrameTranlate_T2 - currentT;

				if (!TransformGizmo_SnapTranslate)
				{
					node->AddWorldOffset(dir * delta);
					RayRayTime(node->GetPosition(), dir, rayPos, rayDir, lastFrameTranlate_T2, u, p1, p2);
				}
				else if (glm::length(dir * delta) >= TransformGizmo_SnapTranslateGrid)
				{
					node->AddWorldOffset(dir * TransformGizmo_SnapTranslateGrid * ((float)(int)(delta / TransformGizmo_SnapTranslateGrid)));
					RayRayTime(node->GetPosition(), dir, rayPos, rayDir, lastFrameTranlate_T2, u, p1, p2);
				}
			}

			return true;
		}

		if (intersect1 || intersect2 || intersect3 || planeX || planeY || planeZ)
		{
			EditorUI::SetCursor(Cursor::Default);
		}

		return false;
	}

	bool ViewportPanel::TransformGizmo_Scale(Node3D* node, const Vec3& dir, const Vec3& upDir, Texture2D* texture)
	{
		m_GizmoBuffer->Bind();
		const Vec3 rayPos = m_EditorCamera->GetPosition();
		const Vec3 rayDir = glm::normalize(m_EditorCamera->GetCameraComponent()->ScreenPosToWorldDirection(EditorUI::GetInput(), GetWidth(), GetHeight()));

		Node3D tr;
		tr.SetPosition(m_EditorCamera->GetPosition() + Vec::Normalized((node->GetPosition() - m_EditorCamera->GetPosition())) * 0.1f);

		tr.SetLookDirection(dir, upDir/*glm::degrees(glm::eulerAngles(glm::conjugate(glm::quat_cast(glm::lookAtLH(Vec3(0.0f), dir, upDir)))))*/);
		tr.SetScale(Vec3(0.0025f));

		Vec3 p1, p2;
		float tt = 0, u = 0;
		float dist = RayRayTime(tr.GetPosition(), dir, rayPos, rayDir, tt, u, p1, p2);
		const bool intersect = (dist < 0.001f && tt <= 0.0f && tt >= -0.02f);

		RenderCommand::SetWireframeMode(false);
		RenderCommand::SetWireframeThickness(1.0f);
		{
			Mesh* arrow = AssetManager::GetAsset<Mesh>(SuoraID("023a8820-b0e7-4f73-8c87-d764ecf5c22c"));
			Material* gizmoMat = AssetManager::GetAsset<Material>(SuoraID("9bc003a3-7ceb-4433-b39d-81537ecdd5c5"));
			gizmoMat->m_UniformSlots[0].m_Texture2D = intersect ? TransformGizmo_SelectionTexture : texture;
			if (arrow)
				Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), tr.GetTransformMatrix(), *arrow, gizmoMat);
		}
		RenderCommand::SetWireframeMode(false);
		RenderCommand::SetWireframeThickness(1.0f);


		if (intersect && NativeInput::GetMouseButtonDown(Mouse::ButtonLeft))
		{
			SetInputMode(EditorInputEvent::TransformGizmo);
			RayRayTime(node->GetPosition(), dir, rayPos, rayDir, lastFrameTranlate_T, u, p1, p2);
			translateDir = dir;
		}
		if (IsInputMode(EditorInputEvent::TransformGizmo) && translateDir == dir)
		{
			float currentT = 0.0f;
			RayRayTime(node->GetPosition(), dir, rayPos, rayDir, currentT, u, p1, p2);
			float delta = lastFrameTranlate_T - currentT;

			node->SetScale(node->GetScale() + dir * delta);
			RayRayTime(node->GetPosition(), dir, rayPos, rayDir, lastFrameTranlate_T, u, p1, p2);

			return true;
		}

		if (intersect)
		{
			EditorUI::SetCursor(Cursor::Default);
		}

		return false;
	}

	bool ViewportPanel::TransformGizmo_Rotate(Node3D* node)
	{
		const Vec3 rayPos = m_EditorCamera->GetPosition();
		const Vec3 rayDir = glm::normalize(m_EditorCamera->GetCameraComponent()->ScreenPosToWorldDirection(EditorUI::GetInput(), GetWidth(), GetHeight()));

		m_TranformGizmoPickingBuffer->Bind();
		RenderCommand::SetViewport(0, 0, m_TranformGizmoPickingBuffer->GetSize().x, m_TranformGizmoPickingBuffer->GetSize().y);
		RenderCommand::Clear();

		Node3D tr;
		tr.SetPosition(m_EditorCamera->GetPosition() + Vec::Normalized((node->GetPosition() - m_EditorCamera->GetPosition())) * 0.1f);
		tr.SetRotation(node->GetRotation());

		tr.SetScale(Vec3(0.0025f));
		tr.RecalculateTransformMatrix();

		Mesh* quad = AssetManager::GetAsset<Mesh>(SuoraID("f959df3f-16d8-44cc-a6da-5f1c5b8ecc71"));
		Material* gizmoMat = AssetManager::GetAsset<Material>(SuoraID("9bc003a3-7ceb-4433-b39d-81537ecdd5c5"));

		// Picking...
		{
			Node3D qTr;
			qTr.SetPosition(tr.GetPosition());
			qTr.SetParent(&tr);
			qTr.SetLocalEulerRotation(Vec3(0, -90, 0));
			qTr.SetScale(Vec3(0.0025f));
			gizmoMat->m_UniformSlots[0].m_Texture2D = AssetManager::GetAsset<Texture2D>(SuoraID("6b0cc5fd-13f5-4eea-b71e-b2df3edf5976"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), qTr.GetTransformMatrix(), *quad, gizmoMat);
			qTr.SetParent(nullptr);
		}
		{
			Node3D qTr;
			qTr.SetPosition(tr.GetPosition());
			qTr.SetParent(&tr);
			qTr.SetLocalEulerRotation(Vec3(90, 0, 0));
			qTr.SetScale(Vec3(0.0025f));
			gizmoMat->m_UniformSlots[0].m_Texture2D = AssetManager::GetAsset<Texture2D>(SuoraID("98110e98-37ef-4a9a-bfe3-ac0fcd33f7c1"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), qTr.GetTransformMatrix(), *quad, gizmoMat);
			qTr.SetParent(nullptr);
		}
		{
			Node3D qTr;
			qTr.SetPosition(tr.GetPosition());
			qTr.SetParent(&tr);
			qTr.SetLocalEulerRotation(Vec3(0, 180, -90));
			qTr.SetScale(Vec3(0.0025f));
			gizmoMat->m_UniformSlots[0].m_Texture2D = AssetManager::GetAsset<Texture2D>(SuoraID("cd206aec-e3d4-487b-9fb6-d775520cc880"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), qTr.GetTransformMatrix(), *quad, gizmoMat);
			qTr.SetParent(nullptr);
		}

		const Vec2 ScreenPos = Vec2(EditorUI::GetInput().x / GetWidth() * (m_TranformGizmoPickingBuffer->GetSize().x), EditorUI::GetInput().y / GetHeight() * (m_TranformGizmoPickingBuffer->GetSize().y));
		const Vec3 PixelReadBack = m_TranformGizmoPickingBuffer->ReadPixel_RGB32F(ScreenPos);
		const bool intersectR = PixelReadBack == Vec3(1.0f, 0.0f, 0.0f);
		const bool intersectG = PixelReadBack == Vec3(0.0f, 1.0f, 0.0f);
		const bool intersectB = PixelReadBack == Vec3(0.0f, 0.0f, 1.0f);

		m_TranformGizmoPickingBuffer->Unbind();
		
		m_GizmoBuffer->Bind();
		// Draw the actual gizmo...
		{
			Node3D qTr;
			qTr.SetPosition(tr.GetPosition());
			qTr.SetParent(&tr);
			qTr.SetLocalEulerRotation(Vec3(0, -90, 0));
			qTr.SetScale(Vec3(0.0025f));
			gizmoMat->m_UniformSlots[0].m_Texture2D = !intersectR ? AssetManager::GetAsset<Texture2D>(SuoraID("c5b3f142-d160-4672-988c-4818c1ef32bf")) : AssetManager::GetAsset<Texture2D>(SuoraID("4f70cf5f-1998-41fa-8d5e-1c747a6dba47"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), qTr.GetTransformMatrix(), *quad, gizmoMat);
			qTr.SetParent(nullptr);
		}
		{
			Node3D qTr;
			qTr.SetPosition(tr.GetPosition());
			qTr.SetParent(&tr);
			qTr.SetLocalEulerRotation(Vec3(90, 0, 0));
			qTr.SetScale(Vec3(0.0025f));
			gizmoMat->m_UniformSlots[0].m_Texture2D = !intersectG ? AssetManager::GetAsset<Texture2D>(SuoraID("5aeb3c3d-d220-4bb9-8e69-7e3cfca16e3a")) : AssetManager::GetAsset<Texture2D>(SuoraID("4f70cf5f-1998-41fa-8d5e-1c747a6dba47"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), qTr.GetTransformMatrix(), *quad, gizmoMat);
			qTr.SetParent(nullptr);
		}
		{
			Node3D qTr;
			qTr.SetPosition(tr.GetPosition());
			qTr.SetParent(&tr);
			qTr.SetLocalEulerRotation(Vec3(0, 180, -90));
			qTr.SetScale(Vec3(0.0025f));
			gizmoMat->m_UniformSlots[0].m_Texture2D = !intersectB ? AssetManager::GetAsset<Texture2D>(SuoraID("5d7364da-4822-4db3-8636-ed3705b38924")) : AssetManager::GetAsset<Texture2D>(SuoraID("4f70cf5f-1998-41fa-8d5e-1c747a6dba47"));
			Renderer3D::DrawMesh(m_EditorCamera->GetCameraComponent(), qTr.GetTransformMatrix(), *quad, gizmoMat);
			qTr.SetParent(nullptr);
		}



		if (intersectR && NativeInput::GetMouseButtonDown(Mouse::ButtonLeft))
		{
			SetInputMode(EditorInputEvent::TransformGizmo);
			translateDir = Vec3(1, 0, 0);
			GetMajorTab()->GetEditorWindow()->GetWindow()->SetCursorLocked(true);
		}
		else if (intersectG && NativeInput::GetMouseButtonDown(Mouse::ButtonLeft))
		{
			SetInputMode(EditorInputEvent::TransformGizmo);
			translateDir = Vec3(0, 1, 0);
			GetMajorTab()->GetEditorWindow()->GetWindow()->SetCursorLocked(true);
		}
		else if (intersectB && NativeInput::GetMouseButtonDown(Mouse::ButtonLeft))
		{
			SetInputMode(EditorInputEvent::TransformGizmo);
			translateDir = Vec3(0, 0, 1);
			GetMajorTab()->GetEditorWindow()->GetWindow()->SetCursorLocked(true);
		}

		if (IsInputMode(EditorInputEvent::TransformGizmo))
		{
			if (NativeInput::GetMouseButtonUp(Mouse::ButtonLeft)) GetMajorTab()->GetEditorWindow()->GetWindow()->SetCursorLocked(false);
			float currentT = 0.0f;
			float delta = lastFrameTranlate_T - currentT;

			node->RotateWithAxis(translateDir, 0.1f * NativeInput::GetMouseDelta().x);

			return true;
		}

		if (intersectR || intersectG || intersectB)
		{
			EditorUI::SetCursor(Cursor::Default);
		}

		return false;
	}

	bool ViewportPanel::DrawTransformGizmo(Node3D* node)
	{
		{
			AssetManager::GetAsset<Texture2D>(SuoraID("816454cc-04c3-473b-b5b5-2df728d1e8d7"))->GetTexture(); // TranslatePlane X
			AssetManager::GetAsset<Texture2D>(SuoraID("56fd0f4b-c6ff-4c86-9438-37f753397084"))->GetTexture(); // TranslatePlane Y
			AssetManager::GetAsset<Texture2D>(SuoraID("98d5cbc3-9672-4379-aa9b-947223cb5103"))->GetTexture(); // TranslatePlane Z
			AssetManager::GetAsset<Texture2D>(SuoraID("9e442432-fffd-4eec-9cbc-f40ef3c806c2"))->GetTexture(); // TranslatePlane Select
			AssetManager::GetAsset<Texture2D>(SuoraID("c5b3f142-d160-4672-988c-4818c1ef32bf"))->GetTexture(); // Rotate X
			AssetManager::GetAsset<Texture2D>(SuoraID("5aeb3c3d-d220-4bb9-8e69-7e3cfca16e3a"))->GetTexture(); // Rotate Y
			AssetManager::GetAsset<Texture2D>(SuoraID("5d7364da-4822-4db3-8636-ed3705b38924"))->GetTexture(); // Rotate Z
			AssetManager::GetAsset<Texture2D>(SuoraID("4f70cf5f-1998-41fa-8d5e-1c747a6dba47"))->GetTexture(); // Rotate Select
		}
		if (!NativeInput::GetMouseButton(Mouse::ButtonLeft) && !NativeInput::GetMouseButton(Mouse::ButtonRight))
		{
			if (NativeInput::GetKeyDown(Key::Q)) TransformGizmo_Tool = 0;
			if (NativeInput::GetKeyDown(Key::W)) TransformGizmo_Tool = 1;
			if (NativeInput::GetKeyDown(Key::E)) TransformGizmo_Tool = 2;
			if (NativeInput::GetKeyDown(Key::R)) TransformGizmo_Tool = 3;
			if (NativeInput::GetKeyDown(Key::L)) TransformGizmo_Local = !TransformGizmo_Local;
		}


		bool result = false;
		// cleared gizmo buffer here before....

		const Vec3 rayPos = m_EditorCamera->GetPosition();
		const Vec3 rayDir = glm::normalize(m_EditorCamera->GetCameraComponent()->ScreenPosToWorldDirection(EditorUI::GetInput(), GetWidth(), GetHeight()));
		const float distance = Vec::Distance(m_EditorCamera->GetPosition(), node->GetPosition());

		if (distance <= 0.1f) return false;

		Node3D tr;
		tr.SetPosition(m_EditorCamera->GetPosition() + Vec::Normalized((node->GetPosition() - m_EditorCamera->GetPosition())) * 0.1f);
		tr.SetRotation(node->GetEulerRotation());
		tr.SetScale(Vec3(0.0015f));
		Node3D handle1 = tr; handle1.SetRotation(Vec3(0.0f));
		Node3D handle2 = tr; handle2.SetRotation(handle2.GetEulerRotation() + Vec::Forward * 90.0f);
		Node3D handle3 = tr; handle3.SetRotation(handle3.GetEulerRotation() - Vec::Up * 90.0f);

		RenderCommand::SetWireframeMode(false);

		TransformGizmo_SelectionTexture = AssetManager::GetAsset<Texture2D>(SuoraID("dbebbd5e-72a1-4acf-b04f-adac5e5e552d"));
		if (TransformGizmo_Tool == 1)
		{
			result = TransformGizmo_Translate(node) ? true : result;
		}
		else if (TransformGizmo_Tool == 3)
		{
			result = TransformGizmo_Scale(node, TransformGizmo_Local ? node->GetRightVector() : Vec3(1.0f, 0.0f, 0.0f), TransformGizmo_Local ? node->GetUpVector() : Vec3(0.0f, 1.0f, 0.0f), AssetManager::GetAsset<Texture2D>(SuoraID("0ab83375-a7e4-4df1-b4b4-2d0b811e61b9"))) ? true : result;
			result = TransformGizmo_Scale(node, TransformGizmo_Local ? node->GetUpVector() : Vec3(0.0f, 1.0f, 0.0f), TransformGizmo_Local ? node->GetRightVector() : Vec3(1.0f, 0.0f, 0.0f), AssetManager::GetAsset<Texture2D>(SuoraID("401106d8-738a-4c92-b00d-dabba0401984"))) ? true : result;
			result = TransformGizmo_Scale(node, TransformGizmo_Local ? node->GetForwardVector() : Vec3(0.0f, 0.0f, 1.0f), TransformGizmo_Local ? node->GetRightVector() : Vec3(1.0f, 0.0f, 0.0f), AssetManager::GetAsset<Texture2D>(SuoraID("6d1941c6-2fb0-4aa3-a789-75c1052f0954"))) ? true : result;
		}
		else if (TransformGizmo_Tool == 2)
		{
			result = TransformGizmo_Rotate(node) ? true : result;
		}

		AssetManager::GetAsset<Material>(SuoraID("9bc003a3-7ceb-4433-b39d-81537ecdd5c5"))->m_UniformSlots[0].m_Texture2D = AssetManager::GetAsset<Texture2D>(SuoraID("60b698ff-23ce-43b8-b280-a9d987039b1a"));

		return result;
	}
}