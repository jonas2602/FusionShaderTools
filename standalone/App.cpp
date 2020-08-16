#include "FusionShaderTools/Utils.h"
#include "FusionShaderTools/SpirVCompiler.h"
#include "FusionShaderTools.h"
#include "FusionShaderTools/ShaderTypes.h"

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>
#include <spirv_parser.hpp>
#include <spirv_reflect.hpp>

void Test() {
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
		// printf("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);
		// 
		// // Modify the decoration to prepare it for GLSL.
		// glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);
		// 
		// // Some arbitrary remapping if we want.
		// glsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
	}

	// Set some options.
	// spirv_cross::CompilerGLSL::Options options;
	// options.version = 310;
	// options.es = true;
	// glsl.set_common_options(options);

	// Compile to GLSL, ready to give to GL driver.
	// std::string source = glsl.compile();
}

enum class EShaderLanguage {
	GLSL,
	HLSL,
	SPV,
	MSL,
};

struct ShaderCompilerConfig {
public:
	bool ContainsLanguage(EShaderLanguage type) const {
		return OutputLanguages.find(type) != OutputLanguages.end();
	}

public:
	fs::path FilePath;
	fs::path OutputDirectory;
	std::set<EShaderLanguage> OutputLanguages;
};

bool ProcessArguments(int argc, char** argv, ShaderCompilerConfig& outConfig) {
	// early out when not enough arguments available
	if (argc < 2) {
		std::cout << "Compiler needs at least the source path specified" << std::endl;
		return false;
	}

	for (int i = 0; i < argc; i++) {
		std::string argument = argv[i];

		if (i == 0) {
			// ignore argument zero for now
		}
		else if (i == 1) {
			// next argument must always be the filepath
			if (argument.front() == '-') {
				std::cout << "first argument must be the filepath not an option" << std::endl;
				return false;
			}

			outConfig.FilePath = argument;
			if (outConfig.FilePath.extension() != ".glsl") {
				std::cout << "fusion shader processor only supports glsl as input" << std::endl;
				return false;
			}
		}
		else {
			argument = FusionShaderTools::StringUtils::ToLowerCase(argument);
			if (argument == "-spv") { outConfig.OutputLanguages.insert(EShaderLanguage::SPV); }
			if (argument == "-glsl") { outConfig.OutputLanguages.insert(EShaderLanguage::GLSL); }
			if (argument == "-hlsl") { outConfig.OutputLanguages.insert(EShaderLanguage::HLSL); }
			if (argument == "-msl") { outConfig.OutputLanguages.insert(EShaderLanguage::MSL); }
			if (argument == "-o") {
				i++;
				if (i >= argc) {
					std::cout << "-o must be followed by the output directory" << std::endl;
					return false;
				}
				std::string argument = argv[i];
				if (argument.front() == '-') {
					std::cout << "-o must be followed by the output directory" << std::endl;
					return false;
				}
				outConfig.OutputDirectory = argument;
			}
		}
	}

	return true;
}

//
// Return codes from main/exit().
//
enum EFailCode {
	Success = 0,
	FailUsage,
	FailCompile,
	FailLink,
	FailCompilerCreate,
	FailThreadCreate,
	FailLinkerCreate
};

//
// Give error and exit with failure code.
//
#define Error(msg, code) \
	std::cerr << msg << std::endl; \
	std::cin.get(); \
	exit(code);


int main(int argc, char** argv)
{
	ShaderCompilerConfig outConfig;
	// std::vector<char*> args = { "i have no idea", "shaders/VulkanShader.glsl", "-glsl", "-spv", "-o", "compiled/" };
	std::cout << "Processing Arguments ..." << std::endl;
	if (!ProcessArguments(argc, argv, outConfig)) {
		Error("Failed to Process Arguments", EFailCode::FailUsage);
	}
	std::cout << "Processing Arguments Finished" << std::endl;

	std::string shaderName = outConfig.FilePath.stem().string();
	fs::path shaderPath = outConfig.FilePath.parent_path();
	fs::path compiledPath = shaderPath / outConfig.OutputDirectory;

	if (outConfig.OutputLanguages.size() > 0) {
		fs::create_directories(compiledPath);
	}
	else {
		std::cout << "No Output Language requested, just generating meta file" << std::endl;
	}

	std::cout << std::endl;

	std::cout << "Reading Source File from: " << outConfig.FilePath << " ..." << std::endl;
	std::string sourceString;
	if (!FileUtils::TryReadFile(outConfig.FilePath, sourceString)) {
		Error("Failed to read Source File", EFailCode::FailUsage);
	}

	FusionShaderTools::ShaderSource shaderSource = { sourceString };
	std::vector<FusionShaderTools::ShaderStage_Source> stagesSources;
	FusionShaderTools::FusionShaderUtils::SplitShaderSource(shaderSource, stagesSources);
	if (stagesSources.empty()) {
		Error("Source contains no Stages", EFailCode::FailUsage);
	}

	std::cout << "Received: ";
	for (const FusionShaderTools::ShaderStage_Source& stage : stagesSources) {
		std::cout << FusionShaderTools::ShaderStageToString(stage.Type) << " ";
	}
	std::cout << "stages" << std::endl;

	std::cout << std::endl;

	FusionShaderTools::ShaderInfo programInfo(shaderName);
	for (const FusionShaderTools::ShaderStage_Source& stage : stagesSources) {
		std::cout << "Start Processing Shader Stage " << FusionShaderTools::ShaderStageToString(stage.Type) << std::endl;

		std::cout << "Compiling ..." << std::endl;
		FusionShaderTools::ShaderStage_SpirV spirv = FusionShaderTools::FusionShaderUtils::CompileStage_SPIRV(stage);

		std::cout << "Generating Meta Data ..." << std::endl;
		FusionShaderTools::ShaderStageInfo stageInfo = FusionShaderTools::FusionShaderUtils::GetStageInfo(spirv);
		stageInfo.Path = outConfig.OutputDirectory / (shaderName + spirv.GetExtensionMinimal());
		programInfo.Stages.push_back(stageInfo);

		if (outConfig.ContainsLanguage(EShaderLanguage::SPV)) {
			std::cout << "Building SPIR-V File ..." << std::endl;
			fs::path writePath = compiledPath / (shaderName + spirv.GetExtension());
			if (!FileUtils::TryWriteFile(writePath, spirv.ToString())) {
				Error("Failed to write into " << writePath, EFailCode::FailUsage);
			}
		}
		if (outConfig.ContainsLanguage(EShaderLanguage::GLSL)) {
			std::cout << "Building GLSL File ..." << std::endl;
			FusionShaderTools::ShaderStage_GLSL glsl = FusionShaderTools::FusionShaderUtils::CompileStage_GLSL(spirv);
			fs::path writePath = compiledPath / (shaderName + glsl.GetExtension());
			if (!FileUtils::TryWriteFile(writePath, glsl.ToString())) {
				Error("Failed to write into " << writePath, EFailCode::FailUsage);
			}
		}
		if (outConfig.ContainsLanguage(EShaderLanguage::HLSL)) {
			std::cout << "Building HLSL File ..." << std::endl;
			FusionShaderTools::ShaderStage_GLSL hlsl = FusionShaderTools::FusionShaderUtils::CompileStage_GLSL(spirv);
			fs::path writePath = compiledPath / (shaderName + hlsl.GetExtension());
			if (!FileUtils::TryWriteFile(writePath, hlsl.ToString())) {
				Error("Failed to write into " << writePath, EFailCode::FailUsage);
			}
		}
		if (outConfig.ContainsLanguage(EShaderLanguage::MSL)) {
			std::cout << "Building MSL File ..." << std::endl;
			FusionShaderTools::ShaderStage_GLSL msl = FusionShaderTools::FusionShaderUtils::CompileStage_GLSL(spirv);
			fs::path writePath = compiledPath / (shaderName + msl.GetExtension());
			if (!FileUtils::TryWriteFile(writePath, msl.ToString())) {
				Error("Failed to write into " << writePath, EFailCode::FailUsage);
			}
		}

		std::cout << "Finished Processing Shader Stage " << FusionShaderTools::ShaderStageToString(stage.Type) << std::endl << std::endl;
	}

	std::cout << "Building Meta File ..." << std::endl;
	nlohmann::json archive = nlohmann::json::object();
	programInfo.Serialize(archive);
	fs::path metaPath = shaderPath / (shaderName + ".fshader");
	if (!FileUtils::TryWriteFile(metaPath, archive.dump(4))) {
		Error("Failed to write into " << metaPath, EFailCode::FailUsage);
	}

	std::cout << "Shader Compiler Finished successfully" << std::endl;

	std::cin.get();
	return 0;
}