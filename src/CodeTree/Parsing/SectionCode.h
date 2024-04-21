// NoodlesPlate Copyright (C) 2017-2024 Stephane Cuillerdier aka Aiekick
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
#include <uTypes/uTypes.h>
#include <CodeTree/CodeTreeGlobals.h>
#include <CodeTree/Parsing/ShaderStageParsing.h>

#include <string>
#include <map>

struct FormatErrorStruct
{
	size_t line = 0;
	std::string file;
	std::string error;
	std::string helplink;
};

struct ShaderParsedCodeStruct
{
	std::string shaderCodePart = "NONE";
	std::string header;
	std::string uniforms;
	std::string code;

	std::map<std::string, std::list<FormatErrorStruct>> errors;

	ShaderParsedCodeStruct() = default;

	std::string GetCode()
	{
		return header + uniforms + code;
	}
};

struct ShaderParsedStruct
{
	std::map<std::string, ShaderParsedCodeStruct> shader;
	ShaderNoteStruct note;

	ShaderParsedCodeStruct GetSection(const std::string& vSection)
	{
		ShaderParsedCodeStruct sh;

		if (shader.find(vSection) != shader.end())
		{
			sh = shader[vSection];
		}

		return sh;
	}

	void ParseNote(const std::string& vNote);

	ShaderNoteStruct GetNote()
	{
		return note;
	}
};

struct CodeTagStruct
{
	std::string ProjectTag = "@PROJECT";
	std::string FrameBufferTag = "@FRAMEBUFFER";
	std::string UniformTag = "@UNIFORMS";
	std::string CommonTag = "@COMMON";
	std::string VertexTag = "@VERTEX";
	std::string GeometryTag = "@GEOMETRY";
	std::string TesselationControlTag = "@TESSCONTROL";
	std::string TesselationEvalTag = "@TESSEVAL";
	std::string FragmentTag = "@FRAGMENT";
	std::string ComputeTag = "@COMPUTE";
	std::string NoteTag = "@NOTE";
	std::string IncludeWordTag = "#include";
	std::string UniformWordTag = "uniform";
	std::string LayoutWordTag = "layout";
	std::string ConfigStartTag = "@CONFIG_START";
	std::string ConfigEndTag = "@CONFIG_END";
	std::string BufferStartTag = "@BUFFER_START";
	std::string BufferEndTag = "@BUFFER_END";

	std::string CodeToReplaceTag = "[[CODE_TO_INSERT]]";
	std::string CodeToReplaceStartTag = "[[";
	std::string CodeToReplaceEndTag = "]]";

	std::string InsertCodeTag = "@INSERT";
	std::string SectionTag = "@SECTION";
	std::string UniformTemporaryTag = "PARSED_UNIFORM_NAME(";
};

class ShaderKey;
class SectionCode
{
public:
	static CodeTagStruct codeTags;

public:
	std::string absoluteFile;
	std::string relativeFile;
	std::string code;
	std::string name;
	std::string inFileBufferName;
	std::string forcedSectionName; // quand on fait #include "toto.glsl":section_name (et section_name c'est dans les ex : @FRAGMENT SECTION(section_name)
	std::string forcedStageName; //quand on fait #include "toto.glsl":@custom_STAGE ou #include "toto.glsl":@custom_STAGE:section_name
	std::string stageName;

	bool isInclude = false;

	ShaderSectionConfig m_ShaderSectionConfig;
	ShaderStageParsing m_ShaderStageParsing;

	std::weak_ptr<ShaderKey> parentKey;

	std::string parentType;
	std::string currentType;

	ReplaceCodeStruct m_ReplaceCode;

	std::list<FormatErrorStruct> errors;

	bool orphan = false;
	std::shared_ptr<SectionCode> parentSection = nullptr;
	std::shared_ptr<SectionCode> m_This = nullptr;

	// on garde l'utilisation de  map car l'ordre est important ici
	std::map<size_t, std::vector<std::shared_ptr<SectionCode>>> subSections;

	size_t level = 0;

	size_t sourceCodeStartLine = 0;
	size_t sourceCodeEndLine = 0;

	//size_t finalCodeStartLine;
	//size_t finalCodeEndLine;

	// on stocke les valeurs pour chaque type de shaders sinon des poritions non utilisee reellement serait retournee si on
	// appelait vertex, puis fragment
	// donc on stock pour chaque typed les lignes finales, il y a vertex, geom, et fragment pour le moment
	// bientot on aura les des tesselation shaders
	std::map<std::string, size_t> finalCodeStartLine;
	std::map<std::string, size_t> finalCodeEndLine;

public:
	static std::shared_ptr<SectionCode> Create();
	static std::shared_ptr<SectionCode> Create(
		ShaderKeyPtr vParentkey,
		std::shared_ptr<SectionCode> vParentSection,
		const std::string& vFile, const std::string& vCode,
		const std::string& vType, const std::string& vParentType,
		const std::string& vName,
		const std::string& vInFileBufferName,
		const size_t& vSourceCodeStartLine,
		const size_t& vSourceCodeEndLine,
		const size_t& vLevel,
		const bool& vIsInclude);

public:
	SectionCode();
	SectionCode(
		std::weak_ptr<ShaderKey> vParentKey,
		std::shared_ptr<SectionCode> vParentSection,
		const std::string& vAbsoluteFile, std::string vCode,
		std::string vType, std::string vParentType,
		const std::string& vName,
		const std::string& vInFileBufferName,
		size_t vSourceCodeStartLine,
		size_t vSourceCodeEndLine,
		size_t vLevel,
		bool vIsInclude);
	~SectionCode();

	void Clear();

	std::string GetSectionPart(
		const std::string& vSectionType,
		size_t vCurrentFinalLine,
		std::string vOriginType,
		std::string vParentType,
		const std::string& vDesiredInFileBufferName,
		const std::string& vDesiredSectionName,
		const std::string& vDesiredConfigName,
		std::map<KEY_TYPE_Enum, std::set<std::string>>* vUsedFiles);

	//std::shared_ptr<SectionCode> GetSectionCodeForSourceLine(size_t vLine);
	std::shared_ptr<SectionCode> GetSectionCodeForTargetLine(const std::string& vSectionName, size_t vLine);

	void ResetFinalLineMarks(const std::string& vSectionName);

	void Parse();

	void SetSyntaxError(const std::string& vKey, const std::string& vErrorType, bool vErrorOrWarnings, const std::string& vError, size_t vLine);
	void SetSyntaxError(std::weak_ptr<ShaderKey> vKey, const std::string& vErrorType, bool vErrorOrWarnings, const std::string& vError, size_t vLine);

private:
	bool ParseConfigs(const std::string& codeToParse, size_t& vLastBlockPos);
	bool ParseIncludes(const std::string& codeToParse, size_t& vLastBlockPos);
	bool ParseUniforms(const std::string& codeToParse, size_t& vLastBlockPos);
	bool ParseReplaceCode(const std::string& codeToParse, size_t& vLastBlockPos);
	bool ParseFragColorLayouts(const std::string& codeToParse);
	void DefineSubSection(
		const std::string& vNewCode,
		size_t vStartPos, size_t vEndPos,
		std::string vType, std::string vParentType,
		bool vIsInclude);
	std::string GetShaderPart(
		std::shared_ptr<SectionCode> vSectionCodeParent,
		const std::string& vSectionType,
		size_t vCurrentFinalLine,
		std::string vRootType,
		const std::string& vDesiredStageName,
		const std::string& vDesiredInFileBufferName,
		const std::string& vDesiredSectionName,
		const std::string& vDesiredConfigName,
		std::map<KEY_TYPE_Enum, std::set<std::string>>* vUsedFiles,
		bool vFound = false, int vLevel = 0);
};
