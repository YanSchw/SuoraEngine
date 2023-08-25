#pragma once
#include "../MajorTab.h"
#include "NodeClassEditor.generated.h"

namespace Suora
{
	struct DockingSpace;

	enum class PlayState
	{
		Editor = 0,
		Playing = 1,
		Simulating
	};

	class NodeClassEditor : public MajorTab
	{
		SUORA_CLASS(4786947683);
	public:
		Ptr<Blueprint> m_BlueprintClass = nullptr;
		Ref<World> m_World = nullptr;
		PlayState m_CurrentPlayState = PlayState::Editor;
		Ptr<Object> m_SelectedObject = nullptr;

		NodeClassEditor();
		NodeClassEditor(Blueprint* blueprint);
		~NodeClassEditor();

		void ResetEnvironment();

		void Init() override;
		virtual void Update(float deltaTime) override;
		virtual void ApplyChangesOfOtherMajorTabs(MajorTab* other) override;
		virtual Texture* GetIconTexture() override;

		void SaveAsset() override;

		void SerializeActor();
		void SerializeAllNodeGraphs();

		void PlayInEditor();
		void StopPlayInEditor();

		World* GetEditorWorld();

		void DrawToolbar(float& x, float y, float height) override;
		Ref<NodeGraphEditor> CreateNodeClassGraphEditorInstance(Blueprint* blueprint, int i);
		void OpenNodeGraph(Blueprint* blueprint, int i);
	private:
		Ptr<Node> m_Actor = nullptr;
		Ref<ViewportPanel> m_ViewportPanel = nullptr;
		Ref<DetailsPanel> m_DetailsPanel = nullptr;
		Ref<LevelOutliner> m_NodeOutliner = nullptr;
		Ref<struct DockingSpace> m_MainDockingSpace = nullptr;
		Array<Ref<NodeGraphEditor>> m_NodeGraphs;
		bool m_InitResetCamera = false;
		uint64_t m_NodeNameUpdateIndex = 0;
	};

}