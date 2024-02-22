#pragma once
#include "Suora/Editor/Panels/MinorTab.h"

namespace Suora
{
	class Node;
	class World;

	class LevelOutliner : public MinorTab
	{
	public:
		LevelOutliner(MajorTab* majorTab);
		~LevelOutliner();

		static std::vector<EditorUI::ContextMenuElement> CreateNodeMenu(World* world, Node* node);

		Texture2D* TexActorIcon = nullptr;
		Texture2D* TexVisible0 = nullptr;
		Texture2D* TexVisible1 = nullptr;
		Texture2D* TexArrowRight = nullptr;
		Texture2D* TexArrowDown = nullptr;

		Texture* GetNodeIconTexture(const Class& cls);

		std::unordered_map<Node*, bool> m_DropDowns;

		Node* GetSelectedObject();
		void SetSelectedObject(Node* node);

		virtual void Render(float deltaTime) override;

		World* GetEditorWorld();
		void DrawNode(float& x, float& y, Node* node);

	private:
		int _EntryCount = 0;
		float m_HeaderSeperator1 = 0.75f;
		float m_HeaderSeperator2 = 0.97f;
		Ptr<Node> m_DragNode = nullptr;
		Ptr<Node> m_RootNode = nullptr;
		std::unordered_map<Node*, float> m_TempYValuesOfParentNodes;

		friend class NodeClassEditor;
	};
}