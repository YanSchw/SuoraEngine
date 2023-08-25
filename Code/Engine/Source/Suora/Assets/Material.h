#pragma once
#include <unordered_map>
#include "Asset.h"
#include "Material.generated.h"
#include <glm/glm.hpp>

namespace Suora
{
	class Shader;
	class ShaderGraph;
	class Texture2D;

	enum class ShaderGraphDataType : int64_t
	{
		None = 0,
		Float,
		Vec2,
		Vec3,
		Vec4,
		Texture2D
	};

	enum class MaterialType : int32_t
	{
		None = 0,
		Material,
		Depth,
		FlatWhite,
		ObjectID,
		Ilum
	};

	struct UniformSlot
	{
		ShaderGraphDataType m_Type;
		std::string m_Label;

		float m_Float = 0.0f;
		glm::vec2 m_Vec2 = glm::vec2();
		glm::vec3 m_Vec3 = glm::vec3();
		glm::vec4 m_Vec4 = glm::vec4();
		Texture2D* m_Texture2D = nullptr;

		UniformSlot(ShaderGraphDataType type, const std::string& label, float value) : m_Type(type), m_Label(label), m_Float(value) { }
		UniformSlot(ShaderGraphDataType type, const std::string& label, const glm::vec2& value) : m_Type(type), m_Label(label), m_Vec2(value) { }
		UniformSlot(ShaderGraphDataType type, const std::string& label, const glm::vec3& value) : m_Type(type), m_Label(label), m_Vec3(value) { }
		UniformSlot(ShaderGraphDataType type, const std::string& label, const glm::vec4& value) : m_Type(type), m_Label(label), m_Vec4(value) { }
		UniformSlot(ShaderGraphDataType type, const std::string& label, Texture2D* value) : m_Type(type), m_Label(label), m_Texture2D(value) { }
	};

	class Material : public Asset
	{
		SUORA_CLASS(15785647);
		ASSET_EXTENSION(".material");

	public:
		Array<UniformSlot> m_UniformSlots;

		Material();
		void PreInitializeAsset(const std::string& str);
		void InitializeAsset(const std::string& str);
		void Serialize(Yaml::Node& root);

		void SetShaderGraph(ShaderGraph* shaderGraph);
		ShaderGraph* GetShaderGraph() const;

		void ApplyUniforms(MaterialType type);
		UniformSlot* GetUniformSlot(const std::string& label);

		virtual bool IsDeferred() const;

		bool m_DepthTest = true;
		bool m_BackfaceCulling = true;

	private:
		ShaderGraph* m_ShaderGraph = nullptr;

		friend class ShaderGraph;
		friend class Renderer3D;
		friend class DetailsPanel;
	};

	struct MaterialSlots
	{
		MaterialSlots();
		MaterialSlots(Material* material);

		bool HasSlots(int count = 1) const { return Materials.Size() >= count; }

		bool OverwritteMaterials = false;
		Array<Material*> Materials;
		void TickSlots();

		void operator=(const MaterialSlots& other);
	private:
		Object* Owner = nullptr;
		friend class Mesh;
		friend class MeshNode;
		friend class DetailsPanel;
	};

}