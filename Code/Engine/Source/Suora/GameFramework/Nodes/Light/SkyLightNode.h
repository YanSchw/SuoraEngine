#pragma once
#include <Suora.h>
#include <glm/glm.hpp>
#include "LightNode.h"
#include "SkyLightNode.generated.h"

namespace Suora
{
	class LightComponent;
	class Texture2D;
	class Material;

	class SkyLightNode : public LightNode
	{
		SUORA_CLASS(547896459);
	public:

		MEMBER()
		Texture2D* m_SkyTexture = nullptr;

		/*MEMBER()
		Material* m_SkyMaterial = nullptr;*/

		MEMBER()
		float m_Red = 1.0f;
		MEMBER()
		float m_Green = 1.0f;
		MEMBER()
		float m_Blue = 1.0f;

		SkyLightNode();
		void Begin() override;
		void WorldUpdate(float deltaTime) override;
		void OnDestroyed();

	};

}