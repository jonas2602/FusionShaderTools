#pragma once

#include "ShaderTypes.h"

#include <string>
#include <vector>
#include <set>
#include <filesystem>
namespace fs = std::filesystem;

#include <nlohmann/json.hpp>

#include <spirv_cross.hpp>
#include <spirv_reflect.hpp>

namespace FusionShaderTools {

	struct ShaderAttributeInfo
	{
	public:
		ShaderAttributeInfo();
		ShaderAttributeInfo(const std::string& name, EShaderDataType type, uint32_t slot);
		ShaderAttributeInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource);

	public:
		bool Serialize(nlohmann::json& archive) const;
		bool Deserialize(const nlohmann::json& archive);

		bool operator<(const ShaderAttributeInfo& other) const {
			return Slot < other.Slot;
		}

	public:
		std::string Name;
		EShaderDataType Type;
		uint32_t Slot;
	};

	struct ShaderUniformBlockInfo 
	{
	public:
		ShaderUniformBlockInfo();
		ShaderUniformBlockInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource);

	public:
		bool Serialize(nlohmann::json& archive) const;
		bool Deserialize(const nlohmann::json& archive);

		bool operator<(const ShaderUniformBlockInfo& other) const {
			return Slot < other.Slot;
		}

	public:
		std::string Name;
		std::string Type;
		std::string Layout;
		uint32_t Slot;
		std::vector<ShaderAttributeInfo> Elements;
	};

	struct ShaderImageSamplerInfo
	{
	public:
		ShaderImageSamplerInfo();
		ShaderImageSamplerInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource);

	public:
		bool Serialize(nlohmann::json& archive) const;
		bool Deserialize(const nlohmann::json& archive);

		bool operator<(const ShaderImageSamplerInfo& other) const {
			return Slot < other.Slot;
		}

	public:
		std::string Name;
		uint32_t Slot;
	};

	struct ShaderStageInfo
	{
	public:
		ShaderStageInfo();
		ShaderStageInfo(EShaderStageType type, spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources);

	public:
		bool Serialize(nlohmann::json& archive) const;
		bool Deserialize(const nlohmann::json& archive);

	public:
		EShaderStageType Type;
		fs::path Path;

		std::set<ShaderAttributeInfo> Inputs;
		std::set<ShaderAttributeInfo> Outputs;
		std::set<ShaderUniformBlockInfo> UniformBlocks;
		std::set<ShaderImageSamplerInfo> ImageSamplers;
	};

	// TODO: rename to program info?
	struct ShaderInfo 
	{
	public:
		ShaderInfo(const std::string& name);

	public:
		bool Serialize(nlohmann::json& archive) const;
		bool Deserialize(const nlohmann::json& archive);

	public:
		std::string Name;
		std::vector<ShaderStageInfo> Stages;
	};

}