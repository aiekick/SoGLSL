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

enum class KEY_TYPE_Enum : uint8_t
{
	KEY_TYPE_SHADER = 0,
	KEY_TYPE_INCLUDE
};

/*
static inline std::string GetCompilableShaderCodePartEnumFromString(const std::string& vString)
{
	if (vString == "NONE")              return "NONE";
	else if (vString == "TEXT")         return "TEXT";
	else if (vString == "FRAMEBUFFER")  return "FRAMEBUFFER";
	else if (vString == "UNIFORMS")     return "UNIFORMS";
	else if (vString == "UNIFORM")      return "UNIFORM";
	else if (vString == "COMMON")       return "COMMON";
	else if (vString == "VERTEX")       return "VERTEX";
	else if (vString == "GEOMETRY")     return "GEOMETRY";
	else if (vString == "TESSCONTROL")  return "TESSCONTROL";
	else if (vString == "TESSEVAL")     return "TESSEVAL";
	else if (vString == "FRAGMENT")     return "FRAGMENT";
	else if (vString == "COMPUTE")      return "COMPUTE";
	else if (vString == "INCLUDE")      return "INCLUDE";
	else if (vString == "PROJECT")      return std::string::SHADER_CODE_PART_PROJECT;
	else if (vString == "CONFIG")       return "CONFIG";
	else if (vString == "REPLACE_CODE") return "REPLACE_CODE";
	else if (vString == "NOTE")         return "NOTE";
	else if (vString == "FULL")         return "FULL";
	else                                return std::string::SHADER_CODE_PART_CUSTOM;

	return "NONE";
}
*/