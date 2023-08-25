#include "Precompiled.h"
#include "SpriteNode.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/Material.h"
#include "Suora/Renderer/Renderer3D.h"
#include "Suora/Renderer/Shader.h"

namespace Suora
{

	SpriteNode::SpriteNode()
	{
		mesh = AssetManager::GetAsset<Mesh>(SuoraID("75f466f7-baec-4c5a-a23b-a5e3dc3d22bc"));
		materials = AssetManager::GetAsset<Material>(SuoraID("49785g48345839"));
		m_CastShadow = false;
	}
	SpriteNode::~SpriteNode()
	{
	}

	void SpriteNode::Begin()
	{
	}
	void SpriteNode::Render()
	{

	}

}