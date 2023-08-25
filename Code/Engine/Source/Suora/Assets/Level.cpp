#include "Precompiled.h"
#include "Level.h"
#include <unordered_map>
#include "Suora/GameFramework/World.h"
#include "Suora/GameFramework/Node.h"
#include "Platform/Platform.h"
#include "Suora/Common/Common.h"
#include "Suora/Common/Filesystem.h"
#include "Suora/Serialization/Yaml.h"

/*
static void SerializeCompositionLayer(Suora::CompositionLayer& layer, Yaml::Node& yaml)
{
	// This Layer
	layer.m_RootCompositionData.Serialize(yaml["m_RootCompositionData"]);
	yaml["m_Name"] = layer.m_Name;
	yaml["m_Transform"] = std::string(layer.Transform_Pos.ToString() + "%" + layer.Transform_Rot.ToString() + "%" + layer.Transform_Scale.ToString());

	// Components
	for (int i = 0; i < layer.m_ActorComponents.Size(); i++)
	{
		layer.m_ActorComponents[i].Serialize(yaml["ActorComponent_" + std::to_string(i)]);
	}

	// SubLayers
	for (int i = 0; i < layer.m_SubCompositionLayers.Size(); i++)
	{
		SerializeCompositionLayer(layer.m_SubCompositionLayers[i], yaml[std::to_string(i)]);
		std::string indentation;
		for (int32_t j = 0; j < layer.m_SubCompositionIndentation[i].Size(); j++)
		{
			indentation += "/" + std::to_string(layer.m_SubCompositionIndentation[i][j]);
		}
		yaml[std::to_string(i)]["Indentation"] = indentation;
	}
}
static void DeserializeCompositionLayer(Suora::CompositionLayer& layer, Yaml::Node& yaml)
{
	// This Layer
	{
		layer.m_RootCompositionData.Deserialize(yaml["m_RootCompositionData"]);
		layer.m_Name = yaml["m_Name"].As<std::string>();
		std::vector<std::string> tranform = Suora::Util::SplitString(yaml["m_Transform"].As<std::string>(), '%');
		layer.Transform_Pos = Suora::Vector3::FromString(tranform.size() != 0 ? tranform[0] : "");
		layer.Transform_Rot = Suora::Vector3::FromString(tranform.size() != 0 ? tranform[1] : "");
		layer.Transform_Scale = Suora::Vector3::FromString(tranform.size() != 0 ? tranform[2] : "1.0/1.0/1.0");
	}
	// Components
	{
		int i = 0;
		while (true)
		{
			Yaml::Node& sub = yaml["ActorComponent_" + std::to_string(i++)];
			if (sub.IsNone()) break;
			layer.m_ActorComponents.Add(Suora::IObjectCompositionData());
			layer.m_ActorComponents[layer.m_ActorComponents.Size() - 1].Deserialize(sub);
		}
	}
	// SubLayers
	{
		int i = 0;
		while (true)
		{
			Yaml::Node& sub = yaml[std::to_string(i++)];
			if (sub.IsNone()) break;
			layer.m_SubCompositionLayers.Add(Suora::CompositionLayer());
			layer.m_SubCompositionIndentation.Add(ArrayList<int32_t>());
			DeserializeCompositionLayer(layer.m_SubCompositionLayers[layer.m_SubCompositionLayers.Size() - 1], sub);
			std::string indentation = sub["Indentation"].As<std::string>();
			std::vector<std::string> split = Suora::Util::SplitString(indentation, '/');
			for (int32_t j = 1; j < split.size(); j++)
			{
				layer.m_SubCompositionIndentation[layer.m_SubCompositionIndentation.Last()].Add(std::stoi(split[j]));
			}
		}
	}
}*/

namespace Suora
{

	void Level::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		Yaml::Node& level = root["LevelComposition"];

		for (int i = 0; i < m_CompositionLayers.Size(); i++)
		{
			CompositionLayer::SerializeCompositionLayer(m_CompositionLayers[i], level[std::to_string(i)]);
		}

	}

	void Level::InitializeAsset(const std::string& str)
	{
		Super::InitializeAsset(str);
		m_CompositionLayers.Clear();

		Yaml::Node root;
		Yaml::Parse(root, str);
		Yaml::Node& level = root["LevelComposition"];

		int i = 0;
		while (true)
		{
			Yaml::Node& layer = level[std::to_string(i++)];
			if (layer.IsNone()) break;
			m_CompositionLayers.Add(CompositionLayer());
			CompositionLayer::DeserializeCompositionLayer(m_CompositionLayers[m_CompositionLayers.Size() - 1], layer);
		}
		m_LevelInitialized = true;
	}

	void Level::_CollectWorldCompositionData(World& world)
	{
		m_CompositionLayers.Clear();

		for (Node* node : world.m_WorldNodes)
		{
			if (IObjectCompositionData* interface = node->GetInterface<IObjectCompositionData>())
			{
				if (!node->GetParent() /* && interface->m_IsActorLayer*/)
				{
					m_CompositionLayers.Add(CompositionLayer());
					_CollectNodeCompositionData(*node, m_CompositionLayers[m_CompositionLayers.Size() - 1]);
				}
			}
		}

	}

	void Level::_CollectNodeCompositionData(Node& node, CompositionLayer& layer)
	{
		layer.m_RootCompositionData = *node.GetInterface<IObjectCompositionData>();
		layer.m_Name = node.m_Name;
		if (Node3D* node3D = node.GetTransform())
		{
			layer.Transform_Pos = node3D->GetPosition();
			layer.Transform_Rot = node3D->GetEulerRotation();
			layer.Transform_Scale = node3D->GetScale();
		}
		
		/*for (Component* component : actor.Components)
		{
			if (!component->Implements<IObjectCompositionData>())
			{
				component->Implement<IObjectCompositionData>();
			}
			layer.m_ActorComponents.Add(*component->GetInterface<IObjectCompositionData>());
		}*/

		// Subcomponent Layers
		Array<int32_t> indentations;
		_CollectSubNodeCompositionData(node, layer, indentations);
	}

	void Level::_CollectSubNodeCompositionData(Node& node, CompositionLayer& layer, const Array<int32_t> indentations)
	{
		for (int32_t i = 0; i < node.GetChildCount(); i++)
		{
			IObjectCompositionData* obj = node.GetChild(i)->GetInterface<IObjectCompositionData>();
			if (obj && obj->m_IsActorLayer)
			{
				layer.m_SubCompositionLayers.Add(CompositionLayer());
				layer.m_SubCompositionIndentation.Add(indentations);
				_CollectNodeCompositionData(*(Node*)node.GetChild(i), layer.m_SubCompositionLayers[layer.m_SubCompositionLayers.Last()]);
			}
			else
			{
				Array<int32_t> subIndentations = indentations;
				subIndentations.Add(i);
				_CollectSubNodeCompositionData(*(Node*)node.GetChild(i), layer, subIndentations);
			}
		}
	}

	void Level::_FillWorldWithCompositionData(World& world, bool includeCompositionData)
	{
		for (CompositionLayer& layer : m_CompositionLayers)
		{
			Node* node = _FillWorldWithCompositionLayer(world, layer, includeCompositionData);
		}
	}

	Node* Level::_FillWorldWithCompositionLayer(World& world, CompositionLayer& layer, bool includeCompositionData)
	{
		Node* node = _CreateNodeInstance(layer, includeCompositionData, nullptr, true);
		node->InitializeNode(world);

		return node;
	}

	Node* Level::_CreateNodeInstance(CompositionLayer& layer, bool includeCompositionData, Node* sampleNode, bool deepestLayer)
	{
		Node* node = sampleNode ? sampleNode : Cast<Node>(New(layer.m_RootCompositionData.m_Class, includeCompositionData, false));
		SUORA_ASSERT(node);
		node->m_Name = layer.m_Name;
		if (Node3D* node3D = node->As<Node3D>()) { node3D->SetPosition(layer.Transform_Pos); node3D->SetRotation(glm::inverse(Quat(layer.Transform_Rot))); node3D->SetScale(layer.Transform_Scale); }

		std::unordered_map<Class, int> componentIndices;

		if (includeCompositionData)
		{
			node->Implement<IObjectCompositionData>();
			IObjectCompositionData& data = *node->GetInterface<IObjectCompositionData>();
			data.m_Class = layer.m_RootCompositionData.m_Class;
			if (deepestLayer) data.m_IsActorLayer = layer.m_RootCompositionData.m_IsActorLayer;
			for (IObjectCompositionData::DefaultMemberValue& value : layer.m_RootCompositionData.m_DefaultMemberValues)
			{
				for (IObjectCompositionData::DefaultMemberValue& targetValues : data.m_DefaultMemberValues)
				{
					if (value.m_Member.m_MemberName == targetValues.m_Member.m_MemberName && value.m_Member.m_Type == targetValues.m_Member.m_Type)
					{
						targetValues.m_ValueChanged = value.m_ValueChanged;
						break;
					}
				}
			}
		}
		for (IObjectCompositionData::DefaultMemberValue& value : layer.m_RootCompositionData.m_DefaultMemberValues)
		{
			if (value.m_ValueChanged) value.Apply(node);
		}

		// Sub Composition Layers
		for (int32_t i = 0; i < layer.m_SubCompositionLayers.Size(); i++)
		{
			Node* sub = _CreateNodeInstance(layer.m_SubCompositionLayers[i], includeCompositionData, nullptr, deepestLayer);
			Node* parent = node;
			for (int32_t j = 0; j < layer.m_SubCompositionIndentation[i].Size(); j++)
			{
				if (parent->GetChildCount() <= layer.m_SubCompositionIndentation[i][j]) break;
				parent = parent->GetChild(layer.m_SubCompositionIndentation[i][j]);
			}
			sub->SetParent(parent);
		}

		return node;
	}

}