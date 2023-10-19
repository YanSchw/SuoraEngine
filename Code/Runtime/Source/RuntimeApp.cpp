#include <Suora.h>
#include <Suora/Core/EntryPoint.h>
#include "Suora/Core/Base.h"
#include "Suora/Renderer/GraphicsContext.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/RenderPipeline.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Assets/SuoraProject.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"

class Runtime : public Suora::Application
{
public:
	Suora::Ref<Framebuffer> m_Framebuffer;
	Suora::Ref<Window> m_StandaloneWindow;

	Runtime()
	{
		extern void Modules_Init();
		Modules_Init();
		
		m_StandaloneWindow = Suora::Ref<Window>(CreateAppWindow(WindowProps()));

		GraphicsContext* context = (GraphicsContext*)(m_StandaloneWindow->GetGraphicsContext());
		context->MakeCurrent();

		{
			FramebufferSpecification spec;
			spec.Width = 1920;
			spec.Height = 1080;
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGBA8);
			spec.Attachments.Attachments.push_back(FramebufferTextureFormat::Depth);
			m_Framebuffer = Framebuffer::Create(spec);
		}
		AssetManager::HotReload();
		SuoraVerify(ProjectSettings::Get());

		m_StandaloneWindow->SetTitle(ProjectSettings::GetProjectName());

		Engine::Get()->CreateGameInstance();
		if (Level* defaultLevel = ProjectSettings::Get()->m_DefaultLevel)
		{
			World* world = Engine::Get()->GetGameInstance()->LoadLevel(defaultLevel);
			Engine::Get()->GetGameInstance()->SwitchToWorld(world);
		}
		else
		{
			SUORA_ERROR(LogCategory::Gameplay, "Could not load Default Level!");
		}
		m_StandaloneWindow->SetCursorLocked(true);
		//m_StandaloneWindow->SetFullscreen(true);
		m_StandaloneWindow->SetVSync(false);
	}

	~Runtime()
	{
	}

	void Update(float deltaTime) override
	{
		m_StandaloneWindow->OnUpdate();

		// Rendering
		SuoraAssert(Engine::Get());
		if (GameInstance* game = Engine::Get()->GetGameInstance())
		{
			if (World* world = game->GetCurrentWorld())
			{
				if (CameraNode* camera = world->GetMainCamera())
				{
					camera->SetAspectRatio((float)m_StandaloneWindow->GetWidth() / (float)m_StandaloneWindow->GetHeight());
					camera->SetPerspectiveFarClip(camera->GetPerspectiveFarClip());

					RenderingParams RParams;
					Engine::Get()->GetRenderPipeline()->Render(*m_Framebuffer.get(), *world, *camera, RParams);
				}
			}
		}
		m_Framebuffer->Bind();
		m_Framebuffer->Unbind();

		RenderCommand::SetViewport(0, 0, m_StandaloneWindow->GetWidth(), m_StandaloneWindow->GetHeight());
		m_Framebuffer->DrawToScreen(0, 0, m_StandaloneWindow->GetWidth(), m_StandaloneWindow->GetHeight());

		if (NativeInput::GetKey(Key::RightAlt) && NativeInput::GetKeyDown(Key::Enter))
		{
			m_StandaloneWindow->SetFullscreen(!m_StandaloneWindow->IsFullscreen());
			m_StandaloneWindow->SetVSync(true);
		}
	}

};

Suora::Application* Suora::CreateApplication()
{
	return new Runtime();
}
