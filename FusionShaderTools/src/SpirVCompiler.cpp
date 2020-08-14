#include "SpirVCompiler.h"

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <StandAlone/ResourceLimits.h>
#include <SPIRV/GLSL.std.450.h>

// Build-time generated includes
#include <glslang/build_info.h>


EShLanguage ShaderTypeFromString(const std::string& stage)
{
	if (stage == "vertex")	return EShLanguage::EShLangVertex;
	if (stage == "fragment")	return EShLanguage::EShLangFragment;
	if (stage == "pixel")	return EShLanguage::EShLangFragment;
	if (stage == "geometry")	return EShLanguage::EShLangGeometry;

	assert(0 && "Unknown shader stage");
	return EShLanguage::EShLangCount;
}

// Parse either a .conf file provided by the user or the default from glslang::DefaultTBuiltInResource
TBuiltInResource ProcessConfigFile(const std::string& filePath)
{
	TBuiltInResource outResources;
	if (filePath.size() == 0) {
		outResources = glslang::DefaultTBuiltInResource;
	}
	else {
		std::string configString;
		FileUtils::TryReadFile(filePath, configString);
		glslang::DecodeResourceLimits(&outResources, configString.data());
	}

	return outResources;
}

SpirVCompiler::SpirVCompiler(const ShaderCompilerConfig& config) {
	// from source: "ShInitialize() should be called exactly once per process, not per thread."
	glslang::InitializeProcess();
}

SpirVCompiler::~SpirVCompiler() {
	glslang::FinalizeProcess();
}

void SpirVCompiler::DumpVersions() {
	printf("Glslang Version: %d:%d.%d.%d%s\n", glslang::GetSpirvGeneratorVersion(), GLSLANG_VERSION_MAJOR, GLSLANG_VERSION_MINOR, GLSLANG_VERSION_PATCH, GLSLANG_VERSION_FLAVOR);
	printf("ESSL Version: %s\n", glslang::GetEsslVersionString());
	printf("GLSL Version: %s\n", glslang::GetGlslVersionString());
	std::string spirvVersion;
	glslang::GetSpirvVersion(spirvVersion);
	printf("SPIR-V Version %s\n", spirvVersion.c_str());
	printf("GLSL.std.450 Version %d, Revision %d\n", GLSLstd450Version, GLSLstd450Revision);
	printf("Khronos Tool ID %d\n", glslang::GetKhronosToolId());
	printf("SPIR-V Generator Version %d\n", glslang::GetSpirvGeneratorVersion());
	printf("GL_KHR_vulkan_glsl version %d\n", 100);
	printf("ARB_GL_gl_spirv version %d\n", 100);
}

void SpirVCompiler::LoadSource(const fs::path& filePath, std::map<EShLanguage, std::string>& outSourceMap) {
	std::string source;
	if (!FileUtils::TryReadFile(filePath, source)) {
		std::cout << "Failed to load Shader Source" << std::endl;
	}

	const char* typeToken = "#type";
	size_t typeTokenLength = strlen(typeToken);

	// find the first type definition
	// everything above will get ignored
	// -> space for comments
	size_t pos = source.find(typeToken, 0);
	while (pos != std::string::npos)
	{
		// Calculate bounds of the type name
		size_t beginType = pos + typeTokenLength + 1;
		size_t endType = source.find_first_of("\r\n", pos);
		assert(endType != std::string::npos && "Syntax error");

		// convert the type name into the used enum type
		std::string typeName = source.substr(beginType, endType - beginType);
		EShLanguage stageType = ShaderTypeFromString(typeName);
		assert(stageType != EShLanguage::EShLangCount && "Invalid shader type specified");

		// skip all following line breaks to find the start of the shader content
		size_t contentStart = source.find_first_not_of("\r\n", endType);

		// find the next appearence of a type definition
		pos = source.find(typeToken, contentStart);

		// read the content between the current and next type definition -> shader content
		// if no next type definition exists the rest of the file is the actual shader content
		size_t contentLength = contentStart == std::string::npos ? source.size() - 1 : contentStart;
		std::string stageSource = source.substr(contentStart, pos - contentLength);
		// ShaderData stage(stageType, stageSource);
		// outStages.push_back(stage);
		outSourceMap[stageType] = stageSource;
	}
}

glslang::TShader* SpirVCompiler::ParseStage(EShLanguage stage, const std::string& source)
{
	glslang::TShader* outShader = new glslang::TShader(stage);

	const char* sourceStr = source.data();
	outShader->setStrings(&sourceStr, 1);

	// std::string entryPoint = "main";
	// shader.setEntryPoint(entryPoint.c_str());
	// shader.setSourceEntryPoint(entryPoint.c_str());

	// Stores additional #define and #undef.
	// TPreamble preamble;
	// shader.setPreamble(preamble.get());
	// shader.addProcesses({});

	// Automatically map attribute bindings and uniform locations if none spezified
	// shader.setAutoMapBindings(true);
	// shader.setAutoMapLocations(true);

	// invert position.Y output in vertex shader
	// shader->setInvertY(true);

	//Set up Vulkan/SpirV Environment
	int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
	glslang::EShClient Client = glslang::EShClientVulkan;
	glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;  // would map to, say, Vulkan 1.0
	glslang::EShTargetLanguage TargetLanguage = glslang::EShTargetSpv;
	glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;    // maps to, say, SPIR-V 1.0

	outShader->setEnvInput(glslang::EShSourceGlsl, stage, Client, ClientInputSemanticsVersion);
	outShader->setEnvClient(Client, VulkanClientVersion);
	outShader->setEnvTarget(TargetLanguage, TargetVersion);

	TBuiltInResource Resources;
	Resources = glslang::DefaultTBuiltInResource;
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	// defines the default glsl version if non defined with "#version XXX"
	int defaultVersion = 100;

	// push directories that contain files with helper functions
	// to include in the actual shader
	DirStackFileIncluder Includer;
	// fs::path folderPath = shaderComp.FilePath.parent_path();
	// Includer.pushExternalLocalDirectory(folderPath.string());

	// might generate intermediate shader string with #if, #define and #include resolved
	std::string PreprocessedGLSL;
	if (!outShader->preprocess(&Resources, defaultVersion, EProfile::ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
	{
		std::cout << "GLSL Preprocessing Failed" << std::endl;
		std::cout << outShader->getInfoLog() << std::endl;
		std::cout << outShader->getInfoDebugLog() << std::endl;
		return nullptr;
	}

	// std::cout << PreprocessedGLSL << std::endl;

	// const char* PreprocessedCStr = PreprocessedGLSL.c_str();
	// shader.setStrings(&PreprocessedCStr, 1);

	if (!outShader->parse(&Resources, defaultVersion, false, messages, Includer))
	{
		std::cout << "GLSL Parsing Failed" << std::endl;
		std::cout << outShader->getInfoLog() << std::endl;
		std::cout << outShader->getInfoDebugLog() << std::endl;
		return nullptr;
	}

	return outShader;
}

glslang::TProgram* SpirVCompiler::LinkProgram(const std::vector<glslang::TShader*>& shaders)
{
	glslang::TProgram* outProgram = new glslang::TProgram();
	for (glslang::TShader* stage : shaders) {
		outProgram->addShader(stage);
	}

	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
	if (!outProgram->link(messages))
	{
		std::cout << "GLSL Linking Failed" << std::endl;
		std::cout << outProgram->getInfoLog() << std::endl;
		std::cout << outProgram->getInfoDebugLog() << std::endl;
		return nullptr;
	}

	if (!outProgram->mapIO())
	{
		std::cout << "GLSL Linking (Mapping IO) Failed" << std::endl;
		std::cout << outProgram->getInfoLog() << std::endl;
		std::cout << outProgram->getInfoDebugLog() << std::endl;
		return nullptr;
	}

	return outProgram;
}

std::vector<uint32_t> SpirVCompiler::GlslToSpv(glslang::TShader* inGlsl) {
	spv::SpvBuildLogger logger;
	glslang::SpvOptions spvOptions;
	// spvOptions.generateDebugInfo = false;
	// spvOptions.stripDebugInfo = false;
	// spvOptions.disableOptimizer = true;
	// spvOptions.optimizeSize = false;
	// spvOptions.disassemble = false;
	// spvOptions.validate = false;

	glslang::TIntermediate* intermediate = inGlsl->getIntermediate();
	std::vector<uint32_t> outSpirV;
	glslang::GlslangToSpv(*intermediate, outSpirV, &logger, &spvOptions);

	if (logger.getAllMessages().length() > 0)
	{
		std::cout << logger.getAllMessages() << std::endl;
	}

	return outSpirV;
}

bool SpirVCompiler::WriteToDisk(glslang::TProgram* program, const fs::path& dir) {
	program->buildReflection();
	int numAttr = program->getNumPipeInputs();
	const glslang::TObjectReflection& input = program->getPipeInput(0);
	return true;
}
