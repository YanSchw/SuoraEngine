#include "DetailsPanel.h"
#include "../../EditorWindow.h"
#include "../Major/NodeClassEditor.h"
#include "../../Util/EditorPreferences.h"
#include "Suora/Serialization/CompositionLayer.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Suora/NodeScript/ScriptTypes.h"
#include "Suora/Assets/SuoraProject.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/Blueprint.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/InputModule.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/Common/Filesystem.h"

namespace Suora
{

	static Texture2D* undoTexture = nullptr;

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

	bool DetailsPanel::DrawHeaderShutter(void* data, const std::string& label, float x, float& y, bool valueChanged, bool& reset)
	{
		y -= 34.0f;
		DrawLabel("", y, 35.0f);
		if (valueChanged) reset = DrawResetButton(y, 35.0f);
		return (EditorUI::CategoryShutter((int64_t)data, label, x, y, GetDetailWidth() - x - 35.0f, 35.0f, ShutterPanelParams()));
	}

	void DetailsPanel::DrawLabel(const std::string& label, float y, float height)
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


	bool DetailsPanel::DrawButton(const std::string& label, const std::string& buttonLabel, float& y)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);

		return EditorUI::Button(buttonLabel, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, 150.0f, 25.0f);
	}

	void DetailsPanel::DrawDropDown(const std::string& label, const std::vector<std::pair<std::string, std::function<void(void)>>>& options, int index, float& y)
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

	DetailsPanel::Result DetailsPanel::DrawFloat(float* f, const std::string& label, float& y, bool valueChanged)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);
		const float dragFloatWidth = (GetDetailWidth() - GetDetailWidth() * m_Seperator) <= 150.0f ? (GetDetailWidth() - GetDetailWidth() * m_Seperator) : 150.0f;
		float before = *f;
		EditorUI::DragFloat(f, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, dragFloatWidth, 25.0f);

		if (valueChanged && DrawResetButton(y, 35.0f))
		{
			return Result::ValueReset;
		}

		return (before == *f) ? Result::None : Result::ValueChange;
	}
	DetailsPanel::Result DetailsPanel::DrawBool(bool* b, const std::string& label, float& y, bool valueChanged)
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
	DetailsPanel::Result DetailsPanel::DrawVec3(Vec3* v, const std::string& label, float& y, bool valueChanged)
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
	static std::unordered_map<Vec4*, int> _Vec4_ColorPickerResults;
	DetailsPanel::Result DetailsPanel::DrawVec4(Vec4* v, const std::string& label, float& y, bool valueChanged)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);
		const float dragFloatWidth = (GetDetailWidth() - GetDetailWidth() * m_Seperator) <= 150.0f ? (GetDetailWidth() - GetDetailWidth() * m_Seperator) : 150.0f;
		EditorUI::ColorPicker(v, GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, dragFloatWidth, 25.0f, {}, [v]() { _Vec4_ColorPickerResults[v] = 1; }, [v]() { _Vec4_ColorPickerResults[v] = 2; });

		if (_Vec4_ColorPickerResults.find(v) != _Vec4_ColorPickerResults.end())
		{
			int result = _Vec4_ColorPickerResults[v];
			_Vec4_ColorPickerResults.erase(v);

			if (result == 1) return Result::ValueChange;
			else if (result == 2) return Result::ValueReset;
		}

		if (valueChanged && DrawResetButton(y, 35.0f))
		{
			return Result::ValueReset;
		}

		return Result::None;
	}
	DetailsPanel::Result DetailsPanel::DrawAsset(Asset** asset, const Class& cls, const std::string& label, float& y, bool valueChanged)
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
				}
			}
		}

		return reset ? Result::ValueReset : (valueChanged ? Result::ValueChange : Result::None);
	}
	DetailsPanel::Result DetailsPanel::DrawClass(Class* cls, const std::string& label, float& y, bool valueChanged)
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
	DetailsPanel::Result DetailsPanel::DrawSubclassOf(TSubclassOf* cls, const std::string& label, float& y, bool valueChanged)
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

	DetailsPanel::Result DetailsPanel::DrawDelegate(TDelegate* delegate, const std::string& label, float& y)
	{
		y -= 34.0f;
		DrawLabel(label, y, 35.0f);

		if (EditorUI::Button("+Event", GetDetailWidth() * m_Seperator + 5.0f, y + 5.0f, 150.0f, 25.0f))
		{
			SuoraVerify(false);
		}

		return Result::None;
	}

	static void DrawVec3Control(Vec3* vec, float x, float y, float width, float height, float defaultVecResetValue = 0, const std::function<void(std::string)>& lambda = nullptr)
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

		//Renderer3D::DrawRect(10, 10, 50, 50, 0, Color(1));

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

	void DetailsPanel::DrawClassMember(float& x, float& y, Object* obj, ClassMember* member, int memberIndex)
	{
		//y -= 10.0f;
		if (!obj->Implements<IObjectCompositionData>()) obj->Implement<IObjectCompositionData>();
		IObjectCompositionData& data = *obj->GetInterface<IObjectCompositionData>();
		if (data.m_DefaultMemberValues.Size() <= memberIndex) return;

		if (member->m_Type == ClassMember::Type::Float)
		{
			float* f = ClassMember::AccessMember<float>(obj, member->m_MemberOffset);
			const Result result = DrawFloat(f, member->m_MemberName, y, data.m_DefaultMemberValues[memberIndex].m_ValueChanged);
			if (result == Result::ValueReset)
			{
				data.m_DefaultMemberValues[memberIndex].Reset(obj);
			}
			else if (result == Result::ValueChange)
			{
				data.m_DefaultMemberValues[memberIndex].m_ValueChanged = true;
				data.m_DefaultMemberValues[memberIndex].m_ValueFloat = *f;
			}
			
		}
		else if (member->m_Type == ClassMember::Type::Bool)
		{
			bool* b = ClassMember::AccessMember<bool>(obj, member->m_MemberOffset);
			const Result result = DrawBool(b, member->m_MemberName, y, data.m_DefaultMemberValues[memberIndex].m_ValueChanged);
			if (result == Result::ValueChange)
			{
				data.m_DefaultMemberValues[memberIndex].m_ValueChanged = true;
				data.m_DefaultMemberValues[memberIndex].m_ValueBool = *b;
			}
			else if (result == Result::ValueReset)
			{
				data.m_DefaultMemberValues[memberIndex].Reset(obj);
			}
			
		}
		else if (member->m_Type == ClassMember::Type::Vector3)
		{
			Vec3* v = ClassMember::AccessMember<Vec3>(obj, member->m_MemberOffset);
			const Result result = DrawVec3(v, member->m_MemberName, y, data.m_DefaultMemberValues[memberIndex].m_ValueChanged);
			if (result == Result::ValueChange)
			{
				data.m_DefaultMemberValues[memberIndex].m_ValueChanged = true;
				data.m_DefaultMemberValues[memberIndex].m_ValueVec3 = *v;
			}
			else if (result == Result::ValueReset)
			{
				data.m_DefaultMemberValues[memberIndex].Reset(obj);
			}

		}
		else if (member->m_Type == ClassMember::Type::Vector4)
		{
			Vec4* v = ClassMember::AccessMember<Vec4>(obj, member->m_MemberOffset);
			const Result result = DrawVec4(v, member->m_MemberName, y, data.m_DefaultMemberValues[memberIndex].m_ValueChanged);
			if (result == Result::ValueChange)
			{
				data.m_DefaultMemberValues[memberIndex].m_ValueChanged = true;
				data.m_DefaultMemberValues[memberIndex].m_ValueVec4 = *v;
			}
			else if (result == Result::ValueReset)
			{
				data.m_DefaultMemberValues[memberIndex].Reset(obj);
			}

		}
		else if (member->m_Type == ClassMember::Type::AssetPtr)
		{
			Asset** asset = ClassMember::AccessMember<Asset*>(obj, member->m_MemberOffset);
			const Result result = DrawAsset(asset, ((ClassMember_AssetPtr*)(member))->m_AssetClass, member->m_MemberName, y, data.m_DefaultMemberValues[memberIndex].m_ValueChanged);
			if (result == Result::ValueChange)
			{
				data.m_DefaultMemberValues[memberIndex].m_ValueChanged = true;
				data.m_DefaultMemberValues[memberIndex].m_ValueAssetPtr = *asset;
			}
			else if (result == Result::ValueReset)
			{
				data.m_DefaultMemberValues[memberIndex].Reset(obj);
			}
			
		}
		else if (member->m_Type == ClassMember::Type::MaterialSlots)
		{
			MaterialSlots* materials = ClassMember::AccessMember<MaterialSlots>(obj, member->m_MemberOffset);
			const Result result = DrawMaterialSlots(materials, y, data.m_DefaultMemberValues[memberIndex].m_ValueChanged);
			if (result == Result::ValueChange)
			{
				data.m_DefaultMemberValues[memberIndex].m_ValueChanged = true;
				data.m_DefaultMemberValues[memberIndex].m_ValueMaterialSlots = *materials;
			}
			else if (result == Result::ValueReset)
			{
				data.m_DefaultMemberValues[memberIndex].Reset(obj);
			}
		}
		else if (member->m_Type == ClassMember::Type::Class)
		{
			Class* cls = ClassMember::AccessMember<Class>(obj, member->m_MemberOffset);
			const Result result = DrawClass(cls, member->m_MemberName, y, data.m_DefaultMemberValues[memberIndex].m_ValueChanged);
			if (result == Result::ValueChange)
			{
				data.m_DefaultMemberValues[memberIndex].m_ValueChanged = true;
				data.m_DefaultMemberValues[memberIndex].m_ValueClass = *cls;
			}
			else if (result == Result::ValueReset)
			{
				data.m_DefaultMemberValues[memberIndex].Reset(obj);
			}
		}
		else if (member->m_Type == ClassMember::Type::SubclassOf)
		{
			TSubclassOf* cls = ClassMember::AccessMember<TSubclassOf>(obj, member->m_MemberOffset);
			const Result result = DrawSubclassOf(cls, member->m_MemberName, y, data.m_DefaultMemberValues[memberIndex].m_ValueChanged);
			if (result == Result::ValueChange)
			{
				data.m_DefaultMemberValues[memberIndex].m_ValueChanged = true;
				data.m_DefaultMemberValues[memberIndex].m_ValueClass = *cls;
			}
			else if (result == Result::ValueReset)
			{
				data.m_DefaultMemberValues[memberIndex].Reset(obj);
			}
		}
		else if (member->m_Type == ClassMember::Type::Delegate)
		{
			TDelegate* delegate = ClassMember::AccessMember<TDelegate>(obj, member->m_MemberOffset);
			const Result result = DrawDelegate(delegate, member->m_MemberName, y);
		}
		else
		{
			SuoraError("{0}: Missing ClassMember implementation!", __FUNCTION__);
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

		if (Node3D* node3D = node->As<Node3D>())
		{
			y -= 35;
			static bool readTransform = true;
			static std::function<void(std::string)> disableReadTransform = [](std::string s) { readTransform = false; };
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

		// Node Derivates
		Array<Class> derivates = node->GetClass().GetInheritanceTree();
		int memberIndex = 0;
		for (int64_t i = node->IsA<Node3D>() ? 3 : 2; i < derivates.Size(); i++)
		{
			y -= 35;
			const ClassReflector& refl = ClassReflector::GetByClass(derivates[i]);
			if (EditorUI::CategoryShutter(1000 + i, refl.m_ClassName, 0, y, GetDetailWidth(), 35, ShutterPanelParams()))
			{
				for (Ref<ClassMember> member : refl.m_ClassMembers)
				{
					float x = 0;
					DrawClassMember(x, y, node, member.get(), memberIndex++);
				}

			}
			else
			{
				memberIndex += refl.m_ClassMembers.Size();
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
				std::vector<std::pair<std::string, std::function<void(void)>>> options;
				std::filesystem::path directory = AssetManager::GetEngineAssetPath() + "/EngineContent/Shaders/ShadergraphBase/";
				std::vector<DirectoryEntry> entries = File::GetAllAbsoluteEntriesOfPath(directory);
				for (int j = 0; j < entries.size(); j++)
				{
					const std::string name = entries[j].path().filename().string();
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
				DrawLabel(graphs[std::to_string(i)]["Label"].As<std::string>(), y, 35.0f);
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

				std::string Label = ScriptDataTypeToLabel(var.m_Type);
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

	static void DrawInputDispatcherDropDown(Ref<InputDispatcher>& dispatcher, InputActionType type, float x, float y, float width, float height)
	{
		EditorUI::ButtonParams params;
		params.ButtonColor = EditorPreferences::Get()->UiInputColor;
		params.ButtonColorHover = EditorPreferences::Get()->UiInputColor;
		if (EditorUI::Button(dispatcher ? dispatcher->m_Label : "None", x, y, width, height, params))
		{
			EditorUI::SelectionOverlay* overlay = EditorUI::CreateOverlay<EditorUI::SelectionOverlay>(x + EditorUI::GetInputOffset().x - 300, y + EditorUI::GetInputOffset().y - 300, 400, 400);

			for (auto& It : InputModule::s_RegisteredInputDispatchers)
			{
				if (It.second->m_ActionSpecifier == type)
				{
					overlay->m_Entries.Add(EditorUI::SelectionOverlay::SelectionOverlayEntry(It.first, {}, [&]() { dispatcher = It.second; }));
				}
			}
			overlay->RefreshEntries();
		}
		EditorUI::DrawTexturedRect(AssetManager::GetAsset<Texture2D>(SuoraID("8742cec8-9ee5-4645-b036-577146904b41"))->GetTexture(), x + width - height - 2.5f, y, height, height, 0.0f, Color(1.0f));

	}

	void DetailsPanel::ViewProjectSettings(float& y, ProjectSettings* settings)
	{
		y -= 35.0f;
		if (EditorUI::CategoryShutter(0, "Rendering", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			DrawFloat(&settings->m_TargetFramerate, "TargetFramerate", y, false);
		}
		y -= 35.0f;
		if (EditorUI::CategoryShutter(1, "Game", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			DrawSubclassOf((TSubclassOf*)&settings->m_GameInstanceClass, "GameInstanceClass", y, false);
			DrawAsset((Asset**)&(Level*)settings->m_DefaultLevel, Level::StaticClass(), "DefaultLevel", y, false);
			DrawAsset((Asset**)&(Texture2D*)settings->m_ProjectIconTexture, Texture2D::StaticClass(), "ProjectIconTexture", y, false);
		}
		y -= 35.0f;
		if (EditorUI::CategoryShutter(2, "Editor", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
		{
			DrawAsset((Asset**)&(settings->m_EditorStartupAsset), Asset::StaticClass(), "Editor Startup Asset", y, false);
		}

#pragma region Input
		y -= 35.0f;
		if (EditorUI::CategoryShutter(3, "Input", 0, y, GetDetailWidth() - 100.0f, 35.0f, ShutterPanelParams()))
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

			y -= 400.0f;
			EditorUI::DrawRect(0, y, GetDetailWidth(), 400.0f, 0.0f, EditorPreferences::Get()->UiColor);
			/** InputCategories */
			static WeakRef<InputCategory> s_SelectedInputCategory = WeakRef<InputCategory>();
			{
				static float scrollY = 0.0f;
				float catY = y + 375.0f + scrollY;
				EditorUI::ButtonParams Params = EditorUI::ButtonParams::Invisible();
				Params.TextOrientation = Vec2(-0.95f, 0.0f);
				for (auto& It : settings->m_InputSettings->m_Categories)
				{
					catY -= 25.0f;
					Params.ButtonColor = (s_SelectedInputCategory.lock() == It) ? EditorPreferences::Get()->UiHighlightColor : Color(0.0f);
					Params.ButtonColorHover = (s_SelectedInputCategory.lock() == It) ? EditorPreferences::Get()->UiHighlightColor : Color(0.0f);
					Params.ButtonColorClicked = (s_SelectedInputCategory.lock() == It) ? EditorPreferences::Get()->UiHighlightColor : Color(0.0f);
					if (EditorUI::Button(It->m_CategoryName, 0.0f, catY, GetDetailWidth() * 0.25f, 25.0f, Params))
					{
						s_SelectedInputCategory = It;
					}
					EditorUI::ButtonParams ParamsRightClick = EditorUI::ButtonParams::Invisible();
					ParamsRightClick.OverrideActivationEvent = true;
					ParamsRightClick.OverrittenActivationEvent = []() { return NativeInput::GetMouseButtonDown(Mouse::ButtonRight); };
					if (EditorUI::Button("", 0.0f, catY, GetDetailWidth() * 0.25f, 25.0f, ParamsRightClick))
					{
						std::vector<EditorUI::ContextMenuElement> context;
						Ref<InputCategory> cat = It;
						const float width = GetDetailWidth();
						context.push_back(EditorUI::ContextMenuElement({}, [cat, catY, width]()
							{
								EditorUI::_SetTextFieldStringPtr(&cat->m_CategoryName, 0.0f, catY, width * 0.25f, 25.0f, false);
							}, "Rename Category", nullptr));
						context.push_back(EditorUI::ContextMenuElement({}, [cat, settings]()
							{
								settings->m_InputSettings->m_Categories.Remove(cat);
							}, "Remove Category", nullptr));
						EditorUI::CreateContextMenu(context);
					}
				}

				EditorUI::DrawRectOutline(0, y, GetDetailWidth() * 0.25f, 400.0f, 3.0f, EditorPreferences::Get()->UiForgroundColor);
				EditorUI::DrawRect(0, y + 375.0f, GetDetailWidth() * 0.25f, 25.0f, 0.0f, EditorPreferences::Get()->UiForgroundColor);
				EditorUI::DrawRect(0, y, GetDetailWidth() * 0.25f, 25.0f, 0.0f, EditorPreferences::Get()->UiForgroundColor);
				EditorUI::Text("Categories", Font::Instance, 5.0f, y + 375.0f, GetDetailWidth() * 0.25f, 25.0f, 22.0f, Vec2(-0.97f, 0.0f), Color(1.0f));
				if (EditorUI::Button("Add Category", GetDetailWidth() * 0.25f - 155.0f, y + 375.0f, 150.0f, 25.0f, EditorUI::ButtonParams::Highlight()))
				{
					settings->m_InputSettings->m_Categories.Add(Ref<InputCategory>(new InputCategory()));
				}
				float scrollDown = catY - y - 50.0f - scrollY;
				EditorUI::ScrollbarVertical(GetDetailWidth() * 0.25f - 5.0f, y, 5.0f, 400.0f, 0.0f, y, GetDetailWidth() * 0.25f, 400.0f, 0.0f, scrollDown > 0 ? 0 : Math::Abs(scrollDown), &scrollY);
			}
			/** InputActions */
			static WeakRef<InputBinding> s_SelectedInputBinding = WeakRef<InputBinding>();
			{
				static float scrollY = 0.0f;
				float actionY = y + 375.0f + scrollY;
				Ref<InputCategory> category = s_SelectedInputCategory.lock();

				if (category)
				{
					int64_t i = 1000;
					for (auto& It : category->m_Actions)
					{
						actionY -= 25.0f;
						const float __y = actionY;
						if (EditorUI::CategoryShutter(i++, It->m_ActionName, GetDetailWidth() * 0.25f, actionY, GetDetailWidth() * 0.4f, 25.0f, EditorUI::ButtonParams::Invisible()))
						{
							for (auto& binding : It->m_Bindings)
							{
								actionY -= 25.0f;
								EditorUI::ButtonParams Params = EditorUI::ButtonParams::Invisible();
								Params.TextOrientation = Vec2(-0.95f, 0.0f);
								Params.ButtonColor = (s_SelectedInputBinding.lock() == binding) ? EditorPreferences::Get()->UiHighlightColor : Color(0.0f);
								Params.ButtonColorHover = (s_SelectedInputBinding.lock() == binding) ? EditorPreferences::Get()->UiHighlightColor : Color(0.0f);
								Params.ButtonColorClicked = (s_SelectedInputBinding.lock() == binding) ? EditorPreferences::Get()->UiHighlightColor : Color(0.0f);
								if (EditorUI::Button(binding->m_Label, GetDetailWidth() * 0.25f + 25.0f, actionY, GetDetailWidth() * 0.4f - 75.0f, 25.0f, Params))
								{
									s_SelectedInputBinding = binding;
								}
								EditorUI::ButtonParams ParamsRightClick = EditorUI::ButtonParams::Invisible();
								ParamsRightClick.OverrideActivationEvent = true;
								ParamsRightClick.OverrittenActivationEvent = []() { return NativeInput::GetMouseButtonDown(Mouse::ButtonRight); };
								if (EditorUI::Button("", GetDetailWidth() * 0.25f + 25.0f, actionY, GetDetailWidth() * 0.4f - 75.0f, 25.0f, ParamsRightClick))
								{
									std::vector<EditorUI::ContextMenuElement> context;
									Ref<InputBinding> bind = binding;
									const float width = GetDetailWidth();
									context.push_back(EditorUI::ContextMenuElement({}, [bind, actionY, width]()
										{
											EditorUI::_SetTextFieldStringPtr(&bind->m_Label, width * 0.25f + 25.0f, actionY, width * 0.4f - 75.0f, 25.0f, false);
										}, "Rename InputBinding", nullptr));
									context.push_back(EditorUI::ContextMenuElement({}, [bind, It]()
										{
											It->m_Bindings.Remove(bind);
										}, "Remove InputBinding", nullptr));
									EditorUI::CreateContextMenu(context);
								}
								EditorUI::DrawRect(GetDetailWidth() * 0.25f + 25.0f + 3.0f, actionY, 3.0f, 25.0f, 1.0f, Color(0.34117647f, 0.0156862745f, 0.25882352f, 1.0f));
							}
						}
						EditorUI::DrawRect(GetDetailWidth() * 0.25f + 3.0f, __y, 3.0f, 25.0f, 1.0f, Color(0.01568627f, 0.34117647f, 0.03921568f, 1.0f));
						EditorUI::ButtonParams ParamsRightClick = EditorUI::ButtonParams::Invisible();
						ParamsRightClick.OverrideActivationEvent = true;
						ParamsRightClick.OverrittenActivationEvent = []() { return NativeInput::GetMouseButtonDown(Mouse::ButtonRight); };
						if (EditorUI::Button("", GetDetailWidth() * 0.25f, __y, GetDetailWidth() * 0.4f, 25.0f, ParamsRightClick))
						{
							std::vector<EditorUI::ContextMenuElement> context;
							Ref<InputAction> action = It;
							const float width = GetDetailWidth();
							context.push_back(EditorUI::ContextMenuElement({}, [action, __y, width]()
								{
									EditorUI::_SetTextFieldStringPtr(&action->m_ActionName, width * 0.25f, __y, width * 0.4f, 25.0f, false);
								}, "Rename Action", nullptr));
							context.push_back(EditorUI::ContextMenuElement({}, [action, category]()
								{
									category->m_Actions.Remove(action);
								}, "Remove Action", nullptr));

							context.push_back(EditorUI::ContextMenuElement(
								{
									EditorUI::ContextMenuElement({}, [action]()
									{
										if (action->m_ActionType != InputActionType::Action)
										{
											action->m_Bindings.Clear();
										}
										action->m_ActionType = InputActionType::Action;
									}, "Action (Button)", nullptr),
									EditorUI::ContextMenuElement({}, [action]()
									{
										if (action->m_ActionType != InputActionType::Axis)
										{
											action->m_Bindings.Clear();
										}
										action->m_ActionType = InputActionType::Axis;
									}, "Axis", nullptr),
									EditorUI::ContextMenuElement({}, [action]()
									{
										if (action->m_ActionType != InputActionType::Axis2D)
										{
											action->m_Bindings.Clear();
										}
										action->m_ActionType = InputActionType::Axis2D;
									}, "Axis2D", nullptr)
								}, []() { }, "Change ActionType", nullptr));

							for (auto& It : s_InputBindables[action->m_ActionType])
							{
								context.push_back(EditorUI::ContextMenuElement({}, [action, It]()
									{
										action->m_Bindings.Add(Ref<InputBinding>(New(It)->As<InputBinding>()));
									}, "Add " + It.GetClassName(), nullptr));
							}

							EditorUI::CreateContextMenu(context);
						}
					}
				}
				else
				{
					EditorUI::Text("Select an InputCategory", Font::Instance, GetDetailWidth() * 0.25f, y, GetDetailWidth() * 0.4f, 400.0f, 26.0f, Vec2(0.0f), Color(1.0f));
				}

				EditorUI::DrawRectOutline(GetDetailWidth() * 0.25f, y, GetDetailWidth() * 0.4f, 400.0f, 3.0f, EditorPreferences::Get()->UiForgroundColor);
				EditorUI::DrawRect(GetDetailWidth() * 0.25f, y + 375.0f, GetDetailWidth() * 0.4f, 25.0f, 0.0f, EditorPreferences::Get()->UiForgroundColor);
				EditorUI::DrawRect(GetDetailWidth() * 0.25f, y, GetDetailWidth() * 0.4f, 25.0f, 0.0f, EditorPreferences::Get()->UiForgroundColor);
				EditorUI::Text("Input Actions", Font::Instance, GetDetailWidth() * 0.25f + 5.0f, y + 375.0f, GetDetailWidth() * 0.4f, 25.0f, 22.0f, Vec2(-0.97f, 0.0f), Color(1.0f));
				if (category && EditorUI::Button("Add Action", GetDetailWidth() * 0.65f - 155.0f, y + 375.0f, 150.0f, 25.0f, EditorUI::ButtonParams::Highlight()))
				{
					category->m_Actions.Add(Ref<InputAction>(new InputAction()));
				}
				float scrollDown = actionY - y - 50.0f - scrollY;
				EditorUI::ScrollbarVertical(GetDetailWidth() * 0.65f - 5.0f, y, 5.0f, 400.0f, GetDetailWidth() * 0.25f, y, GetDetailWidth() * 0.4f, 400.0f, 0.0f, scrollDown > 0 ? 0 : Math::Abs(scrollDown), &scrollY);
			}
			/** InputBindings */
			{
				static float scrollY = 0.0f;
				float bindingsY = y + 375.0f + scrollY;

				if (Ref<InputBinding> binding = s_SelectedInputBinding.lock())
				{
					if (binding->GetClass() == InputBinding_DirectAction::StaticClass())
					{
						EditorUI::Text("Bind", Font::Instance, GetDetailWidth() * 0.65f + 7.0f, y + 350.0f, GetDetailWidth() * 0.1f, 25.0f, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
						DrawInputDispatcherDropDown(binding->As<InputBinding_DirectAction>()->m_Dispatcher, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y + 350.0f, GetDetailWidth() * 0.15f, 25.0f);
					}
					else if (binding->GetClass() == InputBinding_DirectAxis::StaticClass())
					{
						EditorUI::Text("Bind", Font::Instance, GetDetailWidth() * 0.65f + 7.0f, y + 350.0f, GetDetailWidth() * 0.1f, 25.0f, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
						DrawInputDispatcherDropDown(binding->As<InputBinding_DirectAxis>()->m_Dispatcher, InputActionType::Axis, GetDetailWidth() * 0.75f + 7.0f, y + 350.0f, GetDetailWidth() * 0.15f, 25.0f);
					}
					else if (binding->GetClass() == InputBinding_DirectAxis2D::StaticClass())
					{
						EditorUI::Text("Bind", Font::Instance, GetDetailWidth() * 0.65f + 7.0f, y + 350.0f, GetDetailWidth() * 0.1f, 25.0f, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
						DrawInputDispatcherDropDown(binding->As<InputBinding_DirectAxis2D>()->m_Dispatcher, InputActionType::Axis2D, GetDetailWidth() * 0.75f + 7.0f, y + 350.0f, GetDetailWidth() * 0.15f, 25.0f);
					}
					else if (binding->GetClass() == InputBinding_DirectionalActionMapping::StaticClass())
					{
						EditorUI::Text("Bind Positive", Font::Instance, GetDetailWidth() * 0.65f + 7.0f, y + 350.0f, GetDetailWidth() * 0.1f, 25.0f, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
						DrawInputDispatcherDropDown(binding->As<InputBinding_DirectionalActionMapping>()->m_DispatcherPos, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y + 350.0f, GetDetailWidth() * 0.15f, 25.0f);
						EditorUI::Text("Bind Negative", Font::Instance, GetDetailWidth() * 0.65f + 7.0f, y + 325.0f, GetDetailWidth() * 0.1f, 25.0f, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
						DrawInputDispatcherDropDown(binding->As<InputBinding_DirectionalActionMapping>()->m_DispatcherNeg, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y + 325.0f, GetDetailWidth() * 0.15f, 25.0f);
					}
					else if (binding->GetClass() == InputBinding_BidirectionalActionMapping::StaticClass())
					{
						EditorUI::Text("Bind Up", Font::Instance, GetDetailWidth() * 0.65f + 7.0f, y + 350.0f, GetDetailWidth() * 0.1f, 25.0f, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
						DrawInputDispatcherDropDown(binding->As<InputBinding_BidirectionalActionMapping>()->m_DispatcherUp, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y + 350.0f, GetDetailWidth() * 0.15f, 25.0f);
						EditorUI::Text("Bind Down", Font::Instance, GetDetailWidth() * 0.65f + 7.0f, y + 325.0f, GetDetailWidth() * 0.1f, 25.0f, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
						DrawInputDispatcherDropDown(binding->As<InputBinding_BidirectionalActionMapping>()->m_DispatcherDown, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y + 325.0f, GetDetailWidth() * 0.15f, 25.0f);
						EditorUI::Text("Bind Left", Font::Instance, GetDetailWidth() * 0.65f + 7.0f, y + 300.0f, GetDetailWidth() * 0.1f, 25.0f, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
						DrawInputDispatcherDropDown(binding->As<InputBinding_BidirectionalActionMapping>()->m_DispatcherLeft, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y + 300.0f, GetDetailWidth() * 0.15f, 25.0f);
						EditorUI::Text("Bind Right", Font::Instance, GetDetailWidth() * 0.65f + 7.0f, y + 275.0f, GetDetailWidth() * 0.1f, 25.0f, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
						DrawInputDispatcherDropDown(binding->As<InputBinding_BidirectionalActionMapping>()->m_DispatcherRight, InputActionType::Action, GetDetailWidth() * 0.75f + 7.0f, y + 275.0f, GetDetailWidth() * 0.15f, 25.0f);
					}
					else
					{
						EditorUI::Text("Unkown InputBinding Class!", Font::Instance, GetDetailWidth() * 0.65f, y, GetDetailWidth() * 0.35f, 400.0f, 26.0f, Vec2(0.0f), Color(1.0f, 0.0f, 0.0f, 1.0f));
					}
				}
				else
				{
					EditorUI::Text("Select an InputBinding", Font::Instance, GetDetailWidth() * 0.65f, y, GetDetailWidth() * 0.35f, 400.0f, 26.0f, Vec2(0.0f), Color(1.0f));
				}

				EditorUI::DrawRectOutline(GetDetailWidth() * 0.65f, y, GetDetailWidth() * 0.35f, 400.0f, 3.0f, EditorPreferences::Get()->UiForgroundColor);
				EditorUI::DrawRect(GetDetailWidth() * 0.65f, y + 375.0f, GetDetailWidth() * 0.35f, 25.0f, 0.0f, EditorPreferences::Get()->UiForgroundColor);
				EditorUI::DrawRect(GetDetailWidth() * 0.65f, y, GetDetailWidth() * 0.35f, 25.0f, 0.0f, EditorPreferences::Get()->UiForgroundColor);
				EditorUI::Text("Input Bindings", Font::Instance, GetDetailWidth() * 0.65f + 5.0f, y + 375.0f, GetDetailWidth() * 0.35f, 25.0f, 22.0f, Vec2(-0.97f, 0.0f), Color(1.0f));
				
				float scrollDown = bindingsY - y - 50.0f - scrollY;
				EditorUI::ScrollbarVertical(GetDetailWidth() - 5.0f, y, 5.0f, 400.0f, GetDetailWidth() * 0.65f, y, GetDetailWidth() * 0.35f, 400.0f, 0.0f, scrollDown > 0 ? 0 : Math::Abs(scrollDown), &scrollY);
			}
		}
#pragma endregion

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
	}

	float DetailsPanel::GetScrollbarWidth() const
	{
		return m_ScrollbarNeeded ? 10.0f : 0.0f;
	}

}