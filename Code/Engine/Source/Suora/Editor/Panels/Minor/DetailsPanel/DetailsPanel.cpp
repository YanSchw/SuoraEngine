#include "Precompiled.h"
#include "DetailsPanel.h"
#include "Suora/Editor/EditorWindow.h"
#include "Suora/Editor/Panels/Major/NodeClassEditor.h"
#include "Suora/Editor/Util/EditorPreferences.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/Assets/SuoraProject.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/InputModule.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/Filesystem.h"
#include "Suora/Editor/Panels/Major/MeshEditorPanel.h"

// Default Details
#include "NodeDetails.h"
#include "MaterialDetails.h"
#include "MeshDetails.h"
#include "Texture2DDetails.h"
#include "BlueprintDetails.h"
#include "ProjectDetails.h"

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

			Class detailClass = m_Data->GetNativeClass();
			if      (detailClass.Inherits(Node::StaticClass())) { /*ViewNode(y, m_Data->As<Node>());*/ }
			else if (detailClass == Material::StaticClass()) { /*ViewMaterial(y, m_Data->As<Material>(), false);*/ }
			else if (detailClass == Mesh::StaticClass()) { /*ViewMesh(y, m_Data->As<Mesh>());*/ }
			else if (detailClass == ShaderGraph::StaticClass()) { /*ViewMaterial(y, m_Data->As<Material>(), true);*/ }
			else if (detailClass == Texture2D::StaticClass()) { /*ViewTexture2D(y, m_Data->As<Texture2D>());*/ }
			else if (detailClass.Inherits(Blueprint::StaticClass())) { /*ViewBlueprintClass(y, m_Data->As<Blueprint>());*/ }
			else if (detailClass == ProjectSettings::StaticClass()) { /*ViewProjectSettings(y, m_Data->As<ProjectSettings>());*/ }
			else if (detailClass == EditorPreferences::StaticClass()) { ViewEditorPreferences(y, m_Data->As<EditorPreferences>()); }
			else if (detailClass == InputMapping::StaticClass()) { ViewInputMapping(y, m_Data->As<InputMapping>()); }
			else
			{
				SuoraError("No implementation for DetailPanel -> Class: {0}", detailClass.GetClassName());
			}
		}

		EditorUI::DrawRect((m_Seperator * GetWidth())-1, 0, 2, GetHeight(), 0, Color(0.2f, 0.2f, 0.2f, 0.1f));

		float scrollDown = y - m_ScrollY;
		m_ScrollbarNeeded = scrollDown <= 0;
		EditorUI::ScrollbarVertical(GetWidth() - 10, 0, 10, GetHeight(), 0, 0, GetWidth(), GetHeight(), 0, scrollDown > 0 ? 0 : Math::Abs(scrollDown), &m_ScrollY);
	}

	void DetailsPanel::DrawClassMember(float& x, float& y, Node* obj, ClassMemberProperty* member, int memberIndex)
	{
		//y -= 10.0f;
		const auto type = member->m_Property->GetType();
		const auto mname = member->m_MemberName;
		const bool valueChangedBefore = obj->m_OverwrittenProperties.Contains(mname);
		Result result = Result::None;

		if (type == PropertyType::Int32)
		{
			int32_t* i = ClassMemberProperty::AccessMember<int32_t>(obj, member->m_MemberOffset);
			result = DrawInt32(i, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::Float)
		{
			float* f = ClassMemberProperty::AccessMember<float>(obj, member->m_MemberOffset);
			result = DrawFloat(f, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::Bool)
		{
			bool* b = ClassMemberProperty::AccessMember<bool>(obj, member->m_MemberOffset);
			result = DrawBool(b, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::Vec3)
		{
			Vec3* v = ClassMemberProperty::AccessMember<Vec3>(obj, member->m_MemberOffset);
			result = DrawVec3(v, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::Vec4)
		{
			Vec4* v = ClassMemberProperty::AccessMember<Vec4>(obj, member->m_MemberOffset);
			result = DrawVec4(v, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::ObjectPtr)
		{
			const Class objClass = ((ObjectPtrProperty*)(member->m_Property.get()))->m_ObjectClass;
			if (objClass.Inherits(Asset::StaticClass()))
			{
				Asset** asset = ClassMemberProperty::AccessMember<Asset*>(obj, member->m_MemberOffset);
				result = DrawAsset(asset, objClass, mname, y, valueChangedBefore);
			}
		}
		else if (type == PropertyType::MaterialSlots)
		{
			MaterialSlots* materials = ClassMemberProperty::AccessMember<MaterialSlots>(obj, member->m_MemberOffset);
			result = DrawMaterialSlots(materials, y, valueChangedBefore);
		}
		else if (type == PropertyType::Class)
		{
			Class* cls = ClassMemberProperty::AccessMember<Class>(obj, member->m_MemberOffset);
			result = DrawClass(cls, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::SubclassOf)
		{
			TSubclassOf* cls = ClassMemberProperty::AccessMember<TSubclassOf>(obj, member->m_MemberOffset);
			result = DrawSubclassOf(cls, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::Delegate)
		{
			TDelegate* delegate = ClassMemberProperty::AccessMember<TDelegate>(obj, member->m_MemberOffset);
			result = DrawDelegate(delegate, mname, y);
		}
		else
		{
			SuoraError("{0}: Missing ClassMember implementation!", __FUNCTION__);
		}

		if (result == Result::ValueReset)
		{
			obj->ResetProperty(*member);
		}
		else if (result == Result::ValueChange)
		{
			if (!obj->m_OverwrittenProperties.Contains(mname))
			{
				obj->m_OverwrittenProperties.Add(mname);
			}
		}
	}

	


	void DetailsPanel::DrawInputDispatcherDropDown(const String& label, Ref<InputDispatcher>& dispatcher, InputActionType type, float x, float y, float width, float height)
	{
		DrawLabel(label, y, height);

		EditorUI::ButtonParams params;
		params.ButtonColor = EditorPreferences::Get()->UiInputColor;
		params.ButtonColorHover = EditorPreferences::Get()->UiInputColor;
		if (EditorUI::Button(dispatcher ? dispatcher->m_Label : "None", x, y + 5.0f, width, height - 10.0f, params))
		{
			EditorUI::SelectionOverlay* overlay = EditorUI::CreateOverlay<EditorUI::SelectionOverlay>(x + EditorUI::GetInputOffset().x - 300, y + EditorUI::GetInputOffset().y - 300, 400, 400);

			for (auto& It : PlayerInputNode::s_RegisteredInputDispatchers)
			{
				if (It.second->m_ActionSpecifier == type)
				{
					overlay->m_Entries.Add(EditorUI::SelectionOverlay::SelectionOverlayEntry(It.first, {}, [&]() { dispatcher = It.second; }));
				}
			}
			overlay->RefreshEntries();
		}
		EditorUI::DrawTexturedRect(AssetManager::GetAsset<Texture2D>(SuoraID("8742cec8-9ee5-4645-b036-577146904b41"))->GetTexture(), x + width - height - 2.5f, y + 5.0f, height, height - 10.0f, 0.0f, Color(1.0f));

	}

	

	void DetailsPanel::ViewEditorPreferences(float& y, EditorPreferences* settings)
	{
		y -= 35.0f;
		if (EditorUI::CategoryShutter(0, "Theme", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			DrawAsset((Asset**)&Font::Instance, Font::StaticClass(), "Editor Font", y, false);
			DrawVec4(&settings->UiHighlightColor, "HighlightColor", y, false);
			DrawVec4(&settings->UiColor, "Main Color", y, false);
			DrawVec4(&settings->UiBackgroundColor, "Background Color", y, false);
			DrawVec4(&settings->UiForgroundColor, "Foreground Color", y, false);
		}
		DrawFloat(&settings->UiScale, "UiScale", y, false);
	}

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
	void DetailsPanel::ViewInputMapping(float& y, InputMapping* input)
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
		EditorUI::TextField(&input->m_Label, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, 350.0f, 25.0f);

		y -= 34.0f;
		DrawLabel("Type", y, 35.0f);
		const float buttonWidth = (GetDetailWidth() - GetDetailWidth() * m_Seperator) <= 150.0f ? (GetDetailWidth() - GetDetailWidth() * m_Seperator) : 150.0f;
		if (EditorUI::Button(InputActionTypeToString(input->m_ActionType), GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, buttonWidth, 25.0f))
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
		if (EditorUI::Button("Add Binding", GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, buttonWidth, 25.0f))
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
			if (EditorUI::Button("Remove Binding", GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, buttonWidth, 25.0f))
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

	float DetailsPanel::GetScrollbarWidth() const
	{
		return m_ScrollbarNeeded ? 10.0f : 0.0f;
	}

}