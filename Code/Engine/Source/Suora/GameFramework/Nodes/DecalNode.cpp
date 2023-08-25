#include "Precompiled.h"
#include "DecalNode.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"

namespace Suora
{
	
	DecalNode::DecalNode()
	{
		m_Projection = CreateScope<CameraNode>();
	}
	DecalNode::~DecalNode()
	{
	}

	void DecalNode::Begin()
	{
		Super::Begin();
	}
	void DecalNode::WorldUpdate(float deltaTime)
	{
		Super::WorldUpdate(deltaTime);

		m_Projection->SetAspectRatio(Width / Height);
		m_Projection->SetOrthographic(Height, Near, Far);

	}

}