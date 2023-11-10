#include "Precompiled.h"
#include "SuoraProject.h"
#include "Suora/GameFramework/GameInstance.h"
#include "Suora/GameFramework/InputModule.h"
#include "Suora/Assets/Level.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/Texture2D.h"
#include "Suora/Renderer/RenderPipeline.h"

namespace Suora
{

	ProjectSettings::ProjectSettings()
	{
		m_InputSettings = Ref<InputSettings>(new InputSettings());
	}

	void ProjectSettings::PreInitializeAsset(const String& str)
	{
		Super::PreInitializeAsset(str);
		ClearFlag(AssetFlags::WasPreInitialized);

		Yaml::Node root;
		Yaml::Parse(root, str);
		m_UUID = root["UUID"].As<String>();
	}

	void ProjectSettings::InitializeAsset(const String& str)
	{
		Super::InitializeAsset(str);
		ClearFlag(AssetFlags::WasInitialized);

		Yaml::Node root;
		Yaml::Parse(root, str);
		Yaml::Node& settings = root["Settings"];

		m_EnginePath = settings["Engine"]["Path"].IsNone() ? "" : settings["Engine"]["Path"].As<String>();
		m_IsNativeProject = settings["Engine"]["m_IsNativeProject"].As<String>() == "true";

		if (s_SeekingProjectSettings)
		{
			return;
		}

		if (!settings["Engine"]["m_EditorStartupAsset"].IsNone())
		{
			const String id = settings["Engine"]["m_EditorStartupAsset"].As<String>();
			m_EditorStartupAsset = (id != "0") ? AssetManager::GetAsset<Asset>(SuoraID(id)) : nullptr;
		}

		m_TargetFramerate = settings["Rendering"]["m_TargetFramerate"].IsNone() ? 60.0f : std::stof(settings["Rendering"]["m_TargetFramerate"].As<String>());
		m_EnableDeferredRendering = settings["Rendering"]["m_EnableDeferredRendering"].IsNone() ? true : settings["Rendering"]["m_EnableDeferredRendering"].As<String>() == "true";

		const Class gameInstanceClass = Class::FromString(settings["Game"]["m_GameInstanceClass"].As<String>());
		m_GameInstanceClass = gameInstanceClass != Class::None ? gameInstanceClass : GameInstance::StaticClass();
		m_DefaultLevel = settings["Game"]["m_DefaultLevel"].IsNone() ? nullptr : AssetManager::GetAsset<Level>(SuoraID(settings["Game"]["m_DefaultLevel"].As<String>()));

		m_InputSettings = Ref<InputSettings>(new InputSettings());
		m_InputSettings->Deserialize(settings["InputSettings"]);

		m_ProjectIconTexture = settings["Game"]["m_ProjectIconTexture"].As<String>() != "NULL" ? AssetManager::GetAsset<Texture2D>(SuoraID(settings["Game"]["m_ProjectIconTexture"].As<String>())) : nullptr;
	}

	void ProjectSettings::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		Yaml::Node& settings = root["Settings"];
		settings = Yaml::Node();

		settings["Engine"]["VersionMajor"] = std::to_string(SUORA_VERSION_MAJOR);
		settings["Engine"]["VersionMinor"] = std::to_string(SUORA_VERSION_MINOR);
		settings["Engine"]["VersionPatch"] = std::to_string(SUORA_VERSION_PATCH);
		if (m_EnginePath != "") settings["Engine"]["Path"] = m_EnginePath;
		settings["Engine"]["m_IsNativeProject"] = m_IsNativeProject ? "true" : "false";
		settings["Engine"]["m_EditorStartupAsset"] = m_EditorStartupAsset ? m_EditorStartupAsset->m_UUID.GetString() : "0";

		settings["Rendering"]["m_TargetFramerate"] = std::to_string(m_TargetFramerate);
		settings["Rendering"]["m_EnableDeferredRendering"] = m_EnableDeferredRendering ? "true" : "false";

		settings["Game"]["m_GameInstanceClass"] = m_GameInstanceClass.GetClass().ToString();
		settings["Game"]["m_DefaultLevel"] = m_DefaultLevel ? m_DefaultLevel->m_UUID.GetString() : "0";

		m_InputSettings->Serialize(settings["InputSettings"]);

		settings["Game"]["m_ProjectIconTexture"] = m_ProjectIconTexture ? m_ProjectIconTexture->m_UUID.GetString() : "NULL";
	}

	String ProjectSettings::GetEnginePath() const
	{
		return m_EnginePath;
	}

	void ProjectSettings::SetEnginePath(const String& path)
	{
		m_EnginePath = path;
	}

	ProjectSettings* ProjectSettings::Get()
	{
		return AssetManager::GetFirstAssetOfType<ProjectSettings>();
	}
	String ProjectSettings::GetProjectName()
	{
		return Get()->m_Path.stem().string();
	}

}