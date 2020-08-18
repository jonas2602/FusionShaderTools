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

	struct ShaderInfoBase {
	public:
		~ShaderInfoBase() { }

	public:
		virtual bool Serialize(nlohmann::json& archive) const = 0;
		virtual bool Deserialize(const nlohmann::json& archive) = 0;
	};

	struct ShaderAttributeInfo : public ShaderInfoBase
	{
	public:
		ShaderAttributeInfo();
		ShaderAttributeInfo(const std::string& name, EShaderDataType type, uint32_t slot);
		ShaderAttributeInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource);

	public:
		virtual bool Serialize(nlohmann::json& archive) const override;
		virtual bool Deserialize(const nlohmann::json& archive) override;

		bool operator<(const ShaderAttributeInfo& other) const {
			return Slot < other.Slot;
		}

	public:
		std::string Name;
		EShaderDataType Type;
		uint32_t Slot;
	};

	struct ShaderBindingInfo : public ShaderInfoBase
	{
	public:
		ShaderBindingInfo();
		ShaderBindingInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource);

	public:
		virtual bool Serialize(nlohmann::json& archive) const override;
		virtual bool Deserialize(const nlohmann::json& archive) override;

		bool operator<(const ShaderBindingInfo& other) const {
			return std::tie(Set, Binding) < std::tie(other.Set, other.Binding);
		}
	public:
		uint32_t Binding;
		uint32_t Set;
	};

	struct ShaderUniformBlockInfo : public ShaderBindingInfo
	{
	public:
		ShaderUniformBlockInfo();
		ShaderUniformBlockInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource);

	public:
		virtual bool Serialize(nlohmann::json& archive) const override;
		virtual bool Deserialize(const nlohmann::json& archive) override;

	public:
		std::string Name;
		std::string Type;
		std::string Layout;
		std::vector<ShaderAttributeInfo> Elements;
	};

	struct ShaderImageSamplerInfo : public ShaderBindingInfo
	{
	public:
		ShaderImageSamplerInfo();
		ShaderImageSamplerInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource);

	public:
		virtual bool Serialize(nlohmann::json& archive) const override;
		virtual bool Deserialize(const nlohmann::json& archive) override;


	public:
		std::string Name;
	};

	struct ShaderStageInfo : public ShaderInfoBase
	{
	public:
		ShaderStageInfo();
		ShaderStageInfo(EShaderStageType type, spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources);

	public:
		virtual bool Serialize(nlohmann::json& archive) const override;
		virtual bool Deserialize(const nlohmann::json& archive) override;

	public:
		EShaderStageType Type;
		fs::path Path;

		std::set<ShaderAttributeInfo> Inputs;
		std::set<ShaderAttributeInfo> Outputs;
		std::set<ShaderUniformBlockInfo> UniformBlocks;
		std::set<ShaderImageSamplerInfo> ImageSamplers;
	};

	// TODO: rename to program info?
	struct ProgramInfo : public ShaderInfoBase
	{
	public:
		ProgramInfo(const std::string& name);

	public:
		virtual bool Serialize(nlohmann::json& archive) const override;
		virtual bool Deserialize(const nlohmann::json& archive) override;

	public:
		std::string Name;
		std::vector<ShaderStageInfo> Stages;
	};

}