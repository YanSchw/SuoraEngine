#include "Precompiled.h"
#include "ShaderNodeGraph.h"
#include "Suora/Assets/ShaderGraph.h"

namespace Suora
{
	ShaderNodeGraph::ShaderNodeGraph()
	{
	}

	void ShaderNodeGraph::UpdateSupportedNodes()
	{
		VisualNodeGraph::UpdateSupportedNodes();
		{
			Ref<VisualNode> master = CreateRef<VisualNode>();
			master->m_Title = "Master";
			master->m_NodeID = 1;
			master->m_Color = Vec4(0.8f, 0.64f, 0.57f, 1.0f);
			master->m_Size = { 300, 500 };
			AddSupportedNode(master);
		}
		{
			Ref<VisualNode> uniform = CreateRef<VisualNode>();
			uniform->m_Title = "Uniform";
			uniform->m_NodeID = 2;
			uniform->m_Color = Vec4(0.8f, 0.64f, 0.57f, 1.0f);
			uniform->m_Size = { 225, 235 };
			uniform->AddInputPin("Name", Vec4(1.0f), 0, false);
			uniform->m_InputPins[0].m_AdditionalData = "UniformName";
			uniform->AddInputPin("Type", Vec4(1.0f), 0, false);
			uniform->m_InputPins[1].m_AdditionalData = "1";
			uniform->AddInputPin("Default", Vec4(1.0f), 0, false);
			uniform->AddOutputPin("Uniform", Vec4(1.0f), 0, false);
			AddSupportedNode(uniform);
		}
		{
			Ref<VisualNode> vec3 = CreateRef<VisualNode>();
			vec3->m_Title = "Vector 3";
			vec3->m_NodeID = 100;
			vec3->m_Color = Vec4(0.4f, 0.49f, 0.38f, 1.0f);
			vec3->m_Size = { 215, 105 };
			vec3->AddInputPin("X", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			vec3->AddInputPin("Y", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			vec3->AddInputPin("Z", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			vec3->AddOutputPin("Vec3", GetShaderDataTypeColor(ShaderGraphDataType::Vec3), (int64_t)ShaderGraphDataType::Vec3, false);
			AddSupportedNode(vec3);
		}
		{
			Ref<VisualNode> vec4 = CreateRef<VisualNode>();
			vec4->m_Title = "Vector 4";
			vec4->m_NodeID = 101;
			vec4->m_Color = Vec4(0.52f, 0.47f, 0.36f, 1.0f);
			vec4->m_Size = { 215, 135 };
			vec4->AddInputPin("X", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			vec4->AddInputPin("Y", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			vec4->AddInputPin("Z", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			vec4->AddInputPin("W", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			vec4->AddOutputPin("Vec4", GetShaderDataTypeColor(ShaderGraphDataType::Vec4), (int64_t)ShaderGraphDataType::Vec4, false);
			AddSupportedNode(vec4);
		}
		{
			Ref<VisualNode> sampleText2D = CreateRef<VisualNode>();
			sampleText2D->m_Title = "Sample Texture2D";
			sampleText2D->m_NodeID = 102;
			sampleText2D->m_Color = Vec4(0.6f, 0.44f, 0.38f, 1.0f);
			sampleText2D->m_Size = { 215, 185 };
			sampleText2D->AddInputPin("Texture2D", GetShaderDataTypeColor(ShaderGraphDataType::Texture2D), (int64_t)ShaderGraphDataType::Texture2D, true);
			sampleText2D->AddInputPin("UV", GetShaderDataTypeColor(ShaderGraphDataType::Vec2), (int64_t)ShaderGraphDataType::Vec2, true);
			sampleText2D->AddOutputPin("RGBA", GetShaderDataTypeColor(ShaderGraphDataType::Vec4), (int64_t)ShaderGraphDataType::Vec4, false);
			sampleText2D->AddOutputPin("RGB", GetShaderDataTypeColor(ShaderGraphDataType::Vec3), (int64_t)ShaderGraphDataType::Vec3, false);
			sampleText2D->AddOutputPin("R", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, false);
			sampleText2D->AddOutputPin("G", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, false);
			sampleText2D->AddOutputPin("B", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, false);
			sampleText2D->AddOutputPin("A", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, false);
			AddSupportedNode(sampleText2D);
		}
		{
			Ref<VisualNode> uv = CreateRef<VisualNode>();
			uv->m_Title = "UV";
			uv->m_NodeID = 103;
			uv->m_Color = Vec4(0.52f, 0.87f, 0.36f, 1.0f);
			uv->m_Size = { 215, 135 };
			uv->AddOutputPin("Vec2", GetShaderDataTypeColor(ShaderGraphDataType::Vec2), (int64_t)ShaderGraphDataType::Vec2, false);
			AddSupportedNode(uv);
		}
		{
			Ref<VisualNode> splitVec2 = CreateRef<VisualNode>();
			splitVec2->m_Title = "Split Vec2";
			splitVec2->m_NodeID = 104;
			splitVec2->m_Color = Vec4(0.52f, 0.87f, 0.36f, 1.0f);
			splitVec2->m_Size = { 215, 135 };
			splitVec2->AddInputPin("Vec2", GetShaderDataTypeColor(ShaderGraphDataType::Vec2), (int64_t)ShaderGraphDataType::Vec2, true);
			splitVec2->AddOutputPin("X", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, false);
			splitVec2->AddOutputPin("Y", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, false);
			AddSupportedNode(splitVec2);
		}
		{
			Ref<VisualNode> sampleNormal = CreateRef<VisualNode>();
			sampleNormal->m_Title = "Sample Normal";
			sampleNormal->m_NodeID = 105;
			sampleNormal->m_Color = Vec4(0.6f, 0.44f, 0.38f, 1.0f);
			sampleNormal->m_Size = { 215, 185 };
			sampleNormal->AddInputPin("RGB", GetShaderDataTypeColor(ShaderGraphDataType::Vec3), (int64_t)ShaderGraphDataType::Vec3, true);
			sampleNormal->AddInputPin("Scale", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			sampleNormal->AddInputPin("Flip", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			sampleNormal->AddOutputPin("Normal", GetShaderDataTypeColor(ShaderGraphDataType::Vec3), (int64_t)ShaderGraphDataType::Vec3, false);
			AddSupportedNode(sampleNormal);
		}
		{
			Ref<VisualNode> FragDepth = CreateRef<VisualNode>();
			FragDepth->m_Title = "FragDepth";
			FragDepth->m_NodeID = 106;
			FragDepth->m_Color = Vec4(0.52f, 0.87f, 0.36f, 1.0f);
			FragDepth->m_Size = { 215, 135 };
			FragDepth->AddOutputPin("Float", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, false);
			AddSupportedNode(FragDepth);
		}
		{
			Ref<VisualNode> remap = CreateRef<VisualNode>();
			remap->m_Title = "Remap Float";
			remap->m_NodeID = 107;
			remap->m_Color = Vec4(0.52f, 0.87f, 0.36f, 1.0f);
			remap->m_Size = { 415, 155 };
			remap->AddInputPin("Value", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			remap->AddInputPin("From A", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			remap->AddInputPin("From B", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			remap->AddInputPin("To A", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			remap->AddInputPin("To B", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			remap->AddOutputPin("Mapped Value", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, false);
			AddSupportedNode(remap);
		}
		{
			Ref<VisualNode> mul = CreateRef<VisualNode>();
			mul->m_Title = "Float * Float";
			mul->m_NodeID = 1001;
			mul->m_Color = Vec4(0.52f, 0.87f, 0.36f, 1.0f);
			mul->m_Size = { 215, 135 };
			mul->AddInputPin("A", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			mul->AddInputPin("B", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			mul->AddOutputPin("C", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, false);
			AddSupportedNode(mul);
		}
		{
			Ref<VisualNode> mul = CreateRef<VisualNode>();
			mul->m_Title = "Vec3 * Float";
			mul->m_NodeID = 1002;
			mul->m_Color = Vec4(0.52f, 0.87f, 0.36f, 1.0f);
			mul->m_Size = { 215, 135 };
			mul->AddInputPin("A", GetShaderDataTypeColor(ShaderGraphDataType::Vec3), (int64_t)ShaderGraphDataType::Vec3, true);
			mul->AddInputPin("B", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			mul->AddOutputPin("C", GetShaderDataTypeColor(ShaderGraphDataType::Vec3), (int64_t)ShaderGraphDataType::Vec3, false);
			AddSupportedNode(mul);
		}
		{
			Ref<VisualNode> mul = CreateRef<VisualNode>();
			mul->m_Title = "Vec3 * Vec3";
			mul->m_NodeID = 1003;
			mul->m_Color = Vec4(0.52f, 0.87f, 0.36f, 1.0f);
			mul->m_Size = { 215, 135 };
			mul->AddInputPin("A", GetShaderDataTypeColor(ShaderGraphDataType::Vec3), (int64_t)ShaderGraphDataType::Vec3, true);
			mul->AddInputPin("B", GetShaderDataTypeColor(ShaderGraphDataType::Vec3), (int64_t)ShaderGraphDataType::Vec3, true);
			mul->AddOutputPin("C", GetShaderDataTypeColor(ShaderGraphDataType::Vec3), (int64_t)ShaderGraphDataType::Vec3, false);
			AddSupportedNode(mul);
		}
		{
			Ref<VisualNode> mul = CreateRef<VisualNode>();
			mul->m_Title = "Vec2 * Float";
			mul->m_NodeID = 1004;
			mul->m_Color = Vec4(0.52f, 0.87f, 0.36f, 1.0f);
			mul->m_Size = { 215, 135 };
			mul->AddInputPin("A", GetShaderDataTypeColor(ShaderGraphDataType::Vec2), (int64_t)ShaderGraphDataType::Vec2, true);
			mul->AddInputPin("B", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			mul->AddOutputPin("C", GetShaderDataTypeColor(ShaderGraphDataType::Vec2), (int64_t)ShaderGraphDataType::Vec2, false);
			AddSupportedNode(mul);
		}
		{
			Ref<VisualNode> cast = CreateRef<VisualNode>();
			cast->m_Title = "Cast Vec3 to Vec4";
			cast->m_NodeID = 2001;
			cast->m_Color = Vec4(0.52f, 0.87f, 0.36f, 1.0f);
			cast->m_Size = { 215, 135 };
			cast->AddInputPin("Vec3", GetShaderDataTypeColor(ShaderGraphDataType::Vec3), (int64_t)ShaderGraphDataType::Vec3, true);
			cast->AddInputPin("W", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			cast->AddOutputPin("Vec4", GetShaderDataTypeColor(ShaderGraphDataType::Vec4), (int64_t)ShaderGraphDataType::Vec4, false);
			AddSupportedNode(cast);
		}

		{
			Ref<VisualNode> min = CreateRef<VisualNode>();
			min->m_Title = "Min Float";
			min->m_NodeID = 10001;
			min->m_Color = Vec4(0.52f, 0.87f, 0.36f, 1.0f);
			min->m_Size = { 215, 135 };
			min->AddInputPin("A", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			min->AddInputPin("B", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			min->AddOutputPin("C", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, false);
			AddSupportedNode(min);
		}
		{
			Ref<VisualNode> max = CreateRef<VisualNode>();
			max->m_Title = "Max Float";
			max->m_NodeID = 10002;
			max->m_Color = Vec4(0.52f, 0.87f, 0.36f, 1.0f);
			max->m_Size = { 215, 135 };
			max->AddInputPin("A", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			max->AddInputPin("B", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, true);
			max->AddOutputPin("C", GetShaderDataTypeColor(ShaderGraphDataType::Float), (int64_t)ShaderGraphDataType::Float, false);
			AddSupportedNode(max);
		}
	}

	void ShaderNodeGraph::TickAllVisualNodes()
	{
		VisualNodeGraph::TickAllVisualNodes();

		for (auto noderef : m_Nodes)
		{
			VisualNode& node = *noderef;
			
			// Uniform
			if (node.m_NodeID == 2)
			{
				node.m_Color = ShaderNodeGraph::GetShaderDataTypeColor((ShaderGraphDataType)(int64_t)std::stoi(node.m_InputPins[1].m_AdditionalData));
			}
		}
	}

	void ShaderNodeGraph::TickAllVisualNodesInShaderGraphContext(ShaderGraph* shaderGraph)
	{
		for (auto noderef : m_Nodes)
		{
			VisualNode& node = *noderef;

			// MasterNode
			if (node.m_NodeID == 1)
			{
				for (int i = node.m_InputPins.Size() - 1; i >= 0; i--)
				{
					bool hasInput = false;
					for (BaseShaderInput& input : shaderGraph->m_BaseShaderInputs)
					{
						if (node.m_InputPins[i].PinID == (int64_t)input.m_Type && node.m_InputPins[i].Label == input.m_Label) hasInput = true;
					}
					if (!hasInput) node.m_InputPins.RemoveAt(i);
				}
				for (BaseShaderInput& input : shaderGraph->m_BaseShaderInputs)
				{
					bool hasPin = false;
					for (VisualNodePin& pin : node.m_InputPins)
					{
						if (pin.PinID == (int64_t)input.m_Type && pin.Label == input.m_Label) hasPin = true;
					}
					if (!hasPin) node.AddInputPin(input.m_Label, ShaderNodeGraph::GetShaderDataTypeColor(input.m_Type), (int64_t)input.m_Type, true);
				}
			}
		}

	}

	ShaderGraphDataType ShaderNodeGraph::StringToShaderGraphDataType(String str)
	{
		for (int i = str.size() - 1; i >= 0; i--)
		{
			if (str[i] == ' ') str.erase(i, 1);
		}
		if (str == "float") return ShaderGraphDataType::Float;
		if (str == "vec2") return ShaderGraphDataType::Vec2;
		if (str == "vec3") return ShaderGraphDataType::Vec3;
		if (str == "vec4") return ShaderGraphDataType::Vec4;
		return ShaderGraphDataType::None;
	}
	String ShaderNodeGraph::ShaderGraphDataTypeToLabel(ShaderGraphDataType type)
	{
		switch (type)
		{
		case ShaderGraphDataType::Float: return "Float";
		case ShaderGraphDataType::Vec2: return "Vec2";
		case ShaderGraphDataType::Vec3: return "Vec3";
		case ShaderGraphDataType::Vec4: return "Vec4";
		case ShaderGraphDataType::Texture2D: return "Texture2D";
		case ShaderGraphDataType::None:
		default:
			return "None";
		}
	}
	String ShaderNodeGraph::ShaderGraphDataTypeToString(ShaderGraphDataType type)
	{
		switch (type)
		{
		case ShaderGraphDataType::Float: return "float";
		case ShaderGraphDataType::Vec2: return "vec2";
		case ShaderGraphDataType::Vec3: return "vec3";
		case ShaderGraphDataType::Vec4: return "vec4";
		case ShaderGraphDataType::Texture2D: return "sampler2D";
		case ShaderGraphDataType::None:
		default:
			return "None";
		}
	}
	Vec4 ShaderNodeGraph::GetShaderDataTypeColor(ShaderGraphDataType type)
	{
		switch (type)
		{
		case ShaderGraphDataType::Float: return Vec4(0.4f, 0.49f, 0.38f, 1.0f);
		case ShaderGraphDataType::Vec2: return Vec4(0.4f, 0.2f, 0.28f, 1.0f);
		case ShaderGraphDataType::Vec3: return Vec4(0.62f, 0.59f, 0.28f, 1.0f);
		case ShaderGraphDataType::Vec4: return Vec4(0.52f, 0.59f, 0.48f, 1.0f);
		case ShaderGraphDataType::Texture2D: return Vec4(0.9f, 0.49f, 0.38f, 1.0f);
		case ShaderGraphDataType::None:
		default:
			return Vec4(1.0f);
		}
		return Vec4(1.0f);
	}

	/*********************************************************************/

	String ShaderGraphCompiler::CompileShaderNode(VisualNode& node, VisualNodePin& pin, bool vertex, bool& error)
	{
		int outPinIndex = node.m_OutputPins.IndexOf(pin);

		if (node.m_NodeID == 2)
		{
			return "(" + node.m_InputPins[0].m_AdditionalData + ")";
		}

		if (node.m_NodeID == 100)
		{
			return "vec3(" + CompilePin(node.m_InputPins[0], vertex, error) + ", "
				+ CompilePin(node.m_InputPins[1], vertex, error) + ", "
				+ CompilePin(node.m_InputPins[2], vertex, error) + ")";
		}
		if (node.m_NodeID == 101)
		{
			return "vec4(" + CompilePin(node.m_InputPins[0], vertex, error) + ", "
				+ CompilePin(node.m_InputPins[1], vertex, error) + ", "
				+ CompilePin(node.m_InputPins[2], vertex, error) + ", "
				+ CompilePin(node.m_InputPins[3], vertex, error) + ")";
		}
		if (node.m_NodeID == 102)
		{
			if (outPinIndex == 0)
			{
				return "texture(" + CompilePin(node.m_InputPins[0], vertex, error) + ", " + CompilePin(node.m_InputPins[1], vertex, error) + ")";
			}
			else if (outPinIndex == 1)
			{
				return "(texture(" + CompilePin(node.m_InputPins[0], vertex, error) + ", " + CompilePin(node.m_InputPins[1], vertex, error) + ").xyz)";
			}
			else if (outPinIndex == 2) { return "(texture(" + CompilePin(node.m_InputPins[0], vertex, error) + ", " + CompilePin(node.m_InputPins[1], vertex, error) + ").x)"; }
			else if (outPinIndex == 3) { return "(texture(" + CompilePin(node.m_InputPins[0], vertex, error) + ", " + CompilePin(node.m_InputPins[1], vertex, error) + ").y)"; }
			else if (outPinIndex == 4) { return "(texture(" + CompilePin(node.m_InputPins[0], vertex, error) + ", " + CompilePin(node.m_InputPins[1], vertex, error) + ").z)"; }
			else if (outPinIndex == 5) { return "(texture(" + CompilePin(node.m_InputPins[0], vertex, error) + ", " + CompilePin(node.m_InputPins[1], vertex, error) + ").w)"; }
		}
		if (node.m_NodeID == 103)
		{
			if (vertex) return "a_TexCoord";//{ error = true; SuoraWarn("UV error!"); return "!!!UV_ERROR!!!"; }
			return "UV";
		}
		if (node.m_NodeID == 104)
		{
			return "(" + CompilePin(node.m_InputPins[0], vertex, error) + ")"
				+ ((outPinIndex == 0) ? ".x" : ".y");

		}
		if (node.m_NodeID == 105)
		{
			return "(SampleNormal(" + CompilePin(node.m_InputPins[0], vertex, error) + ", " + CompilePin(node.m_InputPins[2], vertex, error) + ") * " + CompilePin(node.m_InputPins[1], vertex, error) + ")";

		}
		if (node.m_NodeID == 106)
		{
			return "(gl_FragCoord.z / gl_FragCoord.w)"; // "(gl_FragDepth * 0.5 + 0.5)";
		}

		if (node.m_NodeID == 107)
		{
			return "(" + CompilePin(node.m_InputPins[3], vertex, error) + " + (" + CompilePin(node.m_InputPins[0], vertex, error) + " - " + CompilePin(node.m_InputPins[1], vertex, error) + ") * (" + CompilePin(node.m_InputPins[4], vertex, error) + " - " + CompilePin(node.m_InputPins[3], vertex, error) + ") / (" + CompilePin(node.m_InputPins[2], vertex, error) + " - " + CompilePin(node.m_InputPins[1], vertex, error) + "))";
		}

		// Multiply
		if (node.m_NodeID == 1001)
		{
			return "((" + CompilePin(node.m_InputPins[0], vertex, error) + ") * (" + CompilePin(node.m_InputPins[1], vertex, error) + "))";
		}
		if (node.m_NodeID == 1002)
		{
			return "((" + CompilePin(node.m_InputPins[0], vertex, error) + ") * (vec3(" + CompilePin(node.m_InputPins[1], vertex, error) + ")))";
		}
		if (node.m_NodeID == 1003)
		{
			return "((" + CompilePin(node.m_InputPins[0], vertex, error) + ") * (" + CompilePin(node.m_InputPins[1], vertex, error) + "))";
		}
		if (node.m_NodeID == 1004)
		{
			return "((" + CompilePin(node.m_InputPins[0], vertex, error) + ") * (vec2(" + CompilePin(node.m_InputPins[1], vertex, error) + ")))";
		}

		// Cast
		if (node.m_NodeID == 2001)
		{
			return "vec4(" + CompilePin(node.m_InputPins[0], vertex, error) + ", " + CompilePin(node.m_InputPins[1], vertex, error) + ")";
		}

		if (node.m_NodeID == 10001)
		{
			return "min(" + CompilePin(node.m_InputPins[0], vertex, error) + ", " + CompilePin(node.m_InputPins[1], vertex, error) + ")";
		}
		if (node.m_NodeID == 10002)
		{
			return "max(" + CompilePin(node.m_InputPins[0], vertex, error) + ", " + CompilePin(node.m_InputPins[1], vertex, error) + ")";
		}

		SuoraError("Could not compile ShaderGraph Node! NodeID: {0}", node.m_NodeID);
		error = true;
		return "";
	}

	String ShaderGraphCompiler::CompilePin(VisualNodePin& pin, bool vertex, bool& error)
	{
		return (pin.Target ? CompileShaderNode(*(pin.Target->GetNode()), *(pin.Target), vertex, error) : (std::to_string(StringUtil::StringToFloat(pin.m_AdditionalData))/*pin.m_AdditionalData != "" ? pin.m_AdditionalData : "0.0"*/));
	}

}