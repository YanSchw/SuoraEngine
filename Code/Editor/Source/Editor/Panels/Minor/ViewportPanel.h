#pragma once
#include "../MinorTab.h"
#include "Suora/Renderer/RenderPipeline.h"
#include <Suora.h>

#define _ENUM_BODY_6476475

namespace Suora
{
	class EditorCamera;

	ENUM(6476475) DebugView : int32_t
	{
		Final = 0,
		GBuffer_BaseColor,
		GBuffer_WorldPosition,
		GBuffer_WorldNormal,
		GBuffer_Emissive,
		GBuffer_MeshID,
		GBuffer_Cluster,
		DeferredLightPass,
		ForwardReadyBuffer,
		Ilum_Surfels,
		Ilum_Probes,
		Ilum_Debug
	};

	class ViewportPanel : public MinorTab
	{
	private:
		Ref<Framebuffer> m_ViewportFramebuffer;
		Ref<EditorCamera> m_EditorCamera = nullptr;
		World* m_World = nullptr;
		DebugView m_DebugView = DebugView::Final;

	public:
		ViewportPanel(MajorTab* majorTab, World* world);
		~ViewportPanel();

	private:
		Ref<Framebuffer> m_GizmoBuffer, m_GizmoBufferSmooth, m_PickingBuffer, m_TranformGizmoPickingBuffer;

		int TransformGizmo_Tool = 1;
		bool TransformGizmo_Local = false;
		bool TransformGizmo_SnapTranslate = true;
		float TransformGizmo_SnapTranslateGrid = 0.5f;

		bool TransformGizmo_Translate(Node3D* node);
		bool TransformGizmo_Scale(Node3D* node, const Vec3& dir, const Vec3& upDir, Texture2D* texture);
		bool TransformGizmo_Rotate(Node3D* node);
	public:
		bool DrawTransformGizmo(Node3D* node);
		Ref<Framebuffer> m_SelectionOutlineFramebuffer;
		Ref<Shader> m_SelectionOutlineShader;
		void DrawSelectionOutline(Node3D* node, const Color& color);
		void DrawDebugShapes(World* world, CameraNode* camera, int* pickingID = nullptr, std::unordered_map<int, Node*>* pickingMap = nullptr);
		void HandleMousePick(const glm::ivec2& pos);
		void DrawDebugView(Framebuffer& buffer, World& world, CameraNode& camera);
		
		void HandleAssetDragDrop();
		Node* AssetDragDropNode = nullptr;
		bool AssetDragDropValidation = false;

		RenderingParams m_CameraPreviewRParams;
		Ref<Framebuffer> m_CameraPreviewBuffer;

		RenderingParams m_RParams;
		bool m_DrawDebugGizmos = false;
		bool m_DrawDebugGizmosDuringPlay = false;
		bool m_DrawWireframe = false;

		EditorCamera* GetEditorCamera() const { return m_EditorCamera.get(); }

		virtual void Render(float deltaTime) override;

		friend class NodeClassEditor;
	};
}