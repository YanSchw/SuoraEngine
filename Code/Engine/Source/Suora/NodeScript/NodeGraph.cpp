#include "Precompiled.h"
#include "NodeGraph.h"

namespace Suora
{
	void VisualNodeGraph::AddSupportedNode(const Ref<VisualNode>& node, const Array<std::string>& tags)
	{
		m_SupportedNodes.Add(node);
		VisualNodeEntry& entry = m_SupportedNodes[m_SupportedNodes.Size() - 1];
		entry.m_Tags = tags;

		Ref<VisualNode> _node = entry.m_Node;
		{
			for (VisualNodePin& pin : _node->m_InputPins)
			{
				pin.m_Node = _node.get();
			}
			for (VisualNodePin& pin : _node->m_OutputPins)
			{
				pin.m_Node = _node.get();
			}
		}

	}

	void VisualNodeGraph::UpdateSupportedNodes()
	{
		m_SupportedNodes.Clear();
	}

	void VisualNodeGraph::TickAllVisualNodes()
	{

	}

	int VisualNodeGraph::IndexOf(VisualNode& node)
	{
		int i = 0;
		for (Ref<VisualNode> it : m_Nodes)
		{
			if (it.get() == &node) return i;
			i++;
		}

		return -1;
	}

	void VisualNodeGraph::ClearNodePin(VisualNodePin& pin)
	{
		if (pin.IsReceivingPin)
		{
			pin.Target = nullptr;
		}
		else
		{
			for (Ref<VisualNode> node : m_Nodes)
			{
				for (VisualNodePin& it : node->m_InputPins)
				{
					if (it.IsReceivingPin && it.Target == &pin) it.Target = nullptr;
				}
				for (VisualNodePin& it : node->m_OutputPins)
				{
					if (it.IsReceivingPin && it.Target == &pin) it.Target = nullptr;
				}
			}
		}
	}
	void VisualNodeGraph::FixNodePins()
	{
		for (Ref<VisualNode> node : m_Nodes)
		{
			for (VisualNodePin& pin : node->m_InputPins)
			{
				pin.m_Node = node.get();
				if (pin.IsReceivingPin && pin.Target && pin.PinID == (int64_t)ScriptDataType::ObjectPtr)
				{
					if (!Class::FromString(pin.Target->m_AdditionalData).Inherits(Class::FromString(pin.m_AdditionalData)))
					{
						pin.Target = nullptr;
					}
				}
			}
			for (VisualNodePin& pin : node->m_OutputPins)
			{
				pin.m_Node = node.get();
				if (pin.IsReceivingPin && pin.Target && pin.PinID == (int64_t)ScriptDataType::ObjectPtr)
				{
					if (!Class::FromString(pin.Target->m_AdditionalData).Inherits(Class::FromString(pin.m_AdditionalData)))
					{
						pin.Target = nullptr;
					}
				}
			}
		}
	}
	void VisualNodeGraph::RemoveVisualNode(VisualNode& node)
	{
		for (VisualNodePin& pin : node.m_InputPins)
		{
			ClearNodePin(pin);
		}
		for (VisualNodePin& pin : node.m_OutputPins)
		{
			ClearNodePin(pin);
		}
		m_Nodes.RemoveAt(IndexOf(node));
	}

	/* -----  Serialization ----- */

	void VisualNodeGraph::SerializeNodeGraph(Yaml::Node& root)
	{
		Yaml::Node& graph = root["NodeGraph"];
		graph = Yaml::Node();

		int index = 0;
		for (Ref<VisualNode> node : m_Nodes)
		{
			Yaml::Node& entry = graph["Node_" + std::to_string(index++)];
			entry["Color"]["r"] = std::to_string(node->m_Color.r);
			entry["Color"]["g"] = std::to_string(node->m_Color.g);
			entry["Color"]["b"] = std::to_string(node->m_Color.b);
			entry["Color"]["a"] = std::to_string(node->m_Color.a);
			entry["NodeID"] = std::to_string(node->m_NodeID);
			entry["Position"]["x"] = std::to_string(node->m_Position.x);
			entry["Position"]["y"] = std::to_string(node->m_Position.y);
			entry["Size"]["x"] = std::to_string(node->m_Size.x);
			entry["Size"]["y"] = std::to_string(node->m_Size.y);
			entry["Title"] = node->m_Title;

			int inputIndex = 0;
			for (VisualNodePin& pin : node->m_InputPins)
			{
				Yaml::Node& entryPin = entry["Input_" + std::to_string(inputIndex++)];
				entryPin["Color"]["r"] = std::to_string(pin.Color.r);
				entryPin["Color"]["g"] = std::to_string(pin.Color.g);
				entryPin["Color"]["b"] = std::to_string(pin.Color.b);
				entryPin["Color"]["a"] = std::to_string(pin.Color.a);
				entryPin["IsReceivingPin"] = pin.IsReceivingPin ? "true" : "false";
				entryPin["Label"] = pin.Label;
				entryPin["m_AdditionalData"] = pin.m_AdditionalData;
				entryPin["PinHeight"] = std::to_string(pin.PinHeight);
				entryPin["PinID"] = std::to_string(pin.PinID);
				entryPin["Target"] = pin.Target ? VisualNodePinToString(*pin.Target) : "NULL";
			}

			int outputIndex = 0;
			for (VisualNodePin& pin : node->m_OutputPins)
			{
				Yaml::Node& entryPin = entry["Output_" + std::to_string(outputIndex++)];
				entryPin["Color"]["r"] = std::to_string(pin.Color.r);
				entryPin["Color"]["g"] = std::to_string(pin.Color.g);
				entryPin["Color"]["b"] = std::to_string(pin.Color.b);
				entryPin["Color"]["a"] = std::to_string(pin.Color.a);
				entryPin["IsReceivingPin"] = pin.IsReceivingPin ? "true" : "false";
				entryPin["Label"] = pin.Label;
				entryPin["m_AdditionalData"] = pin.m_AdditionalData;
				entryPin["PinHeight"] = std::to_string(pin.PinHeight);
				entryPin["PinID"] = std::to_string(pin.PinID);
				entryPin["Target"] = pin.Target ? VisualNodePinToString(*pin.Target) : "NULL";
			}
		}
	}

	void VisualNodeGraph::DeserializeNodeGraph(Yaml::Node& root)
	{
		Yaml::Node& graph = root["NodeGraph"];

		{
			int index = 0;
			while (true)
			{
				Yaml::Node& entry = graph["Node_" + std::to_string(index++)];
				if (entry.IsNone()) break;
				m_Nodes.Add(Ref<VisualNode>(new VisualNode()));
				VisualNode* node = m_Nodes[m_Nodes.Last()].get();

				node->m_Color = glm::vec4(entry["Color"]["r"].As<float>(), entry["Color"]["g"].As<float>(), entry["Color"]["b"].As<float>(), entry["Color"]["a"].As<float>());
				node->m_NodeID = entry["NodeID"].As<int64_t>();
				node->m_Position = glm::vec2(entry["Position"]["x"].As<float>(), entry["Position"]["y"].As<float>());
				node->m_Size = glm::vec2(entry["Size"]["x"].As<float>(), entry["Size"]["y"].As<float>());
				node->m_Title = entry["Title"].As<std::string>();

				int inputIndex = 0;
				while (true)
				{
					Yaml::Node& entryPin = entry["Input_" + std::to_string(inputIndex++)];
					if (entryPin.IsNone()) break;
					node->m_InputPins.Add(VisualNodePin(*node, "", glm::vec4(), 0, false));
					VisualNodePin& pin = node->m_InputPins[node->m_InputPins.Last()];

					pin.Color = glm::vec4(entryPin["Color"]["r"].As<float>(), entryPin["Color"]["g"].As<float>(), entryPin["Color"]["b"].As<float>(), entryPin["Color"]["a"].As<float>());
					pin.IsReceivingPin = entryPin["IsReceivingPin"].As<std::string>() == "true";
					pin.Label = entryPin["Label"].As<std::string>();
					pin.m_AdditionalData = entryPin["m_AdditionalData"].As<std::string>();
					pin.PinHeight = entryPin["PinHeight"].As<float>();
					pin.PinID = entryPin["PinID"].As<int64_t>();
					// Target still left
				}
				int outputIndex = 0;
				while (true)
				{
					Yaml::Node& entryPin = entry["Output_" + std::to_string(outputIndex++)];
					if (entryPin.IsNone()) break;
					node->m_OutputPins.Add(VisualNodePin(*node, "", glm::vec4(), 0, false));
					VisualNodePin& pin = node->m_OutputPins[node->m_OutputPins.Last()];

					pin.Color = glm::vec4(entryPin["Color"]["r"].As<float>(), entryPin["Color"]["g"].As<float>(), entryPin["Color"]["b"].As<float>(), entryPin["Color"]["a"].As<float>());
					pin.IsReceivingPin = entryPin["IsReceivingPin"].As<std::string>() == "true";
					pin.Label = entryPin["Label"].As<std::string>();
					pin.m_AdditionalData = entryPin["m_AdditionalData"].As<std::string>();
					pin.PinHeight = entryPin["PinHeight"].As<float>();
					pin.PinID = entryPin["PinID"].As<int64_t>();
					// Target still left
				}
			}
		}
		// Deserialize Wires
		{
			int index = 0;
			while (true)
			{
				Yaml::Node& entry = graph["Node_" + std::to_string(index++)];
				if (entry.IsNone()) break;
				VisualNode* node = m_Nodes[index - 1].get();

				int inputIndex = 0;
				while (true)
				{
					Yaml::Node& entryPin = entry["Input_" + std::to_string(inputIndex++)];
					if (entryPin.IsNone()) break;
					VisualNodePin& pin = node->m_InputPins[inputIndex - 1];
					if (pin.IsReceivingPin && entryPin["Target"].As<std::string>() != "NULL")
					{
						VisualNodePinFromString(entryPin["Target"].As<std::string>(), pin);
					}
				}
				int outputIndex = 0;
				while (true)
				{
					Yaml::Node& entryPin = entry["Output_" + std::to_string(outputIndex++)];
					if (entryPin.IsNone()) break;
					VisualNodePin& pin = node->m_OutputPins[outputIndex - 1];
					if (pin.IsReceivingPin && entryPin["Target"].As<std::string>() != "NULL")
					{
						VisualNodePinFromString(entryPin["Target"].As<std::string>(), pin);
					}
				}
			}
		}
	}

	std::string VisualNodeGraph::VisualNodePinToString(VisualNodePin& pin)
	{
		std::string str;

		int nodeIndex = IndexOf(*pin.GetNode());
		str += std::to_string(nodeIndex) + "/";

		if (pin.GetNode()->m_InputPins.Contains(pin))
		{
			str += "Input/";
			str += std::to_string(pin.GetNode()->m_InputPins.IndexOf(pin));
		}
		else
		{
			str += "Output/";
			str += std::to_string(pin.GetNode()->m_OutputPins.IndexOf(pin));
		}

		return str;
	}
	void VisualNodeGraph::VisualNodePinFromString(const std::string& str, VisualNodePin& a)
	{
		SUORA_ASSERT(a.IsReceivingPin, "Pin A must be a receiving Pin!");
		std::vector<std::string> strs = StringUtil::SplitString(str, '/');

		Ref<VisualNode> node = m_Nodes[std::stoi(strs[0])];
		const bool isInput = strs[1] == "Input";

		if (isInput)
		{
			a.Target = &node->m_InputPins[std::stoi(strs[2])];
		}
		else
		{
			a.Target = &node->m_OutputPins[std::stoi(strs[2])];
		}

	}

}