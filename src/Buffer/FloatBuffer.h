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
#include <glad/glad.h>
#include <ctools/cTools.h>
#include <string>

#define RECENTERING(v,s) (v) * ((s) - 1)

class FloatBuffer
{
public:
	int w = 0;
	int h = 0;
	size_t size = 0;
	int bytesPerPixel = 0;
	float *buf = nullptr;
	GLenum format = 0;
	GuiBackend_Window puWindow;
	GLuint pbo = 0;
	bool usePBO = false;
	GLuint texture = 0;
	TextureParamsStruct texParams;

public:
	FloatBuffer(const GuiBackend_Window& vWin);
	~FloatBuffer();
	void InitPBO();
	ct::fvec4 getColorAtCoord(const int& vX, const int& vY);
	void getColorAtCoord(const int& vX, const int& vY, ct::fvec4 *vVec4);
	ct::fvec4 getColorAtNormalizedCoord0To1(const float& vX, const float& vY);
	void getColorAtNormalizedCoord0To1(const float& vX, const float& vY, ct::fvec4 *vVec4);
	void getColorAtNormalizedCoord0To1(const float& vX, const float& vY, float *vFloat);
	void SaveOneChannelToFile(const std::string& vFile);
	void SaveFourChannelToFile(const std::string& vFile);
	void CreateTexture();
	void DestroyTexture();
	void ChangeTexParameters(TextureParamsStruct *vTexParams);
};
