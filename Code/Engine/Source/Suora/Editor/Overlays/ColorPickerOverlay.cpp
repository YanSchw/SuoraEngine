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
		SuoraVerify(color);
		if (WasMousePressedOutsideOfOverlay())
		{
			OnColorChange();
			Dispose();
			return;
		}

		Color temp = *color;

		RenderColorCircle();

		EditorUI::Text("R", Font::Instance, x + 15, y + 140, 35.0f, 20, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
		EditorUI::SliderFloat(&color->r, 0.0f, 1.0f, x + 45, y + 140, 225.0f, 20);
		EditorUI::DragFloat(&color->r, x + 45 + 225.0f, y + 140, 125.0f, 20);

		EditorUI::Text("G", Font::Instance, x + 15, y + 110, 35.0f, 20, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
		EditorUI::SliderFloat(&color->g, 0.0f, 1.0f, x + 45, y + 110, 225.0f, 20);
		EditorUI::DragFloat(&color->g, x + 45 + 225.0f, y + 110, 125.0f, 20);

		EditorUI::Text("B", Font::Instance, x + 15, y + 80, 35.0f, 20, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
		EditorUI::SliderFloat(&color->b, 0.0f, 1.0f, x + 45, y + 80, 225.0f, 20);
		EditorUI::DragFloat(&color->b, x + 45 + 225.0f, y + 80, 125.0f, 20);
		
		
		EditorUI::Text("Alpha", Font::Instance, x + 5, y + 50, 35.0f, 20, 24.0f, Vec2(-1.0f, 0.0f), Color(1.0f));
		EditorUI::SliderFloat(&color->a, 0.0f, 1.0f, x + 45, y + 50, 225.0f, 20);
		EditorUI::DragFloat(&color->a, x + 45 + 225.0f, y + 50, 125.0f, 20);

		// Clamp values
		color->r = Math::Clamp(color->r, 0.0f, 1.0f);
		color->g = Math::Clamp(color->g, 0.0f, 1.0f);
		color->b = Math::Clamp(color->b, 0.0f, 1.0f);
		color->a = Math::Clamp(color->a, 0.0f, 1.0f);

		if (EditorUI::Button("Cancel", x + width - 150, y + 20, 125, 25))
		{
			*color = originalColor;
			OnColorReset();
			Dispose();
			return;
		}
		if (EditorUI::Button("Okay", x + width - 300, y + 20, 125, 25, EditorUI::ButtonParams::Highlight()))
		{
			Dispose();
			return;
		}

		if (temp != *color)
		{
			OnColorChange();
		}
	}

	void ColorPickerOverlay::RenderColorCircle()
	{
		// hsv.x:  0.0 - 360.0
		const Color hsv = Math::ConvertRGB2HSV(*color);
		float theta = hsv.x * Math::PI / 180.0;
		float dx = 100.0f * hsv.y * glm::cos(theta);
		float dy = 100.0f * hsv.y * glm::sin(theta);

		EditorUI::DrawRect(x + 25, y + height - 255, 200, 200, 4, *color);

		RenderCommand::SetViewport(x + 25, y + height - 255, 200, 200);
		RenderPipeline::__GetFullscreenQuad()->Bind();
		s_ColorCircleShader->Bind();
		RenderCommand::DrawIndexed(RenderPipeline::__GetFullscreenQuad());

		const uint32 circleCenterX = x + 25 + 100;
		const uint32 circleCenterY = y + height - 255 + 100;
		EditorUI::DrawTexturedRect(Icon::Actor, circleCenterX - dx - 12.5f, circleCenterY - dy - 12.5f, 25.0f, 25.0f, 0.0f, Color(1.0f));

		const float distanceToCenter = glm::distance(Vec2((float)circleCenterX, (float)circleCenterY), EditorUI::GetInput());
		if (distanceToCenter < 100.0f && NativeInput::GetMouseButton(Mouse::ButtonLeft))
		{
			const Vec2 normalizedMousePosInCircle = Vec2(EditorUI::GetInput().x - circleCenterX, EditorUI::GetInput().y - circleCenterY) / 100.0f;

			float ang = glm::atan(normalizedMousePosInCircle.y, normalizedMousePosInCircle.x); // <-pi,+pi>
			float angDeg = Math::Remap(ang, -Math::PI, +Math::PI, 0.0f, 360.0f);
			*color = Math::ConvertHSV2RGB(Color(angDeg, distanceToCenter / 100.0f, hsv.z, color->a));
		}
	}

}