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
#include <Buffer/pingpong.h>
#include <ctools/cTools.h>
#include <string>

class Texture3D : public PingPong
{
public:
	static Texture3DPtr createFromFile(const char* vFilePathName, std::string vSourceFile, bool vGenMipMap = true,
		std::string vWrap = "clamp", std::string vFilter = "linear");
	static Texture3DPtr createFromBuffer(int vCountChannels, float *vBuffer, size_t vCount, ct::ivec3 vVolumeSize, bool vGenMipMap = true,
		std::string vWrap = "clamp", std::string vFilter = "linear");
	static Texture3DPtr createComputeVolume(std::string vFormat, ct::ivec3 vVolumeSize, bool vGenMipMap = true,
		std::string vWrap = "clamp", std::string vFilter = "linear");

public:
	Texture3D();
	virtual ~Texture3D();
	
	bool InitFromFile(const char* vFilePathName, std::string vSourceFile, bool vGenMipMap, std::string vWrap, std::string vFilter);
	bool InitFromBuffer(int vCountChannels, float *vBuffer, size_t vCount, ct::ivec3 vVolumeSize, bool vGenMipMap, std::string vWrap, std::string vFilter);
	bool InitComputeVolume(std::string vFormat, ct::ivec3 vVolumeSize, bool vGenMipMap, std::string vWrap, std::string vFilter);
	
public:
	ct::ivec3 getSize()
	{
		if (puBackTex)
		{
			return ct::ivec3((int)puBackTex->w, (int)puBackTex->h, (int)puBackTex->d);
		}

		return ct::ivec3(0);
	}

private:
	ctTexturePtr PrepareFromFile(const char* vFilePathName, std::string vType, bool vGenMipMap, std::string vWrap, std::string vFilter);
	ctTexturePtr PrepareFromBuffer(int vCountChannels, float *vBuffer, size_t vCount, ct::ivec3 vVolumeSize, bool vGenMipMap, std::string vWrap, std::string vFilter);
	ctTexturePtr PrepareComputeVolume(std::string vFormat, ct::ivec3 vVolumeSize, bool vGenMipMap, std::string vWrap, std::string vFilter);

private:
	uint8_t* GetBufferFromFile_ShaderToy(std::string vFile, ctTexturePtr vTexture = nullptr);
	uint8_t* GetBufferFromFile_MagicaVoxel_Vox(std::string vFile, ctTexturePtr vTexture = nullptr);
};

