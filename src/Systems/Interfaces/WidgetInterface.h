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

#include <memory>
#include <string>
#include <CodeTree/Parsing/UniformParsing.h>

class CodeTree;
class ShaderKey;
class RenderPack;
class UniformVariant;
class WidgetInterface
{
public:
	virtual bool Init(CodeTreePtr vCodeTree) = 0;
	virtual void Unit() = 0;
	virtual bool DrawWidget(CodeTreePtr vCodeTree, UniformVariantPtr vUniPtr,
		const float& vMaxWidth, const float& vFirstColumnWidth, RenderPackWeak vRenderPack,
		const bool& vShowUnUsed, const bool& vShowCustom, const bool& vForNodes, bool* vChange) = 0;
	virtual bool SerializeUniform(UniformVariantPtr vUniform, std::string* vStr) = 0;
	virtual bool DeSerializeUniform(ShaderKeyPtr vShaderKey, UniformVariantPtr vUniform, const std::vector<std::string>& vParams) = 0;
	virtual void Complete_Uniform(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) = 0;
	virtual bool UpdateUniforms(UniformVariantPtr vUniPtr) = 0;
};