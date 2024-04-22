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

#include <Headers/RenderPackHeaders.h>
#include <Headers/RenderPackHeaders.h>

class UniformVariant;

class ShaderKey;

class UniformHelper {
public:
    static ct::fvec2 FBOSizeForMouseUniformNormalization;
    static ct::fvec2 FBOSize;

public:
    static int UploadUniformForGlslType(const GuiBackend_Window& vWin, UniformVariantPtr v, int vTextureSlotId, bool vIsCompute);
    static int UpdateMipMap(const GuiBackend_Window& vWin, UniformVariantPtr v, int vTextureSlotId, bool vForce);
    static std::string SerializeUniform(UniformVariantPtr vUniform);
    static std::string SerializeUniformAsConst(UniformVariantPtr vUniform);
    static std::string SerializeUniformAsWidget(UniformVariantPtr vUniform);
    static std::string SecureAbsolutePath(const std::string& vAbsolutePath);
    static std::string DeSecureAbsolutePath(const std::string& vAbsolutePath);
    static void DeSerializeUniform(ShaderKeyPtr vShaderKey, UniformVariantPtr vUniform, const std::vector<std::string>& vParams);
};
