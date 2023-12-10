#pragma once
#include "DragableOverlay.h"
#include <functional>

namespace Suora
{

	struct SelectAnyClassOverlay : public DragableOverlay
	{
		SelectAnyClassOverlay(const String& title, const Class& rootClass, const std::function<void(Class)>& lambda)
			: DragableOverlay(title), m_RootClass(rootClass), m_Lambda(lambda)
		{
		}
		EditorUI::SubclassHierarchyBoxParams _Params;
		Class m_SelectedClass = Object::StaticClass();
		Class m_RootClass = Object::StaticClass();
		std::function<void(Class)> m_Lambda;

		void Render(float deltaTime) override;
		void SelectClass(const Class& cls);

	};

}