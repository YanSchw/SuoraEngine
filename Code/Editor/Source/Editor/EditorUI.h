#pragma once

#include <Suora.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "Util/Icon.h"
#include "Util/EditorPreferences.h"
#include "Suora/Assets/Font.h"
#include "EditorWindow.h"

namespace Suora
{
	struct EditorUI
	{
		struct Overlay
		{
			float x = 0, y = 0, width = 100, height = 100;

			virtual void Render(float deltaTime) { }
			virtual void BackendRender(float deltaTime) { }
			virtual void OnDispose() { }

			virtual ~Overlay()
			{
			}
			bool WasMousePressedOutsideOfOverlay() 
			{ 
				return NativeInput::GetMouseButtonDown(Mouse::Button0) && m_Lifetime > 2 && (mousePosition.x < x || mousePosition.x > x + width || mousePosition.y < y || mousePosition.y > y + height);
			}

			void Dispose()
			{
				SUORA_ASSERT(CurrentWindow->m_InputEvent == EditorInputEvent::None || CurrentWindow->m_InputEvent == EditorInputEvent::EditorUI_Overlay, "You are disposing an overlay while still having Ui Input!");
				CurrentWindow->m_InputEvent = EditorInputEvent::None;
				OnDispose();
				s_IssuedDisposalsOfOverlays.Add(this);
			}
		private:
			inline static bool s_InputEventFlag = false;
			int m_Lifetime = 0;
			friend struct EditorUI;
		};
		struct SelectionOverlay : public Overlay
		{
			struct SelectionOverlayEntry
			{
				String Label = "Entry";
				Array<String> Tags;
				std::function<void(void)> Lambda;
				SelectionOverlayEntry(const String& label, const Array<String>& tags, const std::function<void(void)>& lambda)
					: Label(label), Tags(tags), Lambda(lambda) { }
			};

			inline static String s_SearchLabel, s_LastSearchLabel;
			Array<SelectionOverlayEntry> m_Entries, m_DisplayEntries;
			float m_ScrollY = 0.0f;
			int32_t m_SelectedItem = 0;
			bool m_InitSearch = true;

			SelectionOverlay() 
			{
				s_SearchLabel = ""; 
				RefreshEntries();
				//EditorUI::_SetTextFieldStringPtr(&s_SearchLabel, x + 5.0f, y + height - 30.0f, width - 10.0f, 25.0f);
			}

			void RefreshEntries()
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
			std::unordered_map<String, bool> m_SearchCategoryClicked;
			struct SearchCategory
			{
				String Label = "";
				Array<Ref<SearchCategory>> SubCategories;
				Array<SelectionOverlayEntry*> SubEntries;
			};
			bool DrawSearchCategories(Array<Ref<SearchCategory>>& categories, float& w, int& i, float _x, float _width)
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
							//if (!NativeInput::GetKey(Key::Enter)) NativeInput::ConsumeInput();
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
			bool DrawSearchEntries(Array<SelectionOverlayEntry*>& entries, float& w, int& i, float _x, float _width)
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
					std::vector<String> strs = StringUtil::SplitString(entry->Label, '/');
					if (EditorUI::Button(strs[strs.size() - 1], _x + 5.0f, w + m_ScrollY, _width, 25.0f, params) || (NativeInput::GetKey(Key::Enter) && m_SelectedItem == i))
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

			virtual void Render(float deltaTime) override
			{
				
				Array<Ref<SearchCategory>> Categories;
				Array<SelectionOverlayEntry*> PlainEntries;
				for (SelectionOverlayEntry& entry : m_DisplayEntries)
				{
					std::vector<String> strs = StringUtil::SplitString(entry.Label, '/');
					if (strs.size() == 1) PlainEntries.Add(&entry);
					else
					{
						Ref<SearchCategory> category;
						{
							for (int i = 0; i < strs.size() - 1; i++)
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
				/*for (SelectionOverlayEntry& entry : m_DisplayEntries)
				{
					w -= 25.0f;
					if (w + m_ScrollY <= y || w + m_ScrollY >= y + height - 40.0f) { i++; continue; }
					params.ButtonColor = (m_SelectedItem == i) ? EditorPreferences::Get()->UiHighlightColor : EditorPreferences::Get()->UiBackgroundColor;
					if (EditorUI::Button(entry.Label, x + 5.0f, w + m_ScrollY, width - 15.0f, 25.0f, params) || (NativeInput::GetKey(Key::Enter) && m_SelectedItem == i))
					{
						entry.Lambda();
						Dispose();
						if (!NativeInput::GetKey(Key::Enter)) NativeInput::ConsumeInput();
					}
					i++;
				}*/
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
		};

		struct ButtonParams
		{
			EditorInputEvent InputMode = EditorInputEvent::None;
			Vec2 TextOrientation = Vec2(0, 0);
			Color TextColor = EditorPreferences::Get()->UiTextColor;
			float TextSize = 28; // 32
			Font* Font = Font::Instance;
			bool TextDropShadow = false;
			float TextOffsetLeft = 0.0f;
			float TextOffsetRight = 0.0f;
			float TextOffsetTop = 0.0f;
			float TextOffsetBottom = 0.0f;

			class Texture2D* CenteredIcon = nullptr;

			bool OverrideActivationEvent = false;
			std::function<bool(void)> OverrittenActivationEvent;

			String TooltipText = "";

			float ButtonRoundness = 4;
			bool ButtonDropShadow = false;
			Color ButtonColor = EditorPreferences::Get()->UiColor;
			Color ButtonColorHover = EditorPreferences::Get()->UiForgroundColor;
			Color ButtonColorClicked = EditorPreferences::Get()->UiForgroundColor;
			Color ButtonOutlineColor = EditorPreferences::Get()->UiBackgroundColor;
			Color ButtonOutlineHoverColor = EditorPreferences::Get()->UiBackgroundColor * 1.1f;
			bool useButtonOutlineHoverColor = false;
			Cursor HoverCursor = Cursor::Hand;

			bool* OutHover = nullptr;

			ButtonParams()
			{
			}

			inline static ButtonParams Invisible()
			{
				ButtonParams Params;

				Params.ButtonColor =        Color(0.0f);
				Params.ButtonColorHover =   Color(0.0f);
				Params.ButtonColorClicked = Color(0.0f);
				Params.ButtonOutlineColor = Color(0.0f);

				return Params;
			}
			inline static ButtonParams Highlight()
			{
				ButtonParams Params;

				Params.ButtonColor = Math::Lerp(EditorPreferences::Get()->UiHighlightColor, Color(0, 0, 0, 1), 0.3f);
				Params.ButtonColorHover = Math::Lerp(EditorPreferences::Get()->UiHighlightColor, Color(0, 0, 0, 1), 0.22f);
				Params.ButtonColorClicked = Math::Lerp(EditorPreferences::Get()->UiHighlightColor, Color(1.0f), 0.45f);
				Params.ButtonOutlineColor = EditorPreferences::Get()->UiBackgroundColor;
				Params.ButtonOutlineHoverColor = EditorPreferences::Get()->UiForgroundColor;
				Params.useButtonOutlineHoverColor = true;
				//Params.TextColor = EditorPreferences::Get()->UiBackgroundColor;

				return Params;
			}
			inline static ButtonParams Outlined()
			{
				ButtonParams Params;

				Params.ButtonOutlineColor = EditorPreferences::Get()->UiHighlightColor;
				Params.ButtonOutlineHoverColor = Math::Lerp(EditorPreferences::Get()->UiHighlightColor, Color(1.0f), 0.35f);
				Params.useButtonOutlineHoverColor = true;
				Params.TextColor = EditorPreferences::Get()->UiHighlightColor;

				return Params;
			}
			inline static ButtonParams BlackButton()
			{
				ButtonParams Params;

				Params.ButtonColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
				Params.ButtonColorHover = Color(0.0f, 0.0f, 0.0f, 1.0f);
				Params.ButtonColorClicked = Color(0.0f, 0.0f, 0.0f, 1.0f);
				Params.ButtonOutlineColor = Color(0.7f, 0.7f, 0.7f, 1.0f);
				Params.ButtonOutlineHoverColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
				Params.useButtonOutlineHoverColor = true;

				return Params;
			}
			inline static ButtonParams DarkerButton()
			{
				ButtonParams Params;

				Params.ButtonColor = Math::Lerp(Params.ButtonColor, Color(0.0f, 0.0f, 0.0f, 1.0f), 0.5f);
				Params.ButtonColorHover = Math::Lerp(Params.ButtonColorHover, Color(0.0f, 0.0f, 0.0f, 1.0f), 0.5f);
				Params.ButtonColorClicked = Math::Lerp(Params.ButtonColorClicked, Color(0.0f, 0.0f, 0.0f, 1.0f), 0.5f);
				Params.ButtonOutlineColor = Math::Lerp(Params.ButtonOutlineColor, Color(0.0f, 0.0f, 0.0f, 1.0f), 0.5f);
				Params.ButtonOutlineHoverColor = Math::Lerp(Params.ButtonOutlineHoverColor, Color(0.0f, 0.0f, 0.0f, 1.0f), 0.5f);
				Params.useButtonOutlineHoverColor = true;

				return Params;
			}
		};

	private:
		inline static Cursor s_CurrentCursor = Cursor::Default;
		inline static Array<Ref<Overlay>> s_Overlays;
		inline static Array<Overlay*> s_IssuedDisposalsOfOverlays;
		inline static Overlay* s_CurrentProcessedOverlay = nullptr;
	public:

		static Array<Ref<Overlay>> _GetOverlays() { return s_Overlays; }

		static void RerenderAssetPreviews();
		static void Init();

		static void Tick(float deltaTime);

		// Input has to be relative to the current Context e.g. Framebuffer / Minortab
		//     ==============================================
		//    ||                                            ||
		//    ||             EditorUI wants Mousepositions  ||
		//    ||             in this format   [PIXELS] (!)  ||
		//    ||                                            ||
		//    ||  Y                                         ||
		//    ||                                            ||
		//    ||  ^                                         ||
		//    ||  |                                         ||
		//    ||  |                                         ||
		//    ||  o----->   X                               ||	
		//    ||                                            ||  
		//     ==============================================
		inline static Vec2 mousePosition = Vec2(), mouseOffset = Vec2();
		static void PushInput(float x, float y, float offsetX, float offsetY);
		static const Vec2& GetInput();
		static const Vec2& GetInputOffset();

	private:
		inline static uint32_t s_WasInputConsumed = 0;
	public:
		static void ConsumeInput();
		static bool WasInputConsumed();

		inline static EditorWindow* CurrentWindow = nullptr;
		inline static void SetCurrentWindow(EditorWindow* window)
		{
			CurrentWindow = window;
		}
		inline static void SetCursor(Cursor cursor)
		{
			s_CurrentCursor = cursor;
		}
	private:
		static void InitDrawRectVAO();
	public:

		static void DrawRect(float x, float y, float width, float height, float roundness, const Color& color);
		static void DrawTexturedRect(Texture* texture, float x, float y, float width, float height, float roundness, const Color& color);
		static void DrawTexturedRect(Ref<Texture> texture, float x, float y, float width, float height, float roundness, const Color& color);
		static void DrawRectOutline(float x, float y, float width, float height, float thickness, const Color& color);
	private:
		inline static Ref<Shader> UiShader, TextShader;
		inline static Ref<Framebuffer> GlassBuffer;
	public:
		static void Text(const String& text, Font* font, float x, float y, float width, float height, float size, const Vec2& orientation, const Color& color, const Array<Color>& colors = {});
	public:
		static bool Button(const String& text, float x, float y, float width, float height, ButtonParams params = ButtonParams());

		static bool DragSource(float x, float y, float width, float height, float offset, EditorInputEvent input = EditorInputEvent::None);
	public:
		static bool DropDown(const std::vector<std::pair<String, std::function<void(void)>>>& options, int index, float x, float y, float width, float height);

	private:
		inline static String* TextField_Str = nullptr;
		inline static Array<char> TextFieldCharBuffer;
		inline static bool TextField_StrFlag = false;
		static void TextFieldCharInput(char keyCode);
	public:
		struct TextFieldOverlay : public Overlay
		{
			String* Str = nullptr;
			float m_TextSize = 32.0f;
			int64_t Cursor = 0, CursorSelectionOffset = 0;
			std::function<void(String)> LambdaCallback;
			bool needsFlag = true;
			TextFieldOverlay(String* str, float textSize, const std::function<void(String)>& lambda = nullptr, bool needsFlag = true) : Str(str), m_TextSize(textSize), LambdaCallback(lambda), needsFlag(needsFlag)
			{
				TextField_StrFlag = true;
				TextField_Str = str;
				TextFieldCharBuffer.Clear();
			}
			void Render(float deltaTime) override
			{
				Overlay::Render(deltaTime);

				if (TextField_StrFlag) TextField_StrFlag = false; else if (needsFlag)	{  if (LambdaCallback) LambdaCallback(*Str); Dispose(); TextField_Str = nullptr;  return; }
				if (WasMousePressedOutsideOfOverlay())									{  if (LambdaCallback) LambdaCallback(*Str); Dispose(); TextField_Str = nullptr;  return; }
				if (NativeInput::GetKeyDown(Key::Enter))										{ if (LambdaCallback) LambdaCallback(*Str); Dispose(); TextField_Str = nullptr;  return; }
				if (NativeInput::GetKeyDown(Key::Tab))										{  if (LambdaCallback) LambdaCallback(*Str); Dispose(); TextField_Str = nullptr;  return; }

				if (NativeInput::GetMouseButton(Mouse::ButtonLeft) && Str)
				{
					int pos = (int)((GetInput().x - x) / 5.0f);
					pos = 0;

					float IterStrWidth = Font::Instance->GetCharWidth((*Str)[0], m_TextSize) * 0.5f;
					while ((GetInput().x - x) > IterStrWidth) { if (Str->size() <= pos) break; IterStrWidth += Font::Instance->GetCharWidth((*Str)[pos++], m_TextSize) * 0.5f; }

					pos = Math::Clamp(pos, 0, Str->length());
					if (NativeInput::GetMouseButtonDown(Mouse::ButtonLeft))
					{
						Cursor = (size_t)pos;
						CursorSelectionOffset = 0;
					}
					else
					{
						int64_t last = Cursor;
						Cursor = (size_t)pos;
						CursorSelectionOffset -= Cursor - last;
					}
				}

				int64_t begin = CursorSelectionOffset > 0 ? Cursor : Cursor + CursorSelectionOffset;
				int64_t offset = CursorSelectionOffset > 0 ? CursorSelectionOffset : -CursorSelectionOffset;

				if (TextFieldCharBuffer.Size() > 0)
				{
					EraseCursorSelection(begin, offset);
				}
				for (int i = TextFieldCharBuffer.Size() - 1; i >= 0; i--)
				{
					(*Str).insert(Cursor++, 1, TextFieldCharBuffer[i]);
					TextFieldCharBuffer.RemoveAt(i);
				}

				if (NativeInput::GetKeyDown(Key::Backspace) && Str->length() > 0 && Cursor > 0 && offset == 0) Str->erase(--Cursor, 1);
				else if (NativeInput::GetKeyDown(Key::Backspace) && Str->length() > 0 && offset > 0) { EraseCursorSelection(begin, offset); }

				if (NativeInput::GetKeyDown(Key::Delete) && Str->length() > 0 && Cursor < Str->length() && offset == 0) Str->erase(Cursor, 1);
				else if (NativeInput::GetKeyDown(Key::Delete) && Str->length() > 0 && Cursor < Str->length() && offset > 0) { EraseCursorSelection(begin, offset); }

				EditorUI::DrawRect(x, y, width, height, 4, EditorPreferences::Get()->UiHighlightColor);
				EditorUI::DrawRect(x + 1, y + 1, width - 2, height - 2, 4, Color(0.0f, 0.0f, 0.0f, 1.0f));
				Text(*Str, Font::Instance, x + 5.0f, y, width - 6.0f, height, m_TextSize, Vec2(-1.0f, 0.0f), Color(1));

				if (NativeInput::GetKeyDown(Key::Left) && !NativeInput::GetKey(Key::LeftControl)) { Cursor--; CursorSelectionOffset = 0; if (Cursor < 0) Cursor = 0; }
				else if (NativeInput::GetKeyDown(Key::Left) && NativeInput::GetKey(Key::LeftControl)) { CursorSelectionOffset--; if (Cursor + CursorSelectionOffset < 0) CursorSelectionOffset++; }
				if (NativeInput::GetKeyDown(Key::Right) && !NativeInput::GetKey(Key::LeftControl)) { Cursor++; CursorSelectionOffset = 0; if (Cursor > Str->length()) Cursor = Str->length(); }
				else if (NativeInput::GetKeyDown(Key::Right) && NativeInput::GetKey(Key::LeftControl)) { CursorSelectionOffset++; if (Cursor + CursorSelectionOffset > Str->length()) CursorSelectionOffset--; }

				{ // Cursor
					String cursor = *Str; cursor.append(" ");
					cursor[Cursor] = '|';
					Array<Color> cols; for (int i = 0; i < cursor.size(); i++) cols.Add(Color(i == Cursor ? 1 : 0));
					Text(cursor, Font::Instance, x - 3.0f + 5.0f, y, width, height, m_TextSize, Vec2(-1.0f, 0.0f), Color(1), cols);
				}

				const Color CursorSelectionColor = Color(EditorPreferences::Get()->UiHighlightColor.r, EditorPreferences::Get()->UiHighlightColor.g, EditorPreferences::Get()->UiHighlightColor.b, 0.35f);
				if (CursorSelectionOffset != 0 && offset != 0)
					EditorUI::DrawRect(x + 5.0f + Font::Instance->GetStringWidth(Str->substr(0, begin), m_TextSize) / 2.f, y, Font::Instance->GetStringWidth(Str->substr(begin, offset), m_TextSize) / 2.f, height, 3, CursorSelectionColor); // Color(0.35f, 0.35f, 0.75f, 0.25f)
			}
			void EraseCursorSelection(int64_t& begin, int64_t& offset)
			{
				Str->erase(begin, offset);
				if (CursorSelectionOffset < 0) Cursor += CursorSelectionOffset;
				Cursor = Math::Clamp(Cursor, 0, Str->length());
				begin = Cursor;
				CursorSelectionOffset = 0;
				offset = 0;
			}
		};

	public:
		inline static ButtonParams TextFieldButtonParams()
		{
			ButtonParams Params;
			Params.TextOrientation = Vec2(-1.0f, 0.0f);
			Params.TextOffsetLeft = 5.0f;
			Params.TextDropShadow = true;
			Params.HoverCursor = Cursor::IBeam;
			return Params;
		}
		static void TextField(String* str, float x, float y, float width, float height, ButtonParams params = TextFieldButtonParams(), const std::function<void(String)>& lambda = nullptr);
		static void _SetTextFieldStringPtr(String* str, float x, float y, float width, float height, bool needsFlag = true);

	private:
		inline static void* DraggedNumberPtr = nullptr;
		inline static int32_t DraggedInt32BeginValue = 0;
		inline static float DraggedFloatBeginValue = 0.0f;
		inline static String DraggedNumberStr = "";
		inline static void* DraggedNumberTabulatePtr = nullptr; inline static bool DraggedNumberTabulateNext = false;
	public:
		static void DragInt32(int32_t* i, float x, float y, float width, float height, const std::function<void(String)>& lambda = nullptr);
		static void DragFloat(float* f, float x, float y, float width, float height, const std::function<void(String)>& lambda = nullptr);
		static void DragNumber(void* n, ClassMember::Type type, float x, float y, float width, float height, const std::function<void(String)>& lambda = nullptr);

	private:
		inline static float* SliderFloat_F = nullptr;
	public:
		static void SliderFloat(float* f, float min, float max, float x, float y, float width, float height);

		static bool Checkbox(bool* b, float x, float y, float width, float height);

		static bool AssetDropdown(Asset** asset, Class assetClass, float x, float y, float width, float height);

		static void DrawAssetPreview(Asset* asset, const Class& assetClass, float x, float y, float width, float height);

		static void SubclassSelectionMenu(const Class& base, const std::function<void(Class)>& lambda);

		// Overlays
		template<class T, class ... Args>
		static T* CreateOverlay(float x, float y, float width, float height, Args&&... args)
		{
			CurrentWindow->m_InputEvent = EditorInputEvent::EditorUI_Overlay;
			Ref<Overlay> overlay = Ref<Overlay>(new T(std::forward<Args>(args)...));
			overlay->x = x >= 0.0f ? x : 0.0f;
			overlay->y = y >= 0.0f ? y : 0.0f;
			overlay->width = width;    if (x + width > CurrentWindow->GetWindow()->GetWidth()) overlay->x -= ((x + width) - CurrentWindow->GetWindow()->GetWidth());
			overlay->height = height;  if (y + height > CurrentWindow->GetWindow()->GetHeight()) overlay->y -= ((y + height) - CurrentWindow->GetWindow()->GetHeight());
			s_Overlays.Add(overlay);

			return (T*)overlay.get();
		}
		static Overlay* GetHoveredOverlay();
		static bool IsNotHoveringOverlays();
		static void RenderOverlays(float deltaTime);

		inline static std::unordered_map<Class, Texture2D*> s_ClassIcons;
		static Texture2D* GetClassIcon(const Class& cls);

	private:
		inline static std::unordered_map<int64_t, bool> CategoryShutterStates;
	public:
		static bool CategoryShutter(int64_t id, const String& category, float x, float& y, float width, float height, ButtonParams params = ButtonParams());

		// Scrollbars
	private:
		inline static void* ScrollbarVerticalPtr = nullptr;
		inline static float ScrollbarVerticalY = 0;
	public:
		static void ScrollbarVertical(float x, float y, float width, float height, float rectX, float rectY, float rectWidth, float rectHeight, float scrollUp, float scrollDown, float* scrollCurrent);

		// Tooltip
		inline static String tooltipText = "Hello, World!";
		inline static float tooltipAlpha = 0;
		inline static float tooltipFrames = 0;
		static void Tooltip(const String& text, float x, float y, float width, float height);
		static void Tooltip(const String& text);

		static void ColorPicker(Color* color, float x, float y, float width, float height, ButtonParams params = ButtonParams(), const std::function<void(void)>& OnColorChange = {}, const std::function<void(void)>& OnColorReset = {});

		struct ContextMenuElement
		{
			std::vector<ContextMenuElement> SubElements;
			std::function<void(void)> Lambda;
			String Label;
			Texture2D* Image = nullptr;

			ContextMenuElement() = default;
			ContextMenuElement(const std::vector<ContextMenuElement>& subElements, const std::function<void(void)>& lambda, const String& label, Texture2D* image)
				: SubElements(subElements), Lambda(lambda), Label(label), Image(image) { }
		};
		static void CreateContextMenu(const std::vector<ContextMenuElement>& elements);
		static void CreateContextMenu(const std::vector<ContextMenuElement>& elements, float x, float y);

		struct SubclassHierarchyBoxParams
		{
			Class RootClass = Object::StaticClass();
			Class CurrentSelectedClass = Class::None;
			float ScrollY = 0.0f;
			std::unordered_map<Class, bool> Dropdowns;
			Color HideColor;

			SubclassHierarchyBoxParams();
		};
		static bool DrawSubclassHierarchyBox(float x, float y, float width, float height, SubclassHierarchyBoxParams& params);
	private:
		static bool DrawSubclassHierarchyEntry(const Class& cls, float x, float& y, float width, float yMin, float yMax, SubclassHierarchyBoxParams& params);

		friend class DetailsPanel;
	};
}