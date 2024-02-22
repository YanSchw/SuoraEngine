#pragma once
#include <Suora.h>
#include <glm/glm.hpp>
#include "LightNode.h"
#include "SkyLightNode.generated.h"

namespace Suora
{
	class Texture2D;
	class Material;

	class SkyLightNode : public LightNode
	{
		SUORA_CLASS(547896459);
	public:

		PROPERTY()
		Texture2D* m_SkyTexture = nullptr;

		/*PROPERTY()
		Material* m_SkyMaterial = nullptr;*/

		PROPERTY()
		Color m_Color = Color(1.0f);

		SkyLightNode();
		void Begin() override;
		void WorldUpdate(float deltaTime) override;
		void OnDestroyed();

	};

}