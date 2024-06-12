#include "Precompiled.h"
#include "ColorPickerOverlay.h"

#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Assets/AssetManager.h"

namespace Suora
{

	static Suora::Ref<Shader> s_ColorCircleShader;

	ColorPickerOverlay::ColorPickerOverlay(Color* color, const std::function<void(void)>& onColorChange, const std::function<void(void)>& onColorReset)
		: color(color), originalColor(*color), OnColorChange(onColorChange), OnColorReset(onColorReset), DragableOverlay("Color Picker")
	{
		s_ColorCircleShader = Shader::Create(AssetManager::GetEngineAssetPath() + "/EditorContent/Shaders/ColorCircle.glsl");
	}

	void ColorPickerOverlay::Render(float deltaTime)
	{
		DragableOverlay::Render(deltaTime);
		if (WasMousePressedOutsideOfOverlay())
		{
			OnColorChange();
			Dispose();
			return;
		}
		EditorUI::DrawRect(x + 50, y + 180, width - 100, height - 250, 4, *color);

		RenderCommand::SetViewport(x + 50, y + 180, width - 100, height - 250);
		RenderPipeline::__GetFullscreenQuad()->Bind();
		s_ColorCircleShader->Bind();
		RenderCommand::DrawIndexed(RenderPipeline::__GetFullscreenQuad());

		Color temp = *color;

		EditorUI::Text("Red", Font::Instance, x + 5, y + 140, 35.0f, 20, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
		EditorUI::SliderFloat(&color->r, 0.0f, 1.0f, x + 45, y + 140, width - 130, 20);
		EditorUI::DragFloat(&color->r, x + 45 + (width - 130), y + 140, width - (width - 130) - 45 - 5, 20);
		EditorUI::Text("Green", Font::Instance, x + 5, y + 110, 35.0f, 20, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
		EditorUI::SliderFloat(&color->g, 0.0f, 1.0f, x + 45, y + 110, width - 130, 20);
		EditorUI::DragFloat(&color->g, x + 45 + (width - 130), y + 110, width - (width - 130) - 45 - 5, 20);
		EditorUI::Text("Blue", Font::Instance, x + 5, y + 80, 35.0f, 20, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
		EditorUI::SliderFloat(&color->b, 0.0f, 1.0f, x + 45, y + 80, width - 130, 20);
		EditorUI::DragFloat(&color->b, x + 45 + (width - 130), y + 80, width - (width - 130) - 45 - 5, 20);
		EditorUI::Text("Alpha", Font::Instance, x + 5, y + 50, 35.0f, 20, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
		EditorUI::SliderFloat(&color->a, 0.0f, 1.0f, x + 45, y + 50, width - 130, 20);
		EditorUI::DragFloat(&color->a, x + 45 + (width - 130), y + 50, width - (width - 130) - 45 - 5, 20);

		// Clamp values
		color->r = Math::Clamp(color->r, 0.0f, 1.0f);
		color->g = Math::Clamp(color->g, 0.0f, 1.0f);
		color->b = Math::Clamp(color->b, 0.0f, 1.0f);
		color->a = Math::Clamp(color->a, 0.0f, 1.0f);

		if (EditorUI::Button("Cancel", x + 20, y + 20, width - 100, 25))
		{
			*color = originalColor;
			OnColorReset();
			Dispose();
			return;
		}

		if (temp != *color)
		{
			OnColorChange();
		}
	}

}