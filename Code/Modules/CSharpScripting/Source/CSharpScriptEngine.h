#pragma once

#include "Suora/Core/EngineSubSystem.h"
#include "CSharpScriptEngine.generated.h"

namespace Coral
{
	class HostInstance;
	class AssemblyLoadContext;
}

namespace Suora
{
	class CSharpScriptEngine : public EngineSubSystem
	{
		SUORA_CLASS(548798543356);
	public:

		virtual bool Initialize() override;
		virtual void Shutdown() override;

		virtual void Tick(float deltaTime) override;

		bool IsDotNetSDKPresent();
		void CompileCSProj(const std::filesystem::path& csproj);

		void ReloadAssemblies();

		bool IsEditor();

	private:
		Ref<Coral::HostInstance> m_HostInstance;
		Ref<Coral::AssemblyLoadContext> m_AssemblyLoadContext;
	};
}