#include "EditorPreferences.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/GameFramework/Nodes/Light/PointLightNode.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/Nodes/PostProcess/PostProcessNode.h"
#include "Suora/GameFramework/Nodes/DecalNode.h"
#include "Suora/GameFramework/Nodes/ShapeNodes.h"
#include "Suora/GameFramework/Nodes/MeshNode.h"
#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Suora/GameFramework/Nodes/OrganizationNodes.h"

namespace Suora
{

	Color EditorPreferences::GetAssetClassColor(NativeClassID AssetClassID)
	{
		static std::unordered_map<NativeClassID, Color> ColorMap;
		if (ColorMap.find(AssetClassID) != ColorMap.end()) return ColorMap[AssetClassID];

		ColorMap[AssetClassID] = Color(((float)(rand() % 256)) / 255.0f, ((float)(rand() % 256)) / 255.0f, ((float)(rand() % 256)) / 255.0f, 1.0f);
		return ColorMap[AssetClassID];
	}

	EditorPreferences* EditorPreferences::Get()
	{
		EditorPreferences* ptr = AssetManager::GetFirstAssetOfType<EditorPreferences>();
		return ptr ? ptr : AssetManager::CreateAsset<EditorPreferences>("EditorPreferences", AssetManager::GetEngineAssetPath());
	}

	void EditorPreferences::PreInitializeAsset(const String& str)
	{
		Super::PreInitializeAsset(str);
		Yaml::Node root;
		Yaml::Parse(root, str);

		int i = 0;
		while (true)
		{
			Yaml::Node& node = root["m_AllCachedProjectPaths"][std::to_string(i++)];
			if (node.IsNone()) break;
			if (!m_AllCachedProjectPaths.Contains(node.As<String>())) m_AllCachedProjectPaths.Add(node.As<String>());
		}
	}
	void EditorPreferences::InitializeAsset(const String& str)
	{
		Super::InitializeAsset(str);
		Yaml::Node root;
		Yaml::Parse(root, str);

	}
	void EditorPreferences::Serialize(Yaml::Node& root)
	{
		root = Yaml::Node();
		for (int i = 0; i < m_AllCachedProjectPaths.Size(); i++)
		{
			root["m_AllCachedProjectPaths"][std::to_string(i)] = m_AllCachedProjectPaths[i];
		}

	}

	uint32_t EditorPreferences::GetAssetFileSize()
	{
		return 0;
	}

	EditorPreferences::EditorPreferences()
	{
	}

}