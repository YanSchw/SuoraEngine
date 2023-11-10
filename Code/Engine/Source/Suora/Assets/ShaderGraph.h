#pragma once
#include "Material.h"
#include <vector>
#include <string>
#include <chrono>
#include "ShaderGraph.generated.h"

namespace Suora
{
	class Shader;
	struct VisualNode;
	struct ShaderNodeGraph;

	enum class ShaderGraphFlags : uint32_t
	{
		None = 0,
		Deferred = 1
	};
	inline ShaderGraphFlags operator|(ShaderGraphFlags a, ShaderGraphFlags b)
	{
		return static_cast<ShaderGraphFlags>(static_cast<int32_t>(a) | static_cast<int32_t>(b));
	}

	struct BaseShaderInput
	{
		String m_Label;
		String m_DefaultSource;
		ShaderGraphDataType m_Type = ShaderGraphDataType::None;
		bool m_InVertexShader = false;
	};

	class ShaderGraph : public Material
	{
		SUORA_CLASS(564364364);
		ASSET_EXTENSION(".shadergraph");

	public:
		ShaderGraph();
		~ShaderGraph();
		void PreInitializeAsset(const String& str) override;
		void InitializeAsset(const String& str) override;
		void Serialize(Yaml::Node& root) override;
		String GetBaseShaderPath() const;
		Shader* GetShaderViaType(MaterialType type);
		Shader* GetShader();
		Shader* GetDepthShader();
		Shader* GetFlatWhiteShader();
		Shader* GetIDShader();

		virtual bool IsDeferred() const override;
		inline bool IsFlagSet(ShaderGraphFlags flag) const;

		Array<BaseShaderInput> m_BaseShaderInputs;
		void LoadBaseShaderInput(BaseShaderInput& input, int64_t& begin, int64_t& end, const String& str);
		void LoadBaseShaderInputs(const String& path);
		void GenerateShaderInput(String& str, int64_t begin, VisualNode* master, bool vertex, bool& error);
		void GenerateShaderGraphSource(ShaderNodeGraph& graph);

		String m_BaseShader;
		String m_ShaderSource;
		Ref<Shader> m_Shader, m_DepthShader, m_FlatWhiteShader, m_IDShader;
		ShaderGraphFlags m_Flags = ShaderGraphFlags::None;
	};
}