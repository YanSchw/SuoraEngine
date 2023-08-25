#pragma once
#include <vector>
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/Array.h"
#include "Suora/Assets/Material.h"
#include "DecalNode.generated.h"

namespace Suora
{
	class Material;
	class CameraNode;

	class DecalNode : public Node3D
	{
		SUORA_CLASS(458739912);

	private:
		Scope<CameraNode> m_Projection;

	public:
		MEMBER() 
		Material* material = nullptr;
		MEMBER()
		float Width = 1.0f;
		MEMBER()
		float Height = 1.0f;
		MEMBER()
		float Near = 0.01f;
		MEMBER()
		float Far = 3.0f;

		DecalNode();
		~DecalNode();
		void Begin() override;
		void WorldUpdate(float deltaTime) override;

		friend class Decima;
		friend class RenderPipeline;
	};
}