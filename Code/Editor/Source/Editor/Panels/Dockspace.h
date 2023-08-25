#pragma once
#include "inttypes.h"
#include "Suora/Core/Base.h"
#include "Suora/Common/Array.h"

namespace Suora
{
	class MajorTab;
	class MinorTab;

	struct DockingSpace
	{
		float x, y, endX, endY;
		float mouseX = 0, mouseY = 0;
		bool wasLeftMBPressedLastFrame = false;
		Array<Ref<MinorTab>> m_MinorTabs;
		uint32_t m_SelectedMinorTab = 0;

		DockingSpace(float x, float y, float endX, float endY, MajorTab* major) : x(x), y(y), endX(endX), endY(endY)
		{
		}
		float GetWidth()
		{
			return endX - x;
		}
		float GetHeight()
		{
			return endY - y;
		}
	};

}