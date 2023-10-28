#pragma once
#include "Suora/Core/EngineSubSystem.h"
#include "GameInstance.generated.h"

namespace Suora
{
	class Blueprint;
	class InputModule;
	class Framebuffer;
	class Level;
	class World;
	class Engine;

	/** GameInstanced is created and managed by the Engine. All GameState and GameFlow happends here. */
	class GameInstance : public EngineSubSystem
	{
		SUORA_CLASS(89437688);
	public:
		GameInstance();
		virtual void Initialize();
		virtual ~GameInstance();

		FUNCTION(Callable)
		World* LoadLevel(Blueprint* level);
		FUNCTION(Callable)
		World* CreateWorld();
		FUNCTION(Callable)
		void SwitchToWorld(World* world);
		FUNCTION(Callable, Pure)
		World* GetCurrentWorld() const;

		void Update(float deltaTime);

		Engine* GetEngine() const { return m_Engine; }
		Framebuffer* GetFinalFramebuffer() const;
		InputModule* GetInputModule() const;
	private:
		Engine* m_Engine = nullptr;
		Array<World*> m_Worlds;
		Ptr<World> m_CurrentWorld;
		Ref<Framebuffer> m_Framebuffer;
		Ref<InputModule> m_InputModule;

		friend class Engine;
	};
}