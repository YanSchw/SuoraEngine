#include "Precompiled.h"
#include "LightNode.h"

namespace Suora
{

	LightNode::LightNode()
	{

	}
	void LightNode::Begin()
	{
		Super::Begin();
	}
	void LightNode::WorldUpdate(float deltaTime)
	{
		Super::WorldUpdate(deltaTime);


	}
	void LightNode::OnDestroyed()
	{

	}

	void LightNode::ShadowMap(World& world, CameraNode& camera, RenderingParams& params)
	{
	}

}