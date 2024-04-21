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

#include <ctools/ConfigAbstract.h>

#include <Headers/RenderPackHeaders.h>

#include <ctools/cTools.h>

#include <stdio.h>

#include <string>
#include <string>
#include <list>
#include <unordered_map>
#include <vector>
#include <list>

#include <future>
#include <functional>
#include <atomic>

class ImporterFromShadertoy
{
private: // buffer links
	std::unordered_map<std::string, std::string> puChannelLinks;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> puTexturesId;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> puCubeMapsId;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> puTextures3DId;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> puTexturesSoundId;
	std::unordered_map<std::string, std::string> puBuffersName;
	std::unordered_map<std::string, std::string> puBuffersId;
	std::unordered_map<std::string, std::string> puBufferCubeMapsId;
	std::unordered_map<std::string, ShaderInfos*> puBuffersInfos;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> puBuffersParams;
	std::unordered_map<std::string, std::string> puCodeReplacements;

public:
	std::list<ShaderInfos> ParseBuffer(
		const std::string& vBuffer,
		const std::string& vId,
		const bool& vImportInOneFile = true);

private:
	void AddUniformsFromCode(ShaderInfos& vOutShaderInfos, std::string& vCode, const ShaderPlaform& vShaderPlaform, const bool& vImportInOneFile, const std::string *vCommonCodePtr = nullptr);
	void DoChannelInput(const std::string& vBaseName, ShaderInfos& vOutShaderInfos, const std::string& vChanId, const size_t& nOccurs, const bool& vImportInOneFile);
	void SetShaderBufferFormat(const std::string& vBufferId);
	std::vector<std::string> ParseUniform(const std::string& vUniformStr);
};
