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

#include <CodeTree/Parsing/SectionCode.h>
#include <uTypes/uTypes.h>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <glslang/glslang/Include/BaseTypes.h>

struct UniformParsedStruct
{
	//uniform(sectionParams) type(params) name [array]; // comment
	bool badSyntaxDetected = false;
	std::string sectionParams;
	std::string type;
	std::string params;
	std::string name;
	std::string array;
	std::string comment; // les \\n serait remaplc� par des \n pour affichage
	std::string commentOriginal; // les \\n reste des \\n
	std::string direction; // for compute, in or out => in is readonly, and out is writeonly
	std::vector<std::string> sectionParamsArray;
	std::unordered_map<std::string, std::vector<std::string>> sectionParamsDico;
	std::vector<std::string> paramsArray;
	std::unordered_map<std::string, std::vector<std::string>> paramsDico;
	size_t sourceCodeLine = 0;
	bool notUploadableToGPU = false; // uniforms de type pure ui pas dans shader comme text
	ct::uvec3 preferedSize;	// size pass� en cpp a preferer si pas de valeur dans le shader
	std::string qualifier; // coherent, volatile, restrict, readonly, writeonly, ou rien
	
	// pilot� par un autre uniform buffer
	bool isPiloted = false; // pilot�
	std::string pilotKey; // clef pilote
	std::string pilotStage; // stage pilote

	std::string originalParams; // pour la generation de template
	std::vector<std::string> originalParamsArray;
	std::unordered_map<std::string, std::vector<std::string>> originalParamsDico;

	// uniform section
	std::string sectionName = "default";
    bool noExport = false;
	int sectionOrder = 0;
	std::string sectionCond;

	// uniform section cond : var0 op var1 like toto==tata
	std::string var0;
	std::string var1;
	std::string op = "eq";

	std::string bufferStructDeclaration; // va contenir le code de declaration d'un type complex, donc en dehors d'un SBO mais seulement dans le cas d'un SBO
	std::string bufferStructContent; // va contenir le code d'un buffer storage special
	std::string bufferType; // uniform ou storage ?
	bool makeUBOContantGlobal = false; // will not add a nam after the ubo declaration, so make his contant global

	UniformParsedStruct()
	{
		badSyntaxDetected = false;
		notUploadableToGPU = false;
		sourceCodeLine = 0;
	}

	bool isOk()
	{
		return !(type.empty() || name.empty() || badSyntaxDetected);
	}

	std::string GetUniformHeaderString()
	{
		return "uniform " + type + " " + name + ";\n";
	}
};

class UniformParsing
{
public:
	static UniformParsedStruct ParseUniformString(const std::string& vUniformString, const size_t& vCurrentFileLine, std::shared_ptr<SectionCode> vSectionCode);
	static void ParseUniformSectionParamsString(UniformParsedStruct* vUniform, const size_t& vCurrentFileLine, std::shared_ptr<SectionCode> vSectionCode);
	static void ParseUniformTypeParamsString(UniformParsedStruct* vUniform, const size_t& vCurrentFileLine, std::shared_ptr<SectionCode> vSectionCode);
	static void ParseUniformSection(UniformParsedStruct* vUniformParsed);
	static void FinaliseUniformSectionParsing(UniformParsedStruct* vUniformParsed);
	static void CompleteUniformParsedStructWithCustomHeaderCode(UniformParsedStruct* vUniformParsed);

	static bool IsUniformTypeSupported(const std::string& vType, const bool& vIsArray, ShaderKeyPtr vKey, const size_t& vCurrentFileLine);
	static bool IsUniformWidgetSupported(const std::string& vWidget, ShaderKeyPtr vKey, const size_t& vCurrentFileLine);
	static bool IsUniformNotUploadableToGPU(const std::string& vType);
};
