#include "DetailsPanel.h"
#include "Editor/EditorWindow.h"
#include "Editor/Panels/Major/NodeClassEditor.h"
#include "Editor/Util/EditorPreferences.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/Assets/SuoraProject.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/InputModule.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/Filesystem.h"
#include "Editor/Panels/Major/MeshEditorPanel.h"

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

	static EditorUI::ButtonParams ShutterPanelParams()
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

	static void DrawVec3Control(Vec3* vec, float x, float y, float width, float height, float defaultVecResetValue = 0, const std::function<void(String)>& lambda = nullptr)
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
	static std::unordered_map<Quat*, Vec3> _DrawQuatCache;
	static void DrawQuatControl(Quat* rot, float x, float y, float width, float height, float defaultVecResetValue = 0)
	{
		//Vec3 vec = glm::degrees(glm::eulerAngles(*rot));
		if (_DrawQuatCache.find(rot) == _DrawQuatCache.end()) _DrawQuatCache[rot] = glm::degrees(glm::eulerAngles(*rot));
		
		DrawVec3Control(&_DrawQuatCache[rot], x, y, width, height, defaultVecResetValue);
		*rot = Quat(glm::radians(_DrawQuatCache[rot]));
	}


	DetailsPanel::DetailsPanel(MajorTab* majorTab)
		: MinorTab(majorTab)
	{
		Name = "Details";
		undoTexture = AssetManager::GetAssetByName<Texture2D>("Undo.texture");
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
			Class detailClass = m_Data->GetNativeClass();
			if      (detailClass.Inherits(Node::StaticClass())) { ViewNode(y, m_Data->As<Node>()); }
			else if (detailClass == Material::StaticClass()) { ViewMaterial(y, m_Data->As<Material>(), false); }
			else if (detailClass == Mesh::StaticClass()) { ViewMesh(y, m_Data->As<Mesh>()); }
			else if (detailClass == ShaderGraph::StaticClass()) { ViewMaterial(y, m_Data->As<Material>(), true); }
			else if (detailClass == Texture2D::StaticClass()) { ViewTexture2D(y, m_Data->As<Texture2D>()); }
			else if (detailClass.Inherits(Blueprint::StaticClass())) { ViewBlueprintClass(y, m_Data->As<Blueprint>()); }
			else if (detailClass == ProjectSettings::StaticClass()) { ViewProjectSettings(y, m_Data->As<ProjectSettings>()); }
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

	void DetailsPanel::ViewNode(float& y, Node* node)
	{

		if (!node)
		{
			EditorUI::Text("Select a Node to view its Details.", Font::Instance, 0, 0, GetWidth(), GetHeight(), 36, Vec2(0, 0), Color(1));
			return;
		}

		bool NodeEnabled = node->m_Enabled;

		y -= 40;
		EditorUI::Button("", 0.0f, y, 40.0f, 40.0f, ShutterPanelParams());
		EditorUI::DrawTexturedRect(EditorUI::GetClassIcon(node->GetClass())->GetTexture(), 5.0f, y + 5.0f, 30.0f, 30.0f, 0.0f, Color(1));
		EditorUI::Button("", 40.0f, y, GetDetailWidth() - 80.0f, 40.0f, ShutterPanelParams());
		EditorUI::TextField(&node->m_Name, 45.0f, y + 7.5f, (GetDetailWidth() - 40.0f) / 2.0f, 25.0f);
		node->SetName(node->GetName());
		EditorUI::Button("", GetDetailWidth() - 40.0f, y, 40.0f, 40.0f, ShutterPanelParams());
		EditorUI::Checkbox(&node->m_Enabled, GetDetailWidth() - 40.0f + 7.5f, y + 7.5f, 25.0f, 25.0f);
		y -= 10;
		EditorUI::Text("=======================================================================", Font::Instance, 0.0f, y + 5.0f, GetDetailWidth(), 5.0f, 18.0f, Vec2(0.0f), Color(0.65f));

		if (NodeEnabled != node->m_Enabled)
		{
			node->SetEnabled(node->m_Enabled);
		}

		bool skipFirstDerivative = false;
		if (Node3D* node3D = node->As<Node3D>())
		{
			skipFirstDerivative = true;
			y -= 35;
			static bool readTransform = true;
			static std::function<void(String)> disableReadTransform = [](String s) { readTransform = false; };
			if (readTransform) // Doesn't work with writing values in the TextBox!
			{
				Transform_Pos = node3D->GetLocalPosition();
				Transform_Rot = node3D->GetEulerRotation();
				Transform_Scale = node3D->GetLocalScale();
				if (Transform_Rot.x == -0.0f || Transform_Rot.x == 0.0f) Transform_Rot.x = 0.0f; if (Transform_Rot.x == -180.0f) Transform_Rot.x = 180.0f;
				if (Transform_Rot.y == -0.0f || Transform_Rot.y == 0.0f) Transform_Rot.y = 0.0f; if (Transform_Rot.y == -180.0f) Transform_Rot.y = 180.0f;
				if (Transform_Rot.z == -0.0f || Transform_Rot.z == 0.0f) Transform_Rot.z = 0.0f; if (Transform_Rot.z == -180.0f) Transform_Rot.z = 180.0f;
			}
			readTransform = true;
			if (EditorUI::CategoryShutter(0, "Node3D", 0, y, GetDetailWidth(), 35, ShutterPanelParams()))
			{
				y -= 34;
				DrawLabel("Position", y, 35.0f);
				//static Vec3 position;
				DrawVec3Control(&Transform_Pos, GetDetailWidth() * m_Seperator, y + 4.5f, GetDetailWidth() - GetDetailWidth() * m_Seperator - 35.0f, 25, 0.0f, disableReadTransform);
				node3D->SetLocalPosition(Transform_Pos);
				y -= 34;
				DrawLabel("Rotation", y, 35.0f);
				if (!GetMajorTab()->IsA<NodeClassEditor>()
					|| (GetMajorTab()->IsA<NodeClassEditor>() && GetMajorTab()->As<NodeClassEditor>()->m_CurrentPlayState == PlayState::Editor)
					|| true)
				{
					//static Vec3 rot;
					DrawVec3Control(&Transform_Rot, GetDetailWidth() * m_Seperator, y + 4.5f, GetDetailWidth() - GetDetailWidth() * m_Seperator - 35.0f, 25, 0.0f, disableReadTransform);
					node3D->SetEulerRotation(Transform_Rot);
				}
				y -= 34;
				DrawLabel("Scale", y, 35.0f);
				//static Vec3 scale;
				DrawVec3Control(&Transform_Scale, GetDetailWidth() * m_Seperator, y + 4.5f, GetDetailWidth() - GetDetailWidth() * m_Seperator - 35.0f, 25, 1.0f, disableReadTransform);
				node3D->SetLocalScale(Transform_Scale);
				y -= 15; // Padding Bottom
				node3D->TickTransform(true);

				Transform_LastNode = node;
			}
		}
		if (UINode* uinode = node->As<UINode>())
		{
			skipFirstDerivative = true;
			y -= 35;
			if (EditorUI::CategoryShutter(0, "UINode", 0, y, GetDetailWidth(), 35, ShutterPanelParams()))
			{
				y -= 34;
				DrawLabel("Anchor", y, 35.0f);
				DragFloat2(&uinode->m_Anchor, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - m_Seperator), 25.0f);
				y -= 34;
				DrawLabel("Width", y, 35.0f);
				EditorUI::Checkbox(&uinode->m_IsWidthRelative, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, 25.0f, 25.0f);
				EditorUI::Text("Is Relative", Font::Instance, GetDetailWidth() * m_Seperator + 35.0f, y + 5.0f, 250.0f, 25.0f, 21.0f, Vec2(-1, 0), Color(1.0f));
				y -= 34;
				DrawLabel("", y, 35.0f);
				EditorUI::DragFloat(&uinode->m_Width, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - m_Seperator), 25.0f);
				y -= 34;
				DrawLabel("Height", y, 35.0f);
				EditorUI::Checkbox(&uinode->m_IsHeightRelative, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, 25.0f, 25.0f);
				EditorUI::Text("Is Relative", Font::Instance, GetDetailWidth() * m_Seperator + 35.0f, y + 5.0f, 250.0f, 25.0f, 21.0f, Vec2(-1, 0), Color(1.0f));
				y -= 34;
				DrawLabel("", y, 35.0f);
				EditorUI::DragFloat(&uinode->m_Height, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - m_Seperator), 25.0f);
				y -= 34;
				DrawLabel("Pivot", y, 35.0f);
				DragFloat2(&uinode->m_Pivot, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - m_Seperator), 25.0f);
				y -= 34;
				DrawLabel("Absolute Pixel Offset", y, 35.0f);
				DragFloat3(&uinode->m_AbsolutePixelOffset, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - m_Seperator), 25.0f);
				y -= 34;
				DrawLabel("Euler Rotation around Anchor", y, 35.0f);
				DragFloat3(&uinode->m_EulerRotationAroundAnchor, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - m_Seperator), 25.0f);
			}
		}

		// Node Derivates
		Array<Class> derivates = node->GetClass().GetInheritanceTree();
		int memberIndex = 0;
		for (int64_t i = skipFirstDerivative ? 3 : 2; i < derivates.Size(); i++)
		{
			y -= 35;
			const ClassReflector& refl = ClassReflector::GetByClass(derivates[i]);
			if (EditorUI::CategoryShutter(1000 + i, refl.m_ClassName, 0, y, GetDetailWidth(), 35, ShutterPanelParams()))
			{
				for (Ref<ClassMemberProperty> member : refl.m_ClassProperties)
				{
					float x = 0;
					DrawClassMember(x, y, node, member.get(), memberIndex++);
				}

			}
			else
			{
				memberIndex += refl.m_ClassProperties.Size();
			}
		}

		y -= 50;
	}

	void DetailsPanel::ViewMaterial(float& y, Material* material, bool isShaderGraph)
	{
		if (!isShaderGraph)
		{
			y -= 36.0f;
			if (EditorUI::CategoryShutter(0, "ShaderGraph", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
			{
				const ShaderGraph* temp = material->m_ShaderGraph;
				DrawAsset((Asset**)&material->m_ShaderGraph, ShaderGraph::StaticClass(), "ShaderGraph", y, false);

				if (temp != material->m_ShaderGraph && material->m_ShaderGraph)
				{
					material->m_UniformSlots = material->m_ShaderGraph->m_UniformSlots;
				}
				y -= 20;
			}
		}
		else
		{
			y -= 36.0f;
			if (EditorUI::CategoryShutter(0, "Shader", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
			{
				int i = 0;
				std::vector<std::pair<String, std::function<void(void)>>> options;
				std::filesystem::path directory = AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/ShadergraphBase/";
				std::vector<DirectoryEntry> entries = File::GetAllAbsoluteEntriesOfPath(directory);
				for (int j = 0; j < entries.size(); j++)
				{
					const String name = entries[j].path().filename().string();
					if (name == material->As<ShaderGraph>()->m_BaseShader) i = j;
					options.push_back({ name, [name, material]() { material->As<ShaderGraph>()->m_BaseShader = name; } });
				}

				DrawDropDown("Shader", options, i, y);
			}
		}

		y -= 35.0f;
		if (EditorUI::CategoryShutter(1, "Uniforms", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			for (UniformSlot& slot : material->m_UniformSlots)
			{
				if (slot.m_Type == ShaderGraphDataType::Float)
				{
					DrawFloat(&slot.m_Float, slot.m_Label, y, false);
				}
				else if (slot.m_Type == ShaderGraphDataType::Vec3)
				{
					DrawVec3(&slot.m_Vec3, slot.m_Label, y, false);
				}
				else if (slot.m_Type == ShaderGraphDataType::Texture2D)
				{
					DrawAsset((Asset**)&slot.m_Texture2D, Texture2D::StaticClass(), slot.m_Label, y, false);
				}
				else
				{
					SuoraError("Unkown Type!");
				}
			}
			y -= 20;
		}

		y -= 35.0f;
		if (EditorUI::CategoryShutter(2, "Material", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			DrawBool(&material->m_BackfaceCulling, "BackfaceCulling", y, false);
			y -= 20;
		}
	}

	void DetailsPanel::ViewMesh(float& y, Mesh* mesh)
	{
		y -= 35.0f;
		if (EditorUI::CategoryShutter(0, "Mesh", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			DrawVec3(&mesh->m_ImportScale, "Import Scale", y, false);
		}
		y -= 35.0f;
		if (EditorUI::CategoryShutter(2, "Materials", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			DrawMaterialSlots(&mesh->m_Materials, y, false);
		}
		y -= 35.0f;
		if (EditorUI::CategoryShutter(3, "Decima", 0, y, GetDetailWidth(), 35.0f, ShutterPanelParams()))
		{
			DrawBool(&mesh->m_IsDecimaMesh, "IsDecimaMesh", y, false);
			y -= 20;
		}
	}

	void DetailsPanel::ViewTexture2D(float& y, Texture2D* texture)
	{

	}

	void DetailsPanel::ViewBlueprintClass(float& y, Blueprint* blueprint)
	{
		if (!blueprint) return;

		y -= 36.0f;
		if (EditorUI::Button("Add", GetDetailWidth() - 101.0f, y + 1.0f, 101.0f, 35.0f, ShutterPanelParams()))
		{
			Yaml::Node& graphs = blueprint->m_YamlNode_EditorOnly["Node"]["Graphs"];
			int i = 0;
			while (true)
			{
				Yaml::Node& graph = graphs[std::to_string(i)];
				if (graph.IsNone()) break;
				i++;
			}
			SuoraVerify(GetMajorTab()->IsA<NodeClassEditor>());
			Ref<NodeGraphEditor> NodeGraph = GetMajorTab()->As<NodeClassEditor>()->CreateNodeClassGraphEditorInstance(blueprint, i);
			NodeGraph->m_Graph->SerializeNodeGraph(graphs[std::to_string(i)]);
			graphs[std::to_string(i)]["Label"] = "New NodeGraph";
		}
		if (EditorUI::CategoryShutter(100, "Graphs", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			Yaml::Node& graphs = blueprint->m_YamlNode_EditorOnly["Node"]["Graphs"];
			int i = 0;
			while (true)
			{
				Yaml::Node& graph = graphs[std::to_string(i)];
				if (graph.IsNone()) break;

				y -= 35.0f;
				DrawLabel(graphs[std::to_string(i)]["Label"].As<String>(), y, 35.0f);
				if (EditorUI::Button("Edit Graph", GetDetailWidth() * m_Seperator + 50.0f, y + 5.0f, 120.0f, 25.0f, EditorUI::ButtonParams::Highlight()))
				{
					NodeClassEditor* editor = GetMajorTab()->As<NodeClassEditor>();
					editor->OpenNodeGraph(blueprint, i);
				}
				i++;
			}
			y -= 20;
		}

		y -= 36.0f;
		if (EditorUI::Button("Add", GetDetailWidth() - 101.0f, y + 1.0f, 101.0f, 35.0f, ShutterPanelParams()))
		{
			blueprint->m_ScriptClass->m_ScriptVars.push_back(ScriptVar());
		}
		if (EditorUI::CategoryShutter(101, "Variables", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			for (ScriptVar& var : blueprint->m_ScriptClass->m_ScriptVars)
			{
				y -= 35.0f;
				DrawLabel(var.m_VarName, y, 35.0f);
				EditorUI::ButtonParams params = EditorUI::ButtonParams::Invisible();
				params.TextColor = Color(0.0f);
				EditorUI::TextField(&var.m_VarName, 1.0f, y, GetDetailWidth() * m_Seperator - 2.0f, 35.0f, params);

				EditorUI::ButtonParams typeParams;
				typeParams.ButtonColor = GetScriptDataTypeColor(var.m_Type) * 0.5f;
				typeParams.ButtonColorHover = GetScriptDataTypeColor(var.m_Type) * 0.6f;
				typeParams.ButtonOutlineColor = GetScriptDataTypeColor(var.m_Type) * 0.25f;
				typeParams.ButtonRoundness = 22.5f / 2.0f;
				typeParams.TextDropShadow = true;

				String Label = ScriptDataTypeToLabel(var.m_Type);
				if (var.m_Type == ScriptDataType::ObjectPtr || var.m_Type == ScriptDataType::Class)
					Label = Class::FromString(var.m_VarParams).GetClassName();
				if (EditorUI::Button(Label, GetDetailWidth() * m_Seperator + 50.0f, y + 5.0f, 120.0f, 25.0f, typeParams))
				{
					EditorUI::SelectionOverlay* overlay = EditorUI::CreateOverlay<EditorUI::SelectionOverlay>(NativeInput::GetMousePosition().x, GetMajorTab()->GetEditorWindow()->GetWindow()->GetHeight() - NativeInput::GetMousePosition().y - 400.0f, 400.0f, 400.0f);
					for (int64_t type = (int64_t)ScriptDataType::None + 1; type < (int64_t)ScriptDataType::COUNT; type++)
					{
						if (type == (int64_t)ScriptDataType::ObjectPtr) continue;
						ScriptVar* VAR = &var;
						overlay->m_Entries.Add(EditorUI::SelectionOverlay::SelectionOverlayEntry(ScriptDataTypeToLabel((ScriptDataType)type), {}, [VAR, type]()
						{
							VAR->m_Type = (ScriptDataType)type;
						}));
					}
					Array<Class> classes = Class::GetAllClasses();
					for (Class cls : classes)
					{
						ScriptVar* VAR = &var;
						overlay->m_Entries.Add(EditorUI::SelectionOverlay::SelectionOverlayEntry(cls.GetClassName() + " Pointer", {}, [VAR, cls]()
						{
							VAR->m_Type = ScriptDataType::ObjectPtr;
							VAR->m_VarParams = cls.ToString();
						})); 
						overlay->m_Entries.Add(EditorUI::SelectionOverlay::SelectionOverlayEntry(cls.GetClassName() + " Class", {}, [VAR, cls]()
						{
							VAR->m_Type = ScriptDataType::Class;
							VAR->m_VarParams = cls.ToString();
						}));
					}
				}

			}

			y -= 20;
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

	void DetailsPanel::ViewProjectSettings(float& y, ProjectSettings* settings)
	{
		y -= 35.0f;
		if (EditorUI::CategoryShutter(0, "Rendering", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			DrawFloat(&settings->m_TargetFramerate, "Target Framerate", y, false);
			DrawBool(&settings->m_EnableDeferredRendering, "Enable Deferred Rendering", y, false);
		}
		y -= 35.0f;
		if (EditorUI::CategoryShutter(1, "Game", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			DrawSubclassOf((TSubclassOf*)&settings->m_GameInstanceClass, "GameInstanceClass", y, false);
			DrawAsset((Asset**)(&settings->m_DefaultLevel), Level::StaticClass(), "DefaultLevel", y, false);
			DrawAsset((Asset**)(&settings->m_ProjectIconTexture), Texture2D::StaticClass(), "ProjectIconTexture", y, false);
		}
		y -= 35.0f;
		if (EditorUI::CategoryShutter(2, "Editor", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			DrawAsset((Asset**)&(settings->m_EditorStartupAsset), Asset::StaticClass(), "Editor Startup Asset", y, false);
		}

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