#include "Precompiled.h"
#include "DetailsPanel.h"
#include "Suora/Assets/Texture2D.h"
#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/Editor/Panels/Major/MeshEditorPanel.h"

// Default Details
#include "NodeDetails.h"
#include "MaterialDetails.h"
#include "MeshDetails.h"
#include "Texture2DDetails.h"
#include "BlueprintDetails.h"
#include "ProjectDetails.h"
#include "EditorPreferencesDetails.h"
#include "InputMappingDetails.h"
#include "ExportProjectDetails.h"

namespace Suora
{

	static Texture2D* undoTexture = nullptr;

	static void DragFloat2(Vec2* vec, float x, float y, float width, float height)
	{
		float singleWidth = width / 2.0f;
		EditorUI::DragFloat(&vec->x, x + singleWidth * 0.0f, y, singleWidth, height);
		EditorUI::DragFloat(&vec->y, x + singleWidth * 1.0f, y, singleWidth, height);
	}
	static void DragFloat3(Vec3* vec, float x, float y, float width, float height)
	{
		float singleWidth = width / 3.0f;
		EditorUI::DragFloat(&vec->x, x + singleWidth * 0.0f, y, singleWidth, height);
		EditorUI::DragFloat(&vec->y, x + singleWidth * 1.0f, y, singleWidth, height);
		EditorUI::DragFloat(&vec->z, x + singleWidth * 2.0f, y, singleWidth, height);
	}

	EditorUI::ButtonParams DetailsPanel::ShutterPanelParams()
	{
		EditorUI::ButtonParams PanelButtonParam;
		PanelButtonParam.ButtonColor = EditorPreferences::Get()->UiForgroundColor;//Color(0.22f, 0.215f, 0.22f, 1.0f);
		PanelButtonParam.ButtonOutlineColor = EditorPreferences::Get()->UiBackgroundColor; //Color(0.13f, 0.13f, 0.131f, 1);
		PanelButtonParam.ButtonColorHover = Math::Lerp(EditorPreferences::Get()->UiForgroundColor, Color(1.0f), 0.05f);
		PanelButtonParam.ButtonColorClicked = EditorPreferences::Get()->UiForgroundColor;
		PanelButtonParam.ButtonRoundness = 0;
		PanelButtonParam.TextDropShadow = false;
		PanelButtonParam.TextOrientation = Vec2(-0.1f, 0);
		PanelButtonParam.TextSize = 24.0f;

		return PanelButtonParam;
	}

	bool DetailsPanel::DrawHeaderShutter(void* data, const String& label, float x, float& y, bool valueChanged, bool& reset)
	{
		y -= 34.0f;
		DrawLabel("", y, 35.0f);
		if (valueChanged) reset = DrawResetButton(y, 35.0f);
		return (EditorUI::CategoryShutter((int64_t)data, label, x, y, GetDetailWidth() - x - 35.0f, 35.0f, ShutterPanelParams()));
	}

	void DetailsPanel::DrawLabel(const String& label, float y, float height)
	{
		static EditorUI::ButtonParams params;
		params.ButtonRoundness = 0.0f;
		params.ButtonColorClicked = Color(0.0f);
		params.ButtonOutlineColor = EditorPreferences::Get()->UiForgroundColor;
		params.ButtonColor = EditorPreferences::Get()->UiColor;
		params.ButtonColorHover = EditorPreferences::Get()->UiForgroundColor;
		params.HoverCursor = Cursor::Default;

		EditorUI::Button("", 0.0f, y, GetDetailWidth(), height /*- 1.0f*/, params);
		//EditorUI::Text(label, Font::Instance, 5.0f + 2.0f, y - 2.0f, GetDetailWidth() * m_Seperator - 5.0f, height, 21.0f, Vec2(-1.0f, 0.0f), Color(0, 0, 0, 0.25f));
		EditorUI::Text(label, Font::Instance, 5.0f, y, GetDetailWidth() * m_Seperator - 5.0f, height, 22.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
		EditorUI::DrawRectOutline(0.0f, y, GetDetailWidth()              , height, 1.0f, EditorPreferences::Get()->UiBackgroundColor);
		EditorUI::DrawRectOutline(0.0f, y, GetDetailWidth() * m_Seperator, height, 1.0f, EditorPreferences::Get()->UiBackgroundColor);
		EditorUI::DrawRectOutline(GetDetailWidth() - m_ResetSeperator, y, m_ResetSeperator, height, 1.0f, EditorPreferences::Get()->UiBackgroundColor);
	}


	bool DetailsPanel::DrawButton(const String& label, const String& buttonLabel, float& y)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);

		return EditorUI::Button(buttonLabel, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, 150.0f, 25.0f);
	}

	void DetailsPanel::DrawDropDown(const String& label, const std::vector<std::pair<String, std::function<void(void)>>>& options, int index, float& y)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);

		EditorUI::DropDown(options, index, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, 150.0f, 25.0f);
	}

	bool DetailsPanel::DrawResetButton(float y, float height)
	{
		static EditorUI::ButtonParams params;
		params.ButtonRoundness = 0.0f;
		params.ButtonColorClicked = Color(0.0f);
		params.ButtonOutlineColor = EditorPreferences::Get()->UiForgroundColor;
		params.ButtonColor = EditorPreferences::Get()->UiColor;
		params.ButtonColorHover = EditorPreferences::Get()->UiForgroundColor;

		const bool reset = EditorUI::Button("", GetDetailWidth() - m_ResetSeperator + (m_ResetSeperator - 25.0f) / 2.0f, y + (height - 25.0f) / 2.0f, 25.0f, 25.0f, params);
		EditorUI::DrawTexturedRect(undoTexture->GetTexture(), GetDetailWidth() - m_ResetSeperator + (m_ResetSeperator - 25.0f) / 2.0f, y + (height - 25.0f) / 2.0f, 25.0f, 25.0f, 0.0f, Color(1.0f));

		return reset;
	}

	DetailsPanel::Result DetailsPanel::DrawInt32(int32_t* i, const String& label, float& y, bool valueChanged)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);

		const float dragIntWidth = (GetDetailWidth() - GetDetailWidth() * m_Seperator) <= 150.0f ? (GetDetailWidth() - GetDetailWidth() * m_Seperator) : 150.0f;
		
		EditorUI::DragInt32(i, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, dragIntWidth, 25.0f);

		if (valueChanged && DrawResetButton(y, 35.0f))
		{
			m_DrawInt32Values.Remove(i);
			return Result::ValueReset;
		}

		Result outResult = Result::None;
		if (m_DrawInt32Values.ContainsKey(i) && m_DrawInt32Values[i] != *i)
		{
			outResult = Result::ValueChange;
		}
		m_DrawInt32Values[i] = *i;

		return outResult;
	}
	DetailsPanel::Result DetailsPanel::DrawFloat(float* f, const String& label, float& y, bool valueChanged)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);

		const float dragFloatWidth = (GetDetailWidth() - GetDetailWidth() * m_Seperator) <= 150.0f ? (GetDetailWidth() - GetDetailWidth() * m_Seperator) : 150.0f;
		
		EditorUI::DragFloat(f, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, dragFloatWidth, 25.0f);

		if (valueChanged && DrawResetButton(y, 35.0f))
		{
			m_DrawFloatValues.Remove(f);
			return Result::ValueReset;
		}

		Result outResult = Result::None;
		if (m_DrawFloatValues.ContainsKey(f) && m_DrawFloatValues[f] != *f)
		{
			outResult = Result::ValueChange;
		}
		m_DrawFloatValues[f] = *f;

		return outResult;
	}
	DetailsPanel::Result DetailsPanel::DrawBool(bool* b, const String& label, float& y, bool valueChanged)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);
		const bool hasChanged = EditorUI::Checkbox(b, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, 25.0f, 25.0f);

		if (hasChanged)
		{
			return Result::ValueChange;
		}

		if (valueChanged && DrawResetButton(y, 35.0f))
		{
			return Result::ValueReset;
		}

		return Result::None;
	}
	DetailsPanel::Result DetailsPanel::DrawVec3(Vec3* v, const String& label, float& y, bool valueChanged)
	{
		Vec3 temp = *v;

		y -= 34.0f;
		DrawLabel(label, y, 35.0f);
		const float dragFloatWidth = (GetDetailWidth() - GetDetailWidth() * m_Seperator) <= 250.0f ? (GetDetailWidth() - GetDetailWidth() * m_Seperator) : 250.0f;
		DragFloat3(v, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, dragFloatWidth, 25.0f);

		if (valueChanged && DrawResetButton(y, 35.0f))
		{
			return Result::ValueReset;
		}

		return (temp != *v) ? Result::ValueChange : Result::None;
	}
	
	DetailsPanel::Result DetailsPanel::DrawVec4(Vec4* v, const String& label, float& y, bool valueChanged)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);
		const float dragFloatWidth = (GetDetailWidth() - GetDetailWidth() * m_Seperator) <= 150.0f ? (GetDetailWidth() - GetDetailWidth() * m_Seperator) : 150.0f;
		EditorUI::ColorPicker(v, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, dragFloatWidth, 25.0f, {}, [v, this]() { m_Vec4_ColorPickerResults[v] = 1; }, [v, this]() { m_Vec4_ColorPickerResults[v] = 2; });

		if (m_Vec4_ColorPickerResults.ContainsKey(v))
		{
			int32_t result = m_Vec4_ColorPickerResults[v];
			m_Vec4_ColorPickerResults.Remove(v);

			if (result == 1) return Result::ValueChange;
			else if (result == 2) return Result::ValueReset;
		}

		if (valueChanged && DrawResetButton(y, 35.0f))
		{
			return Result::ValueReset;
		}

		return Result::None;
	}
	DetailsPanel::Result DetailsPanel::DrawAsset(Asset** asset, const Class& cls, const String& label, float& y, bool valueChanged)
	{
		y -= 59.0f;
		DrawLabel(label, y, 60.0f);
		float width = GetDetailWidth() - GetDetailWidth() * m_Seperator - 25.0f - m_ResetSeperator;
		width = Math::Clamp(width, 0.0f, 300.0f);
		const bool hasChanged = EditorUI::AssetDropdown(asset, cls, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, width, 50.0f);
		
		if (hasChanged)
		{
			return Result::ValueChange;
		}

		if (valueChanged && DrawResetButton(y, 60.0f))
		{
			return Result::ValueReset;
		}

		return Result::None;
	}

	DetailsPanel::Result DetailsPanel::DrawMaterialSlots(MaterialSlots* materials, float& y, bool valueChanged)
	{
		if (!materials) return Result::None;
		materials->TickSlots();

		bool reset = false;
		if (DrawHeaderShutter(materials, "Materials", 5.0f, y, valueChanged, reset))
		{
			if (!materials->Owner) return Result::None;
			if (materials->Owner->IsA<MeshNode>())
			{
				Result result = DrawBool(&materials->OverwritteMaterials, "Overwritte Materials", y, false);
				if (result == Result::ValueChange) valueChanged = true;
			}
			if (materials->OverwritteMaterials || !materials->Owner->IsA<MeshNode>())
			{
				for (int i = 0; i < materials->Materials.Size(); i++)
				{
					Result result = DrawAsset((Asset**)&materials->Materials[i], Material::StaticClass(), "Slot " + std::to_string(i), y, false);
					if (result == Result::ValueChange) valueChanged = true;


					// Hover over MaterialSlot will display a MeshOutline in the MeshEditorPanel
					EditorUI::ButtonParams HoverParams = EditorUI::ButtonParams::Invisible();
					bool hover = false;
					HoverParams.OutHover = &hover;
					EditorUI::Button("", 0.0f, y, GetDetailWidth(), 60.0f , HoverParams);
					if (hover)
					{
						if (MeshEditorPanel* editor = GetMajorTab()->As<MeshEditorPanel>())
						{
							editor->m_MaterialSlotOutline = i;
						}
					}
				}
			}
		}

		return reset ? Result::ValueReset : (valueChanged ? Result::ValueChange : Result::None);
	}
	DetailsPanel::Result DetailsPanel::DrawClass(Class* cls, const String& label, float& y, bool valueChanged)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);
		const float classWidth = (GetDetailWidth() - GetDetailWidth() * m_Seperator) <= 150.0f ? (GetDetailWidth() - GetDetailWidth() * m_Seperator) : 150.0f;

		if (EditorUI::Button(cls->GetClassName(), GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, classWidth, 25.0f))
		{
			EditorUI::SubclassSelectionMenu(Object::StaticClass(), [cls](Class selection) { *cls = selection; });
		}

		if (valueChanged && DrawResetButton(y, 35.0f))
		{
			return Result::ValueReset;
		}

		return Result::None;
	}
	DetailsPanel::Result DetailsPanel::DrawSubclassOf(TSubclassOf* cls, const String& label, float& y, bool valueChanged)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);
		const float classWidth = (GetDetailWidth() - GetDetailWidth() * m_Seperator) <= 150.0f ? (GetDetailWidth() - GetDetailWidth() * m_Seperator) : 150.0f;

		if (EditorUI::Button(cls->GetClass().GetClassName(), GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, classWidth, 25.0f))
		{
			EditorUI::SubclassSelectionMenu(cls->GetBase(), [cls](Class selection) { cls->SetClass(selection); });
		}

		if (valueChanged && DrawResetButton(y, 35.0f))
		{
			return Result::ValueReset;
		}

		return Result::None;
	}

	DetailsPanel::Result DetailsPanel::DrawDelegate(TDelegate* delegate, const String& label, float& y)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);

		if (EditorUI::Button("+Event", GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, 150.0f, 25.0f))
		{
			SuoraVerify(false);
		}

		return Result::None;
	}

	void DetailsPanel::DrawVec3Control(Vec3* vec, float x, float y, float width, float height, float defaultVecResetValue, const std::function<void(String)>& lambda)
	{
		EditorUI::ButtonParams params;
		params.TextColor = Vec4(0, 0, 0, 1);
		const float singleWidth = width / 3.0f;
		
		params.ButtonColor = Vec4(0.82353f, 0.301961f, 0.3764706f, 1.0f); params.ButtonColorHover = Vec4(0.92353f, 0.401961f, 0.4764706f, 1.0f);
		if (EditorUI::Button("X", x + singleWidth * 0.0f, y, height, height, params)) vec->x = defaultVecResetValue;
		EditorUI::DragFloat(&vec->x, x + singleWidth * 0.0f + height, y, singleWidth - height, height, lambda);

		params.ButtonColor = Vec4(0.52549f, 0.803922f, 0.05098f, 1.0f); params.ButtonColorHover = Vec4(0.62549f, 0.903922f, 0.15098f, 1.0f);
		if (EditorUI::Button("Y", x + singleWidth * 1.0f, y, height, height, params)) vec->y = defaultVecResetValue;
		EditorUI::DragFloat(&vec->y, x + singleWidth * 1.0f + height, y, singleWidth - height, height, lambda);

		params.ButtonColor = Vec4(0.286275f, 0.537255f, 0.796078f, 1.0f); params.ButtonColorHover = Vec4(0.386275f, 0.637255f, 0.896078f, 1.0f);
		if (EditorUI::Button("Z", x + singleWidth * 2.0f, y, height, height, params)) vec->z = defaultVecResetValue;
		EditorUI::DragFloat(&vec->z, x + singleWidth * 2.0f + height, y, singleWidth - height, height, lambda);
	}
	//static std::unordered_map<Quat*, Vec3> _DrawQuatCache;
	//static void DrawQuatControl(Quat* rot, float x, float y, float width, float height, float defaultVecResetValue = 0)
	//{
	//	//Vec3 vec = glm::degrees(glm::eulerAngles(*rot));
	//	if (_DrawQuatCache.find(rot) == _DrawQuatCache.end()) _DrawQuatCache[rot] = glm::degrees(glm::eulerAngles(*rot));
	//	
	//	DrawVec3Control(&_DrawQuatCache[rot], x, y, width, height, defaultVecResetValue);
	//	*rot = Quat(glm::radians(_DrawQuatCache[rot]));
	//}


	DetailsPanel::DetailsPanel(MajorTab* majorTab)
		: MinorTab(majorTab)
	{
		Name = "Details";
		undoTexture = AssetManager::GetAssetByName<Texture2D>("Undo.texture");

		Array<Class> implementations = Class::GetSubclassesOf(DetailsPanelImplementation::StaticClass());
		for (const auto& It : implementations)
		{
			auto Impl = Ref<DetailsPanelImplementation>(New(It)->As<DetailsPanelImplementation>());
			if (Impl)
			{
				Impl->SetDetailsPanel(this);
				m_Implementations.Add(Impl);
			}
		}

		m_Implementations.Sort([](const Ref<DetailsPanelImplementation>& a, const Ref<DetailsPanelImplementation>& b)
		{
			return a->GetOrderIndex() < b->GetOrderIndex();
		});
	}

	DetailsPanel::~DetailsPanel()
	{

	}

	int DetailsPanel::GetDetailWidth() const
	{
		return Super::GetWidth() - GetScrollbarWidth();
	}

	void DetailsPanel::Render(float deltaTime)
	{
		EditorUI::DrawRect(0, 0, GetWidth(), GetHeight(), 0, EditorPreferences::Get()->UiColor);
		EditorUI::DrawRect(4, 4, GetWidth() * m_Seperator - 4, GetHeight() - 4, 4, EditorPreferences::Get()->UiColor * 0.95f);

		if (IsInputValid())
		{
			if (IsInputMode(EditorInputEvent::None))
			{
				if ((EditorUI::GetInput().x - 2) / (float)GetWidth() <= m_Seperator && (EditorUI::GetInput().x + 2) / (float)GetWidth() >= m_Seperator && NativeInput::IsMouseButtonPressed(Mouse::ButtonLeft))
					SetInputMode(EditorInputEvent::DetailsPanel_MoveSperator);
			}
			else if (IsInputMode(EditorInputEvent::DetailsPanel_MoveSperator))
			{
				m_Seperator = Math::Clamp(EditorUI::GetInput().x / (float)GetWidth(), 0.1f, 0.9f);
				if (!NativeInput::IsMouseButtonPressed(Mouse::ButtonLeft)) SetInputMode(EditorInputEvent::None);
			}
		}

		float y = GetHeight() + m_ScrollY;

		if (m_Data)
		{
			for (Ref<DetailsPanelImplementation> It : m_Implementations)
			{
				if (It)
				{
					It->ViewObject(m_Data, y);
				}
			}
		}

		EditorUI::DrawRect((m_Seperator * GetWidth())-1, 0, 2, GetHeight(), 0, Color(0.2f, 0.2f, 0.2f, 0.1f));

		float scrollDown = y - m_ScrollY;
		m_ScrollbarNeeded = scrollDown <= 0;
		EditorUI::ScrollbarVertical(GetWidth() - 10, 0, 10, GetHeight(), 0, 0, GetWidth(), GetHeight(), 0, scrollDown > 0 ? 0 : Math::Abs(scrollDown), &m_ScrollY);
	}

	float DetailsPanel::GetScrollbarWidth() const
	{
		return m_ScrollbarNeeded ? 10.0f : 0.0f;
	}

}