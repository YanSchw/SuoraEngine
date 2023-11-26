#include "Precompiled.h"
#include "Material.h"
#include "ShaderGraph.h"
#include "Mesh.h"
#include "../Renderer/Shader.h"
#include "Platform/Platform.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/GameFramework/Nodes/MeshNode.h"

#include "Texture2D.h"
#include "Suora/Renderer/Texture.h"

namespace Suora
{

	Material::Material()
	{

	}

	void Material::PreInitializeAsset(Yaml::Node& root)
	{
		Super::PreInitializeAsset(root);

		m_UUID = root["UUID"].As<String>();
	}

	void Material::InitializeAsset(Yaml::Node& root)
	{
		Super::InitializeAsset(root);

		Yaml::Node& material = root["Material"];

		if (!this->IsA<ShaderGraph>())
		{
			SuoraID uuid = SuoraID(material["ShaderGraph"].As<String>());
			// TODO: Fix the Asset Asignment !
			m_ShaderGraph = uuid.GetString() != "NULL" ? AssetManager::GetAsset<ShaderGraph>(uuid) : nullptr;
		}

		int index = 0;
		while (true)
		{
			Yaml::Node& uniform = material["Uniform_" + std::to_string(index++)];
			if (uniform.IsNone()) break;

			const ShaderGraphDataType type = (ShaderGraphDataType)uniform["m_Type"].As<int64_t>();
			const String label = uniform["m_Label"].As<String>();
			switch (type)
			{
			case ShaderGraphDataType::Float:
				m_UniformSlots.Add(UniformSlot(type, label, uniform["m_Float"].As<float>()));
				break;
			case ShaderGraphDataType::Vec3:
				m_UniformSlots.Add(UniformSlot(type, label, Vec::FromString<Vec3>(uniform["m_Vec3"].As<String>())));
				break;
			case ShaderGraphDataType::Texture2D:
				m_UniformSlots.Add(UniformSlot(type, label, AssetManager::GetAsset<Texture2D>(uniform["m_Texture2D"].As<String>())));
				break;
			case ShaderGraphDataType::None:
			default:
				SuoraError("Material::InitializeAsset - Unkown Type!");
			}
		}

		String str_BackfaceCulling = material["m_BackfaceCulling"].As<String>();
		if (str_BackfaceCulling == "true" || str_BackfaceCulling == "false") m_BackfaceCulling = (str_BackfaceCulling == "true");
	}

	void Material::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		Yaml::Node& material = root["Material"];
		material = Yaml::Node();

		material["ShaderGraph"] = GetShaderGraph() ? GetShaderGraph()->m_UUID.GetString() : "NULL";

		for (int index = 0; index < m_UniformSlots.Size(); index++)
		{
			Yaml::Node& uniform = material["Uniform_" + std::to_string(index)];
			uniform["m_Type"] = std::to_string((int64_t)m_UniformSlots[index].m_Type);
			uniform["m_Label"] = m_UniformSlots[index].m_Label;

			uniform["m_Float"] = std::to_string(m_UniformSlots[index].m_Float);
			uniform["m_Vec3"] = Vec::ToString(m_UniformSlots[index].m_Vec3);
			uniform["m_Texture2D"] = (m_UniformSlots[index].m_Texture2D) ? m_UniformSlots[index].m_Texture2D->m_UUID.GetString() : "0";
			// TODO: Diffrent Data Types.
		}


		material["m_BackfaceCulling"] = m_BackfaceCulling ? "true" : "false";
	}

	void Material::SetShaderGraph(ShaderGraph* shaderGraph)
	{
		m_ShaderGraph = shaderGraph;
		m_UniformSlots = m_ShaderGraph->m_UniformSlots;
	}
	ShaderGraph* Material::GetShaderGraph() const
	{
		return m_ShaderGraph;
	}

	void Material::ApplyUniforms(MaterialType type)
	{
		SUORA_ASSERT(GetShaderGraph(), "No ShaderGraph asigned!");
		if (!GetShaderGraph()) return;
		
		Shader* shader = nullptr;
		switch (type)
		{
		case MaterialType::Material:
			shader = GetShaderGraph()->GetShader();
			break;
		case MaterialType::Depth:
			shader = GetShaderGraph()->GetDepthShader();
			break;
		case MaterialType::FlatWhite:
			shader = GetShaderGraph()->GetFlatWhiteShader();
			break;
		case MaterialType::ObjectID:
			shader = GetShaderGraph()->GetIDShader();
			break;
		case MaterialType::Ilum:
			SuoraWarn("Material::ApplyUniforms(): MaterialType not implemented yet!");
			shader = GetShaderGraph()->GetShader();
			break;
		case MaterialType::None:
		default:
			SuoraVerify(false, "Material::ApplyUniforms(): MaterialType not implemented!");
			break;
		}
		shader->Bind();

		if (!this->IsA<ShaderGraph>())
		{
			//GetShaderGraph()->ApplyUniforms(type);
		}

		int TextureSlot = 0;

		// Apply Uniform Slots
		for (UniformSlot& slot : m_UniformSlots)
		{
			switch (slot.m_Type)
			{
			case ShaderGraphDataType::Float:
				shader->SetFloat(slot.m_Label, slot.m_Float);
				break;
			case ShaderGraphDataType::Vec3:
				shader->SetFloat3(slot.m_Label, slot.m_Vec3);
				break;
			case ShaderGraphDataType::Texture2D:
				if (slot.m_Texture2D) slot.m_Texture2D->GetTexture()->Bind(TextureSlot++);
				break;
			case ShaderGraphDataType::None:
			default:
				SuoraError("Unkown Type!");
			}
		}

	}
	UniformSlot* Material::GetUniformSlot(const String& label)
	{
		for (auto& It : m_UniformSlots)
		{
			if (It.m_Label == label)
			{
				return &It;
			}
		}

		return nullptr;
	}

	bool Material::IsDeferred() const
	{
		return GetShaderGraph() ? m_ShaderGraph->IsDeferred() : false;
	}


	MaterialSlots::MaterialSlots()
	{
		//Materials.Add(nullptr);
	}
	MaterialSlots::MaterialSlots(Material* material)
	{
		Materials.Add(material);
	}

	void MaterialSlots::TickSlots()
	{
		if (!Owner)
		{
			SuoraError("MaterialSlots::TickSlots(): MaterialSlots does not have an owner!");
			return;
		}

		if (Mesh* mesh = Owner->As<Mesh>())
		{
			// Have as many slots as the Mesh has SubMeshes
			const int64_t count = mesh->m_Submeshes.Size() > 0 ? mesh->m_Submeshes.Size() : 1;
			while (Materials.Size() < count) Materials.Add(nullptr);
			while (Materials.Size() > count) Materials.RemoveAt(Materials.Last());
		}
		else if (MeshNode* node = Owner->As<MeshNode>())
		{
			// Have as many slots as the MeshNodes Mesh has SubMeshes
			const int64_t count = node->GetMesh() ? (node->GetMesh()->m_Submeshes.Size() > 0 ? node->GetMesh()->m_Submeshes.Size() : 1) : 0;
			while (Materials.Size() < count) Materials.Add(nullptr);
			while (Materials.Size() > count) Materials.RemoveAt(Materials.Last());
		}
		else
		{
			SuoraVerify(false);
		}

	}

	void MaterialSlots::operator=(const MaterialSlots& other)
	{
		OverwritteMaterials = other.OverwritteMaterials;
		Materials = other.Materials;
	}

}