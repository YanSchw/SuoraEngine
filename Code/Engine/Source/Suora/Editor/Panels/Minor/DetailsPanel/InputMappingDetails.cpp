#include "Precompiled.h"
#include "InputMappingDetails.h"

#include "Suora/Editor/Overlays/SelectionOverlay.h"
#include "Suora/GameFramework/InputModule.h"
#include "Suora/Assets/Texture2D.h"

namespace Suora
{

	static String InputActionTypeToString(InputActionType type)
	{
		switch (type)
		{
		case InputActionType::Action: return "Action (Button)";
		case InputActionType::Axis: return "Axis";
		case InputActionType::Axis2D: return "Axis2D";
		}
		return "unkown";
	}
	
	void InputMappingDetails::ViewObject(Object* obj, float& y)
	{
		if (obj->IsA<InputMapping>())
		{
			ViewInputMapping(y, obj->As<InputMapping>());
		}
	}

	void InputMappingDetails::ViewInputMapping(float& y, InputMapping* input)
	{
		static std::unordered_map<InputActionType, Array<Class>> s_InputBindables;

		/** Init all InputBinding Classes */
		if (s_InputBindables.empty())
		{
			Array<Class> classes = Class::GetSubclassesOf(InputBinding::StaticClass());
			for (const Class& cls : classes)
			{
				InputBinding* binding = New(cls)->As<InputBinding>();
				SuoraAssert(binding);
				s_InputBindables[binding->m_TargetActionType].Add(cls);
				delete binding;
			}
		}
		y -= 34.0f;
		DrawLabel("Label", y, 35.0f);
		EditorUI::TextField(&input->m_Label, GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, 350.0f, 25.0f);

		y -= 34.0f;
		DrawLabel("Type", y, 35.0f);
		const float buttonWidth = (GetDetailWidth() - GetDetailWidth() * GetSeperator()) <= 150.0f ? (GetDetailWidth() - GetDetailWidth() * GetSeperator()) : 150.0f;
		if (EditorUI::Button(InputActionTypeToString(input->m_ActionType), GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, buttonWidth, 25.0f))
		{
			EditorUI::CreateContextMenu(
				{ EditorUI::ContextMenuElement({}, [input]()
							{
								if (input->m_ActionType != InputActionType::Action)
								{
									input->m_Bindings.Clear();
								}
								input->m_ActionType = InputActionType::Action;
							}, "Action (Button)", nullptr), EditorUI::ContextMenuElement({}, [input]()
							{
								if (input->m_ActionType != InputActionType::Axis)
								{
									input->m_Bindings.Clear();
								}
								input->m_ActionType = InputActionType::Axis;
							}, "Axis", nullptr), EditorUI::ContextMenuElement({}, [input]()
							{
								if (input->m_ActionType != InputActionType::Axis2D)
								{
									input->m_Bindings.Clear();
								}
								input->m_ActionType = InputActionType::Axis2D;
							}, "Axis2D", nullptr)
				});
		}

		y -= 34.0f;
		DrawLabel("", y, 35.0f);
		if (EditorUI::Button("Add Binding", GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, buttonWidth, 25.0f))
		{
			std::vector<EditorUI::ContextMenuElement> context;
			const float width = GetDetailWidth();

			for (auto& It : s_InputBindables[input->m_ActionType])
			{
				context.push_back(EditorUI::ContextMenuElement({}, [input, It]()
					{
						input->m_Bindings.Add(Ref<InputBinding>(New(It)->As<InputBinding>()));
					}, "Add " + It.GetClassName(), nullptr));
			}

			EditorUI::CreateContextMenu(context);
		}

		int32_t idx = 0;
		for (Ref<InputBinding> binding : input->m_Bindings)
		{
			y -= 34.0f;
			if (!EditorUI::CategoryShutter(idx, "Binding " + std::to_string(idx++), 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
			{
				continue;
			}

			y -= 34.0f;
			DrawLabel("", y, 35.0f);
			if (EditorUI::Button("Remove Binding", GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, buttonWidth, 25.0f))
			{
				input->m_Bindings.Remove(binding);
				return;
			}

			if (binding->GetClass() == InputBinding_DirectAction::StaticClass())
			{
				y -= 34.0f;
				DrawInputDispatcherDropDown("", binding->As<InputBinding_DirectAction>()->m_Dispatcher, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y, GetDetailWidth() * 0.15f, 35.0f);
			}
			else if (binding->GetClass() == InputBinding_DirectAxis::StaticClass())
			{
				y -= 34.0f;
				DrawInputDispatcherDropDown("", binding->As<InputBinding_DirectAxis>()->m_Dispatcher, InputActionType::Axis, GetDetailWidth() * 0.75f + 7.0f, y, GetDetailWidth() * 0.15f, 35.0f);
			}
			else if (binding->GetClass() == InputBinding_DirectAxis2D::StaticClass())
			{
				y -= 34.0f;
				DrawInputDispatcherDropDown("", binding->As<InputBinding_DirectAxis2D>()->m_Dispatcher, InputActionType::Axis2D, GetDetailWidth() * 0.75f + 7.0f, y, GetDetailWidth() * 0.15f, 35.0f);
			}
			else if (binding->GetClass() == InputBinding_DirectionalActionMapping::StaticClass())
			{
				y -= 34.0f;
				DrawInputDispatcherDropDown("Bind Positive", binding->As<InputBinding_DirectionalActionMapping>()->m_DispatcherPos, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y, GetDetailWidth() * 0.15f, 35.0f);
				y -= 34.0f;
				DrawInputDispatcherDropDown("Bind Negative", binding->As<InputBinding_DirectionalActionMapping>()->m_DispatcherNeg, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y, GetDetailWidth() * 0.15f, 35.0f);
			}
			else if (binding->GetClass() == InputBinding_BidirectionalActionMapping::StaticClass())
			{
				y -= 34.0f;
				DrawInputDispatcherDropDown("Bind Up", binding->As<InputBinding_BidirectionalActionMapping>()->m_DispatcherUp, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y, GetDetailWidth() * 0.15f, 35.0f);
				y -= 34.0f;
				DrawInputDispatcherDropDown("Bind Down", binding->As<InputBinding_BidirectionalActionMapping>()->m_DispatcherDown, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y, GetDetailWidth() * 0.15f, 35.0f);
				y -= 34.0f;
				DrawInputDispatcherDropDown("Bind Left", binding->As<InputBinding_BidirectionalActionMapping>()->m_DispatcherLeft, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y, GetDetailWidth() * 0.15f, 35.0f);
				y -= 34.0f;
				DrawInputDispatcherDropDown("Bind Right", binding->As<InputBinding_BidirectionalActionMapping>()->m_DispatcherRight, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y, GetDetailWidth() * 0.15f, 35.0f);
			}
			else
			{
				SuoraVerify(false);
			}
		}

	}

	void InputMappingDetails::DrawInputDispatcherDropDown(const String& label, Ref<InputDispatcher>& dispatcher, InputActionType type, float x, float y, float width, float height)
	{
		DrawLabel(label, y, height);

		EditorUI::ButtonParams params;
		params.ButtonColor = EditorPreferences::Get()->UiInputColor;
		params.ButtonColorHover = EditorPreferences::Get()->UiInputColor;
		if (EditorUI::Button(dispatcher ? dispatcher->m_Label : "None", x, y + 5.0f, width, height - 10.0f, params))
		{
			SelectionOverlay* overlay = EditorUI::CreateOverlay<SelectionOverlay>(x + EditorUI::GetInputOffset().x - 300, y + EditorUI::GetInputOffset().y - 300, 400, 400);

			for (auto& It : PlayerInputNode::s_RegisteredInputDispatchers)
			{
				if (It.second->m_ActionSpecifier == type)
				{
					overlay->m_Entries.Add(SelectionOverlay::SelectionOverlayEntry(It.first, {}, [&]() { dispatcher = It.second; }));
				}
			}
			overlay->RefreshEntries();
		}
		EditorUI::DrawTexturedRect(AssetManager::GetAsset<Texture2D>(SuoraID("8742cec8-9ee5-4645-b036-577146904b41"))->GetTexture(), x + width - height - 2.5f, y + 5.0f, height, height - 10.0f, 0.0f, Color(1.0f));

	}

}