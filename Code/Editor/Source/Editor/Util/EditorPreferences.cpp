#include "EditorPreferences.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/GameFramework/Nodes/Light/PointLightNode.h"
#include "Suora/GameFramework/Nodes/Light/DirectionalLightNode.h"
#include "Suora/GameFramework/Nodes/PostProcess/PostProcessNode.h"
#include "Suora/GameFramework/Nodes/DecalNode.h"
#include "Suora/GameFramework/Nodes/PhysicsNodes.h"
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
	Texture* EditorPreferences::GetNodeIconTexture(const Class& cls)
	{
		NativeClassID native = cls.GetNativeClassID();

		if (native == MeshNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("b14b065c-a2c0-4dc9-9272-ab0415ada141"))->GetTexture();
		if (native == DecalNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("9d81a066-2336-42f5-bf35-7bb1c4c65d66"))->GetTexture();
		if (native == PointLightNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("f789d2bf-dcda-4e30-b2d9-3db979b7c6da"))->GetTexture();
		if (native == DirectionalLightNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("64738d74-08a9-4383-8659-620808d5269a"))->GetTexture();
		if (native == CameraNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("8952ef88-cbd0-41ab-9d3c-d4c4b39a30f9"))->GetTexture();
		if (native == BoxCollisionNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("269931d5-7e60-4934-a89a-26b7993ae0f3"))->GetTexture();
		if (native == SphereCollisionNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("7e43f48b-3dc8-4eab-b91a-b4e2e7999190"))->GetTexture();
		if (native == CapsuleCollisionNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("b7221496-4fc6-4e08-9f23-655d5edfe820"))->GetTexture();
		if (cls.Inherits(PostProcessEffect::StaticClass())) return AssetManager::GetAsset<Texture2D>(SuoraID("9bdeac52-f671-4e0a-9167-aeaa30c47711"))->GetTexture();
		if (cls.Inherits(LevelNode::StaticClass()) || native == LevelNode::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("3578494c-3c74-4aa5-8d34-4d28959a21f5"))->GetTexture();
		if (cls.Inherits(Component::StaticClass()) || native == Component::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"))->GetTexture();
		if (native == FolderNode::StaticClass().GetNativeClassID() || native == FolderNode3D::StaticClass().GetNativeClassID()) return AssetManager::GetAsset<Texture2D>(SuoraID("99898caa-a2b2-4fc4-9db7-5baacaed03e5"))->GetTexture();

		return AssetManager::GetAsset<Texture2D>(SuoraID("ad168979-55cd-408e-afd2-a24cabf26922"))->GetTexture();
	}

	EditorPreferences* EditorPreferences::Get()
	{
		EditorPreferences* ptr = AssetManager::GetFirstAssetOfType<EditorPreferences>();
		return ptr ? ptr : AssetManager::CreateAsset<EditorPreferences>("EditorPreferences", AssetManager::GetAssetRootPath());
	}

	void EditorPreferences::PreInitializeAsset(const std::string& str)
	{
		Super::PreInitializeAsset(str);
		Yaml::Node root;
		Yaml::Parse(root, str);

		int i = 0;
		while (true)
		{
			Yaml::Node& node = root["m_AllCachedProjectPaths"][std::to_string(i++)];
			if (node.IsNone()) break;
			if (!m_AllCachedProjectPaths.Contains(node.As<std::string>())) m_AllCachedProjectPaths.Add(node.As<std::string>());
		}
	}
	void EditorPreferences::InitializeAsset(const std::string& str)
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