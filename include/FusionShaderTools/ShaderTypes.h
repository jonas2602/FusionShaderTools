#pragma once

#include <string>
#include <vector>

namespace FusionShaderTools {

	enum class EShaderDataType
	{
		None = -1,
		Float, Float2, Float3, Float4,
		Mat2, Mat3, Mat4,
		Int, Int2, Int3, Int4,
		Bool,
	};

	static EShaderDataType ShaderDataFromString(const std::string& name) {
		if (name == "float")	{ return EShaderDataType::Float; }
		if (name == "float2")	{ return EShaderDataType::Float2; }
		if (name == "float3")	{ return EShaderDataType::Float3; }
		if (name == "float4")	{ return EShaderDataType::Float4; }
		if (name == "mat2")		{ return EShaderDataType::Mat2; }
		if (name == "mat3")		{ return EShaderDataType::Mat3; }
		if (name == "mat4")		{ return EShaderDataType::Mat4; }
		if (name == "int")		{ return EShaderDataType::Int; }
		if (name == "int2")		{ return EShaderDataType::Int2; }
		if (name == "int3")		{ return EShaderDataType::Int3; }
		if (name == "int4")		{ return EShaderDataType::Int4; }
		if (name == "bool")		{ return EShaderDataType::Bool; }

		return EShaderDataType::None;
	}

	static std::string ShaderDataToString(EShaderDataType type) {
		switch (type) {
		case EShaderDataType::Float:	return "float";
		case EShaderDataType::Float2:	return "float2";
		case EShaderDataType::Float3:	return "float3";
		case EShaderDataType::Float4:	return "float4";
		case EShaderDataType::Mat2:		return "mat2";
		case EShaderDataType::Mat3:		return "mat3";
		case EShaderDataType::Mat4:		return "mat4";
		case EShaderDataType::Int:		return "int";
		case EShaderDataType::Int2:		return "int2";
		case EShaderDataType::Int3:		return "int3";
		case EShaderDataType::Int4:		return "int4";
		case EShaderDataType::Bool:		return "bool";
		}
		
		return "";
	}

	enum class EShaderStageType {
		None = -1,
		Vertex,
		Geometry,
		Fragment,
	};

	static EShaderStageType ShaderStageFromString(const std::string& name)
	{
		if (name == "vertex")		return EShaderStageType::Vertex;
		if (name == "vert")			return EShaderStageType::Vertex;
		if (name == "geometry")		return EShaderStageType::Geometry;
		if (name == "geom")			return EShaderStageType::Geometry;
		if (name == "fragment")		return EShaderStageType::Fragment;
		if (name == "frag")			return EShaderStageType::Fragment;
		if (name == "pixel")		return EShaderStageType::Fragment;

		return EShaderStageType::None;
	}

	static std::string ShaderStageToString(EShaderStageType type) {
		switch (type) {
		case EShaderStageType::Vertex:		return "vertex";
		case EShaderStageType::Geometry:	return "geometry";
		case EShaderStageType::Fragment:	return "fragment";
		}

		return "";
	}

	static std::string ShaderStageToExtension(EShaderStageType type) {
		switch (type) {
		case EShaderStageType::Vertex:		return "vert";
		case EShaderStageType::Geometry:	return "geom";
		case EShaderStageType::Fragment:	return "frag";
		}

		return "";
	}

	struct ShaderSource {
		std::string Data;
	};

	struct ProgramSource {

	};

	struct ShaderStage
	{
	public:
		ShaderStage(EShaderStageType type)
			: Type(type)
		{ }

		virtual ~ShaderStage() { }

		virtual std::string ToString() const = 0;
		std::string GetExtensionMinimal() const {
			return "." + ShaderStageToExtension(Type);
		}
		virtual std::string GetExtension() const {
			return GetExtensionMinimal();
		}

	public:
		EShaderStageType Type;
	};

	struct ShaderStage_Source : public ShaderStage
	{
	public:
		ShaderStage_Source(EShaderStageType type, const std::string& source)
			: ShaderStage(type)
			, Source(source)
		{ }

		virtual std::string ToString() const override { return Source; }
		virtual std::string GetExtension() const override {
			return ShaderStage::GetExtension() + ".glsl";
		}

	public:
		std::string Source;
	};

	struct ShaderStage_SpirV : public ShaderStage {
	public:
		ShaderStage_SpirV(EShaderStageType type, const std::vector<uint32_t>& source)
			: ShaderStage(type)
			, Source(source)
		{ }

		virtual std::string ToString() const override {
			return std::string((char*)Source.data(), Source.size() * sizeof(uint32_t));
		}

		virtual std::string GetExtension() const override {
			return ShaderStage::GetExtension() + ".spv";
		}

	public:
		std::vector<uint32_t> Source;
	};

	struct ShaderStage_GLSL : public ShaderStage {
	public:
		ShaderStage_GLSL(EShaderStageType type, const std::string& source)
			: ShaderStage(type)
			, Source(source)
		{ }

		virtual std::string ToString() const override { return Source; }
		virtual std::string GetExtension() const override {
			return ShaderStage::GetExtension() + ".glsl";
		}
	public:
		std::string Source;
	};

	struct ShaderStage_HLSL : public ShaderStage {
	public:
		ShaderStage_HLSL(EShaderStageType type, const std::string& source)
			: ShaderStage(type)
			, Source(source)
		{ }

		virtual std::string ToString() const override { return Source; }
		virtual std::string GetExtension() const override {
			return ShaderStage::GetExtension() + ".hlsl";
		}
	public:
		std::string Source;
	};

	struct ShaderStage_MSL : public ShaderStage {
	public:
		ShaderStage_MSL(EShaderStageType type, const std::string& source)
			: ShaderStage(type)
			, Source(source)
		{ }

		virtual std::string ToString() const override { return Source; }
		virtual std::string GetExtension() const override {
			return ShaderStage::GetExtension() + ".msl";
		}
	public:
		std::string Source;
	};

}