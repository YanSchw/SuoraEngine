#pragma once
#include "../MinorTab.h"
#include <Suora.h>

namespace Suora
{
	class EditorConsolePanel : public MinorTab
	{
	public:
		EditorConsolePanel(MajorTab* majorTab);
		~EditorConsolePanel();

		virtual void Render(float deltaTime) override;
	private:
		float m_ScrollY = 0.0f;
		inline static std::string s_Input;
	};
}