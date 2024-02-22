#pragma once
#include "Suora/Editor/Panels/MinorTab.h"
#include "Suora/Assets/Material.h"
#include "Suora/GameFramework/InputModule.h"
#include "Suora/Common/Map.h"

namespace Suora
{

	class Material;
	class Texture2D;
	class ProjectSettings;

	class DetailsPanel : public MinorTab
	{
		enum class Result : uint32_t
		{
			None = 0,
			ValueChange,
			ValueReset
		};
		using Super = MinorTab;
	private:
		Map<int32_t*, int32_t> m_DrawInt32Values;
		Map<float*, float> m_DrawFloatValues;
		Map<Vec4*, int32_t> m_Vec4_ColorPickerResults;
	public:
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

		// Transform Util
		void* Transform_LastNode = nullptr;
		Vec3 Transform_Pos = Vec::Zero;
		Vec3 Transform_Rot = Vec::Zero;
		Vec3 Transform_Scale = Vec::One;

		void ViewNode(float& y, Node* node);
		void ViewMaterial(float& y, Material* material, bool isShaderGraph);
		void ViewMesh(float& y, Mesh* mesh);
		void ViewTexture2D(float& y, Texture2D* texture);
		void ViewBlueprintClass(float& y, Blueprint* blueprint);
		void ViewProjectSettings(float& y, ProjectSettings* settings);
		void ViewEditorPreferences(float& y, EditorPreferences* settings);
		void ViewInputMapping(float& y, class InputMapping* input);

		void DrawInputDispatcherDropDown(const String& label, Ref<InputDispatcher>& dispatcher, InputActionType type, float x, float y, float width, float height);
		
		bool m_ScrollbarNeeded = false;
		float GetScrollbarWidth() const;

	};
}