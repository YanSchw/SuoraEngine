#pragma once
#include "../MinorTab.h"
#include <Suora.h>
#include <unordered_map>

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
	public:
		Ptr<Object> m_Data = nullptr;
		//DetailViewType m_ViewType = DetailViewType::None;
		float m_Seperator = 0.35f;
		float m_ScrollY = 0.0f;
		float m_ResetSeperator = 35.0f;

		DetailsPanel(MajorTab* majorTab);
		~DetailsPanel();

		int GetDetailWidth() const;

		virtual void Render(float deltaTime) override;

		void DrawClassMember(float& x, float& y, Node* obj, ClassMember* member, int memberIndex);

		/** UI Utility */
		bool DrawHeaderShutter(void* data, const std::string& label, float x, float& y, bool valueChanged, bool& reset);
		void DrawLabel(const std::string& label, float y, float height);
		bool DrawButton(const std::string& label, const std::string& buttonLabel, float& y);
		void DrawDropDown(const std::string& label, const std::vector<std::pair<std::string, std::function<void(void)>>>& options, int index, float& y);
		bool DrawResetButton(float y, float height);
		Result DrawFloat(float* f, const std::string& label, float& y, bool valueChanged);
		Result DrawBool(bool* b, const std::string& label, float& y, bool valueChanged);
		Result DrawVec3(Vec3* v, const std::string& label, float& y, bool valueChanged);
		Result DrawVec4(Vec4* v, const std::string& label, float& y, bool valueChanged);
		Result DrawAsset(Asset** asset, const Class& cls, const std::string& label, float& y, bool valueChanged);
		Result DrawMaterialSlots(MaterialSlots* materials, float& y, bool valueChanged);
		Result DrawClass(Class* cls, const std::string& label, float& y, bool valueChanged);
		Result DrawSubclassOf(TSubclassOf* cls, const std::string& label, float& y, bool valueChanged);
		Result DrawDelegate(TDelegate* delegate, const std::string& label, float& y);

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

		bool m_ScrollbarNeeded = false;
		float GetScrollbarWidth() const;

	};
}