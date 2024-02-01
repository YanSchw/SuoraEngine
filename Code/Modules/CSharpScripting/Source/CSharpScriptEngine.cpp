#include "CSharpScriptEngine.h"
#include "CSharpProjectGenerator.h"
#include "Suora/Platform/Platform.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Core/NativeInput.h"

#include "HostInstance.hpp"

namespace Suora
{
    static bool s_SDK_Found = false;
    static LogCategory s_CSharpLog = LogCategory::None;
#define CSHARP_INFO(...)  SUORA_LOG(s_CSharpLog, LogLevel::Info,  __VA_ARGS__)
#define CSHARP_WARN(...)  SUORA_LOG(s_CSharpLog, LogLevel::Warn,  __VA_ARGS__)
#define CSHARP_ERROR(...) SUORA_LOG(s_CSharpLog, LogLevel::Error, __VA_ARGS__)

    void ExceptionCallback(std::string_view InMessage)
    {
        CSHARP_ERROR("Unhandled native exception: {0}", InMessage);
    }

    bool CSharpScriptEngine::Initialize()
    {
        s_CSharpLog = Log::CustomCategory("C#");
        CSHARP_INFO("Initializing C# ScriptEngine");

        s_SDK_Found = IsDotNetSDKPresent();
        if (!s_SDK_Found)
        {
            CSHARP_ERROR(".NET SDK 8 not found on System. C# Script Engine cannot be used!");
            return false;
        }

        CSHARP_INFO("Compiling Coral.Managed.dll");
        CompileCSProj(AssetManager::GetEngineAssetPath() + "/../Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Managed/Coral.Managed.csproj");
        Platform::CopyDirectory(AssetManager::GetEngineAssetPath() + "/../Code/Modules/CSharpScripting/ThirdParty/Coral/Coral.Managed/Coral.Managed.runtimeconfig.json",
                                AssetManager::GetEngineAssetPath() + "/../Build/CSharp/Release");

        CSHARP_INFO("Setting up .NET Core Host");
        auto coralDir = AssetManager::GetEngineAssetPath() + "/../Build/CSharp/Release";
        Coral::HostSettings settings =
        {
            .CoralDirectory = coralDir,
            .ExceptionCallback = ExceptionCallback
        };
        m_HostInstance = CreateRef<Coral::HostInstance>();
        m_HostInstance->Initialize(settings);

        CSHARP_INFO("Initialized C# ScriptEngine");

        ReloadAssemblies();

        return true;
    }
    void CSharpScriptEngine::Shutdown()
    {
        CSHARP_INFO("Shutdown C# ScriptEngine"); 

        if (!s_SDK_Found)
        {
            return;
        }
    }

    void CSharpScriptEngine::Tick(float deltaTime)
    {
        if (!s_SDK_Found)
        {
            return;
        }

        static bool s_WasProjectCSCodeCompiledOnce = false;
        if (!s_WasProjectCSCodeCompiledOnce)
        {
            if (AssetManager::GetProjectAssetPath() != "")
            {
                BuildAndReloadAllCSProjects();
                s_WasProjectCSCodeCompiledOnce = true;
            }
        }

        if (NativeInput::GetKeyDown(Key::F3))
        {
            BuildAndReloadAllCSProjects();
        }
    }

    bool CSharpScriptEngine::IsDotNetSDKPresent()
    {
        return std::filesystem::exists(Coral::HostInstance::GetHostFXRPath());
    }

    void CSharpScriptEngine::BuildAllCSProjects()
    {
        if (!s_SDK_Found)
        {
            return;
        }

        std::filesystem::path path = AssetManager::GetProjectAssetPath() + "/../Code/CSharp";
        if (std::filesystem::exists(path))
        {
            for (auto dir : std::filesystem::directory_iterator(path))
            {
                if (dir.is_directory())
                {
                    CompileCSProj(dir.path() / (dir.path().filename().string() + ".csproj"));
                }
            }
        }

    }

    void CSharpScriptEngine::CompileCSProj(const std::filesystem::path& csproj)
    {
        if (!s_SDK_Found)
        {
            return;
        }
        CSHARP_INFO("Compiling {0}", csproj.string());
        Platform::CommandLine("dotnet build " + csproj.string() + " -c Release");
    }

    void CSharpScriptEngine::BuildAndReloadAllCSProjects()
    {
        if (!s_SDK_Found)
        {
            return;
        }

        CSharpProjectGenerator::GenerateCSProjectFiles();

        BuildAllCSProjects();

        ReloadAssemblies();
    }

    void CSharpScriptEngine::ReloadAssemblies()
    {
        if (!s_SDK_Found)
        {
            return;
        }
        CSHARP_INFO("Reloading C# Script Assemblies...");

        if (m_AssemblyLoadContext)
        {
            m_HostInstance->UnloadAssemblyLoadContext(*m_AssemblyLoadContext);
        }

        auto loadContext = m_HostInstance->CreateAssemblyLoadContext("MainAssembly");
        m_AssemblyLoadContext = CreateRef<Coral::AssemblyLoadContext>();
        *m_AssemblyLoadContext = loadContext;

        std::filesystem::path assemblyDir;
        if (IsEditor())
        {
            assemblyDir = AssetManager::GetProjectAssetPath() + "/../Build/CSharp/Release";
        }
        else
        {
            assemblyDir = AssetManager::GetProjectAssetPath() + "/../Binaries/CSharp";
        }

        if (std::filesystem::exists(assemblyDir))
        {
            for (const auto& file : std::filesystem::directory_iterator(assemblyDir))
            {
                if (file.is_directory())
                    continue;

                std::string filename = file.path().filename().string();
                if (filename == "Coral.Managed.dll")
                    continue;

                if (filename.ends_with(".dll"))
                {
                    CSHARP_INFO("Loading {0}", file.path().string());
                    m_AssemblyLoadContext->LoadAssembly(file.path().string());
                }
                
            }
        }

        CSHARP_INFO("Reloaded C# Script Assemblies!");
    }

    bool CSharpScriptEngine::IsEditor()
    {
        return true;
    }

}