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

#include <Headers/RenderPackHeaders.h>
#include <Mesh/Model/BaseModel.h>
#include <CodeTree/CodeTree.h>

#include <unordered_map>
#include <string>
// ATTENTION
/*
1) InitShaderProgram
2) ajoute les attribute par addAttribute
3) link => LinkShaderProgram
4) updateUniforms // init les matrices
*/
// glGetUniformLocation peut renvoyer -1 si un uniform est d�clar� dans un shader mais jamais utilis�. opengl fait ces simplifications

class Shader
{
public:
	static ShaderPtr Create(const GuiBackend_Window& vWin, std::string vShaderName);

private:
	//GLint puuUniforms[kUniforpuMAX];
	GLenum err = 0;
	bool _isValid = false;
	GuiBackend_Window puWindow;

public:
	GLuint puuProgram = 0;
	GLint linked = 0;
	std::string puShaderName;
	ProgramMsg puState;
	ShaderPtr m_This = nullptr;

public:
	Shader(const GuiBackend_Window& vWin, std::string vShaderName);
	~Shader();

	bool IsValid();

	ProgramMsg InitAndLinkShaderProgram(
		bool vIsCompute,
		ShaderParsedStruct shaderCode,
		std::string geomLayoutParams,
		std::string version);
	ShaderMsg LinkShaderProgram();

	// return 0 if NOK
	GLuint LoadFromString(
		ShaderTypeEnum type,
		ShaderParsedCodeStruct shaderCode,
		std::string version);

	void Use();
	void UnUse();

	void Render();

	void LogToOutput(ShaderTypeEnum  type, std::string vBuild, bool vErase, ShaderMsg vMsg);

	std::string GetLastShaderErrorString(ShaderTypeEnum vShaderTypeEnum){ return puError[vShaderTypeEnum]; }
	std::string GetLastShaderWarningsString(ShaderTypeEnum vShaderTypeEnum) { return puWarnings[vShaderTypeEnum]; }

	std::string GetUsedPrecisionString(){ return puPrecision; }
	std::string GetUsedGlslVersionString(){ return puVersion; }
	std::string GetUsedUniformsString(){ return puUniforms; }
	std::string GetUsedHeaderString(ShaderTypeEnum vKey) { if (puHeader.find(vKey) != puHeader.end()) return puHeader[vKey]; return ""; }
	
	void SaveToFile(std::string vCode, const std::string& vFilePathName);
	
	GLint getUniformLocationForName(std::string name);

public: // WebGl
	static std::string GetWebGlTemplate_CreateShader(std::string vOffset);
	static std::string GetWebGlTemplate_LoadFromString(std::string vOffset);
	static std::string GetWebGlTemplate_LinkShader(std::string vOffset);
	static std::string GetWebGlTemplate_TestCreateShaderWithPrecision(std::string vOffset);
	static std::string GetWebGlTemplate_DetermineShaderPrecision(std::string vOffset);

private:
	std::unordered_map<ShaderTypeEnum, std::string> puError;
	std::unordered_map<ShaderTypeEnum, std::string> puWarnings;
	std::string puPrecision;
	std::string puVersion;
	std::string puUniforms;
	std::unordered_map<ShaderTypeEnum, std::string> puHeader;
};
