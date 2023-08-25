#include "Precompiled.h"
#include "ShaderGraph.h"
#include "AssetManager.h"
#include "Suora/Renderer/Shader.h"
#include "Suora/Common/Common.h"

namespace Suora
{

	ShaderGraph::ShaderGraph()
	{
		m_ShaderGraph = this;
	}
	ShaderGraph::~ShaderGraph()
	{
	}

	void ShaderGraph::PreInitializeAsset(const std::string& str)
	{
		Super::PreInitializeAsset(str);
		Yaml::Node root;
		Yaml::Parse(root, str);
		m_UUID = SuoraID(root["UUID"].As<std::string>());
	}

	void ShaderGraph::InitializeAsset(const std::string& str)
	{
		Super::InitializeAsset(str);
		Yaml::Node root;
		Yaml::Parse(root, str);

		m_ShaderGraph = this;

		m_BaseShader = root["m_BaseShader"].As<std::string>();

		std::string writtenSource = root["m_ShaderSource"].As<std::string>();
		while (writtenSource.find("\\n") != std::string::npos) Util::ReplaceSequence(writtenSource, "\\n", "\n");
		while (writtenSource.find("\\t") != std::string::npos) Util::ReplaceSequence(writtenSource, "\\t", "\t");
		while (writtenSource.find("\\c") != std::string::npos) Util::ReplaceSequence(writtenSource, "\\c", "#");
		m_ShaderSource = writtenSource;

		m_SourceFileTime = std::chrono::time_point<std::chrono::steady_clock>(std::chrono::nanoseconds(root["m_SourceFileTime"].As<unsigned long long>()));
		const std::chrono::time_point<std::chrono::steady_clock> baseShaderTime(std::filesystem::last_write_time(GetBaseShaderPath()).time_since_epoch());
		m_RequiresRegeneration = m_SourceFileTime != baseShaderTime;
	}

	void ShaderGraph::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);
		root["m_BaseShader"] = m_BaseShader;

		std::string writtenSource = m_ShaderSource;
		while (writtenSource.find("\n") != std::string::npos) Util::ReplaceSequence(writtenSource, "\n", "\\n");
		while (writtenSource.find("\r") != std::string::npos) Util::ReplaceSequence(writtenSource, "\r", "");
		while (writtenSource.find("\t") != std::string::npos) Util::ReplaceSequence(writtenSource, "\t", "\\t");
		while (writtenSource.find("#") != std::string::npos) Util::ReplaceSequence(writtenSource, "#", "\\c");
		if (writtenSource[writtenSource.size() - 1] == '\n') writtenSource.erase(writtenSource.size() - 1, 1);
		root["m_ShaderSource"] = writtenSource;

		root["m_SourceFileTime"] = std::to_string(m_SourceFileTime.time_since_epoch().count());
	}

	std::string ShaderGraph::GetBaseShaderPath() const
	{
		return AssetManager::GetAssetRootPath() + "/EngineContent/Shaders/ShadergraphBase/" + m_BaseShader;
	}

	Shader* ShaderGraph::GetShaderViaType(MaterialType type)
	{
		switch (type)
		{
		case MaterialType::Material: return GetShader();
		case MaterialType::Depth: return GetDepthShader();
		case MaterialType::FlatWhite: return GetFlatWhiteShader();
		case MaterialType::ObjectID: return GetIDShader();
		case MaterialType::None:
		default:
			SuoraError("ShaderGraph::GetShaderViatype(): Not Implemented!");
			return nullptr;
		}
	}

	Shader* ShaderGraph::GetShader()
	{
		if (!m_Shader.get())
		{
			m_Shader = Ref<Shader>(Shader::CreatePtr(*this));
		}
		return m_Shader.get();
	}
	Shader* ShaderGraph::GetDepthShader()
	{
		// TODO: Opacity in Fragment shader
		if (!m_DepthShader.get())
		{
			const std::string vertex = Shader::PreProcess(m_ShaderSource)["vertex"];
			std::string fragment = "\
#version 330 core\n\
\n\
in vec2 UV;\n\
out float out_Depth;\n\
\n\
\n\
void main(void)\n\
{\n\
	float ndcDepth =\
	(2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);\
	float clipDepth = ndcDepth / gl_FragCoord.w;\
	out_Depth = vec4((clipDepth * 0.5) + 0.5).r; \n\
}\n";

			fragment = "\
#version 330 core\n\
\n\
in vec2 UV;\n\
out float out_Depth;\n\
\n\
\n\
void main(void)\n\
{\n\
	 \n\
}\n";
			m_DepthShader = Shader::Create("MaterialDepth", vertex, fragment);
		}
		return m_DepthShader.get();
	}
	Shader* ShaderGraph::GetFlatWhiteShader()
	{
		if (!m_FlatWhiteShader.get())
		{
			const std::string vertex = Shader::PreProcess(m_ShaderSource)["vertex"];
			const std::string fragment = "\
#version 330 core\n\
\n\
in vec2 UV;\n\
out vec4 out_Color;\n\
\n\
\n\
void main(void)\n\
{\n\
	out_Color = vec4(1.0);\n\
}\n";
			m_FlatWhiteShader = Shader::Create("FlatWhite", vertex, fragment);
		}
		return m_FlatWhiteShader.get();
	}
	Shader* ShaderGraph::GetIDShader()
	{
		if (!m_IDShader.get())
		{
			const std::string vertex = Shader::PreProcess(m_ShaderSource)["vertex"];
			const std::string fragment = "\
#version 330 core\n\
\n\
in vec2 UV;\n\
out int out_ID;\n\
uniform int u_ID;\n\
\n\
void main(void)\n\
{\n\
	out_ID = u_ID;\n\
}\n";
			m_IDShader = Shader::Create("IDShader", vertex, fragment);
		}
		m_IDShader->Bind();
		//m_IDShader->SetInt("u_ID", 13);
		return m_IDShader.get();
	}

	bool ShaderGraph::IsDeferred() const
	{
		return m_BaseShader == "DeferredLit.glsl";
	}

}