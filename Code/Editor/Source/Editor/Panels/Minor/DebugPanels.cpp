#include "DebugPanels.h"

namespace Suora
{

	static float scrollY = 0;
	static float BaseEntryHeight = 30.0f;
	static float EntryHeight = BaseEntryHeight;

	Profiler::Profiler(MajorTab* majorTab)
		: MinorTab(majorTab)
	{
		Name = "Profiler";
		TexArrowRight = AssetManager::GetAsset<Texture2D>(SuoraID("970c3d0e-c5b0-4a2e-a548-661d9b00d977")); //Texture::Create("../Content/EditorContent/Icons/ArrowRight.png");
		TexArrowDown = AssetManager::GetAsset<Texture2D>(SuoraID("8742cec8-9ee5-4645-b036-577146904b41")); //Texture::Create("../Content/EditorContent/Icons/ArrowDown.png");
	}

	Profiler::~Profiler()
	{

	}

	void Profiler::Render(float deltaTime)
	{
		EditorUI::DrawRect(0, 0, GetWidth(), GetHeight(), 0, Math::Lerp(EditorPreferences::Get()->UiBackgroundColor, EditorPreferences::Get()->UiColor, 0.55f));


		EntryHeight = BaseEntryHeight * EditorPreferences::Get()->UiScale;

		_EntryCount = 0;
		/*for (auto& it : Instrumentor::Get().m_LastFrameResults)
		{
			float y = GetHeight() - EntryHeight;
			if (EditorUI::CategoryShutter(476893 + _EntryCount, "Thread " + std::to_string(*(unsigned int*)&it.first), 0.0f, y, GetWidth(), EntryHeight))
			{
				_EntryCount++;
				for (auto& result : it.second)
				{
					DrawResult(20.0f, y, result);
				}
			}
		}*/

		float scrollDown = GetHeight() - EntryHeight - EntryHeight - 5.0f - EntryHeight * _EntryCount - EntryHeight;
		EditorUI::ScrollbarVertical(GetWidth()-10, 0, 10, GetHeight(), 0, 0, GetWidth(), GetHeight(), 0, scrollDown > 0 ? 0 : Math::Abs(scrollDown), &scrollY);
	}


	void Profiler::DrawResult(float x, float& y, InstrumentationResult& result)
	{
		_EntryCount++; 
		y -= EntryHeight;

		const bool b = EditorUI::CategoryShutter(476893 + _EntryCount, result.Result.Name, x, y, GetWidth() - x, EntryHeight);
		EditorUI::Text(std::to_string(result.Result.ElapsedTime.count() * 0.001) + "ms", Font::Instance, x, y, GetWidth() - x, EntryHeight, 23.0f, Vec2(0.8f, 0.0f), Color(1.0f));

		if (b)
		{
			for (auto& it : result.Children)
			{
				DrawResult(x + 20.0f, y, it);
			}
		}

	}
}