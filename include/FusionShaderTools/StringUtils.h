#pragma once

#include <map>
#include <cassert>

#include <fstream>
#include <ostream>

#include <iostream>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

namespace FusionShaderTools {

	class StringUtils
	{
	public:
		static bool TryReadFile(const fs::path& path, std::string& outString)
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

		static bool TryWriteFile(const fs::path& path, const std::string& data)
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

	public:
		static std::string StringUtils::ToLowerCase(const std::string& str)
		{
			std::string data = str;
			std::transform(data.begin(), data.end(), data.begin(),
				[](unsigned char c) { return std::tolower(c); }
			);

			return data;
		}

		static bool SplitRegions(const std::string& source, std::unordered_map<std::string, std::string>& outRegions)
		{
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
				// EShLanguage stageType = ShaderTypeFromString(typeName);
				// assert(stageType != EShLanguage::EShLangCount && "Invalid shader type specified");

				// skip all following line breaks to find the start of the shader content
				size_t contentStart = source.find_first_not_of("\r\n", endType);

				// find the next appearence of a type definition
				pos = source.find(typeToken, contentStart);

				// read the content between the current and next type definition -> shader content
				// if no next type definition exists the rest of the file is the actual shader content
				size_t contentLength = contentStart == std::string::npos ? source.size() - 1 : contentStart;
				std::string stageSource = source.substr(contentStart, pos - contentLength);
				outRegions[typeName] = stageSource;
			}

			return true;
		}

	};

}