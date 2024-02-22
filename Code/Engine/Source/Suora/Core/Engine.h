#pragma once

#include <iostream>
#include <memory>
#include <chrono>
#include <filesystem>
#include "Suora/Common/Filesystem.h"
#include "Suora/Core/Object/Pointer.h"

namespace Suora::Physics
{
	class PhysicsEngine;
}

namespace Suora
{
	class World;
	class AssetManager;
	class GameInstance;
	class RenderPipeline;
	class EngineSubSystem;

	/** The Engine controls its Subsystems and GameFlow */
	class Engine final
	{
		Array<Ref<EngineSubSystem>> m_Subsystems;
		Ref<GameInstance> m_GameInstance;
		Ref<RenderPipeline> m_RenderPipeline = nullptr;
		Ref<Physics::PhysicsEngine> m_PhysicsEngine = nullptr;
		std::chrono::time_point<std::chrono::steady_clock> m_PreviousTime;
		FilePath m_RootPath;

		float m_DeltaTime = 0.0f;

	public:
		static Ref<Engine> Create();
		~Engine();

		GameInstance* GetGameInstance() const;
		void CreateGameInstance();
		void CreateGameInstance(const Class& cls);
		void DisposeGameInstance();
		RenderPipeline* GetRenderPipeline();
		Physics::PhysicsEngine* GetPhysicsEngine() const;
		Array<Ref<EngineSubSystem>> GetEngineSubsystems() const;

		String GetRootPath() const;
		float GetDeltaTime() const;

		void Tick();
		void Update(float deltaTime);
		void Exit();
		static Engine* Get();
		static int32_t GetFramerate() { return Get()->m_FPS; }

	private:
		int32_t m_FPS = 0;
		int32_t m_FramesThisSecond = 0;
		float m_FrameDeltaTimeAccumulator = 0.0f;
	};


}