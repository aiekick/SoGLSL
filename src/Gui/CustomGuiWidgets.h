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
#include <ImGuiPack.h>
#include <ctools/cTools.h>

namespace ImGui {
ImVec4 GetUniformLocColor(int vLoc);
void Texture(ctTexturePtr vTex, float vWidth, ImVec4 vColor, float vBorderThick);
void Texture(ctTexturePtr vTex, float vWidth, float vHeight, ImVec4 vColor, float vBorderThick);
bool TextureButton(ctTexturePtr vTex, float vWidth, ImVec4 /*vColor*/, float /*vBorderThick*/);
bool TextureButton(ctTexturePtr vTex, float vWidth, ImVec4 /*vColor*/, ImVec4 vBorderColor);
void Texture(float vWidth, const char* vName, TextureCubePtr vCubeMap, GLuint vLoc);
void PlotFVec4Histo(const char* vLabel,
                    ct::fvec4* vDatas,
                    int vDataCount,
                    bool* vShowChannel = 0,
                    ImVec2 frame_size = ImVec2(0, 0),
                    ct::fvec4 scale_min = FLT_MAX,
                    ct::fvec4 scale_max = FLT_MAX,
                    int* vHoveredIdx = 0);
}  // namespace ImGui
