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
#include <ctools/cTools.h>
#include <string>
#include <vector>

class Texture2D;
class TextureCube
{
public:
	static TextureCubePtr create(const char* vFilePathName);
	static TextureCubePtr create(unsigned char const* buffer, int len);
	static TextureCubePtr create(std::vector<std::string> vFileNames);

private:
	Texture2DPtr puTexture2D[6] = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
	GLuint CubeMapId = 0;

public:
	TextureCube();
	virtual ~TextureCube();
	
	bool Init(unsigned char const *buffer, int len);
	bool Init(const char* vFilePathName);
	bool Init(std::vector<std::string> vFileNames);
	void clean();

	GLuint getCubeMapId();
	Texture2DPtr GetTexture2D(int vIdx);

	bool ReplaceTexture(const char* vFilePathName, int vIdx);

private:
	bool Prepare(const char* vFilePathName);
	bool Prepare(std::vector<std::string> vFileNames);
};
