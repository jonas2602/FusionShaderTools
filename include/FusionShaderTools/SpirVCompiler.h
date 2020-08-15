#pragma once

#include "Utils.h"
#include "FusionShaderTools/ShaderTypes.h"

#include <glslang/Public/ShaderLang.h>

namespace FusionShaderTools {

	struct ShaderCompilerConfig {

	};

	struct CompilerResult {


	};

	//struct ShaderData {
	//public:
	//	ShaderData(EShLanguage type, const std::string& source)
	//		: m_Type(type)
	//		, m_Source(source)
	//		, m_Native(type)
	//	{ }
	//
	//public:
	//	EShLanguage m_Type;
	//	std::string m_Source;
	//
	//	glslang::TShader m_Native;
	//};
	//
	//struct ProgramData {
	//public:
	//	void AddShader(ShaderData* shader) {
	//		m_Stages.push_back(shader);
	//		m_Native.addShader(&shader->m_Native);
	//	}
	//
	//public:
	//	std::vector<ShaderData*> m_Stages;
	//
	//	glslang::TProgram m_Native;
	//
	//};

	class SpirVCompiler
	{
	public:
		SpirVCompiler(const ShaderCompilerConfig& config);
		~SpirVCompiler();

	public:
		void DumpVersions();

	public:
		void LoadSource(const fs::path& filePath, std::map<EShaderStageType, std::string>& outSourceMap);
		glslang::TShader* ParseStage(EShaderStageType stage, const std::string& source);
		glslang::TProgram* LinkProgram(const std::vector<glslang::TShader*>& shaders);

		std::vector<uint32_t> GlslToSpv(glslang::TShader* inGlsl);

		/*std::vector<char> SpvToGlsl(const std::vector<uint32_t>& inSpirV) {

			std::stringstream ss;
			// Disassemble the final programm to glsl and print it to the console
			spv::Disassemble(ss, inSpirV);

			std::string outGlsl = ss.str();
			return std::vector<char>(outGlsl.begin(), outGlsl.end());
		}*/

		bool WriteToDisk(glslang::TProgram* program, const fs::path& dir);
	};

}