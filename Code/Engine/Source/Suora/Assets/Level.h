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
	class Component;

	class Level : public Blueprint
	{
		SUORA_CLASS(142984220);
		ASSET_EXTENSION(".level");

	public:
		/* Disk -> Level */
		void InitializeAsset(const std::string& str) override;
		/* Level -> Disk */
		virtual void Serialize(Yaml::Node& root) override;

		Array<CompositionLayer> m_CompositionLayers;
		bool m_LevelInitialized = false;

		/* World -> Level */
		void _CollectWorldCompositionData(World& world);
		void _CollectNodeCompositionData(Node& node, CompositionLayer& layer);
		void _CollectSubNodeCompositionData(Node& node, CompositionLayer& layer, const Array<int32_t> indentations);

		/* Level -> World */
		void _FillWorldWithCompositionData(World& world, bool includeCompositionData);
		Node* _FillWorldWithCompositionLayer(World& world, CompositionLayer& layer, bool includeCompositionData);
		Node* _CreateNodeInstance(CompositionLayer& layer, bool includeCompositionData, Node* sampleNode = nullptr, bool deepestLayer = false);
		
	};
}