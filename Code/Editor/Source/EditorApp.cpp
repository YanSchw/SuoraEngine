#include <Suora.h>
#include <Suora/Core/EntryPoint.h>

#include "Suora/Core/Application.h"
#include "Suora/Core/Window.h"

#include "Suora/Core/Object/Object.h"

#include "Suora/Editor/EditorWindow.h"
#include "Suora/Editor/EditorUI.h"
#include "Suora/Editor/Util/EditorPreferences.h"

extern void Modules_Init();

namespace Suora 
{

	class Editor : public Application
	{
	private:
		Array<EditorWindow*> EditorWindows;
	public:

		bool m_StopPIE_Flag = false;

		Editor(const ApplicationParams& params)
			: Application(params)
		{
			AssetManager::s_AssetHotReloading = true;

			EditorWindows.Add(new EditorWindow());

			AssetManager::LoadAsset(AssetManager::GetEngineAssetPath() + "/EditorPreferences.editor");

			Font::Instance = AssetManager::GetAssetByName<Font>("Jetbrains Mono NL Medium.font");

			Modules_Init();
			
			EditorUI::Init();
		}

		virtual void Update(float deltaTime) override
		{
			EditorUI::Tick(deltaTime);

			if (m_StopPIE_Flag)
			{
				for (EditorWindow* window : EditorWindows)
				{
					window->StopPlayInEditor();
				}
				m_StopPIE_Flag = false;
			}

			for (EditorWindow* window : EditorWindows)
			{
				window->Update(deltaTime);
			}

		}

		virtual void Close() override
		{
			m_StopPIE_Flag = true;
		}

	};

	Application* CreateApplication()
	{
		ApplicationParams params;
		params.IsEditor = true;
		return new Editor(params);
	}

}
