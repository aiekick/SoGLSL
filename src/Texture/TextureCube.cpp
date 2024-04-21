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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "TextureCube.h"
#include <ctools/Logger.h>
#include <stb/stb_image.h>
#include <Texture/Texture2D.h>
#include <Profiler/TracyProfiler.h>

TextureCubePtr TextureCube::create(const char* vFilePathName)
{
	auto res = std::make_shared<TextureCube>();
	if (!res->Init(vFilePathName))
	{
		res.reset();
	}
	return res;
}

TextureCubePtr TextureCube::create(unsigned char const* buffer, int len)
{
	auto res = std::make_shared<TextureCube>();
	if (!res->Init(buffer, len))
	{
		res.reset();
	}
	return res;
}

TextureCubePtr TextureCube::create(std::vector<std::string> vFileNames)
{
	auto res = std::make_shared<TextureCube>();
	if (!res->Init(vFileNames))
	{
		res.reset();
	}
	return res;
}

TextureCube::TextureCube()
{
	for (auto i = 0; i < 6; i++)
	{
		puTexture2D[i].reset();
	}
	CubeMapId = 0;
}

TextureCube::~TextureCube()
{
	clean();
}

void TextureCube::clean()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	for (auto i = 0; i < 6; i++)
	{
		puTexture2D[i].reset();
	}
	
	if (CubeMapId > 0)
	{
		glDeleteTextures(1, &CubeMapId);
	}
}

bool TextureCube::Init(unsigned char const* /*buffer*/, int /*len*/)
{
	return false;
}

// sprite normal
bool TextureCube::Init(const char* vFilePathName)
{
	return Prepare(vFilePathName);
}
	
bool TextureCube::Init(std::vector<std::string> vFileNames)
{
	return Prepare(vFileNames);
}

bool TextureCube::Prepare(const char* vFilePathName)
{
	auto ret = false;

	TracyGpuZone("TextureCube::Prepare");
	
	if (vFilePathName != nullptr)
	{
		glGenTextures(1, &CubeMapId);

		glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapId);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		LogGlError();

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
		LogGlError();

		//std::string filePathName = vFilePathName;

		//string fileName = filePathName;

		// on recupere le nom sans le numero de la texture
		//string name = fileName.substr(0, fileName.length() - 1);

		for (auto i = 0; i < 6; i++)
		{
			// la verif de l'existence du fichier est faite dans LoadCubeMapFromFilePathName
			std::string file = "";// filePathName.GetPath() + "\\" + name + ct::toStr(i) + "." + filePathName.GetExt();
			auto res = Texture2D::createFromFile(file.c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, false, false);
			if (res != nullptr)
			{
				puTexture2D[i].reset();
				puTexture2D[i] = res;
			}
			else
			{
				ret = false;
				break;
			}
			ret = true;
		}

		if (!ret)
		{
			clean();
		}

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		LogGlError();

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	
	return ret;
}

bool TextureCube::Prepare(std::vector<std::string> vFileNames)
{
	auto ret = false;

	TracyGpuZone("TextureCube::Prepare");

	if (vFileNames.size() == 6)
	{
		glGenTextures(1, &CubeMapId);

		glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapId);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		LogGlError();

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
		LogGlError();

		auto idx = 0;
		for (auto it = vFileNames.begin(); it != vFileNames.end(); ++it)
		{
			// la verif de l'existence du fichier est faite dans LoadCubeMapFromFilePathName
			auto file = *it;
			auto res = Texture2D::createFromFile(file.c_str(), GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx, false, false);
			if (res != nullptr)
			{
				puTexture2D[idx].reset();
				res.reset();
				puTexture2D[idx] = Texture2D::createFromFile(file.c_str(), GL_TEXTURE_2D, false, false);;
			}
			else
			{
				ret = false;
				break;
			}
			ret = true;

			idx++;
		}

		if (!ret)
		{
			clean();
		}

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		LogGlError();

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	return ret;
}

bool TextureCube::ReplaceTexture(const char* vFilePathName, int vIdx)
{
	if (vIdx >= 0 && vIdx < 6)
	{
		auto ret = false;

		TracyGpuZone("TextureCube::ReplaceTexture");

		glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapId);

		auto tex = Texture2D::createFromFile(vFilePathName, GL_TEXTURE_CUBE_MAP_POSITIVE_X + vIdx);
		if (tex != nullptr)
		{
			puTexture2D[vIdx].reset();
			puTexture2D[vIdx] = tex;
			ret = true;
		}

		tex.reset();

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		return ret;
	}

	LogVarError("vIdx < 0 && vIdx > 5");

	return false;
	
}

GLuint TextureCube::getCubeMapId()
{
	return CubeMapId;
}

Texture2DPtr TextureCube::GetTexture2D(int vIdx)
{
	return puTexture2D[vIdx];
}
