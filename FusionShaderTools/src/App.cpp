#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <StandAlone/ResourceLimits.h>
#include <SPIRV/GLSL.std.450.h>

// Build-time generated includes
#include <glslang/build_info.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <vector>
#include <string>
#include <fstream>
#include <ostream>
#include <sstream>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;


EShLanguage ShaderTypeFromString(const std::string& stage)
{
	if (stage == "vertex")	return EShLanguage::EShLangVertex;
	if (stage == "fragment")	return EShLanguage::EShLangFragment;
	if (stage == "pixel")	return EShLanguage::EShLangFragment;
	if (stage == "geometry")	return EShLanguage::EShLangGeometry;

	assert(0 && "Unknown shader stage");
	return EShLanguage::EShLangCount;
}

bool TryReadFile(const fs::path& path, std::string& outString)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (!file) {
		std::cerr << "Could not open file: " << path.string() << std::endl;
		return false;
	}

	// Move to the end of the file to resize the resulting string
	file.seekg(0, std::ios::end);
	outString.resize(file.tellg());

	// Copy file from start to the end into the out string
	file.seekg(0, std::ios::beg);
	file.read(&outString[0], outString.size());

	file.close();
	return true;
}

bool TryWriteFile(const fs::path& path, const std::string& data)
{
	std::ofstream file(path, std::ios::binary | std::ios::out);
	if (!file) {
		std::cerr << "Could not open file: " << path.string() << std::endl;
		return false;
	}

	file << data;
	file.close();

	return true;
}


struct ShaderComponent
{
public:
	ShaderComponent(const fs::path& filePath)
		: FilePath(filePath)
	{
		std::string extension = FilePath.extension().string();
		Stage = ShaderTypeFromString(extension.substr(1));

		if (!TryReadFile(FilePath, Source)) {
			std::cout << "Failed to load Shader Source" << std::endl;
		}
	}

public:
	EShLanguage Stage;
	std::string Source;
	fs::path FilePath;
	std::vector<unsigned int> SpirV;
};

static bool glslangInitialized = false;

void DumpVersions() {
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


//
// Parse either a .conf file provided by the user or the default from glslang::DefaultTBuiltInResource
//
TBuiltInResource ProcessConfigFile(const std::string& filePath)
{
	TBuiltInResource outResources;
	if (filePath.size() == 0) {
		outResources = glslang::DefaultTBuiltInResource;
	}
	else {
		std::string configString;
		TryReadFile(filePath, configString);
		glslang::DecodeResourceLimits(&outResources, configString.data());
	}

	return outResources;
}

//TODO: Multithread, manage SpirV that doesn't need recompiling (only recompile when dirty)
bool CompileGLSL(ShaderComponent& shaderComp)
{
	// rationalize client and target language
	/*if (TargetLanguage == glslang::EShTargetNone) {
		switch (ClientVersion) {
		case glslang::EShTargetVulkan_1_0:
			TargetLanguage = glslang::EShTargetSpv;
			TargetVersion = glslang::EShTargetSpv_1_0;
			break;
		case glslang::EShTargetVulkan_1_1:
			TargetLanguage = glslang::EShTargetSpv;
			TargetVersion = glslang::EShTargetSpv_1_3;
			break;
		case glslang::EShTargetVulkan_1_2:
			TargetLanguage = glslang::EShTargetSpv;
			TargetVersion = glslang::EShTargetSpv_1_5;
			break;
		case glslang::EShTargetOpenGL_450:
			TargetLanguage = glslang::EShTargetSpv;
			TargetVersion = glslang::EShTargetSpv_1_0;
			break;
		default:
			break;
		}
	}*/


	glslang::TShader shader(shaderComp.Stage);
	const char* sourceStr = shaderComp.Source.c_str();
	//const char* fileName = shaderComp.FilePath.string().c_str();
	shader.setStrings(&sourceStr, 1);
	//shader.setStringsWithLengthsAndNames(&sourceStr, NULL, &fileName, 1);

	// std::string entryPoint = "main";
	// shader.setEntryPoint(entryPoint.c_str());
	// shader.setSourceEntryPoint(entryPoint.c_str());

	// Stores additional #define and #include.
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

	shader.setEnvInput(glslang::EShSourceGlsl, shaderComp.Stage, Client, ClientInputSemanticsVersion);
	shader.setEnvClient(Client, VulkanClientVersion);
	shader.setEnvTarget(TargetLanguage, TargetVersion);

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
	if (!shader.preprocess(&Resources, defaultVersion, EProfile::ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
	{
		std::cout << "GLSL Preprocessing Failed" << std::endl;
		std::cout << shader.getInfoLog() << std::endl;
		std::cout << shader.getInfoDebugLog() << std::endl;
		return false;
	}

	// std::cout << PreprocessedGLSL << std::endl;

	// const char* PreprocessedCStr = PreprocessedGLSL.c_str();
	// shader.setStrings(&PreprocessedCStr, 1);

	if (!shader.parse(&Resources, defaultVersion, false, messages, Includer))
	{
		std::cout << "GLSL Parsing Failed" << std::endl;
		std::cout << shader.getInfoLog() << std::endl;
		std::cout << shader.getInfoDebugLog() << std::endl;
		return false;
	}

	glslang::TProgram program;
	program.addShader(&shader);

	if (!program.link(messages))
	{
		std::cout << "GLSL Linking Failed" << std::endl;
		std::cout << shader.getInfoLog() << std::endl;
		std::cout << shader.getInfoDebugLog() << std::endl;
		return false;
	}

	if (!program.mapIO())
	{
		std::cout << "GLSL Linking (Mapping IO) Failed" << std::endl;
		std::cout << shader.getInfoLog() << std::endl;
		std::cout << shader.getInfoDebugLog() << std::endl;
		return false;
	}

	spv::SpvBuildLogger logger;
	glslang::SpvOptions spvOptions;
	// spvOptions.generateDebugInfo = false;
	// spvOptions.stripDebugInfo = false;
	// spvOptions.disableOptimizer = true;
	// spvOptions.optimizeSize = false;
	// spvOptions.disassemble = false;
	// spvOptions.validate = false;
	glslang::TIntermediate* intermediate = program.getIntermediate(shaderComp.Stage);
	glslang::GlslangToSpv(*intermediate, shaderComp.SpirV, &logger, &spvOptions);

	//TODO: Manage spirv that's already compiled
	//glslang::OutputSpvBin(SpirV, (GetSuffix(filename) + std::string(".spv")).c_str());

	if (logger.getAllMessages().length() > 0)
	{
		std::cout << logger.getAllMessages() << std::endl;
	}

	// Disassemble the final programm to glsl and print it to the console
	// spv::Disassemble(std::cout, shaderComp.SpirV);

	return true;
}

struct ShaderCompilerConfig {

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

struct CompilerResult {


};

class FusionShaderTools
{
public:
	FusionShaderTools(const ShaderCompilerConfig& config) {
		// from source: "ShInitialize() should be called exactly once per process, not per thread."
		glslang::InitializeProcess();
	}

	~FusionShaderTools() {
		glslang::FinalizeProcess();
	}

public:
	/*std::unique_ptr<ProgramData> Compile(const fs::path& filePath) {
		std::map<EShLanguage, std::string> shaderSources;
		LoadSource(filePath, shaderSources);

		return Compile(shaderSources);
	}

	std::unique_ptr<ProgramData> Compile(const std::map<EShLanguage, std::string> shaderSources) {

		std::unique_ptr<ProgramData> program = std::make_unique<ProgramData>();
		for (auto& [stage, source] : shaderSources) {
			ShaderData shader = Parse(stage, source);
			program.
		}


		Compile();

		return program;
	}*/

public:
	void LoadSource(const fs::path& filePath, std::map<EShLanguage, std::string>& outSourceMap) {
		std::string source;
		if (!TryReadFile(filePath, source)) {
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

	glslang::TShader* ParseStage(EShLanguage stage, const std::string& source)
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

	glslang::TProgram* LinkProgram(const std::vector<glslang::TShader*>& shaders)
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

	std::vector<uint32_t> GlslToSpv(glslang::TShader* inGlsl) {
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

	/*std::vector<char> SpvToGlsl(const std::vector<uint32_t>& inSpirV) {

		std::stringstream ss;
		// Disassemble the final programm to glsl and print it to the console
		spv::Disassemble(ss, inSpirV);

		std::string outGlsl = ss.str();
		return std::vector<char>(outGlsl.begin(), outGlsl.end());
	}*/

	bool WriteToDisk(glslang::TProgram* program, const fs::path& dir) {
		program->buildReflection();
		int numAttr = program->getNumPipeInputs();
		const glslang::TObjectReflection& input = program->getPipeInput(0);
		return true;
	}

};

int main(int argc, char** argv)
{
	// std::vector<char*> args = { argv[0], "-v", "-V", "shaders/shader.vert" };
	// maininternal(args.size(), args.data());

	ShaderCompilerConfig config;
	FusionShaderTools shaderTools(config);

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
	TryWriteFile("shaders/VulkanShader.vert.spv", onlineVertSpv);

	std::vector<uint32_t> fragSpirV = shaderTools.GlslToSpv(stageShaders[EShLanguage::EShLangFragment].get());
	std::string onlineFragSpv = std::string((char*)fragSpirV.data(), fragSpirV.size() * sizeof(uint32_t));
	TryWriteFile("shaders/VulkanShader.frag.spv", onlineFragSpv);

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

	std::cin.get();
	return 0;
}