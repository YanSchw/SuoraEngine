#pragma once
#include "DetailsPanel.h"
#include "InputMappingDetails.generated.h"

namespace Suora
{
	class InputMapping;

	class InputMappingDetails : public DetailsPanelImplementation
	{
		SUORA_CLASS(879543332);
	public:

		virtual void ViewObject(Object* obj, float& y) override;
		void ViewInputMapping(float& y, class InputMapping* input);

		void DrawInputDispatcherDropDown(const String& label, Ref<InputDispatcher>& dispatcher, InputActionType type, float x, float y, float width, float height);
	};

}