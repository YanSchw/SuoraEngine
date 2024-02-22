#pragma once
#include "DetailsPanel.h"
#include "NodeDetails.generated.h"

namespace Suora
{
	class Node;

	class NodeDetails : public DetailsPanelImplementation
	{
		SUORA_CLASS(85438795342);
	public:

		virtual void ViewObject(Object* obj, float& y) override;
		void ViewNode(float& y, Node* node);

		// Transform Util
		void* Transform_LastNode = nullptr;
		Vec3 Transform_Pos = Vec::Zero;
		Vec3 Transform_Rot = Vec::Zero;
		Vec3 Transform_Scale = Vec::One;
	};

}