#pragma once
#include "Editor/Panels/MinorTab.h"

namespace Suora
{
	class EditorConsolePanel : public MinorTab
	{
	public:
		EditorConsolePanel(MajorTab* majorTab);

		virtual void Render(float deltaTime) override;

		static class Texture* GetLogLevelIcon(const LogLevel level);
	private:
		float m_ScrollY = 0.0f;
		inline static String s_InputCommand;
	};
}