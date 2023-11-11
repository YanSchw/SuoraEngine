#include "Precompiled.h"
#include "Suora/Renderer/Renderer3D.h"

#include "Suora/Renderer/VertexArray.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Renderer/RenderCommand.h"
#include "Suora/Renderer/Framebuffer.h"
#include "Suora/Renderer/Decima.h"

#include "Suora/GameFramework/Nodes/CameraNode.h"
#include "Suora/GameFramework/Nodes/MeshNode.h"

#include <glm/gtc/matrix_transform.hpp>
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/Mesh.h"
#include "Suora/Assets/Material.h"
#include "Suora/Assets/ShaderGraph.h"
#include "Suora/Assets/Font.h"
#include "Suora/Common/Math.h"
#include "Suora/Core/Application.h"

namespace Suora 
{

	void Renderer3D::DrawMeshNode(CameraNode* camera, MeshNode* node, MaterialType type, int32_t meshID)
	{
		if (!node->IsEnabled()) return;

		if (node->mesh && node->GetMaterials().Materials.Size() > 0 && node->GetMaterials().Materials[0] && node->GetMaterials().Materials[0]->GetShaderGraph())
		{
			if (node->mesh->IsDecimaMesh())
			{
				node->mesh->GetVertexArray(); // To load the Mesh and initiate the decimation process
				if (Decima::m_DecimaMeshes.find(node) == Decima::m_DecimaMeshes.end()) return;
				RenderCommand::SetDepthTest(node->GetMaterials().Materials[0]->m_DepthTest);
				RenderCommand::SetCullingMode(node->GetMaterials().Materials[0]->m_BackfaceCulling ? CullingMode::Backface : CullingMode::None);

				node->GetMaterials().Materials[0]->ApplyUniforms(type);
				node->GetMaterials().Materials[0]->GetShaderGraph()->GetShaderViaType(type)->SetMat4("u_Transform", node->GetTransformMatrix());

				glm::mat4 viewProj = camera->GetProjectionMatrix() * glm::inverse(camera->GetTransformMatrix());
				node->GetMaterials().Materials[0]->GetShaderGraph()->GetShaderViaType(type)->SetMat4("u_ViewProjection", viewProj);

				const glm::mat4 normalMat = glm::transpose(glm::inverse(glm::inverse(camera->GetTransformMatrix()) * node->GetTransformMatrix()));
				node->GetMaterials().Materials[0]->GetShaderGraph()->GetShaderViaType(type)->SetMat4("u_NormalMatrix", normalMat);

				node->GetMaterials().Materials[0]->GetShaderGraph()->GetShaderViaType(type)->SetInt("u_MeshID", meshID);

				RenderCommand::MultiDraw(Decima::m_DecimaMeshes[node]);
			}
			else
			{
				Renderer3D::DrawMesh(camera, node->GetTransformMatrix(), *node->mesh, node->GetMaterials());
			}
		}

	}

	void Renderer3D::DrawMesh(CameraNode* camera, const glm::mat4& transform, Mesh& mesh, Material* material, MaterialType type)
	{
		if (!material) return;
		if (!material->GetShaderGraph()) return;
		if (!material->GetShaderGraph()->GetShaderViaType(type)) return;

		VertexArray* vao = mesh.GetVertexArray();
		if (!vao) return;

		RenderCommand::SetDepthTest(material->m_DepthTest);
		RenderCommand::SetCullingMode(material->m_BackfaceCulling ? CullingMode::Backface : CullingMode::None);

		material->ApplyUniforms(type);
		material->GetShaderGraph()->GetShaderViaType(type)->SetMat4("u_Transform", transform);

		material->GetShaderGraph()->GetShaderViaType(type)->SetMat4("u_ViewProjection", camera->GetViewProjectionMatrix());

		const glm::mat4 normalMat = glm::transpose(glm::inverse(glm::mat3(glm::inverse(camera->GetTransformMatrix()) * transform)));
		material->GetShaderGraph()->GetShaderViaType(type)->SetMat4("u_NormalMatrix", normalMat);

		material->GetShaderGraph()->GetShaderViaType(type)->SetInt("u_MeshID", 0);
		
		vao->Bind();
		RenderCommand::DrawIndexed(vao);
	}

	void Renderer3D::DrawMesh(CameraNode* camera, const glm::mat4& transform, Mesh& mesh, const MaterialSlots& materials, MaterialType type)
	{
		if (mesh.IsMasterMesh())
		{
			for (int i = 0; i < mesh.m_Submeshes.Size(); i++)
			{
				if (materials.HasSlots(i + 1) && materials.Materials[i]) DrawMesh(camera, transform, *mesh.m_Submeshes[i], materials.Materials[i], type);
			}
		}
		else
		{
			if (materials.HasSlots() && materials.Materials[0]) DrawMesh(camera, transform, mesh, materials.Materials[0], type);
		}
	}


}
