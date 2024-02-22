#include "Precompiled.h"
#include "RenderableNode3D.h"
#include "Suora/GameFramework/World.h"

namespace Suora
{

	RenderableNode3D::~RenderableNode3D()
	{
	}

	void RenderableNode3D::InitializeNode(World& world)
	{
		Super::InitializeNode(world);

		if (m_WasInitliazed)
		{
			return;
		}

		m_WasInitliazed = true;

		if (IsDeferredRenderable())
		{
			world.m_DeferredRenderables.Add(this);
		}
		if (IsForwardRenderable())
		{
			world.m_ForwardRenderables.Add(this);
		}
		if (IsShadowRenderable())
		{
			world.m_ShadowRenderables.Add(this);
		}
	}

	void RenderableNode3D::UnInitializeNode(World& world)
	{
		CleanUp();
	}

	void RenderableNode3D::RenderDeferredSingleInstance(World& world, CameraNode& camera, RenderingParams& params, int32_t ID)
	{
		/* Nothing */
	}

	void RenderableNode3D::RenderForwardSingleInstance(World& world, CameraNode& camera, RenderingParams& params, int32_t ID)
	{
		/* Nothing */
	}

	void RenderableNode3D::RenderShadowSingleInstance(World& world, CameraNode& lightCamera, RenderingParams& params, LightNode* light, int32_t ID)
	{
		/* Nothing */
	}

	void RenderableNode3D::CleanUp()
	{
		if (GetWorld() && m_WasInitliazed)
		{
			if (IsDeferredRenderable())
			{
				GetWorld()->m_DeferredRenderables.Remove(this);
			}
			if (IsForwardRenderable())
			{
				GetWorld()->m_ForwardRenderables.Remove(this);
			}
			if (IsShadowRenderable())
			{
				GetWorld()->m_ShadowRenderables.Remove(this);
			}
		}
	}

}