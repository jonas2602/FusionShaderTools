#include "FusionShaderTools/Utils.h"
#include "FusionShaderTools/SpirVCompiler.h"

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>


int main(int argc, char** argv)
{
	ShaderCompilerConfig config;
	SpirVCompiler shaderTools(config);

	std::map<EShLanguage, std::string> stageSources;
	shaderTools.LoadSource("shaders/VulkanShader.glsl", stageSources);

	std::map<EShLanguage, std::shared_ptr<glslang::TShader>> stageShaders;
	for (auto& [stage, source] : stageSources) {
		glslang::TShader* shader = shaderTools.ParseStage(stage, source);
		if (shader == nullptr) {
			// Error
		}

		stageShaders[stage] = std::shared_ptr<glslang::TShader>(shader);
	}

	std::vector<uint32_t> vertSpirV = shaderTools.GlslToSpv(stageShaders[EShLanguage::EShLangVertex].get());
	std::string onlineVertSpv = std::string((char*)vertSpirV.data(), vertSpirV.size() * sizeof(uint32_t));
	FileUtils::TryWriteFile("shaders/VulkanShader.vert.spv", onlineVertSpv);

	std::vector<uint32_t> fragSpirV = shaderTools.GlslToSpv(stageShaders[EShLanguage::EShLangFragment].get());
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
	std::for_each(stageShaders.begin(), stageShaders.end(), [&rawShaders](const std::pair<EShLanguage, std::shared_ptr<glslang::TShader>>& item) {
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

	spirv_cross::CompilerGLSL glsl(fragSpirV);

	// The SPIR-V is now parsed, and we can perform reflection on it.
	spirv_cross::ShaderResources resources = glsl.get_shader_resources();
	
	// Get all sampled images in the shader.
	//for (auto& resource : resources.sampled_images)
	//{
	//	unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
	//	unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
	//	printf("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);
	//
	//	// Modify the decoration to prepare it for GLSL.
	//	glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);
	//
	//	// Some arbitrary remapping if we want.
	//	glsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
	//}

	// Set some options.
	// spirv_cross::CompilerGLSL::Options options;
	// options.version = 310;
	// options.es = true;
	// glsl.set_common_options(options);

	// Compile to GLSL, ready to give to GL driver.
	std::string source = glsl.compile();


	std::cin.get();
	return 0;
}