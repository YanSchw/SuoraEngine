#pragma once
#include "../MinorTab.h"
#include <Suora.h>

namespace Suora
{
	class Profiler : public MinorTab
	{
	public:
		Profiler(MajorTab* majorTab);
		~Profiler();

		Texture2D* TexArrowRight = nullptr;
		Texture2D* TexArrowDown = nullptr;

		virtual void Render(float deltaTime) override;

		void DrawResult(float x, float& y, InstrumentationResult& result);

		int64_t _EntryCount = 0;
	};
}