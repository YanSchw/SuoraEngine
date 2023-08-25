#include "Precompiled.h"
#include "GameInstance.h"
#include "World.h"
#include "Suora/Core/Engine.h"
#include "Suora/Assets/Level.h"
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
		if (!m_InputModule)
		{
			m_InputModule = Ref<InputModule>(new InputModule());
		}
	}
	GameInstance::~GameInstance()
	{
		m_CurrentWorld = nullptr;
		for (World* world : m_Worlds)
		{
			delete world;
		}
	}

	World* GameInstance::LoadLevel(Blueprint* level)
	{
		if (!level) return nullptr;

		World* world = CreateWorld();

		// Todo: false, false but Actorlayer has to be stored in the Node
		Node* root = level->CreateInstance(true, true)->As<Node>();
		root->InitializeNode(*world);
		root->Implement<IObjectCompositionData>();
		root->GetInterface<IObjectCompositionData>()->m_IsActorLayer = true;

		world->m_SourceLevel = level->As<Level>();

		// Dispatch Begin
		for (Node* node : world->m_WorldNodes)
		{
			node->Begin();
			/*for (Component* component : actor->GetComponents<Component>())
			{
				component->Begin();
			}*/
		}

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
		SUORA_PROFILE_SCOPE("GameInstance::Update(float)");

		m_InputModule->Tick();

		for (auto& It : m_InputModule->m_BlueprintInstanceBindings)
		{
			m_InputModule->ProcessInputForBlueprintInstance(It.first->As<Node>());
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

	InputModule* GameInstance::GetInputModule() const
	{
		return m_InputModule.get();
	}

}