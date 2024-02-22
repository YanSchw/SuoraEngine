#pragma once
#include "DetailsPanel.h"
#include "EditorPreferencesDetails.generated.h"

namespace Suora
{
	class Mesh;

	class EditorPreferencesDetails : public DetailsPanelImplementation
	{
		SUORA_CLASS(78465376847);
	public:

		virtual void ViewObject(Object* obj, float& y) override;
		void ViewEditorPreferences(float& y, EditorPreferences* settings);
	};

}