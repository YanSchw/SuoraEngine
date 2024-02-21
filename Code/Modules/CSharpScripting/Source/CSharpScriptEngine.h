#pragma once

#include <future>
#include "Suora/NodeScript/External/ScriptEngine.h"
#include "CSharpScriptEngine.generated.h"

#include "Type.hpp"

namespace Coral
{
	class Type;
	class HostInstance;
	class AssemblyLoadContext;
	class ManagedAssembly;
}

namespace Suora
{
	struct NativeFunction;

	class CSharpScriptEngine : public ScriptEngine
	{
		SUORA_CLASS(548798543356);
	public:

		virtual bool Initialize() override;
		virtual void Shutdown() override;

		virtual void Tick(float deltaTime) override;

		virtual String GetScriptClassDomain() const override;
		virtual Array<Class> GetAllScriptClasses() override;
		virtual Class GetScriptParentClass(String scriptClass) override;
		virtual Object* CreateScriptClassInstance(const String& scriptClass, bool isRootNode) override;
		virtual void InvokeManagedEvent(Object* obj, size_t hash, ScriptStack& stack) override;

		bool IsDotNetSDKPresent();
		void BuildAllCSProjects();
		void CompileCSProj(const std::filesystem::path& csproj);
		void BuildAndReloadAllCSProjects();

		void ReloadAssemblies();
		void ProcessReloadedSuoraAssembly(Coral::ManagedAssembly& assembly);
		void ProcessReloadedAssembly(Coral::ManagedAssembly& assembly);

		bool IsEditor() const;

		/** Leave managedType empty to auto detect it */
		void CreateManagedObject(Object* obj, const String& managedType = "");
		void DestroyManagedObject(Object* obj);

		static CSharpScriptEngine* Get();

		static NativeFunction* GetNativeFunctionFromHash(size_t hash);

		
	private:
		Ref<Coral::HostInstance> m_HostInstance;
		Ref<Coral::AssemblyLoadContext> m_AssemblyLoadContext;

		static void CallNativeFunctionFromManagedHost(uint64_t hash);
	};
}