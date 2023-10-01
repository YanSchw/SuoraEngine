#include "Precompiled.h"
#include "Engine.h"
#include "Application.h"
#include "NativeInput.h"

#include "../Assets/AssetManager.h"
#include "Suora/Assets/SuoraProject.h"
#include "Suora/GameFramework/GameInstance.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/NodeScript/Scripting/ScriptVM.h"
#include "Suora/Physics/PhysicsEngine.h"

namespace Suora
{
	static void LogPreEngineCreateInfo()
	{
		for (int i = 0; i < NativeFunction::s_NativeFunctions.size(); i++)
		{
			NativeFunction* func = NativeFunction::s_NativeFunctions[i];
			SUORA_LOG(LogCategory::Core, LogLevel::Info, "Registered NativeFunction: {0} {1}", func->m_ReturnType, func->m_Label);
		}
	}

	Ref<Engine> Engine::Create()
	{
		LogPreEngineCreateInfo();
		Ref<Engine> engine = Ref<Engine>(new Engine());

		engine->m_RootPath = std::filesystem::current_path(); 
		SUORA_LOG(LogCategory::Core, LogLevel::Info, "Seeking Engine/Project RootPath from: {0}", (engine->m_RootPath /= "Content").string());
		while (!std::filesystem::is_directory(engine->m_RootPath.append("Content")))
		{
			engine->m_RootPath = engine->m_RootPath.parent_path().parent_path();
		}
		engine->m_RootPath = engine->m_RootPath.parent_path();
		SUORA_LOG(LogCategory::Core, LogLevel::Info, "Found Engine/Project RootPath in: {0}", engine->m_RootPath.string());

		AssetManager::Initialize(FilePath(engine->m_RootPath).append("Content"));

		// Dark magic.....
		Class::None.GetClassDefaultObject(true);

		engine->m_PreviousTime = std::chrono::steady_clock::now();

		return engine;
	}

	Engine::~Engine()
	{
		m_GameInstance = nullptr;
	}

	GameInstance* Engine::GetGameInstance() const
	{
		return m_GameInstance.get();
	}

	void Engine::CreateGameInstance()
	{
		CreateGameInstance(ProjectSettings::Get()->m_GameInstanceClass);
	}
	void Engine::CreateGameInstance(const Class& cls)
	{
		m_GameInstance = (Ref<GameInstance>)(Cast<GameInstance>(New(cls)));
		m_GameInstance->m_Engine = this;
		m_GameInstance->Initialize();
	}
	void Engine::DisposeGameInstance()
	{
		m_GameInstance = nullptr;
	}

	RenderPipeline* Engine::GetRenderPipeline()
	{
		if (!m_RenderPipeline)
		{
			m_RenderPipeline = Ref<RenderPipeline>(New<RenderPipeline>());
			m_RenderPipeline->Initialize();
		}
		return m_RenderPipeline.get();
	}

	std::string Engine::GetRootPath() const
	{
		return m_RootPath.string();
	}

	float Engine::GetDeltaTime() const
	{
		return m_DeltaTime;
	}

	void Engine::Tick()
	{
		// Updating
		auto currentTime = std::chrono::steady_clock::now();
		auto elapsed = currentTime.time_since_epoch() - m_PreviousTime.time_since_epoch();

		m_DeltaTime = elapsed.count() / 1000000000.0f;

		// Cap the Framerate in Application

		m_PreviousTime = currentTime;
		Update(m_DeltaTime);

	}
	void Engine::Update(float deltaTime)
	{
		m_FramesThisSecond++;
		m_FrameDeltaTimeAccumulator += deltaTime;
		if (m_FrameDeltaTimeAccumulator >= 1.0f)
		{
			m_FPS = m_FramesThisSecond;
			m_FramesThisSecond = 0;
			m_FrameDeltaTimeAccumulator = 0.0f;
		}

		NativeInput::Tick(deltaTime);

		ScriptEngine::CleanUp();

		AssetManager::Update(deltaTime);

		if (m_GameInstance)
		{
			m_GameInstance->Update(deltaTime);
		}
	}
	void Engine::Exit()
	{
		Application::Get().Close();
	}
	Engine* Engine::Get()
	{
		return Application::Get().m_Engine.get();
	}
}