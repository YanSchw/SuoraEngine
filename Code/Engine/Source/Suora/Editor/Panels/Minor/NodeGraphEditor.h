#pragma once
#include "Suora/Editor/Panels/MinorTab.h"
#include <cstdint>
#include "Suora/Serialization/Yaml.h"
#include "Suora/NodeScript/NodeGraph.h"

namespace Suora
{
	struct VisualNode;

	class NodeGraphEditor : public MinorTab
	{
	private:
		Ref<Framebuffer> m_NodeBuffer;
	public:
		Ref<VisualNodeGraph> m_Graph;
		Vec2 m_CameraPos = Vec2(0), m_CameraPosTarget = Vec2(0);
		Vec2 m_MouseCursorPosInGraph;
		float m_Zoom = 1.0f, m_ZoomTarget = 1.0f;
		Array<VisualNode*> m_SelectedNodes;
		VisualNodePin* m_DragPin = nullptr;
		Array<int64_t> m_InvertDragPinForIDs;

		Ref<Texture> m_Checkerboard;
		Ref<Texture> m_PinConnectionTexture;
		Ref<Texture> m_PinConnectionTexture2;
		Ref<Texture> m_PinConnectionExecTexture;
		Ref<Texture> m_PinConnectionExecTexture2;
		Ref<Shader> m_LineShader;

		NodeGraphEditor(MajorTab* majorTab, const Ref<VisualNodeGraph>& graph);
		~NodeGraphEditor();

		Font* GetNodeGraphFont() const;

		virtual void OnNodeGraphRender(float deltaTime) = 0;
		virtual void DrawVisualNode(VisualNode& node) = 0;
		virtual float DrawVisualNodePin(VisualNode& node, VisualNodePin& pin, bool inputPin, float y) = 0;
		virtual void ProccessNodePinIDConversion(VisualNodePin& receivingPin, VisualNodePin& targetPin) = 0;

		virtual Ref<Texture> GetPinIconTexture(int64_t pinID, bool hasOtherPin);

		void DefaultDrawVisualNode(VisualNode& node);
		void DefaultDrawVisualNodePin(VisualNode& node, VisualNodePin& pin, bool inputPin, float y);

		void Render(float deltaTime) override;
		float DrawNodeInputPins(VisualNode& node, float headerOffset);
		void DrawWires(VisualNodePin& a, VisualNodePin& b);
		void DrawWires(const Vec2& a, const Vec2& b, const Color& color);
		void DrawWireLine(float x1, float y1, float x2, float y2, const Color& color);
		void ConnectNodePins(VisualNodePin& a, VisualNodePin& b);

		void UpdateMouseCursorInGraph();
	};
}