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
#include "LineFileErrors.h"
#include <Headers/RenderPackHeaders.h>
#include <string>
#include <map>
#include <functional>

class Shader;
class ShaderKey;
class SyntaxErrors
{
public:
	typedef std::function<void(std::string, std::string, std::string, ErrorLine)> SyntaxMessagingFunction;
	static SyntaxMessagingFunction s_SyntaxMessagingFunction;

public:
	std::map<std::string, std::map<bool, std::map<std::string, std::map<std::string, std::map<size_t, ErrorLine>>>>> puInfos;
	bool puIsThereSomeErrors = false;
	bool puIsThereSomeWarnings = false;
	std::string puParentKeyName;

public:
	SyntaxErrors();
	~SyntaxErrors();

	void clear(const std::string& vErrorConcern = "");
	void SetSyntaxError(
		std::weak_ptr<ShaderKey> vKey, const std::string& vErrorConcern,
		const std::string& vErrorType, bool vErrorOrWarnings,
		const LineFileErrors& vLineFileErrorsType);
	bool isThereSomeSyntaxMessages(std::weak_ptr<ShaderKey> vKey, bool vErrorOrWarnings);

	std::string toString(std::weak_ptr<ShaderKey> vKey, bool vErrorOrWarnings);

public: // ImGui
	bool CollapsingHeaderError(const char* vLabel, bool vForceExpand = false, bool vShowEditButton = false, bool* vEditCatched = nullptr);
	bool CollapsingHeaderWarnings(const char* vLabel, bool vForceExpand = false, bool vShowEditButton = false, bool* vEditCatched = nullptr);
	bool ImGui_DisplayMessages(ShaderKeyPtr vKey, const char* vLabel, bool vForceExpand = false, bool vShowEditButton = false, bool* vEditCatched = nullptr);
	bool ImGui_DisplayMessages(bool vErrorOrWarnings, ShaderKeyPtr vKey, bool vForceExpand = false, bool vShowEditButton = false, bool* vEditCatched = nullptr);
	void CompleteWithShader(ShaderKeyPtr vKey, ShaderPtr vShader);
};
