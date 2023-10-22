#pragma once
#include "Asset.h"
#include "Suora/Serialization/CompositionLayer.h"
#include "Blueprint.h"
#include <vector>
#include <string>
#include "Level.generated.h"

namespace Suora
{
	class World;
	class Node;

	class Level : public Blueprint
	{
		SUORA_CLASS(142984220);
		ASSET_EXTENSION(".level");

	public:
		virtual void Serialize(Yaml::Node& root) override;
		
	};
}