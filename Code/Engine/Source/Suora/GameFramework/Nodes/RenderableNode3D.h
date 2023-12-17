#pragma once
#include "Suora/GameFramework/Node.h"
#include "RenderableNode3D.generated.h"

namespace Suora
{
	class World;
	class CameraNode;
	class LightNode;
	struct RenderingParams;

	class RenderableNode3D : public Node3D
	{
		SUORA_CLASS(8795432125);
	public:
		virtual ~RenderableNode3D();
		virtual void InitializeNode(World& world) override;

		virtual bool IsDeferredRenderable() const { return false; }
		virtual bool IsForwardRenderable()  const { return false; }
		virtual bool IsShadowRenderable()   const { return false; }

		virtual void RenderDeferredSingleInstance(World& world, CameraNode& camera, RenderingParams& params, int32_t ID);
		virtual void RenderForwardSingleInstance(World& world, CameraNode& camera, RenderingParams& params, int32_t ID);
		virtual void RenderShadowSingleInstance(World& world, CameraNode& lightCamera, RenderingParams& params, LightNode* light, int32_t ID);
	private:
		bool m_WasInitliazed = false;
	};

}