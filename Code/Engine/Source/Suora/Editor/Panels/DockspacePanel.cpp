#include "Precompiled.h"
#include "DockspacePanel.h"
#include "Suora/Editor/Util/EditorPreferences.h"
#include "Suora/Editor/EditorUI.h"

namespace Suora
{
	DockspacePanel::DockspacePanel()
	{
		m_Window = EditorWindow::GetCurrent();
	}

	void DockspacePanel::Update(float deltaTime)
	{
		float const ui = EditorPreferences::Get()->UiScale;

		if (!EditorUI::IsNotHoveringOverlays() || GetEditorWindow()->GetWindow()->IsCursorLocked())
		{
			return;
		}

		// Dockspace Handle
		if (GetEditorWindow()->m_InputEvent == EditorInputEvent::None || GetEditorWindow()->m_InputEvent == EditorInputEvent::Dockspace_Dragging
			|| GetEditorWindow()->m_InputEvent == EditorInputEvent::Dockspace_PrepareDragging || GetEditorWindow()->m_InputEvent == EditorInputEvent::Dockspace_Resizing)
		{
			for (int i = 0; i < m_DockingSpaces.Size(); i++)
			{
				DockingSpace& space = *m_DockingSpaces[i];

				const Vec2 mouse = NativeInput::GetMousePosition();
				uint32_t x = m_PanelX + m_PanelWidth * space.x + 2;
				for (int t = 0; t < space.m_MinorTabs.Size(); t++)
				{
					if (m_CurrentDraggedTab != space.m_MinorTabs[t] && m_CurrentDraggedTab != nullptr) continue;
					if (mouse.x >= x && mouse.x <= x + 150 * ui && mouse.y <= GetEditorWindow()->GetWindow()->GetHeight() - (m_PanelY + m_PanelHeight * space.y + m_PanelHeight * space.GetHeight() - 2 - (22 * ui))
						&& mouse.y >= GetEditorWindow()->GetWindow()->GetHeight() - ((m_PanelY + m_PanelHeight * space.y + m_PanelHeight * space.GetHeight() - 2 - (22 * ui)) + (22 * ui)) && GetEditorWindow()->m_InputEvent == EditorInputEvent::None)
					{
						if (NativeInput::IsMouseButtonPressed(Mouse::ButtonLeft) && !space.wasLeftMBPressedLastFrame)
						{
							space.m_SelectedMinorTab = t;
							space.mouseX = mouse.x; space.mouseY = mouse.y;
							m_CurrentDraggedTab = space.m_MinorTabs[t];
							GetEditorWindow()->m_InputEvent = EditorInputEvent::Dockspace_PrepareDragging;
						}
					}

					if (GetEditorWindow()->m_InputEvent == EditorInputEvent::Dockspace_PrepareDragging && glm::distance(mouse, Vec2(space.mouseX, space.mouseY)) > 20.0f * ui)
					{
						GetEditorWindow()->m_InputEvent = EditorInputEvent::Dockspace_Dragging; GetEditorWindow()->GetWindow()->SetCursor(Cursor::Hand);
					}

					if (!NativeInput::IsMouseButtonPressed(Mouse::ButtonLeft) && space.wasLeftMBPressedLastFrame)
					{
						if (GetEditorWindow()->m_InputEvent == EditorInputEvent::Dockspace_PrepareDragging) GetEditorWindow()->m_InputEvent = EditorInputEvent::None;
						if (GetEditorWindow()->m_InputEvent == EditorInputEvent::Dockspace_Dragging)
						{
							GetEditorWindow()->m_InputEvent = EditorInputEvent::None;
							// Swap in docking space
							uint32_t x2 = m_PanelX + m_PanelWidth * space.x + 2;
							for (int b = 0; b < space.m_MinorTabs.Size(); b++)
							{
								if (mouse.x >= x2 && mouse.x <= x2 + 150 * ui && mouse.y <= GetEditorWindow()->GetWindow()->GetHeight() - (m_PanelY + m_PanelHeight * space.y + m_PanelHeight * space.GetHeight() - 2 - (22 * ui))
									&& mouse.y >= GetEditorWindow()->GetWindow()->GetHeight() - ((m_PanelY + m_PanelHeight * space.y + m_PanelHeight * space.GetHeight() - 2 - (22 * ui)) + (22 * ui)) && b != t)
								{
									space.m_MinorTabs.SwapElements(b, t);
									space.m_SelectedMinorTab = b;
								}
								x2 += 150.0f * ui;
							}
						}
						m_CurrentDraggedTab = nullptr;
						GetEditorWindow()->GetWindow()->SetCursor(Cursor::Default);
					}

					x += 150.0f * ui;
				}
				space.wasLeftMBPressedLastFrame = NativeInput::IsMouseButtonPressed(Mouse::ButtonLeft);
			}

			Resize();
			
		}
	}

	void DockspacePanel::Resize()
	{
		struct ResizeBorder
		{
			Array<DockingSpace*> From;
			Array<DockingSpace*> To;
			bool bIsHorizontal = false;
			float f = 0.5f;
			float limitMin, limitMax;

			ResizeBorder(bool horizontal, float f, DockingSpace* from, DockingSpace* to, float min, float max) : bIsHorizontal(horizontal), f(f), limitMin(min), limitMax(max) { if (from)From.Add(from); if (to)To.Add(to); }
			
			static ResizeBorder Join(ResizeBorder& a, ResizeBorder& b)
			{
				ResizeBorder Result = { a.bIsHorizontal, a.f, nullptr, nullptr, (a.limitMin < b.limitMin) ? a.limitMin : b.limitMin, (a.limitMax > b.limitMax) ? a.limitMax : b.limitMax };
				Result.From = a.From + b.From; // TODO: Remove Duplicates !
				Result.To = a.To + b.To;
				return Result;
			}
		};
		Array<ResizeBorder> borders;

		for (int i = 0; i < m_DockingSpaces.Size(); i++)
		{
			DockingSpace& space = *m_DockingSpaces[i];
			if (space.x > 0.0f) borders.Add(ResizeBorder(false, space.x, nullptr, &space, space.y, space.endY));
			if (space.y > 0.0f) borders.Add(ResizeBorder(true, space.y, nullptr, &space, space.x, space.endX));
			if (space.endX < 1.0f) borders.Add(ResizeBorder(false, space.endX, &space, nullptr, space.y, space.endY));
			if (space.endY < 1.0f) borders.Add(ResizeBorder(true, space.endY, &space, nullptr, space.x, space.endX));
		}

		// Join Resizeborders...
		while (true)
		{
			bool bDone = true;
			for (int i = 0; i < borders.Size(); i++)
			{
				for (int j = 0; j < borders.Size(); j++)
				{
					if (i == j) continue;
					if (i >= borders.Size()) continue;
					if (j >= borders.Size()) continue;
					ResizeBorder* I = &borders[i]; ResizeBorder* J = &borders[j];
					if (I->bIsHorizontal == J->bIsHorizontal && I->f == J->f) // TODO: Also check if one Element of FROM or TO is the same (!)
					{
						borders.Add(ResizeBorder::Join(*I, *J));
						if (i < j) { borders.RemoveAt(j); borders.RemoveAt(i); }
						else { borders.RemoveAt(i); borders.RemoveAt(j); }
						bDone = false;
					}
				}
			}
			if (bDone) break;
		}
		// Check Mouse Input
		if (GetEditorWindow()->m_InputEvent == EditorInputEvent::None)
		{
			for (int i = 0; i < borders.Size(); i++)
			{
				if (!borders[i].bIsHorizontal)
				{
					EditorUI::DrawRect(m_PanelX + borders[i].f * m_PanelWidth - 2, m_PanelY + borders[i].limitMin * m_PanelHeight, 3, (borders[i].limitMax - borders[i].limitMin) * m_PanelHeight, 1, EditorPreferences::Get()->UiForgroundColor * 0.5f);
				}
				if (!borders[i].bIsHorizontal && NativeInput::GetMousePosition().x - 3 < m_PanelX + borders[i].f * m_PanelWidth && NativeInput::GetMousePosition().x + 3 > m_PanelX + borders[i].f * m_PanelWidth)
				{
					if ((GetEditorWindow()->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y) < m_PanelY + borders[i].limitMin * m_PanelHeight || (GetEditorWindow()->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y) > m_PanelY + borders[i].limitMax * m_PanelHeight) continue;
					EditorUI::DrawRect(m_PanelX + borders[i].f * m_PanelWidth - 2, m_PanelY + borders[i].limitMin * m_PanelHeight, 3, (borders[i].limitMax - borders[i].limitMin) * m_PanelHeight, 1, EditorPreferences::Get()->UiHighlightColor * 0.5f);
					EditorUI::SetCursor(Cursor::HorizontalResize);
					if (NativeInput::IsMouseButtonPressed(Mouse::ButtonLeft))
					{
						m_DockspaceCurrentResize = i;
						GetEditorWindow()->m_InputEvent = EditorInputEvent::Dockspace_Resizing;
					}
				}
				//const float hh = 52 * EditorPreferences::Get()->UiScale; // HeaderHeight
				if (borders[i].bIsHorizontal)
				{
					EditorUI::DrawRect(m_PanelX + borders[i].limitMin * m_PanelWidth, m_PanelY + borders[i].f * m_PanelHeight - 2, (borders[i].limitMax - borders[i].limitMin) * m_PanelWidth, 3, 1, EditorPreferences::Get()->UiForgroundColor * 0.5f);
				}
				if (borders[i].bIsHorizontal && (GetEditorWindow()->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y) - 3 < m_PanelY + borders[i].f * m_PanelHeight && (GetEditorWindow()->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y) + 3 > m_PanelY + borders[i].f * m_PanelHeight)
				{
					if (NativeInput::GetMousePosition().x < m_PanelX + borders[i].limitMin * m_PanelWidth || NativeInput::GetMousePosition().x > m_PanelX + borders[i].limitMax * m_PanelWidth) continue;
					EditorUI::DrawRect(m_PanelX + borders[i].limitMin * m_PanelWidth, m_PanelY + borders[i].f * m_PanelHeight - 2, (borders[i].limitMax - borders[i].limitMin) * m_PanelWidth, 3, 1, EditorPreferences::Get()->UiHighlightColor * 0.5f);
					EditorUI::SetCursor(Cursor::VerticalResize);
					if (NativeInput::IsMouseButtonPressed(Mouse::ButtonLeft))
					{
						m_DockspaceCurrentResize = i;
						GetEditorWindow()->m_InputEvent = EditorInputEvent::Dockspace_Resizing;
					}
				}
			}
		}
		else if (GetEditorWindow()->m_InputEvent == EditorInputEvent::Dockspace_Resizing)
		{
			if (!NativeInput::IsMouseButtonPressed(Mouse::ButtonLeft)) GetEditorWindow()->m_InputEvent = EditorInputEvent::None;
			if (!borders[m_DockspaceCurrentResize].bIsHorizontal)
			{
				float pos = (NativeInput::GetMousePosition().x - m_PanelX) / m_PanelWidth;
				for (int i = 0; i < borders[m_DockspaceCurrentResize].From.Size(); i++) borders[m_DockspaceCurrentResize].From[i]->endX = pos;
				for (int i = 0; i < borders[m_DockspaceCurrentResize].To.Size(); i++) borders[m_DockspaceCurrentResize].To[i]->x = pos;
			}
			if (borders[m_DockspaceCurrentResize].bIsHorizontal)
			{
				//const float hh = 52 * EditorPreferences::Get()->UiScale; // HeaderHeight
				float pos = (((float)GetEditorWindow()->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y) - m_PanelY) / m_PanelHeight; // hh = HeaderHeight
				for (int i = 0; i < borders[m_DockspaceCurrentResize].From.Size(); i++) borders[m_DockspaceCurrentResize].From[i]->endY = pos;
				for (int i = 0; i < borders[m_DockspaceCurrentResize].To.Size(); i++) borders[m_DockspaceCurrentResize].To[i]->y = pos;
			}
		}
	}

	EditorWindow* DockspacePanel::GetEditorWindow() const
	{
		return m_Window;
	}

}