#pragma once
#include "../MajorTab.h"
#include "../Minor/ViewportPanel.h"
#include "Suora/NodeScript/ShaderNodeGraph.h"
#include "ShaderGraphEditorPanel.generated.h"

namespace Suora
{
	class ShaderGraph;
	class World;
	class Actor;
	class MeshComponent;

	struct BaseShaderInput
	{
		std::string m_Label;
		std::string m_DefaultSource;
		ShaderGraphDataType m_Type = ShaderGraphDataType::None;
		bool m_InVertexShader = false;
	};

	class ShaderGraphNodeEditor : public NodeGraphEditor
	{
		Array<BaseShaderInput> m_BaseShaderInputs;
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

		virtual Ref<Texture> GetPinIconTexture(int64_t pinID, bool hasOtherPin) override;

		void LoadBaseShaderInput(BaseShaderInput& input, int64_t& begin, int64_t& end, const std::string& str);
		void LoadBaseShaderInputs(const std::string& path);
		void GenerateShaderInput(std::string& str, int64_t begin, VisualNode* master, bool vertex, bool& error);
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

		void GenerateShaderGraphSource();
		void Init() override;
		virtual void Update(float deltaTime) override;
		virtual Texture* GetIconTexture() override;
		void SaveAsset() override;
	private:
		bool m_InitResetCamera = false;
		std::string m_LastBaseShader;
	};

}