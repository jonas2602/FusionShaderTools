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

	enum class EShaderStageType {
		None = -1,
		Vertex,
		Geometry,
		Fragment,
	};

	static EShaderStageType ShaderStageFromString(const std::string& name)
	{
		if (name == "vertex")	return EShaderStageType::Vertex;
		if (name == "fragment")	return EShaderStageType::Fragment;
		if (name == "pixel")	return EShaderStageType::Fragment;
		if (name == "geometry")	return EShaderStageType::Geometry;

		return EShaderStageType::None;
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

	public:
		std::string Source;
	};

}