#pragma once
#include "Suora/Editor/Panels/MinorTab.h"
#include "Suora/Assets/Material.h"
#include "Suora/GameFramework/InputModule.h"
#include "Suora/Common/Map.h"

#include "DetailsPanel.generated.h"

namespace Suora
{

	class Material;
	class Texture2D;
	class ProjectSettings;
	class DetailsPanelImplementation;

	class DetailsPanel : public MinorTab
	{
	public:
		enum class Result : uint32_t
		{
			None = 0,
			ValueChange,
			ValueReset
		};

		Ptr<Object> m_Data = nullptr;
		float m_Seperator = 0.35f;
		float m_ScrollY = 0.0f;
		float m_ResetSeperator = 35.0f;

		DetailsPanel(MajorTab* majorTab);
		~DetailsPanel();

		int GetDetailWidth() const;

		virtual void Render(float deltaTime) override;

		void DrawClassMember(float& x, float& y, Node* obj, ClassMemberProperty* member, int memberIndex);

		/** UI Utility */
		bool DrawHeaderShutter(void* data, const String& label, float x, float& y, bool valueChanged, bool& reset);
		void DrawLabel(const String& label, float y, float height);
		bool DrawButton(const String& label, const String& buttonLabel, float& y);
		void DrawDropDown(const String& label, const std::vector<std::pair<String, std::function<void(void)>>>& options, int index, float& y);
		bool DrawResetButton(float y, float height);
		Result DrawInt32(int32_t* i, const String& label, float& y, bool valueChanged);
		Result DrawFloat(float* f, const String& label, float& y, bool valueChanged);
		Result DrawBool(bool* b, const String& label, float& y, bool valueChanged);
		Result DrawVec3(Vec3* v, const String& label, float& y, bool valueChanged);
		Result DrawVec4(Vec4* v, const String& label, float& y, bool valueChanged);
		Result DrawAsset(Asset** asset, const Class& cls, const String& label, float& y, bool valueChanged);
		Result DrawMaterialSlots(MaterialSlots* materials, float& y, bool valueChanged);
		Result DrawClass(Class* cls, const String& label, float& y, bool valueChanged);
		Result DrawSubclassOf(TSubclassOf* cls, const String& label, float& y, bool valueChanged);
		Result DrawDelegate(TDelegate* delegate, const String& label, float& y);

		static void DrawVec3Control(Vec3* vec, float x, float y, float width, float height, float defaultVecResetValue = 0, const std::function<void(String)>& lambda = nullptr);

		void ViewInputMapping(float& y, class InputMapping* input);

		void DrawInputDispatcherDropDown(const String& label, Ref<InputDispatcher>& dispatcher, InputActionType type, float x, float y, float width, float height);
		
		bool m_ScrollbarNeeded = false;
		float GetScrollbarWidth() const;

		static EditorUI::ButtonParams ShutterPanelParams();

	private:
		Map<int32_t*, int32_t> m_DrawInt32Values;
		Map<float*, float> m_DrawFloatValues;
		Map<Vec4*, int32_t> m_Vec4_ColorPickerResults;

		Array<Ref<DetailsPanelImplementation>> m_Implementations;

		using Super = MinorTab;
	};

	class DetailsPanelImplementation : public Object
	{
		SUORA_CLASS(87953422398);
	public:
		// To be overwritten
		virtual void ViewObject(Object* obj, float& y) = 0;
		virtual int32_t GetOrderIndex() const { return 0; }

		// Helperfunctions
		DetailsPanel* GetDetailsPanel() const { return m_DetailsPanel; }

		// MinorTab
		int GetWidth() const { return GetDetailsPanel()->GetWidth(); }
		int GetHeight() const { return GetDetailsPanel()->GetHeight(); }
		Vec2 GetMinorMousePos() const { return GetDetailsPanel()->GetMinorMousePos(); }
		MajorTab* GetMajorTab() const { return GetDetailsPanel()->GetMajorTab(); }
		bool IsInputValid() { return GetDetailsPanel()->IsInputValid(); }
		bool IsInputMode(EditorInputEvent event) { return GetDetailsPanel()->IsInputMode(event); }
		void SetInputMode(EditorInputEvent event) { return GetDetailsPanel()->SetInputMode(event); }

		// DetailsPanel
		int GetDetailWidth() const { return GetDetailsPanel()->GetDetailWidth(); }
		static EditorUI::ButtonParams ShutterPanelParams() { return DetailsPanel::ShutterPanelParams(); }
		float GetSeperator() const { return GetDetailsPanel()->m_Seperator; }

		bool DrawHeaderShutter(void* data, const String& label, float x, float& y, bool valueChanged, bool& reset) { return GetDetailsPanel()->DrawHeaderShutter(data, label, x, y, valueChanged, reset); }
		void DrawLabel(const String& label, float y, float height) { GetDetailsPanel()->DrawLabel(label, y, height); }
		bool DrawButton(const String& label, const String& buttonLabel, float& y) { return GetDetailsPanel()->DrawButton(label, buttonLabel, y); }
		void DrawDropDown(const String& label, const std::vector<std::pair<String, std::function<void(void)>>>& options, int index, float& y) { GetDetailsPanel()->DrawDropDown(label, options, index, y); }
		bool DrawResetButton(float y, float height) { return GetDetailsPanel()->DrawResetButton(y, height); }
		DetailsPanel::Result DrawInt32(int32_t* i, const String& label, float& y, bool valueChanged) { return GetDetailsPanel()->DrawInt32(i, label, y, valueChanged); }
		DetailsPanel::Result DrawFloat(float* f, const String& label, float& y, bool valueChanged) { return GetDetailsPanel()->DrawFloat(f, label, y, valueChanged); }
		DetailsPanel::Result DrawBool(bool* b, const String& label, float& y, bool valueChanged) { return GetDetailsPanel()->DrawBool(b, label, y, valueChanged); }
		DetailsPanel::Result DrawVec3(Vec3* v, const String& label, float& y, bool valueChanged) { return GetDetailsPanel()->DrawVec3(v, label, y, valueChanged); }
		DetailsPanel::Result DrawVec4(Vec4* v, const String& label, float& y, bool valueChanged) { return GetDetailsPanel()->DrawVec4(v, label, y, valueChanged); }
		DetailsPanel::Result DrawAsset(Asset** asset, const Class& cls, const String& label, float& y, bool valueChanged) { return GetDetailsPanel()->DrawAsset(asset, cls, label, y, valueChanged); }
		DetailsPanel::Result DrawMaterialSlots(MaterialSlots* materials, float& y, bool valueChanged) { return GetDetailsPanel()->DrawMaterialSlots(materials, y, valueChanged); }
		DetailsPanel::Result DrawClass(Class* cls, const String& label, float& y, bool valueChanged) { return GetDetailsPanel()->DrawClass(cls, label, y, valueChanged); }
		DetailsPanel::Result DrawSubclassOf(TSubclassOf* cls, const String& label, float& y, bool valueChanged) { return GetDetailsPanel()->DrawSubclassOf(cls, label, y, valueChanged); }
		DetailsPanel::Result DrawDelegate(TDelegate* delegate, const String& label, float& y) { return GetDetailsPanel()->DrawDelegate(delegate, label, y); }

	private:
		void SetDetailsPanel(DetailsPanel* panel) { m_DetailsPanel = panel; }
		DetailsPanel* m_DetailsPanel = nullptr;

		friend class DetailsPanel;
	};

}