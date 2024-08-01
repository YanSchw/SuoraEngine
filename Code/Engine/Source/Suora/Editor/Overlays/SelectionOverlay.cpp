#include "Precompiled.h"
#include "SelectionOverlay.h"

namespace Suora
{

	SelectionOverlay::SelectionOverlay()
	{
		s_SearchLabel = "";
		RefreshEntries();
	}

	void SelectionOverlay::RefreshEntries()
	{
		m_DisplayEntries.Clear();
		if (s_SearchLabel == "")
		{
			m_DisplayEntries = m_Entries;
			m_SearchCategoryClicked = {};
			return;
		}
		else
		{
			for (auto& It : m_SearchCategoryClicked)
			{
				It.second = true;
			}
		}
		for (SelectionOverlayEntry& entry : m_Entries)
		{
			if (StringUtil::ToLower(entry.Label).find(StringUtil::ToLower(s_SearchLabel)) != String::npos)
			{
				m_DisplayEntries.Add(entry);
			}
		}
	}

	bool SelectionOverlay::DrawSearchCategories(Array<Ref<SearchCategory>>& categories, float& w, int& i, float _x, float _width)
	{
		EditorUI::ButtonParams params;
		params.OverrideActivationEvent = true;
		params.OverrittenActivationEvent = [=]() { return NativeInput::GetMouseButtonDown(Mouse::ButtonLeft) && EditorUI::GetInput().y >= y + 25.0f && EditorUI::GetInput().y <= y + height - 40.0f; };
		params.TextOrientation = Vec2(-0.95f, 0.0f);
		for (auto& entry : categories)
		{
			w -= 25.0f;

			if (w + m_ScrollY <= y || w + m_ScrollY >= y + height - 40.0f) { if (!m_SearchCategoryClicked[entry->Label]) { i++; continue; } }
			params.ButtonColor = (m_SelectedItem == i) ? EditorPreferences::Get()->UiHighlightColor : EditorPreferences::Get()->UiBackgroundColor;
			if (!(w + m_ScrollY <= y || w + m_ScrollY >= y + height - 40.0f))
			{
				if (EditorUI::Button("", _x + 5.0f, w + m_ScrollY, _width, 25.0f, params) || (NativeInput::GetKeyDown(Key::Enter) && m_SelectedItem == i))
				{
					m_SearchCategoryClicked[entry->Label] = !m_SearchCategoryClicked[entry->Label];
					m_SelectedItem = i;
				}
				{
					EditorUI::ButtonParams _TxtParam = EditorUI::ButtonParams::Invisible();
					_TxtParam.TextOrientation = Vec2(-0.95f, 0.0f);
					EditorUI::Button(entry->Label, _x + 5.0f + 25.0f, w + m_ScrollY, _width, 25.0f, _TxtParam);
					EditorUI::DrawTexturedRect(m_SearchCategoryClicked[entry->Label] ? Icon::ArrowDown : Icon::ArrowRight, _x + 5.0f, w + m_ScrollY, 25.0f, 25.0f, 0.0f, Color(1.0f));
				}
			}
			i++;

			if (m_SearchCategoryClicked[entry->Label])
			{
				if (DrawSearchCategories(entry->SubCategories, w, i, _x + 25.0f, _width - 25.0f))
				{
					return true;
				}
				if (DrawSearchEntries(entry->SubEntries, w, i, _x + 25.0f, _width - 25.0f))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool SelectionOverlay::DrawSearchEntries(Array<SelectionOverlayEntry*>& entries, float& w, int& i, float _x, float _width)
	{
		EditorUI::ButtonParams params;
		params.OverrideActivationEvent = true;
		params.OverrittenActivationEvent = [=]() { return NativeInput::GetMouseButtonDown(Mouse::ButtonLeft) && EditorUI::GetInput().y >= y + 25.0f && EditorUI::GetInput().y <= y + height - 40.0f; };
		params.TextOrientation = Vec2(-0.95f, 0.0f);
		params.ButtonOutlineColor = Color(0.0f);
		params.ButtonColor = Color(0.0f);
		for (auto& entry : entries)
		{
			w -= 25.0f;
			if (w + m_ScrollY <= y || w + m_ScrollY >= y + height - 40.0f) { i++; continue; }
			params.ButtonColor = (m_SelectedItem == i) ? EditorPreferences::Get()->UiHighlightColor : EditorPreferences::Get()->UiBackgroundColor;
			Array<String> strs = StringUtil::SplitString(entry->Label, '/');
			if (EditorUI::Button(strs[strs.Last()], _x + 5.0f, w + m_ScrollY, _width, 25.0f, params) || (NativeInput::GetKey(Key::Enter) && m_SelectedItem == i))
			{
				entry->Lambda();
				Dispose();
				if (!NativeInput::GetKey(Key::Enter)) NativeInput::ConsumeInput();
				return true;
			}
			i++;
		}
		return false;
	}

	void SelectionOverlay::Render(float deltaTime)
	{
		Array<Ref<SearchCategory>> Categories;
		Array<SelectionOverlayEntry*> PlainEntries;
		for (SelectionOverlayEntry& entry : m_DisplayEntries)
		{
			Array<String> strs = StringUtil::SplitString(entry.Label, '/');
			if (strs.Size() == 1) PlainEntries.Add(&entry);
			else
			{
				Ref<SearchCategory> category;
				{
					for (int i = 0; i < strs.Size() - 1; i++)
					{
						if (!category)
						{
							for (int j = 0; j < Categories.Size(); j++)
							{
								if (Categories[j]->Label == strs[i])
								{
									category = Categories[j];
									goto nextCat;
								}
							}
							Ref<SearchCategory> newCategory = CreateRef<SearchCategory>();
							newCategory->Label = strs[i];
							Categories.Add(newCategory);
							category = newCategory;
						}
						else
						{
							for (int j = 0; j < category->SubCategories.Size(); j++)
							{
								if (category->SubCategories[j]->Label == strs[i])
								{
									category = category->SubCategories[j];
									goto nextCat;
								}
							}
							Ref<SearchCategory> newCategory = CreateRef<SearchCategory>();
							newCategory->Label = strs[i];
							category->SubCategories.Add(newCategory);
							category = newCategory;
						}
					nextCat:;
					}
				}
				category->SubEntries.Add(&entry);
			}
		}

		EditorUI::DrawRect(x, y, width, height, 2.0f, EditorPreferences::Get()->UiColor);

		if (m_InitSearch)
		{
			m_InitSearch = false;
			EditorUI::_SetTextFieldStringPtr(&s_SearchLabel, x + 5.0f, y + height - 30.0f, width - 10.0f, 25.0f);
		}

		EditorUI::ButtonParams params;
		params.OverrideActivationEvent = true;
		params.OverrittenActivationEvent = [=]() { return NativeInput::GetMouseButtonDown(Mouse::ButtonLeft) && EditorUI::GetInput().y >= y + 25.0f && EditorUI::GetInput().y <= y + height - 40.0f; };
		params.TextOrientation = Vec2(-0.95f, 0.0f);
		float w = y + height - 40.0f;
		int i = 0;
		if (DrawSearchCategories(Categories, w, i, x, width - 15.0f)) return;
		if (DrawSearchEntries(PlainEntries, w, i, x, width - 15.0f)) return;

		if (NativeInput::GetKeyDown(Key::Up)) m_SelectedItem--;
		if (NativeInput::GetKeyDown(Key::Down)) m_SelectedItem++;
		m_SelectedItem = Math::Clamp(m_SelectedItem, 0, i - 1);

		EditorUI::DrawRect(x, y, width, 25.0f, 2.0f, EditorPreferences::Get()->UiBackgroundColor);
		EditorUI::DrawRect(x, y + height - 40.0f, width, 40.0f, 2.0f, EditorPreferences::Get()->UiBackgroundColor);

		EditorUI::TextField(&s_SearchLabel, x + 5.0f, y + height - 30.0f, width - 10.0f, 25.0f);
		if (s_SearchLabel == "") EditorUI::Text("Search...", Font::Instance, x + 5.0f, y + height - 30.0f, width - 10.0f, 25.0f, 18.0f, Vec2(-0.95f, 0.0f), Color(1.0f));
		if (s_SearchLabel != s_LastSearchLabel)
		{
			s_LastSearchLabel = s_SearchLabel;
			RefreshEntries();
		}


		const float EntryHeight = 25.0f;
		const float scrollDown = height - EntryHeight * (i + 1) - (EntryHeight + 15.0f) - 35.0f;
		EditorUI::ScrollbarVertical(x + width - 6.0f, y + 30.0f, 5.0f, height - 65.0f, x, y, width, height, 0.0f, scrollDown > 0 ? 0 : Math::Abs(scrollDown), &m_ScrollY);

		if (WasMousePressedOutsideOfOverlay()) Dispose();
	}

}