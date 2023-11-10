#include "Precompiled.h"
#include "Suora/Renderer/Shader.h"

#include "Suora/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Suora 
{

	Ref<Shader> Shader::Create(const String& filepath)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLShader>(filepath);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Shader> Shader::Create(const String& name, const String& vertexSrc, const String& fragmentSrc)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Shader* Shader::CreatePtr(const String& filepath)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return new OpenGLShader(filepath);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Shader* Shader::CreatePtr(const String& name, const String& vertexSrc, const String& fragmentSrc)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return new OpenGLShader(name, vertexSrc, fragmentSrc);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Shader* Shader::CreatePtr(const ShaderGraph& shader)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None:    SUORA_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return new OpenGLShader(shader);
		}

		SUORA_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	std::unordered_map<String, String> Shader::PreProcess(const String& source)
	{
		std::unordered_map<String, String> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
		while (pos != String::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
			SUORA_ASSERT(eol != String::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			String type = source.substr(begin, eol - begin);

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			SUORA_ASSERT(nextLinePos != String::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[type] = (pos == String::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

}