#include "Precompiled.h"
#include "SelectClassOverlay.h"

namespace Suora
{

	void SelectAnyClassOverlay::Render(float deltaTime)
	{
		EditorUI::DrawRect(0, 0, EditorUI::CurrentWindow->GetWindow()->GetWidth(), EditorUI::CurrentWindow->GetWindow()->GetHeight(), 0.0f, Color(1, 1, 1, 0.2f));
		DragableOverlay::Render(deltaTime);

		if (EditorUI::Button("Cancel", x + width - 140.0f, y + 15.0f, 125.0f, 30.0f))
		{
			Dispose();
			NativeInput::ConsumeInput();
			return;
		}
		if (EditorUI::Button("Select", x + width - 280.0f, y + 15.0f, 125.0f, 30.0f, EditorUI::ButtonParams::Highlight()))
		{
			SelectClass(m_SelectedClass);
			NativeInput::ConsumeInput();

			Dispose();
			return;
		}

		_Params.HideColor = EditorPreferences::Get()->UiColor;
		_Params.CurrentSelectedClass = m_SelectedClass;
		_Params.RootClass = m_RootClass;
		if (EditorUI::DrawSubclassHierarchyBox(x + 15.0f, y + 50.0f, 550, height - 100.0f, _Params))
		{
			m_SelectedClass = _Params.CurrentSelectedClass;
		}

	}

	void SelectAnyClassOverlay::SelectClass(const Class& cls)
	{
		m_Lambda(cls);
	}


}