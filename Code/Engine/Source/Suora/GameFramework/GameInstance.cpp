#include "Precompiled.h"
#include "GameInstance.h"
#include "World.h"
#include "Suora/Core/Engine.h"
#include "Suora/Assets/Level.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "InputModule.h"

namespace Suora
{

	GameInstance::GameInstance()
	{
		
	}
	void GameInstance::Initialize()
	{
		{
			FramebufferSpecification spec;
			spec.Width = 1920;
			spec.Height = 1080;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGB32F);
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_Framebuffer = Framebuffer::Create(spec);
		}

		Array<Class> implementations = Class::GetSubclassesOf(GameModule::StaticClass());
		for (const auto& It : implementations)
		{
			auto Impl = Ref<GameModule>(New(It)->As<GameModule>());
			if (Impl)
			{
				SUORA_LOG(LogCategory::Gameplay, LogLevel::Trace, "Created GameModule: {0}", It.GetClassName());
				m_GameModules.Add(Impl);
			}
		}
		for (auto& It : m_GameModules)
		{
			It->Initialize();
		}
	}
	GameInstance::~GameInstance()
	{
		m_CurrentWorld = nullptr;
		for (int32_t i = 0; i < m_Worlds.Size(); i++)
		{
			delete m_Worlds[i];
		}
	}

	World* GameInstance::LoadLevel(Blueprint* level)
	{
		if (!level) return nullptr;

		World* world = CreateWorld();

		// Todo: false, false but Actorlayer has to be stored in the Node
		Node* root = level->CreateInstance(false)->As<Node>();
		root->InitializeNode(*world);
		root->m_IsActorLayer = true;

		world->m_SourceLevel = level->As<Level>();

		return world;
	}

	World* GameInstance::CreateWorld()
	{
		World* world = new World();
		world->m_GameInstance = this;
		m_Worlds.Add(world);
		return world;
	}

	void GameInstance::SwitchToWorld(World* world)
	{
		for (World* It : m_Worlds)
		{
			if (It == world)
			{
				m_CurrentWorld = world;
				return;
			}
		}

		SUORA_ASSERT(false, "World must be known to GameInstance!");
	}

	World* GameInstance::GetCurrentWorld() const
	{
		return m_CurrentWorld;
	}

	void GameInstance::Update(float deltaTime)
	{
		for (auto& It : m_GameModules)
		{
			It->Update(deltaTime);
		}
		if (m_CurrentWorld)
		{
			m_CurrentWorld->Update(deltaTime);

			if (GetCurrentWorld()->GetMainCamera())
			{
				//GetEngine()->GetRenderPipeline()->Render(*m_Framebuffer.get(), *GetCurrentWorld(), *GetCurrentWorld()->GetMainCamera());
			}
		}
	}

	Framebuffer* GameInstance::GetFinalFramebuffer() const
	{
		return m_Framebuffer.get();
	}

}