#include "FusionShaderTools/Utils.h"
#include "FusionShaderTools/SpirVCompiler.h"
#include "FusionShaderTools.h"

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>
#include <spirv_parser.hpp>
#include <spirv_reflect.hpp>

int main(int argc, char** argv)
{
	FusionShaderTools::ShaderCompilerConfig config;
	FusionShaderTools::SpirVCompiler shaderTools(config);

	std::map<FusionShaderTools::EShaderStageType, std::string> stageSources;
	shaderTools.LoadSource("shaders/VulkanShader.glsl", stageSources);

	std::map<FusionShaderTools::EShaderStageType, std::shared_ptr<glslang::TShader>> stageShaders;
	for (auto& [stage, source] : stageSources) {
		glslang::TShader* shader = shaderTools.ParseStage(stage, source);
		if (shader == nullptr) {
			// Error
		}

		stageShaders[stage] = std::shared_ptr<glslang::TShader>(shader);
	}

	std::vector<uint32_t> vertSpirV = shaderTools.GlslToSpv(stageShaders[FusionShaderTools::EShaderStageType::Vertex].get());
	std::string onlineVertSpv = std::string((char*)vertSpirV.data(), vertSpirV.size() * sizeof(uint32_t));
	FileUtils::TryWriteFile("shaders/VulkanShader.vert.spv", onlineVertSpv);

	std::vector<uint32_t> fragSpirV = shaderTools.GlslToSpv(stageShaders[FusionShaderTools::EShaderStageType::Fragment].get());
	std::string onlineFragSpv = std::string((char*)fragSpirV.data(), fragSpirV.size() * sizeof(uint32_t));
	FileUtils::TryWriteFile("shaders/VulkanShader.frag.spv", onlineFragSpv);

	//std::vector<std::shared_ptr<glslang::TProgram>> stagePrograms;
	//for (std::shared_ptr<glslang::TShader> shader : stageShaders) {
	//	glslang::TProgram* program = shaderTools.LinkProgram({ shader.get() });
	//	if (program == nullptr) {
	//		// Error
	//	}
	//
	//	stagePrograms.push_back(std::shared_ptr<glslang::TProgram>(program));
	//}

	std::vector<glslang::TShader*> rawShaders;
	std::for_each(stageShaders.begin(), stageShaders.end(), [&rawShaders](const std::pair<FusionShaderTools::EShaderStageType, std::shared_ptr<glslang::TShader>>& item) {
		rawShaders.push_back(item.second.get());
		});
	glslang::TProgram* program = shaderTools.LinkProgram(rawShaders);
	shaderTools.WriteToDisk(program, "");

	// from source: "ShInitialize() should be called exactly once per process, not per thread."
	/*glslang::InitializeProcess();

	ShaderComponent vertComp("shaders/shader.vert");
	if (CompileGLSL(vertComp)) {
		std::string onlineVertSpv = std::string((char*)vertComp.SpirV.data(), vertComp.SpirV.size() * sizeof(uint32_t));
		TryWriteFile("shaders/online.vert.spv", onlineVertSpv);
	}

	ShaderComponent fragComp("shaders/shader.frag");
	if (CompileGLSL(fragComp)) {
		std::string onlineFragSpv = std::string((char*)fragComp.SpirV.data(), fragComp.SpirV.size() * sizeof(uint32_t));
		TryWriteFile("shaders/online.frag.spv", onlineFragSpv);
	}

	glslang::FinalizeProcess();*/


	// Read SPIR-V from disk or similar.
	// std::vector<uint32_t> spirv_binary = load_spirv_file();

	spirv_cross::CompilerGLSL glsl(vertSpirV);

	// The SPIR-V is now parsed, and we can perform reflection on it.
	spirv_cross::ShaderResources resources = glsl.get_shader_resources();

	spirv_cross::Resource res = resources.uniform_buffers[0];
	const spirv_cross::SPIRType& type = glsl.get_type(res.base_type_id);
	std::string name = glsl.get_member_name(res.base_type_id, 0);

	// Get all sampled images in the shader.
	for (auto& resource : resources.sampled_images)
	{
		unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
		unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
	//	printf("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);
	//
	//	// Modify the decoration to prepare it for GLSL.
	//	glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);
	//
	//	// Some arbitrary remapping if we want.
	//	glsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
	}

	// Set some options.
	// spirv_cross::CompilerGLSL::Options options;
	// options.version = 310;
	// options.es = true;
	// glsl.set_common_options(options);

	// Compile to GLSL, ready to give to GL driver.
	// std::string source = glsl.compile();




	std::string sourceString;
	FileUtils::TryReadFile("shaders/VulkanShader.glsl", sourceString);
	FusionShaderTools::ShaderSource shaderSource = { sourceString };
	std::vector<FusionShaderTools::ShaderStage_Source> stagesSources;
	FusionShaderTools::FusionShaderUtils::SplitShaderSource(shaderSource, stagesSources);

	FusionShaderTools::ShaderStage_SpirV spirv = FusionShaderTools::FusionShaderUtils::CompileStage_SPIRV(stagesSources[1]);
	FusionShaderTools::ShaderStageInfo info = FusionShaderTools::FusionShaderUtils::GetStageInfo(spirv);

	std::cin.get();
	return 0;
}