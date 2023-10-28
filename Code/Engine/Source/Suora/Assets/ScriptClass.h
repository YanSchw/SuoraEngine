#pragma once
#include "Asset.h"
#include "ScriptClass.generated.h"

namespace Suora
{
	struct ScriptClassInternal;
	struct CompositionLayer;
	class DetailsPanel;

	class ScriptClass : public Asset
	{
		SUORA_CLASS(91746473);
		ASSET_EXTENSION(".script");

	private:
		Class m_ParentClass = Class::None;
		Ref<ScriptClassInternal> m_ScriptClass;
	public:
		ScriptClass();
		void PreInitializeAsset(const std::string& str) override;
		Class GetScriptParentClass() const;
		Object* CreateInstance(bool isRootNode);

		friend class DetailsPanel;
		friend class NodeClassEditor;
		friend class NodeClassNodeGraph;
		friend struct VisualScriptCompiler;
		friend struct ScriptEngine;
	};
}