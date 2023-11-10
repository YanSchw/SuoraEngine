#pragma once

#include "NodeGraph.h"

namespace Suora
{
	class ShaderGraph;

	struct ShaderGraphCompiler
	{
		static String CompileShaderNode(VisualNode& node, VisualNodePin& pin, bool vertex, bool& error);
		static String CompilePin(VisualNodePin& pin, bool vertex, bool& error);
	};

	struct ShaderNodeGraph : VisualNodeGraph
	{
		ShaderNodeGraph();

		void UpdateSupportedNodes() override;

		virtual void TickAllVisualNodes() override;
		void TickAllVisualNodesInShaderGraphContext(ShaderGraph* shaderGraph);

		static ShaderGraphDataType StringToShaderGraphDataType(String str);
		static String ShaderGraphDataTypeToLabel(ShaderGraphDataType type);
		static String ShaderGraphDataTypeToString(ShaderGraphDataType type);
		static glm::vec4 GetShaderDataTypeColor(ShaderGraphDataType type);
	};

}