#pragma once
#include <vector>
#include "Suora/Renderer/Texture.h"
#include "Suora/Assets/Material.h"

namespace Suora 
{
	struct Vertex;
	struct MaterialSlots;
	class Mesh;
	class MeshNode;
	class Material;
	class MeshComponent;
	class Font;
	class CameraNode;

	class Renderer3D
	{
	public:
		static void DrawMeshNode(CameraNode* camera, MeshNode* node, MaterialType type = MaterialType::Material, int32_t meshID = 0);
		static void DrawMesh(CameraNode* camera, const glm::mat4& transform, Mesh& mesh, Material* material, MaterialType type = MaterialType::Material);
		static void DrawMesh(CameraNode* camera, const glm::mat4& transform, Mesh& mesh, const MaterialSlots& materials, MaterialType type = MaterialType::Material);
	};

}
