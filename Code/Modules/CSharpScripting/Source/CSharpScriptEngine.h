#pragma once

#include "Suora/Core/EngineSubSystem.h"
#include "CSharpScriptEngine.generated.h"

namespace Coral
{
	class HostInstance;
	class AssemblyLoadContext;
	class ManagedAssembly;
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
		void BuildAllCSProjects();
		void CompileCSProj(const std::filesystem::path& csproj);
		void BuildAndReloadAllCSProjects();

		void ReloadAssemblies();
		void ProcessReloadedSuoraAssembly(const Coral::ManagedAssembly& assembly);
		void ProcessReloadedAssembly(const Coral::ManagedAssembly& assembly);

		bool IsEditor();

	private:
		Ref<Coral::HostInstance> m_HostInstance;
		Ref<Coral::AssemblyLoadContext> m_AssemblyLoadContext;
	};
}