#pragma once
#include "DragableOverlay.h"
#include <functional>

namespace Suora
{

	struct ColorPickerOverlay : public DragableOverlay
	{
		Color* color = nullptr;
		Color originalColor;
		std::function<void(void)> OnColorChange;
		std::function<void(void)> OnColorReset;

		ColorPickerOverlay(Color* color, const std::function<void(void)>& onColorChange, const std::function<void(void)>& onColorReset);
		void Render(float deltaTime) override;
		void RenderColorCircle();
	};

}