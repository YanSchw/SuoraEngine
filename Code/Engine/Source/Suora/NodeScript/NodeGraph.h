#pragma once

#include <string>
#include <array>
#include <glm/glm.hpp>
#include <cstdint>
#include "Suora/Serialization/Yaml.h"
#include <Suora.h>

namespace Suora
{
	class ScriptNode;
	struct VisualNode;

	struct VisualNodePin
	{
		VisualNodePin(VisualNode& node, const String& label, const Vec4& color, int64_t id, bool receive, float pinHeight = 25.0f)
			: m_Node(&node), Label(label), Color(color), PinID(id), IsReceivingPin(receive), PinHeight(pinHeight) { }
		String Label;
		Vec4 Color;
		VisualNodePin* Target = nullptr;
		bool IsReceivingPin = false;
		int64_t PinID = 0;
		float PinHeight = 25.0f;
		String m_AdditionalData = "";
		String Tooltip = "";

		bool operator==(const VisualNodePin& other) const
		{
			return (this == &other);
		}
		VisualNode* GetNode() const { return m_Node; }
		bool HasOtherPin(const Array<Ref<VisualNode>>& nodes);
	private:
		VisualNode* m_Node = nullptr;
		Vec2 PinConnectionPoint = Vec2(0.0f);
		friend class VisualNodeGraph;
		friend class NodeGraphEditor;
		friend struct VisualNodeSearchOverlay;
	};

	struct VisualNode
	{
		int64_t m_NodeID = 0;
		Vec2 m_Position = { 0, 0 };
		Vec2 m_Size = { 160, 100 };
		Vec4 m_Color = Vec4(0.25f, 0.25f, 0.555f, 1);
		Vec4 m_BackgroundColor = Vec4(0.05f, 0.05f, 0.055f, 0.9f);
		String m_Title = "VisualNode";
		Array<VisualNodePin> m_InputPins;
		Array<VisualNodePin> m_OutputPins;
		bool operator==(const VisualNode& other) const
		{
			return (this == &other);
		}
		void AddInputPin(const String& label, const Vec4& color, int64_t id, bool receive, float pinHeight = 25.0f)
		{
			m_InputPins.Add(VisualNodePin(*this, label, color, id, receive, pinHeight));
		}
		void AddOutputPin(const String& label, const Vec4& color, int64_t id, bool receive, float pinHeight = 25.0f)
		{
			m_OutputPins.Add(VisualNodePin(*this, label, color, id, receive, pinHeight));
		}
	};

	struct VisualNodeEntry
	{
		VisualNodeEntry(const Ref<VisualNode>& node)
			: m_Node(node) { }
		Ref<VisualNode> m_Node;
		Array<String> m_Tags;
	};

	struct VisualNodeGraph
	{
		Array<Ref<VisualNode>> m_Nodes;

		Array<VisualNodeEntry> m_SupportedNodes;
		void AddSupportedNode(const Ref<VisualNode>& node, const Array<String>& tags = {});
		virtual void UpdateSupportedNodes();

		virtual void TickAllVisualNodes();

		int IndexOf(VisualNode& node);

		void ClearNodePin(VisualNodePin& pin);
		void FixNodePins();
		void RemoveVisualNode(VisualNode& node);

		void SerializeNodeGraph(Yaml::Node& root);
		void DeserializeNodeGraph(Yaml::Node& root);
		String VisualNodePinToString(VisualNodePin& pin);
		void VisualNodePinFromString(const String& str, VisualNodePin& a);
	};

}