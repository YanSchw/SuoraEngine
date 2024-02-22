#pragma once

#include "Suora/Renderer/Shader.h"
#include <glm/glm.hpp>

// TODO: REMOVE!
typedef unsigned int GLenum;

namespace Suora 
{

	class ShaderGraph;

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const String& filepath);
		OpenGLShader(const String& name, const String& vertexSrc, const String& fragmentSrc);
		OpenGLShader(const ShaderGraph& shader);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetInt(const String& name, int value) override;
		virtual void SetIntArray(const String& name, int* values, uint32_t count) override;
		virtual void SetFloat(const String& name, float value) override;
		virtual void SetFloat2(const String& name, const Vec2& value) override;
		virtual void SetFloat3(const String& name, const Vec3& value) override;
		virtual void SetFloat4(const String& name, const Vec4& value) override;
		virtual void SetMat4(const String& name, const Mat4& value) override;
		virtual void SetBool(const String& name, bool value) override;

		virtual const String& GetName() const override { return m_Name; }

		void UploadUniformInt(const String& name, int value);
		void UploadUniformIntArray(const String& name, int* values, uint32_t count);

		void UploadUniformFloat(const String& name, float value);
		void UploadUniformFloat2(const String& name, const Vec2& value);
		void UploadUniformFloat3(const String& name, const Vec3& value);
		void UploadUniformFloat4(const String& name, const Vec4& value);

		void UploadUniformMat3(const String& name, const glm::mat3& matrix);
		void UploadUniformMat4(const String& name, const Mat4& matrix);

		void UploadUniformBool(const String& name, bool value);
	private:
		String ReadFile(const String& filepath);
		std::unordered_map<GLenum, String> PreProcess(const String& source);
		void Compile(const std::unordered_map<GLenum, String>& shaderSources);
	private:
		uint32_t m_RendererID;
		String m_Name;
	};

}