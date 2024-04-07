#pragma once
#include "ViewportPanel.h"
#include "Suora/Common/Map.h"
#include "ViewportModules.generated.h"

namespace Suora
{
	class World;
	class CameraNode;

	class ViewportCameraGizmo : public ViewportDebugGizmo
	{
		SUORA_CLASS(46783823473333);
	public:
		virtual void DrawDebugGizmos(World* world, CameraNode* camera, int* pickingID, Map<int, Node*>* pickingMap, bool isHandlingMousePick)  override;

	};

	class ViewportPointLightGizmo : public ViewportDebugGizmo
	{
		SUORA_CLASS(879543879523232);
	public:
		virtual void DrawDebugGizmos(World* world, CameraNode* camera, int* pickingID, Map<int, Node*>* pickingMap, bool isHandlingMousePick)  override;

	};

	class ViewportDirectionalLightGizmo : public ViewportDebugGizmo
	{
		SUORA_CLASS(578457489353);
	public:
		virtual void DrawDebugGizmos(World* world, CameraNode* camera, int* pickingID, Map<int, Node*>* pickingMap, bool isHandlingMousePick)  override;
	};

	class ViewportGridGizmo : public ViewportDebugGizmo
	{
		SUORA_CLASS(98075328790333);
	public:
		virtual void DrawDebugGizmos(World* world, CameraNode* camera, int* pickingID, Map<int, Node*>* pickingMap, bool isHandlingMousePick)  override;
	};

	class ViewportShapeGizmos : public ViewportDebugGizmo
	{
		SUORA_CLASS(578964325349);
	public:
		virtual void DrawDebugGizmos(World* world, CameraNode* camera, int* pickingID, Map<int, Node*>* pickingMap, bool isHandlingMousePick)  override;
	};

}