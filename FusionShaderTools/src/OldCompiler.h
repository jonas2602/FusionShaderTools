//#pragma once
//
//struct ShaderComponent
//{
//public:
//	ShaderComponent(const fs::path& filePath)
//		: FilePath(filePath)
//	{
//		std::string extension = FilePath.extension().string();
//		Stage = ShaderTypeFromString(extension.substr(1));
//
//		if (!TryReadFile(FilePath, Source)) {
//			std::cout << "Failed to load Shader Source" << std::endl;
//		}
//	}
//
//public:
//	EShLanguage Stage;
//	std::string Source;
//	fs::path FilePath;
//	std::vector<unsigned int> SpirV;
//};
//
//static bool glslangInitialized = false;
//
////TODO: Multithread, manage SpirV that doesn't need recompiling (only recompile when dirty)
//bool CompileGLSL(ShaderComponent& shaderComp)
//{
//	// rationalize client and target language
//	/*if (TargetLanguage == glslang::EShTargetNone) {
//		switch (ClientVersion) {
//		case glslang::EShTargetVulkan_1_0:
//			TargetLanguage = glslang::EShTargetSpv;
//			TargetVersion = glslang::EShTargetSpv_1_0;
//			break;
//		case glslang::EShTargetVulkan_1_1:
//			TargetLanguage = glslang::EShTargetSpv;
//			TargetVersion = glslang::EShTargetSpv_1_3;
//			break;
//		case glslang::EShTargetVulkan_1_2:
//			TargetLanguage = glslang::EShTargetSpv;
//			TargetVersion = glslang::EShTargetSpv_1_5;
//			break;
//		case glslang::EShTargetOpenGL_450:
//			TargetLanguage = glslang::EShTargetSpv;
//			TargetVersion = glslang::EShTargetSpv_1_0;
//			break;
//		default:
//			break;
//		}
//	}*/
//
//
//	glslang::TShader shader(shaderComp.Stage);
//	const char* sourceStr = shaderComp.Source.c_str();
//	//const char* fileName = shaderComp.FilePath.string().c_str();
//	shader.setStrings(&sourceStr, 1);
//	//shader.setStringsWithLengthsAndNames(&sourceStr, NULL, &fileName, 1);
//
//	// std::string entryPoint = "main";
//	// shader.setEntryPoint(entryPoint.c_str());
//	// shader.setSourceEntryPoint(entryPoint.c_str());
//
//	// Stores additional #define and #include.
//	// TPreamble preamble;
//	// shader.setPreamble(preamble.get());
//	// shader.addProcesses({});
//
//	// Automatically map attribute bindings and uniform locations if none spezified
//	// shader.setAutoMapBindings(true);
//	// shader.setAutoMapLocations(true);
//
//	// invert position.Y output in vertex shader
//	// shader->setInvertY(true);
//
//	//Set up Vulkan/SpirV Environment
//	int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100
//	glslang::EShClient Client = glslang::EShClientVulkan;
//	glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;  // would map to, say, Vulkan 1.0
//	glslang::EShTargetLanguage TargetLanguage = glslang::EShTargetSpv;
//	glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;    // maps to, say, SPIR-V 1.0
//
//	shader.setEnvInput(glslang::EShSourceGlsl, shaderComp.Stage, Client, ClientInputSemanticsVersion);
//	shader.setEnvClient(Client, VulkanClientVersion);
//	shader.setEnvTarget(TargetLanguage, TargetVersion);
//
//	TBuiltInResource Resources;
//	Resources = glslang::DefaultTBuiltInResource;
//	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
//
//	// defines the default glsl version if non defined with "#version XXX"
//	int defaultVersion = 100;
//
//	// push directories that contain files with helper functions
//	// to include in the actual shader
//	DirStackFileIncluder Includer;
//	// fs::path folderPath = shaderComp.FilePath.parent_path();
//	// Includer.pushExternalLocalDirectory(folderPath.string());
//
//	// might generate intermediate shader string with #if, #define and #include resolved
//	std::string PreprocessedGLSL;
//	if (!shader.preprocess(&Resources, defaultVersion, EProfile::ENoProfile, false, false, messages, &PreprocessedGLSL, Includer))
//	{
//		std::cout << "GLSL Preprocessing Failed" << std::endl;
//		std::cout << shader.getInfoLog() << std::endl;
//		std::cout << shader.getInfoDebugLog() << std::endl;
//		return false;
//	}
//
//	// std::cout << PreprocessedGLSL << std::endl;
//
//	// const char* PreprocessedCStr = PreprocessedGLSL.c_str();
//	// shader.setStrings(&PreprocessedCStr, 1);
//
//	if (!shader.parse(&Resources, defaultVersion, false, messages, Includer))
//	{
//		std::cout << "GLSL Parsing Failed" << std::endl;
//		std::cout << shader.getInfoLog() << std::endl;
//		std::cout << shader.getInfoDebugLog() << std::endl;
//		return false;
//	}
//
//	glslang::TProgram program;
//	program.addShader(&shader);
//
//	if (!program.link(messages))
//	{
//		std::cout << "GLSL Linking Failed" << std::endl;
//		std::cout << shader.getInfoLog() << std::endl;
//		std::cout << shader.getInfoDebugLog() << std::endl;
//		return false;
//	}
//
//	if (!program.mapIO())
//	{
//		std::cout << "GLSL Linking (Mapping IO) Failed" << std::endl;
//		std::cout << shader.getInfoLog() << std::endl;
//		std::cout << shader.getInfoDebugLog() << std::endl;
//		return false;
//	}
//
//	spv::SpvBuildLogger logger;
//	glslang::SpvOptions spvOptions;
//	// spvOptions.generateDebugInfo = false;
//	// spvOptions.stripDebugInfo = false;
//	// spvOptions.disableOptimizer = true;
//	// spvOptions.optimizeSize = false;
//	// spvOptions.disassemble = false;
//	// spvOptions.validate = false;
//	glslang::TIntermediate* intermediate = program.getIntermediate(shaderComp.Stage);
//	glslang::GlslangToSpv(*intermediate, shaderComp.SpirV, &logger, &spvOptions);
//
//	//TODO: Manage spirv that's already compiled
//	//glslang::OutputSpvBin(SpirV, (GetSuffix(filename) + std::string(".spv")).c_str());
//
//	if (logger.getAllMessages().length() > 0)
//	{
//		std::cout << logger.getAllMessages() << std::endl;
//	}
//
//	// Disassemble the final programm to glsl and print it to the console
//	// spv::Disassemble(std::cout, shaderComp.SpirV);
//
//	return true;
//}