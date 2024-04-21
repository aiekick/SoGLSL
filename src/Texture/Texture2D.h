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
#include <Buffer/pingpong.h>
#include <ctools/cTools.h>
#include <string>

class Texture2D : public PingPong
{
public:
	static Texture2DPtr createFromFile(const char* vFilePathName, GLenum vTexType = GL_TEXTURE_2D, bool vInvertY = false, 
		bool vGenMipMap = true, std::string vWrap = "clamp", std::string vFilter = "linear");
	static Texture2DPtr createFromBuffer(unsigned char const *buffer, int len, 
		GLenum vTexType = GL_TEXTURE_2D, bool vInvertY = false, bool vGenMipMap = true);
	static Texture2DPtr createComputeTexture(std::string vFormat, ct::ivec2 vSize,
		bool vGenMipMap = true, std::string vWrap = "clamp", std::string vFilter = "linear");
	static Texture2DPtr createEmpty(GLenum vTexType = GL_TEXTURE_2D);

public:
	Texture2D();
	~Texture2D();
	
	bool InitEmpty(GLenum vTexType = GL_TEXTURE_2D);
	bool InitFromBuffer(unsigned char const *buffer, int len, GLenum vTexType = GL_TEXTURE_2D, bool vInvertY = false, bool vGenMipMap = true);
	bool InitFromFile(const char* vFilePathName, GLenum vTexType = GL_TEXTURE_2D, bool vInvertY = false, bool vGenMipMap = true, std::string wrap = "clamp", std::string mode = "linear"); // sprite normal
	bool InitComputeTexture(std::string vFormat, ct::ivec2 vTextureSize, bool vGenMipMap, std::string vWrap, std::string vFilter);

public:
	ct::ivec2 getSize()
	{
		if (puBackTex)
		{
			return ct::ivec2((int)puBackTex->w, (int)puBackTex->h);
		}

		return ct::ivec2(0);
	}
	float getRatio()
	{
		if (puBackTex)
		{
			if (puBackTex->w != 0 && puBackTex->h != 0)
				return (float)puBackTex->w / (float)puBackTex->h;
		}

		return 0.0f;
	}

private:
	ctTexturePtr PrepareEmpty(GLenum vTexType = GL_TEXTURE_2D);
	ctTexturePtr PrepareFromBuffer(int vCountChannels, float *vBuffer, size_t vCount, ct::ivec2 vTextureSize, bool vGenMipMap, std::string vWrap, std::string vFilter);
	ctTexturePtr PrepareFromFile(const char* vFilePathName, GLenum vTexType, bool vInvertY, bool vGenMipMap, std::string vWrap, std::string vFilter);
	ctTexturePtr PrepareComputeTexture(std::string vFormat, ct::ivec2 vTextureSize, bool vGenMipMap, std::string vWrap, std::string vFilter);
};
