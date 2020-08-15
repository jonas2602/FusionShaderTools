#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <ostream>
#include <sstream>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

class FileUtils 
{
protected:
	static bool TryReadFile_Internal(const fs::path& path, char* storage) {

	}

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

	template<typename T>
	static bool TryReadFile(const fs::path& path, std::vector<T>& outData)
	{
		std::ifstream file(path, std::ios::in | std::ios::binary);
		if (!file) {
			std::cerr << "Could not open file: " << path.string() << std::endl;
			return false;
		}

		// Move to the end of the file to resize the resulting string
		file.seekg(0, std::ios::end);
		outData.resize(file.tellg() / sizeof(T));

		// Copy file from start to the end into the out string
		file.seekg(0, std::ios::beg);
		file.read(&outData[0], outString.size() * sizeof(T));

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

};