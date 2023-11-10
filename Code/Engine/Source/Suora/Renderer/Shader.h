#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Suora 
{
	class ShaderGraph;

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const String& name, int value) = 0;
		virtual void SetIntArray(const String& name, int* values, uint32_t count) = 0;
		virtual void SetFloat(const String& name, float value) = 0;
		virtual void SetFloat2(const String& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const String& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const String& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const String& name, const glm::mat4& value) = 0;
		virtual void SetBool(const String& name, bool value) = 0;

		virtual const String& GetName() const = 0;

		static Ref<Shader> Create(const String& filepath);
		static Ref<Shader> Create(const String& name, const String& vertexSrc, const String& fragmentSrc);
		static Shader* CreatePtr(const String& filepath);
		static Shader* CreatePtr(const String& name, const String& vertexSrc, const String& fragmentSrc);
		static Shader* CreatePtr(const ShaderGraph& shader);

		static std::unordered_map<String, String> PreProcess(const String& source);
	};

}