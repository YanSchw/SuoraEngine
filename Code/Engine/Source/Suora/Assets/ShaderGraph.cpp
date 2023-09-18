#include "Precompiled.h"
#include "ShaderGraph.h"
#include "AssetManager.h"
#include "Platform/Platform.h"
#include "Suora/NodeScript/ShaderNodeGraph.h"
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

		ShaderNodeGraph graph;
		graph.DeserializeNodeGraph(root);
		GenerateShaderGraphSource(graph);
	}

	void ShaderGraph::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);
		root["m_BaseShader"] = m_BaseShader;

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
			if (m_ShaderSource == "")
			{
				return nullptr;
			}
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
		return IsFlagSet(ShaderGraphFlags::Deferred);
	}

	inline bool ShaderGraph::IsFlagSet(ShaderGraphFlags flag) const
	{
		return 0 != ((uint32_t)m_Flags & (uint32_t)flag);
	}


	void ShaderGraph::LoadBaseShaderInput(BaseShaderInput& input, int64_t& begin, int64_t& end, const std::string& str)
	{
		while (str[begin++] != '"');
		end = begin + 1;
		while (str[end++] != '"');
		input.m_Label = str.substr(begin, end - begin - 1);

		begin = end;
		while (str[begin++] != ',');
		end = begin;
		while (str[end++] != ',');
		input.m_Type = ShaderNodeGraph::StringToShaderGraphDataType(str.substr(begin, end - begin - 1));

		begin = end;
		while (str[begin++] != '{');
		end = begin;
		while (str[end++] != '}');
		input.m_DefaultSource = str.substr(begin, end - begin - 2);
	}

	void ShaderGraph::LoadBaseShaderInputs(const std::string& path)
	{
		std::string str = Platform::ReadFromFile(path);
		Util::RemoveCommentsFromString(str);
		int64_t begin = 0, end = 0;
		m_BaseShaderInputs.Clear();
		while (true)
		{
			begin = str.find("$VERT_INPUT", begin);
			if (begin == std::string::npos) break;
			BaseShaderInput input;
			input.m_InVertexShader = true;
			LoadBaseShaderInput(input, begin, end, str);
			m_BaseShaderInputs.Add(input);
		}
		begin = 0;
		while (true)
		{
			begin = str.find("$FRAG_INPUT", begin);
			if (begin == std::string::npos) break;
			BaseShaderInput input;
			input.m_InVertexShader = false;
			LoadBaseShaderInput(input, begin, end, str);
			m_BaseShaderInputs.Add(input);
		}
	}

	void ShaderGraph::GenerateShaderInput(std::string& str, int64_t begin, VisualNode* master, bool vertex, bool& error)
	{
		int64_t labelBegin = begin;
		int64_t end = begin;
		while (str[labelBegin++] != '"');
		end = labelBegin + 1;
		while (str[end++] != '"');
		std::string label = str.substr(labelBegin, end - labelBegin - 1);
		int brackets = 1;
		while (brackets > 0)
		{
			if (str[end] == '(') brackets++;
			if (str[end] == ')') brackets--;
			end++;
		}
		str.erase(begin, end - begin);

		for (VisualNodePin& pin : master->m_InputPins)
		{
			if (pin.Label == label)
			{
				for (BaseShaderInput& input : m_BaseShaderInputs)
				{
					if (pin.PinID == (int64_t)input.m_Type && pin.Label == input.m_Label && input.m_InVertexShader == vertex)
					{
						if (pin.Target)
						{
							std::string src = ShaderGraphCompiler::CompileShaderNode(*(pin.Target->GetNode()), *(pin.Target), vertex, error);
							str.insert(begin, src);
							return;
						}
						else
						{
							str.insert(begin, input.m_DefaultSource);
							return;
						}
					}
				}
				return;
			}
		}
	}

	void ShaderGraph::GenerateShaderGraphSource(ShaderNodeGraph& graph)
	{
		LoadBaseShaderInputs(GetBaseShaderPath());
		m_Flags = ShaderGraphFlags::None;

		bool error = false;
		VisualNode* master = nullptr;
		bool isMasterNodeTemporary = false;
		for (Ref<VisualNode> node : graph.m_Nodes)
		{
			if (node->m_Title == "Master" && node->m_NodeID == 1)
			{
				master = node.get();
			}
		}

		if (master == nullptr)
		{
			SUORA_WARN(LogCategory::Rendering, "No Master-node found in {0}!", GetAssetName());
			isMasterNodeTemporary = true;
			Ref<VisualNode> ref = CreateRef<VisualNode>();
			graph.m_Nodes.Add(ref);
			master = ref.get();
			master->m_Title = "Master";
			master->m_NodeID = 1;
			graph.TickAllVisualNodesInShaderGraphContext(this);
		}

		std::string src = Platform::ReadFromFile(GetBaseShaderPath());
		Util::RemoveCommentsFromString(src);

		// Uniforms
		Array<UniformSlot> oldSlots = m_UniformSlots;
		m_UniformSlots.Clear();
		std::string uniforms;
		for (Ref<VisualNode> node : graph.m_Nodes)
		{
			if (node->m_NodeID == 2)
			{
				const ShaderGraphDataType type = (ShaderGraphDataType)std::stoi(node->m_InputPins[1].m_AdditionalData);
				uniforms += "uniform " + ShaderNodeGraph::ShaderGraphDataTypeToString(type) + " " + node->m_InputPins[0].m_AdditionalData + ";\n";

				switch (type)
				{
				case ShaderGraphDataType::Float:
					m_UniformSlots.Add(UniformSlot(type, node->m_InputPins[0].m_AdditionalData, 0.0f));
					break;
				case ShaderGraphDataType::Vec2:
					m_UniformSlots.Add(UniformSlot(type, node->m_InputPins[0].m_AdditionalData, 0.0f));
					break;
				case ShaderGraphDataType::Vec3:
					m_UniformSlots.Add(UniformSlot(type, node->m_InputPins[0].m_AdditionalData, 0.0f));
					break;
				case ShaderGraphDataType::Vec4:
					m_UniformSlots.Add(UniformSlot(type, node->m_InputPins[0].m_AdditionalData, 0.0f));
					break;
				case ShaderGraphDataType::Texture2D:
					m_UniformSlots.Add(UniformSlot(type, node->m_InputPins[0].m_AdditionalData, nullptr));
					break;
				case ShaderGraphDataType::None:
				default:
					SuoraError("ShaderGraph::GenerateShaderGraphSource(): Unkown UniformType!");
				}
			}
		}

		// If the old UniformSlots had Values in them, reapply them now...
		for (UniformSlot& old : oldSlots)
		{
			for (UniformSlot& newer : m_UniformSlots)
			{
				if (old.m_Label == newer.m_Label && old.m_Type == newer.m_Type)
				{
					newer = old;
				}
			}
		}

		// Proccess BaseShader
		while (src.find("$VERT_INPUTS") != std::string::npos) Util::ReplaceSequence(src, "$VERT_INPUTS", "" + uniforms);
		while (src.find("$FRAG_INPUTS") != std::string::npos) Util::ReplaceSequence(src, "$FRAG_INPUTS", "" + uniforms);

		while (src.find("$DEFERRED") != std::string::npos)
		{
			// This removes the $Symbol, but marks the generated Source with a comment
			Util::ReplaceSequence(src, "$DEFERRED", "/* DEFERRED */");
			m_Flags = m_Flags | ShaderGraphFlags::Deferred;
		}

		int64_t begin = 0;
		while (true)
		{
			begin = src.find("$VERT_INPUT", begin);
			if (begin == std::string::npos) break;
			GenerateShaderInput(src, begin, master, true, error);
		}
		begin = 0;
		while (true)
		{
			begin = src.find("$FRAG_INPUT", begin);
			if (begin == std::string::npos) break;
			GenerateShaderInput(src, begin, master, false, error);
		}

		if (!error)
		{
			m_ShaderSource = src;
		}
		else
		{
			SuoraError("ShaderGraph compilation failed!");
			return;
		}

		// Remove Master-node, if it was just temporary
		if (isMasterNodeTemporary)
		{
			for (int64_t i = 0; i < graph.m_Nodes.Size(); i++)
			{
				if (graph.m_Nodes[i]->m_Title == "Master" && graph.m_Nodes[i]->m_NodeID == 1)
				{
					graph.m_Nodes.RemoveAt(i);
					break;
				}
			}
		}

		m_Shader = nullptr;
	}

}