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

#include <string>
#include <cstdint>

namespace uType
{
	enum class uTypeEnum
	{
		U_VOID = 0,
		U_FLOAT,
		U_VEC2,
		U_VEC3,
		U_VEC4,
		U_INT, // UINT est deja pris par le system et cree des erreurs de merde a la compilation
		U_IVEC2,
		U_IVEC3,
		U_IVEC4,
		U_UINT,
		U_UVEC2,
		U_UVEC3,
		U_UVEC4,
		U_BOOL,
		U_BVEC2,
		U_BVEC3,
		U_BVEC4,
		U_IMAGE1D, // IMAGE sans sampler pour du storage dans du compute
		U_IMAGE2D, // IMAGE sans sampler pour du storage dans du compute
		U_IMAGE3D, // IMAGE sans sampler pour du storage dans du compute
		U_SAMPLER1D,
		U_SAMPLER1D_ARRAY, // dans le cas des buffer avec plusieurs attachments
		U_SAMPLER2D,
		U_SAMPLER2D_ARRAY, // dans le cas des buffer avec plusieurs attachments
		U_SAMPLER3D,
		U_SAMPLER3D_ARRAY, // dans le cas des buffer avec plusieurs attachments
		U_SAMPLER2D_TEXTUREARRAY, // uniform sampler2DArray tex3d;
		U_SAMPLERCUBE,
		U_MAT2,
		U_MAT3,
		U_MAT4,
		U_FLOAT_ARRAY,
		U_VEC2_ARRAY,
		U_VEC3_ARRAY,
		U_VEC4_ARRAY,
		U_INT_ARRAY,
		U_MAT4_ARRAY,
		U_STRUCT,
		U_TYPE, // any type
		U_VEC, // any vec types
		U_MAT, // any mat types
		U_TEXT, // special type for display text
		U_FLOW, // graph flow
		U_PROGRAM, // program
		U_GLOBAL, // for write global var in shader (outside of functions)
		U_Count
	};

	bool IsTypeSplitable(const uTypeEnum& vType);
	bool IsTypeCombinable(const uTypeEnum& vType);
	std::string ConvertUniformsTypeEnumToString(const uTypeEnum& vType);
	uTypeEnum GetBaseGlslTypeFromType(const uTypeEnum& vType, uint32_t* vCountChannels);
	uTypeEnum GetBaseGlslTypeFromString(const  std::string& vType, bool vIsArray, uint32_t* vCountChannels);
	uTypeEnum GetGlslTypeFromString(const std::string& vType, bool vIsArray = false);
	uint32_t GetCountChannelForType(const uTypeEnum& vType);
}
