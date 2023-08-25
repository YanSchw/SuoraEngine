#pragma once

#include "NodeGraph.h"

namespace Suora
{
	class Blueprint;
	struct ScriptFunction;

	struct BlueprintCompiler
	{
		struct LocalVar
		{
			VisualNode* node = nullptr;
			VisualNodePin* pin = nullptr;
		};
		struct CompilerCache
		{
			Array<LocalVar> LocalVars;
		};

		static void Compile(Blueprint& blueprint);

	private:
		static bool IsVisualNodePinUsed(VisualNodePin& pin, Ref<VisualNodeGraph> graph);
		static bool IsVisualNodePinLocalVar(VisualNodePin& pin, CompilerCache& cache, int& ID);
		static void Compile(Blueprint& blueprint, const Array<Ref<VisualNodeGraph>>& eventGraphs);
		static void CompileEvent(Blueprint& blueprint, VisualNode& event, VisualNodePin& exec, Ref<VisualNodeGraph> graph, size_t hash);
		static bool CompileVisualNode(ScriptFunction& func, const Class& cls, VisualNode& node, VisualNodePin* exec, CompilerCache& cache, Ref<VisualNodeGraph> graph);
		static bool CompileVisualNodePin(ScriptFunction& func, const Class& cls, VisualNodePin& pin, CompilerCache& cache, Ref<VisualNodeGraph> graph);
	};

	struct BlueprintNodeGraph : VisualNodeGraph
	{
		Blueprint* m_BlueprintClass = nullptr;

		BlueprintNodeGraph(Blueprint* blueprint);

		void UpdateSupportedNodes() override;
		void TickAllVisualNodes() override;

	private:
		void RecursiveNodeDelegates(Node* node, bool isChild);

	};

}