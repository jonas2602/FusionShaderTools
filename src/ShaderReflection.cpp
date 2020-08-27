#include "FusionShaderTools/ShaderReflection.h"

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



	ShaderAttributeInfo::ShaderAttributeInfo()
		: Name(""), Slot(-1)
		, Type(EShaderDataType::None)
	{ }

	ShaderAttributeInfo::ShaderAttributeInfo(const std::string& name, EShaderDataType type, uint32_t slot)
		: Name(name), Type(type), Slot(slot)
	{ }

	ShaderAttributeInfo::ShaderAttributeInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource)
	{
		Name = resource.name;

		const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
		Type = ExtractBaseType(type);

		Slot = compiler.get_decoration(resource.id, spv::DecorationLocation);
	}

	bool ShaderAttributeInfo::Serialize(nlohmann::json& archive) const
	{
		archive["name"] = Name;
		archive["type"] = ShaderDataToString(Type);
		archive["slot"] = Slot;

		return true;
	}

	bool ShaderAttributeInfo::Deserialize(const nlohmann::json& archive)
	{
		if (!archive.is_object()) { return false; }

		Name = archive["name"];
		Type = ShaderDataFromString(archive["type"]);
		Slot = archive["slot"];

		return true;
	}

	ShaderBindingInfo::ShaderBindingInfo()
		: Set(0), Binding(0), Count(0)
	{ }

	ShaderBindingInfo::ShaderBindingInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource)
	{
		Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

		// extract the array size (zero means not an array)
		const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
		Count = type.array[0];
	}

	bool ShaderBindingInfo::Serialize(nlohmann::json& archive) const
	{
		archive["binding"] = Binding;
		archive["set"] = Set;
		archive["count"] = Count;
		return true;
	}

	bool ShaderBindingInfo::Deserialize(const nlohmann::json& archive)
	{
		Set = archive["set"];
		Binding = archive["binding"];
		Count = archive["count"];
		return true;
	}

	ShaderUniformBlockInfo::ShaderUniformBlockInfo()
		: ShaderBindingInfo()
		, Name(""), Type("")
		, Layout("std140")
	{ }

	ShaderUniformBlockInfo::ShaderUniformBlockInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource)
		: ShaderBindingInfo(compiler, resource)
	{
		Name = compiler.get_name(resource.id);
		Type = resource.name;
		// TODO: Determine Layout (Alignment + Packing -> e.g. std140 or std430)
		Layout = "std140";

		const spirv_cross::SPIRType& type = compiler.get_type(resource.base_type_id);
		for (int i = 0; i < type.member_types.size(); i++) {
			spirv_cross::TypeID memberID = type.member_types[i];
			std::string memberName = compiler.get_member_name(resource.base_type_id, i);
			EShaderDataType memberType = ExtractBaseType(compiler.get_type(memberID));

			Elements.push_back(ShaderAttributeInfo(memberName, memberType, i));
		}
	}

	bool ShaderUniformBlockInfo::Serialize(nlohmann::json& archive) const
	{
		ShaderBindingInfo::Serialize(archive);

		archive["name"] = Name;
		archive["type"] = Type;
		archive["layout"] = Layout;

		archive["elements"] = nlohmann::json::array();
		for (const ShaderAttributeInfo& attribute : Elements) {
			nlohmann::json elementArchive = nlohmann::json::object();
			if (attribute.Serialize(elementArchive)) {
				archive["elements"].push_back(elementArchive);
			}
		}

		return true;
	}

	bool ShaderUniformBlockInfo::Deserialize(const nlohmann::json& archive)
	{
		if (!archive.is_object()) { return false; }

		ShaderBindingInfo::Deserialize(archive);

		Name = archive["name"];
		Type = archive["type"];
		Layout = archive["layout"];

		for (const nlohmann::json& elementArchive : archive["elements"]) {
			ShaderAttributeInfo attribute;
			if (attribute.Deserialize(elementArchive)) {
				Elements.push_back(attribute);
			}
		}

		return true;
	}

	ShaderImageSamplerInfo::ShaderImageSamplerInfo()
		: ShaderBindingInfo()
		, Name("")
	{ }

	ShaderImageSamplerInfo::ShaderImageSamplerInfo(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource)
		: ShaderBindingInfo(compiler, resource)
	{
		const spirv_cross::SPIRType& type = compiler.get_type(resource.type_id);
		Name = compiler.get_name(resource.id);
	}

	bool ShaderImageSamplerInfo::Serialize(nlohmann::json& archive) const
	{
		ShaderBindingInfo::Serialize(archive);

		archive["name"] = Name;
		return true;
	}

	bool ShaderImageSamplerInfo::Deserialize(const nlohmann::json& archive)
	{
		ShaderBindingInfo::Deserialize(archive);

		Name = archive["name"];
		return true;
	}

	ShaderStageInfo::ShaderStageInfo()
		: Type(EShaderStageType::None)
		, Path("")
	{ }

	ShaderStageInfo::ShaderStageInfo(EShaderStageType type, spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources)
	{
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

		for (spirv_cross::Resource& element : resources.sampled_images) {
			ImageSamplers.insert(ShaderImageSamplerInfo(compiler, element));
		}
	}

	bool ShaderStageInfo::Serialize(nlohmann::json& archive) const
	{
		archive["type"] = ShaderStageToString(Type);
		archive["source"] = Path.string();

		archive["inputs"] = nlohmann::json::array();
		for (const ShaderAttributeInfo& element : Inputs) {
			nlohmann::json elementArchive = nlohmann::json::object();
			if (element.Serialize(elementArchive)) {
				archive["inputs"].push_back(elementArchive);
			}
		}

		archive["outputs"] = nlohmann::json::array();
		for (const ShaderAttributeInfo& element : Outputs) {
			nlohmann::json elementArchive = nlohmann::json::object();
			if (element.Serialize(elementArchive)) {
				archive["outputs"].push_back(elementArchive);
			}
		}

		archive["uniform_blocks"] = nlohmann::json::array();
		for (const ShaderUniformBlockInfo& element : UniformBlocks) {
			nlohmann::json elementArchive = nlohmann::json::object();
			if (element.Serialize(elementArchive)) {
				archive["uniform_blocks"].push_back(elementArchive);
			}
		}

		archive["textures"] = nlohmann::json::array();
		for (const ShaderImageSamplerInfo& element : ImageSamplers) {
			nlohmann::json elementArchive = nlohmann::json::object();
			if (element.Serialize(elementArchive)) {
				archive["textures"].push_back(elementArchive);
			}
		}

		return true;
	}

	bool ShaderStageInfo::Deserialize(const nlohmann::json& archive)
	{
		if (!archive.is_object()) { return false; }

		Type = ShaderStageFromString(archive["type"]);
		Path = std::string(archive["source"]);

		for (const nlohmann::json& elementArchive : archive["inputs"]) {
			ShaderAttributeInfo element;
			if (element.Deserialize(elementArchive)) {
				Inputs.insert(element);
			}
		}

		for (const nlohmann::json& elementArchive : archive["outputs"]) {
			ShaderAttributeInfo element;
			if (element.Deserialize(elementArchive)) {
				Outputs.insert(element);
			}
		}

		for (const nlohmann::json& elementArchive : archive["uniform_blocks"]) {
			ShaderUniformBlockInfo element;
			if (element.Deserialize(elementArchive)) {
				UniformBlocks.insert(element);
			}
		}

		for (const nlohmann::json& elementArchive : archive["textures"]) {
			ShaderImageSamplerInfo element;
			if (element.Deserialize(elementArchive)) {
				ImageSamplers.insert(element);
			}
		}

		return true;
	}

	ProgramInfo::ProgramInfo(const std::string& name)
		: Name(name)
	{ }

	bool ProgramInfo::Serialize(nlohmann::json& archive) const
	{
		archive["name"] = Name;

		archive["stages"] = nlohmann::json::array();
		for (const ShaderStageInfo& element : Stages) {
			nlohmann::json elementArchive = nlohmann::json::object();
			if (element.Serialize(elementArchive)) {
				archive["stages"].push_back(elementArchive);
			}
		}
		return true;
	}

	bool ProgramInfo::Deserialize(const nlohmann::json& archive)
	{
		Name = archive["name"];

		for (const nlohmann::json& elementArchive : archive["stages"]) {
			ShaderStageInfo stage;
			if (stage.Deserialize(elementArchive)) {
				Stages.push_back(stage);
			}

		}
		return true;
	}

}