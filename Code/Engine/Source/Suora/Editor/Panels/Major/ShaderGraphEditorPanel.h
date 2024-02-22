#pragma once
#include "Suora/Editor/Panels/MajorTab.h"
#include "Suora/Editor/Panels/Minor/ViewportPanel.h"
#include "Suora/NodeScript/ShaderNodeGraph.h"
#include "ShaderGraphEditorPanel.generated.h"

namespace Suora
{
	class ShaderGraph;
	class World;

	class ShaderGraphNodeEditor : public NodeGraphEditor
	{
		Array<float> m_TempDragFloatFields;
		int64_t PinIndex = 0;
	public:
		ShaderGraphNodeEditor(MajorTab* majorTab)
			: NodeGraphEditor(majorTab, CreateRef<ShaderNodeGraph>())
		{
		}
		~ShaderGraphNodeEditor()
		{
		}
		ShaderNodeGraph* GetShaderNodeGraph()
		{
			return (ShaderNodeGraph*)m_Graph.get();
		}
		virtual Ref<Texture> GetPinIconTexture(int64_t pinID, bool hasOtherPin) override;

		void OnNodeGraphRender(float deltaTime) override;
		void DrawVisualNode(VisualNode& node) override;
		float DrawVisualNodePin(VisualNode& node, VisualNodePin& pin, bool inputPin, float y) override;
		void ProccessNodePinIDConversion(VisualNodePin& receivingPin, VisualNodePin& targetPin) override
		{
		}
	};

	class ShaderGraphEditorPanel : public MajorTab
	{
		SUORA_CLASS(845879543);
	public:
		Ptr<ShaderGraph> m_ShaderGraph = nullptr;
		World m_World;
		Ref<ViewportPanel> m_ViewportPanel = nullptr;
		Ref<class ShaderGraphNodeEditor> m_NodeEditor = nullptr;

		ShaderGraphEditorPanel();
		ShaderGraphEditorPanel(ShaderGraph* material);
		~ShaderGraphEditorPanel();

		void Init() override;
		virtual void Update(float deltaTime) override;
		virtual Texture* GetIconTexture() override;
		void SaveAsset() override;
	private:
		bool m_InitResetCamera = false;
		String m_LastBaseShader;
	};

}