// NoodlesPlate Copyright (C) 2017-2023 Stephane Cuillerdier aka Aiekick
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <ctools/cTools.h>
#include <CodeTree/CodeTreeGlobals.h>
#include <Headers/RenderPackHeaders.h>
#include <set>
#include <string>
#include <unordered_map>

#if defined(USE_VULKAN)
#include <vulkan/vulkan.hpp>
enum class ModelRenderModeEnum : uint8_t
{
	MODEL_RENDER_MODE_POINTS = (uint8_t)vk::PrimitiveTopology::ePointList,
	MODEL_RENDER_MODE_LINES = (uint8_t)vk::PrimitiveTopology::eLineList,
	MODEL_RENDER_MODE_LINE_STRIP = (uint8_t)vk::PrimitiveTopology::eLineStrip,
	MODEL_RENDER_MODE_TRIANGLES = (uint8_t)vk::PrimitiveTopology::eTriangleList,
	MODEL_RENDER_MODE_TRIANGLE_STRIP = (uint8_t)vk::PrimitiveTopology::eTriangleStrip,
	MODEL_RENDER_MODE_TRIANGLE_FAN = (uint8_t)vk::PrimitiveTopology::eTriangleFan,
	MODEL_RENDER_MODE_PATCHES = (uint8_t)vk::PrimitiveTopology::ePatchList, // tesselation shader
	MODEL_RENDER_MODE_NONE,
	MODEL_RENDER_MODE_Count
};
#elif defined(USE_OPENGL)
#include <glad/glad.h>
enum class ModelRenderModeEnum : uint8_t
{
	MODEL_RENDER_MODE_POINTS = (uint8_t)GL_POINTS,
	MODEL_RENDER_MODE_LINES = (uint8_t)GL_LINES,
	MODEL_RENDER_MODE_LINE_STRIP = (uint8_t)GL_LINE_STRIP,
	MODEL_RENDER_MODE_TRIANGLES = (uint8_t)GL_TRIANGLES,
	MODEL_RENDER_MODE_TRIANGLE_STRIP = (uint8_t)GL_TRIANGLE_STRIP,
	MODEL_RENDER_MODE_TRIANGLE_FAN = (uint8_t)GL_TRIANGLE_FAN,
	MODEL_RENDER_MODE_PATCHES = (uint8_t)GL_PATCHES, // tesselation shader
	MODEL_RENDER_MODE_NONE,
	MODEL_RENDER_MODE_Count
};
#else
enum class ModelRenderModeEnum : uint8_t
{
	MODEL_RENDER_MODE_POINTS = 0,
	MODEL_RENDER_MODE_LINES,
	MODEL_RENDER_MODE_LINE_STRIP,
	MODEL_RENDER_MODE_TRIANGLES,
	MODEL_RENDER_MODE_TRIANGLE_STRIP,
	MODEL_RENDER_MODE_TRIANGLE_FAN,
	MODEL_RENDER_MODE_PATCHES, // tesselation shader
	MODEL_RENDER_MODE_NONE,
	MODEL_RENDER_MODE_Count
};
#endif

static inline ModelRenderModeEnum GetModelRenderModeEnumFromString(const std::string& vString)
{
	if (vString == "POINTS") return ModelRenderModeEnum::MODEL_RENDER_MODE_POINTS;
	if (vString == "LINE_STRIP") return ModelRenderModeEnum::MODEL_RENDER_MODE_LINE_STRIP;
	if (vString == "LINES") return ModelRenderModeEnum::MODEL_RENDER_MODE_LINES;
	if (vString == "TRIANGLE_STRIP") return ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_STRIP;
	if (vString == "TRIANGLE_FAN") return ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_FAN;
	if (vString == "TRIANGLES") return ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES;
	if (vString == "PATCHES") return ModelRenderModeEnum::MODEL_RENDER_MODE_PATCHES;
	return ModelRenderModeEnum::MODEL_RENDER_MODE_NONE;
}

static inline const char* GetModelRenderModeEnumString(ModelRenderModeEnum vModelRenderModeEnum)
{
	switch (vModelRenderModeEnum)
	{
	case ModelRenderModeEnum::MODEL_RENDER_MODE_POINTS:			return "POINTS";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_LINE_STRIP:		return "LINE_STRIP";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_LINES:			return "LINES";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_STRIP:	return "TRIANGLE_STRIP";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_FAN:	return "TRIANGLE_FAN";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES:		return "TRIANGLES";
	case ModelRenderModeEnum::MODEL_RENDER_MODE_PATCHES:		return "PATCHES";
	}
	return "";
}

enum class BaseMeshEnum : uint8_t
{
	PRIMITIVE_TYPE_QUAD = 0,
	PRIMITIVE_TYPE_POINTS,
	PRIMITIVE_TYPE_MESH,
	PRIMITIVE_TYPE_NONE,
	PRIMITIVE_TYPE_Count
};

static inline const char* GetModelTypeEnumString(BaseMeshEnum vBaseMeshEnum)
{
	switch (vBaseMeshEnum)
	{
	case BaseMeshEnum::PRIMITIVE_TYPE_QUAD:		return "QUAD";
	case BaseMeshEnum::PRIMITIVE_TYPE_POINTS:	return "POINTS";
	case BaseMeshEnum::PRIMITIVE_TYPE_MESH:		return "MESH";
	}
	return "";
}

static inline BaseMeshEnum GetModelTypeEnumFromString(const std::string& vString)
{
	if (vString == "QUAD")		return BaseMeshEnum::PRIMITIVE_TYPE_QUAD;
	if (vString == "POINTS")	return BaseMeshEnum::PRIMITIVE_TYPE_POINTS;
	if (vString == "MESH")		return BaseMeshEnum::PRIMITIVE_TYPE_MESH;
	return BaseMeshEnum::PRIMITIVE_TYPE_NONE;
}

struct ReplaceCodeStruct
{
	std::string key;					// key, by ex : << SLOT:name:params // help >>
	std::string type;					// type (slot by ex)
	std::string name;					// name key
	std::vector<std::string> params;	// params
	std::string defCode;				// default code
	std::string newCode;				// new code
	std::string newCodeFromKey;			// shaderkey id where the new code come from
};

struct GeometryLayoutStruct
{
	size_t pos = 0;
	std::string line;
	std::vector<std::string> params;
	std::string dir;
};

struct ShaderNoteStruct
{
	std::map<std::string, std::vector<std::string>> dico;
	std::map<std::string, std::string> urls;

	void clear();
};

struct ConfigTagParsedStruct
{
	std::string tag;
	// many tags, tag content, tag items
	std::vector<std::vector<std::vector<std::string>>> params;
};

struct ProjectConfigStruct
{
	std::unordered_map<std::string, ConfigTagParsedStruct> tags;
	int countChanges = 0;
	bool needSceneUpdate = false;

	void ApplyDefault();
	void Complete(ProjectConfigStruct* vConfigToComplete);
	void CheckChangeWith(ProjectConfigStruct* vConfigToCcompareForChanges);
};

struct SceneConfigStruct
{
	std::set<std::string> shaders;
	int countChanges = 0;
	size_t line = 0;
	bool needSceneUpdate = false;

	void ApplyDefault();
	void Complete(SceneConfigStruct* vConfigToComplete);
	void CheckChangeWith(SceneConfigStruct* vConfigToCcompareForChanges);
};

struct FrameBufferShaderConfigStruct
{
	int countChanges = 0; // sert a savoir si ya eu des changement ici

	float ratio = 0.0f;
	ct::ivec3 size = 0;
	bool needSizeUpdate = false;

	std::string format = "float";
	bool mipmap = false;
	std::string wrap = "clamp";
	std::string filter = "linear";
	int count = 1; // attachments
	std::string bufferName; // buffer name
	bool needFBOUpdate = false;

	ct::uvec4 countIterations = ct::uvec4(1U, 100U, 1U, 1U); // inf, sup, def, value
	bool needCountIterationsUpdate = false;

	void Complete(FrameBufferShaderConfigStruct* vConfigToComplete);
	void CheckChangeWith(FrameBufferShaderConfigStruct* vConfigToCcompareForChanges);
};

struct VertexShaderConfigStruct
{
	int countChanges = 0; // sert a savoir si ya eu des changement ici

	std::string type;

	bool needCountPointUpdate = false;
	bool needLineWidthUpdate = false;
	bool needModelUpdate = false;
	bool needDisplayModeUpdate = false;

	BaseMeshEnum meshType = BaseMeshEnum::PRIMITIVE_TYPE_QUAD;
	BaseMeshFormatEnum meshFormat = BaseMeshFormatEnum::PRIMITIVE_FORMAT_PNTBTC;

	std::unordered_map<std::string, ModelRenderModeEnum> displayMode;
	std::string defaultDisplayMode;
	std::string displayListString;

	ct::uvec4 countVertexs = ct::uvec4(0U, 0U, 1U, 1U); // inf, sup, def, value
	ct::uvec4 countInstances = ct::uvec4(0U, 0U, 1U, 1U); // inf, sup, def, value
	ct::fvec4 lineWidth = ct::fvec4(0.0f, 10.0f, 1.0f, 1.0f); // inf, sup, def, value
	std::string modelFileToLoad;

	size_t line = 0;

	void ApplyDefault();
	void Complete(VertexShaderConfigStruct* vConfigToComplete);
	void CheckChangeWith(VertexShaderConfigStruct* vConfigToCompareForChanges);
};

struct ComputeShaderConfigStruct
{
	int countChanges = 0; // sert a savoir si ya eu des changement ici

	std::string type;
	size_t line = 0;

	bool needSizeUpdate = false;
	bool needWorkgroupUpdate = false;
	bool needCountIterationsUpdate = false;

	ct::ivec3 size;
	ct::ivec3 workgroups;
	ct::uvec4 countIterations = ct::uvec4(1U, 100U, 1U, 1U); // inf, sup, def, value

	void ApplyDefault();
	void Complete(ComputeShaderConfigStruct* vConfigToComplete);
	void CheckChangeWith(ComputeShaderConfigStruct* vConfigToCcompareForChanges);
};

struct SectionConfigStruct
{
	SceneConfigStruct sceneConfig;
	FrameBufferShaderConfigStruct framebufferConfig;
	VertexShaderConfigStruct vertexConfig;
	ComputeShaderConfigStruct computeConfig;
	ProjectConfigStruct projectConfig;

	void Complete(SectionConfigStruct* vConfigToComplete);
	void CheckChangeWith(SectionConfigStruct* vConfigToCcompareForChanges);
};

typedef std::unordered_map<std::string, ConfigTagParsedStruct> ShaderSectionConfig;

class SectionCode;
class ShaderKey;
class ShaderStageParsing
{
public:
	static bool IfInCommentZone(const std::string& vCode, size_t vPos);
	static bool IfOnIncludeLine(const std::string& vCode, size_t vPos);
	static std::string supressCommentedCode(const std::string& vText);
	static size_t GetTagPos(const std::string& vCode, const std::string& vTag, size_t vStartPos, bool vCheckIfInCommentZone = true, bool vWordOnly = false);
	static std::string GetTagIfFound(const std::string& vCode, size_t vStartPos, size_t* vApproxPos, bool vCheckIfInCommentZone = true);
	static bool IsTagFound(const std::string& vCode, const std::string& vTag, size_t vStartPos, size_t* vApproxPos, bool vCheckIfInCommentZone = true, bool vWordOnly = false);
	static bool FoundTagInCode(const std::string& vCode, const std::string& vTag, size_t* vPos, std::map<size_t, std::string>* vDico);
	static std::string GetStringBetweenChars(char vOpenChar, char vCloseChar, std::string& vCode, size_t vStartPos, ct::uvec2* vBlockLoc = 0);
	static size_t GetPosAtCoordInString(std::string& vCode, ct::uvec2 vCoord);
	static std::string GetStringFromPosUntilChar(const std::string& vCode, const size_t& vPos, const char& vLimitChar);

public:
	ShaderSectionConfig ParseShaderSectionConfigLine(const std::string& vConfigLine, size_t vCurrentFileLine);

	bool ParseSectionConfig_ReturnSectionName_SayIfWeCanUse(
		std::shared_ptr<SectionCode> vSectionCode,
		const std::string& vSectionLine,
		std::string vSectionType,
		size_t vCurrentFileLine,
		const std::string& vDesiredStageName,
		std::string* vSectionNameToReturn,
		std::vector<std::string>* vInFileBufferNameToReturn);

	ReplaceCodeStruct ParseReplaceCode(std::shared_ptr<SectionCode> vSectionCode, const std::string& vExtractedCode, size_t vCurrentFileLine);
	std::weak_ptr<ShaderKey> ParseIncludeLine(std::shared_ptr<SectionCode> vSectionCode, const std::string& vCodeLine, size_t vCurrentFileLine);

private:
	void ParseNoteConfig(std::shared_ptr<SectionCode> vSectionCode, const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags, size_t vCurrentFileLine);
	ProjectConfigStruct ParseProjectConfig(std::shared_ptr<SectionCode> vSectionCode, const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags, size_t vCurrentFileLine);
	SceneConfigStruct ParseSceneConfig(std::shared_ptr<SectionCode> vSectionCode, const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags, size_t vCurrentFileLine);
	FrameBufferShaderConfigStruct ParseFrameBufferConfig(std::shared_ptr<SectionCode> vSectionCode, const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags, size_t vCurrentFileLine);
	VertexShaderConfigStruct ParseVertexConfig(std::shared_ptr<SectionCode> vSectionCode, const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags, size_t vCurrentFileLine);
	ComputeShaderConfigStruct ParseComputeConfig(std::shared_ptr<SectionCode> vSectionCode, const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags, size_t vCurrentFileLine);
	void ParseGeometryConfig(std::shared_ptr<SectionCode> vSectionCode, const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags, size_t vCurrentFileLine);
	void ParseTesselationControlConfig(std::shared_ptr<SectionCode> vSectionCode, const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags, size_t vCurrentFileLine);
	void ParseTesselationEvalConfig(std::shared_ptr<SectionCode> vSectionCode, const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags, size_t vCurrentFileLine);
	void ParseCommonConfig(std::shared_ptr<SectionCode> vSectionCode, const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags, size_t vCurrentFileLine);
	void ParseFragmentConfig(std::shared_ptr<SectionCode> vSectionCode, const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags, size_t vCurrentFileLine);
	bool CheckIfExist_InfiniteIncludeLoop(std::shared_ptr<SectionCode> vSectionCode, const std::string& vIncludeName);
	void ParseGeometryShaderForGetInAndOutPrimitives(std::shared_ptr<SectionCode> vSectionCode, const std::string& vCodeToParse);
	std::vector<GeometryLayoutStruct> SearchForGeometryLayout(const std::string& vCodeToParse, const std::string& vTagToSearch,
		char vFirstChar, char vEndChar, char vSeparator, char vEndLineChar);
};