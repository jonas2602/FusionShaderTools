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
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>

namespace FusionShaderTools {

	class FusionShaderUtils {
	public:
		static void SplitShaderSource(const ShaderSource& source, std::vector<ShaderStage_Source>& outStages) {

			std::unordered_map<std::string, std::string> regions;
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

			glslang::TProgram* program = shaderTools.LinkProgram({ shader });
			std::vector<uint32_t> spirv = shaderTools.GlslToSpv(program, shader->getStage());

			delete program;
			delete shader;

			return ShaderStage_SpirV(source.Type, spirv);
		}

		static ShaderStage_GLSL CompileStage_GLSL(const ShaderStage_SpirV& source, const std::map<uint32_t, uint32_t>& setOffsets) {
			spirv_cross::CompilerGLSL compiler(source.Source);
			spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();
			std::vector<spirv_cross::Resource> uniformResources;
			uniformResources.insert(uniformResources.end(), shaderResources.sampled_images.begin(), shaderResources.sampled_images.end());
			uniformResources.insert(uniformResources.end(), shaderResources.uniform_buffers.begin(), shaderResources.uniform_buffers.end());

			/*std::set<std::tuple<uint32_t, uint32_t>> bindingSet;
			for (const spirv_cross::Resource& resource : uniformResources) {
				uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
				bindingSet.insert({ set, binding });
			}
			std::map<uint32_t, uint32_t> setOffsets;
			for (auto& [set, binding] : bindingSet) {
				setOffsets[set] = binding;
			}
			uint32_t offset = 0;
			for (std::pair<uint32_t, uint32_t> set : setOffsets) {
				setOffsets[set.first] = offset;
				offset += set.second + 1;
			}*/

			for (const spirv_cross::Resource& resource : uniformResources) {
				uint32_t oldSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t oldBinding = compiler.get_decoration(resource.id, spv::DecorationBinding);

				uint32_t offset = 0;
				if (setOffsets.find(oldSet) != setOffsets.end()) {
					offset = setOffsets.find(oldSet)->second;
				}

				uint32_t newBinding = offset + oldBinding;
				compiler.set_decoration(resource.id, spv::DecorationDescriptorSet, 0);
				compiler.set_decoration(resource.id, spv::DecorationBinding, newBinding);
			}

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

