#pragma once
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/Array.h"
#include "Suora/Assets/Texture2D.h"
#include "MeshNode.h"
#include "SpriteNode.generated.h"

namespace Suora
{
	class Material;

	class SpriteNode : public MeshNode
	{
		SUORA_CLASS(785645479);

	public:
		SpriteNode();
		~SpriteNode();

		void Begin() override;
		void Render();
	};

}