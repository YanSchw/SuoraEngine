#pragma once
#include "Suora/Editor/EditorUI.h"
#include <functional>

namespace Suora
{

	struct SelectionOverlay : public EditorUI::Overlay
	{
		struct SelectionOverlayEntry;
		struct SearchCategory;
		
		SelectionOverlay();
		void RefreshEntries();
		bool DrawSearchCategories(Array<Ref<SearchCategory>>& categories, float& w, int& i, float _x, float _width);
		bool DrawSearchEntries(Array<SelectionOverlayEntry*>& entries, float& w, int& i, float _x, float _width);
		virtual void Render(float deltaTime) override;

	public:
		struct SelectionOverlayEntry
		{
			String Label = "Entry";
			Array<String> Tags;
			std::function<void(void)> Lambda;
			SelectionOverlayEntry(const String& label, const Array<String>& tags, const std::function<void(void)>& lambda)
				: Label(label), Tags(tags), Lambda(lambda) { }
		};

		std::unordered_map<String, bool> m_SearchCategoryClicked;
		struct SearchCategory
		{
			String Label = "";
			Array<Ref<SearchCategory>> SubCategories;
			Array<SelectionOverlayEntry*> SubEntries;
		};

		inline static String s_SearchLabel, s_LastSearchLabel;
		Array<SelectionOverlayEntry> m_Entries, m_DisplayEntries;
		float m_ScrollY = 0.0f;
		int32_t m_SelectedItem = 0;
		bool m_InitSearch = true;
	};

}