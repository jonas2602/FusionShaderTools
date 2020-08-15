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

	static EShaderDataType ExtractBaseType(const spirv_cross::SPIRType& type) {
		switch (type.basetype) {
		case spirv_cross::SPIRType::BaseType::Boolean: {
			if (type.columns == 1 && type.vecsize == 1) {
				return EShaderDataType::Bool;
			}
			break;
		}
		case spirv_cross::SPIRType::BaseType::Int: {
			switch (type.vecsize) {
			case 1: return EShaderDataType::Int;
			case 2: return EShaderDataType::Int2;
			case 3: return EShaderDataType::Int3;
			case 4: return EShaderDataType::Int4;
			}
			break;
		}
		case spirv_cross::SPIRType::BaseType::Float: {
			switch (type.columns) {
			case 1: {
				switch (type.vecsize) {
				case 1: return EShaderDataType::Float;
				case 2: return EShaderDataType::Float2;
				case 3: return EShaderDataType::Float3;
				case 4: return EShaderDataType::Float4;
				}
				break;
			}
			case 2: {
				if (type.vecsize == 2) {
					return EShaderDataType::Mat2;
				}
				break;
			}
			case 3: {
				if (type.vecsize == 3) {
					return EShaderDataType::Mat3;
				}
				break;
			}
			case 4: {
				if (type.vecsize == 4) {
					return EShaderDataType::Mat4;
				}
				break;
			}
			}
			break;
		}
		}

		return EShaderDataType::None;
	}

	struct ShaderAttributeInfo {
	public:
		ShaderAttributeInfo()
		{ }

		ShaderAttributeInfo(const std::string& name, EShaderDataType type, uint32_t slot)
			: Name(name), Type(type), Slot(slot)
		{ }

		ShaderAttributeInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource)
		{
			Name = resource.name;

			const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
			Type = ExtractBaseType(type);

			Slot = compiler.get_decoration(resource.id, spv::DecorationLocation);
		}

	public:
		bool Serialize(nlohmann::json& archive) const {
			archive["name"] = Name;
			archive["type"] = Type;
			archive["slot"] = Slot;

			return true;
		}

		bool Deserialize(const nlohmann::json& archive) {
			if (!archive.is_object()) { return false; }

			Name = archive["name"];
			Type = archive["type"];
			Slot = archive["slot"];

			return true;
		}

		bool operator<(const ShaderAttributeInfo& other) const {
			return other.Slot < Slot;
		}

	public:
		std::string Name;
		EShaderDataType Type;
		uint32_t Slot;
	};

	struct ShaderUniformBlockInfo {
	public:
		ShaderUniformBlockInfo()
		{ }

		ShaderUniformBlockInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource) {
			Name = compiler.get_name(resource.id);
			Type = resource.name;
			// TODO: Determine Layout (Alignment + Packing -> e.g. std140 or std430)
			Layout = "std140";
			Slot = compiler.get_decoration(resource.id, spv::DecorationBinding);

			const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
			for (int i = 0; i < type.member_types.size(); i++) {
				spirv_cross::TypeID memberID = type.member_types[i];
				std::string memberName = compiler.get_member_name(resource.base_type_id, i);
				EShaderDataType memberType = ExtractBaseType(compiler.get_type(memberID));

				Elements.push_back(ShaderAttributeInfo(memberName, memberType, i));
			}
		}

		bool Serialize(nlohmann::json& archive) const {
			archive["name"] = Name;
			archive["type"] = Type;
			archive["layout"] = Layout;
			archive["slot"] = Slot;

			archive["elements"] = nlohmann::json::array();
			for (const ShaderAttributeInfo& attribute : Elements) {
				nlohmann::json elementArchive = nlohmann::json::object();
				if (attribute.Serialize(elementArchive)) {
					archive["elements"].push_back(elementArchive);
				}
			}

			return true;
		}

		bool Deserialize(const nlohmann::json& archive) {
			if (!archive.is_object()) { return false; }

			Name = archive["name"];
			Type = archive["type"];
			Layout = archive["layout"];
			Slot = archive["slot"];

			for (const nlohmann::json& elementArchive : archive["elements"]) {
				ShaderAttributeInfo attribute;
				if (attribute.Deserialize(elementArchive)) {
					Elements.push_back(attribute);
				}
			}

			return true;
		}

		bool operator<(const ShaderUniformBlockInfo& other) const {
			return other.Slot < Slot;
		}

	public:
		std::string Name;
		std::string Type;
		std::string Layout;
		uint32_t Slot;
		std::vector<ShaderAttributeInfo> Elements;
	};

	struct ShaderStageInfo {
	public:
		ShaderStageInfo(EShaderStageType type, spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources) {
			Type = type;

			for (spirv_cross::Resource& element : resources.stage_inputs) {
				Inputs.insert(ShaderAttributeInfo(compiler, element));
			}

			for (spirv_cross::Resource& element : resources.stage_outputs) {
				Outputs.insert(ShaderAttributeInfo(compiler, element));
			}

			for (spirv_cross::Resource& element : resources.uniform_buffers) {
				UniformBlocks.insert(ShaderUniformBlockInfo(compiler, element));
			}
		}

	public:
		bool Serialize(nlohmann::json& archive) const {
			archive["type"] = Type;
			archive["source"] = Path;

			archive["inputs"] = nlohmann::json::array();
			for (const ShaderAttributeInfo& attribute : Inputs) {
				nlohmann::json elementArchive = nlohmann::json::object();
				if (attribute.Serialize(elementArchive)) {
					archive["inputs"].push_back(elementArchive);
				}
			}

			archive["outputs"] = nlohmann::json::array();
			for (const ShaderAttributeInfo& attribute : Outputs) {
				nlohmann::json elementArchive = nlohmann::json::object();
				if (attribute.Serialize(elementArchive)) {
					archive["outputs"].push_back(elementArchive);
				}
			}

			archive["uniform_blocks"] = nlohmann::json::array();
			for (const ShaderUniformBlockInfo& uniformblock : UniformBlocks) {
				nlohmann::json elementArchive = nlohmann::json::object();
				if (uniformblock.Serialize(elementArchive)) {
					archive["uniform_blocks"].push_back(elementArchive);
				}
			}

			return true;
		}

		bool Deserialize(const nlohmann::json& archive) {
			if (!archive.is_object()) { return false; }

			Type = archive["type"];
			Path = std::string(archive["source"]);

			for (const nlohmann::json& elementArchive : archive["inputs"]) {
				ShaderAttributeInfo attribute;
				if (attribute.Deserialize(elementArchive)) {
					Inputs.insert(attribute);
				}
			}

			for (const nlohmann::json& elementArchive : archive["outputs"]) {
				ShaderAttributeInfo attribute;
				if (attribute.Deserialize(elementArchive)) {
					Outputs.insert(attribute);
				}
			}

			for (const nlohmann::json& elementArchive : archive["uniform_blocks"]) {
				ShaderUniformBlockInfo uniformBlock;
				if (uniformBlock.Deserialize(elementArchive)) {
					UniformBlocks.insert(uniformBlock);
				}
			}
			return true;
		}

	public:
		EShaderStageType Type;
		fs::path Path;

		std::set<ShaderAttributeInfo> Inputs;
		std::set<ShaderAttributeInfo> Outputs;
		std::set<ShaderUniformBlockInfo> UniformBlocks;
	};

	// TODO: rename to program info?
	struct ShaderInfo {

	public:
		bool Serialize(nlohmann::json& archive) const {
			return true;
		}

		bool Deserialize(const nlohmann::json& archive) {
			return true;
		}

	public:
		std::string Name;
		std::vector<ShaderStageInfo> Stages;
	};

}