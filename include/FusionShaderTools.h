#pragma once

#include "FusionShaderTools/ShaderTypes.h"
#include "FusionShaderTools/ShaderReflection.h"

#include "FusionShaderTools/StringUtils.h"

#include <string>
#include <vector>
#include <map>
#include <filesystem>
namespace fs = std::filesystem;

#include <nlohmann/json.hpp>

#include <spirv_cross.hpp>
#include <spirv_reflect.hpp>

namespace FusionShaderTools {

	class FusionShaderUtils {
	public:
		static void SplitShaderSource(const ShaderSource& source, std::vector<ShaderStage_Source>& outStages) {

			std::map<std::string, std::string> regions;
			StringUtils::SplitRegions(source.Data, regions);
			for (auto& [name, data] : regions) {
				EShaderStageType type = ShaderStageFromString(name);
				assert(type != EShaderStageType::None && "Unexpected Shader Stage");
				outStages.push_back(ShaderStage_Source(type, data));
			}
		}

		static ShaderStage_SpirV CompileStage_SPIRV(const ShaderStage_Source& source) {
			ShaderCompilerConfig config;
			SpirVCompiler shaderTools(config);

			glslang::TShader* shader = shaderTools.ParseStage(source.Type, source.Source);
			std::vector<uint32_t> spirv = shaderTools.GlslToSpv(shader);
			return ShaderStage_SpirV(source.Type, spirv);
		}

		static ShaderStage_GLSL CompileStage_GLSL(const ShaderStage_SpirV& source) {
			spirv_cross::CompilerGLSL compiler(source.Source);
			std::string glsl = compiler.compile();
			return ShaderStage_GLSL(source.Type, glsl);
		}

		static ShaderStage_HLSL CompileStage_HLSL(const ShaderStage_SpirV& source) {
			spirv_cross::CompilerHLSL compiler(source.Source);
			std::string glsl = compiler.compile();
			return ShaderStage_HLSL(source.Type, glsl);
		}

		static ShaderStage_MSL CompileStage_MSL(const ShaderStage_SpirV& source) {
			spirv_cross::CompilerMSL compiler(source.Source);
			std::string glsl = compiler.compile();
			return ShaderStage_MSL(source.Type, glsl);
		}

		static ShaderStageInfo GetStageInfo(const ShaderStage_SpirV& stage) {
			spirv_cross::CompilerReflection compiler(stage.Source);
			spirv_cross::ShaderResources resources = compiler.get_shader_resources();
			ShaderStageInfo info = ShaderStageInfo(stage.Type, compiler, resources);

			return info;
		}
	};

}

