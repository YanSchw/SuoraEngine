#pragma once
#include "Asset.h"
#include "Blueprint.generated.h"

namespace Suora
{
	struct ScriptClassInternal;
	class DetailsPanel;
	enum class InputActionKind : uint32_t;

	class Blueprint : public Asset
	{
		SUORA_CLASS(1258473980);
		ASSET_EXTENSION(".node");

	private:
		PROPERTY()
		Class m_ParentClass = Class::None;

		Yaml::Node m_Composition;
		Ref<ScriptClassInternal> m_ScriptClass;
		Yaml::Node m_YamlNode_EditorOnly;

		bool m_InputEventsAreForPawnsOnly = false;

		struct DelegateEventBind
		{
			String ChildName;
			String DelegateName;
			size_t ScriptFunctionHash;
			DelegateEventBind(const String& childName, const String& delegateName, size_t hash)
				: ChildName(childName), DelegateName(delegateName), ScriptFunctionHash(hash)
			{
			}
		};
		struct InputEventBind
		{
			String Label;
			size_t ScriptFunctionHash;
			InputActionKind Flags;
			InputEventBind(const String& label, size_t hash, InputActionKind flags)
				: Label(label), ScriptFunctionHash(hash), Flags(flags)
			{
			}
		};
		Array<DelegateEventBind> m_DelegateEventsToBindDuringGameplay;
		Array<InputEventBind> m_InputEventsToBeBound;

	public:
		Blueprint();
		void PreInitializeAsset(Yaml::Node& root) override;
		Class GetNodeParentClass() const;
		Object* CreateInstance(bool isRootNode);

		void Serialize(Yaml::Node& root) override;

	private:
		void SetParentClass(const Class& cls);
		void InitComposition(const Class& cls);

		friend class Level;
		friend class World;
		friend class GameInstance;
		friend class BlueprintDetails;
		friend class NodeClassEditor;
		friend class NodeClassNodeGraph;
		friend class CreateClassOverlay;
		friend class CreateLevelOverlay;
		friend struct BlueprintCompiler;
		friend class INodeScriptObject;
	};
}