#pragma once
#include "Material.h"
#include <vector>
#include <string>
#include <chrono>
#include "ShaderGraph.generated.h"

namespace Suora
{
	class Shader;

	class ShaderGraph : public Material
	{
		SUORA_CLASS(564364364);
		ASSET_EXTENSION(".shadergraph");

	public:
		ShaderGraph();
		~ShaderGraph();
		void PreInitializeAsset(const std::string& str) override;
		void InitializeAsset(const std::string& str) override;
		void Serialize(Yaml::Node& root) override;
		std::string GetBaseShaderPath() const;
		Shader* GetShaderViaType(MaterialType type);
		Shader* GetShader();
		Shader* GetDepthShader();
		Shader* GetFlatWhiteShader();
		Shader* GetIDShader();

		virtual bool IsDeferred() const override;

		std::string m_BaseShader;
		std::string m_ShaderSource;
		Ref<Shader> m_Shader, m_DepthShader, m_FlatWhiteShader, m_IDShader;
		std::chrono::time_point<std::chrono::steady_clock> m_SourceFileTime = std::chrono::steady_clock::time_point(std::chrono::nanoseconds(0));
		bool m_RequiresRegeneration = true;
	};
}