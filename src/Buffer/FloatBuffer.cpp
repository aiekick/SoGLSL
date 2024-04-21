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

#include "FloatBuffer.h"

#include <fstream>
#include <ctools/Logger.h>

/*#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb\stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb\stb_image_resize.h"

#define USE_GET_TEX_IMAGE*/

FloatBuffer::FloatBuffer(const GuiBackend_Window& vWin) : puWindow(vWin)
{
	buf = nullptr;
	size = 0;
	w = 0;
	h = 0;
	pbo = 0;
	usePBO = false;
	texture = 0;
	bytesPerPixel = 0;
	format = 0;

	// texParams defini les attributs de texture
}

void FloatBuffer::InitPBO()
{
	GuiBackend::MakeContextCurrent(puWindow);
	const int RGBAOffset = 4;
	size = RGBAOffset * w * h;
	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
	glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(float) * size, buf, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	usePBO = true;
}

FloatBuffer::~FloatBuffer()
{
	GuiBackend::MakeContextCurrent(puWindow);
	SAFE_DELETE_GL_BUFFER(pbo);
	SAFE_DELETE_ARRAY(buf);
}

ct::fvec4 FloatBuffer::getColorAtCoord(const int& vX, const int& vY)
{
	const int RGBAOffset = 4;
	const size_t pos = vY * w * RGBAOffset + vX * RGBAOffset;
	const float r = buf[pos + 0];
	const float g = buf[pos + 1];
	const float b = buf[pos + 2];
	const float a = buf[pos + 3];
	return ct::fvec4(r, g, b, a);
}

void FloatBuffer::getColorAtCoord(const int& vX, const int& vY, ct::fvec4 *vVec4)
{
	const int RGBAOffset = 4;
	const size_t pos = vY * w * RGBAOffset + vX * RGBAOffset;
	vVec4->x = buf[pos + 0];
	vVec4->y = buf[pos + 1];
	vVec4->z = buf[pos + 2];
	vVec4->w = buf[pos + 3];
}

ct::fvec4 FloatBuffer::getColorAtNormalizedCoord0To1(const float& vX, const float& vY)
{
	// il faut vraiment que vX et vY soient < � 1.0f sinon ca va crasher
	// pour des raisons de parallelisme on ne va rien clamper ici
	const int x = (int)RECENTERING(vX, w);
	const int y = (int)RECENTERING(vY, h);

	// il faut trouver a faire min sans conditions pour maximiser le parallelisme
	//x = min(x, w - 1);
	//y = min(y, h - 1);

	const int RGBAOffset = 4;
	const size_t pos = y * w * RGBAOffset + x * RGBAOffset;
	const float r = buf[pos + 0];
	const float g = buf[pos + 1];
	const float b = buf[pos + 2];
	const float a = buf[pos + 3];

	return ct::fvec4(r, g, b, a);
}

void FloatBuffer::getColorAtNormalizedCoord0To1(const float& vX, const float& vY, ct::fvec4 *vVec4)
{
	// il faut vraiment que vX et vY soient < � 1.0f sinon ca va crasher
	// pour des raisons de parallelisme on ne va rien clamper ici
	const int x = (int)RECENTERING(vX, w);
	const int y = (int)RECENTERING(vY, h);

	// il faut trouver a faire min sans conditions pour maximiser le parallelisme
	//x = min(x, w - 1);
	//y = min(y, h - 1);

	const int RGBAOffset = 4;
	const size_t pos = y * w * RGBAOffset + x * RGBAOffset;
	vVec4->x = buf[pos + 0];
	vVec4->y = buf[pos + 1];
	vVec4->z = buf[pos + 2];
	vVec4->w = buf[pos + 3];
}

void FloatBuffer::getColorAtNormalizedCoord0To1(const float& vX, const float& vY, float *vFloat)
{
	// il faut vraiment que vX et vY soient < � 1.0f sinon ca va crasher
	// pour des raisons de parallelisme on ne va rien clamper ici
	const int x = (int)RECENTERING(vX, w);
	const int y = (int)RECENTERING(vY, h);

	// il faut trouver a faire min sans conditions pour maximiser le parallelisme
	//x = min(x, w - 1);
	//y = min(y, h - 1);

	const int RGBAOffset = 4;
	const size_t pos = y * w * RGBAOffset + x * RGBAOffset;
	*vFloat = buf[pos + 0];
	*(vFloat + 1) = buf[pos + 1];
	*(vFloat + 2) = buf[pos + 2];
	*(vFloat + 3) = buf[pos + 3];
	//return ct::fvec4(r, g, b, a);
}

void FloatBuffer::SaveOneChannelToFile(const std::string& vFile)
{
	std::ofstream fileWriter(vFile, std::ios::out);
	if (fileWriter.bad() == false)
	{
		const int RGBAOffset = 4;
		for (int y = 0; y < h; y++)
		{
			std::string row;
			for (int x = 0; x < w; x++)
			{
				const size_t i = y * w * RGBAOffset + x * RGBAOffset;
				row += ct::toStr(floorf(buf[i] * 100.0f) / 100.0f) + ";";
			}
			fileWriter << row << "\n";
		}

		fileWriter.close();
	}
}

void FloatBuffer::SaveFourChannelToFile(const std::string& vFile)
{
	std::ofstream fileWriter(vFile, std::ios::out);
	if (fileWriter.bad() == false)
	{
		const int RGBAOffset = 4;
		for (int y = 0; y < h; y++)
		{
			std::string row;
			for (int x = 0; x < w; x++)
			{
				const size_t i = y * w * RGBAOffset + x;
				row += ct::toStr(floorf(buf[i + 0] * 100.0f) / 100.0f) + ";";
				row += ct::toStr(floorf(buf[i + 1] * 100.0f) / 100.0f) + ";";
				row += ct::toStr(floorf(buf[i + 2] * 100.0f) / 100.0f) + ";";
				row += ct::toStr(floorf(buf[i + 3] * 100.0f) / 100.0f) + ";";
			}
			fileWriter << row << "\n";
		}

		fileWriter.close();
	}
}

void FloatBuffer::CreateTexture()
{
	GuiBackend::MakeContextCurrent(puWindow);

	glGenTextures(1, &texture);
	LogGlError();

	glBindTexture(GL_TEXTURE_2D, texture);
	LogGlError();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei)w, (GLsizei)h, 0, GL_RGBA, GL_FLOAT, nullptr);
	LogGlError();

	if (texParams.useMipMap)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, texParams.maxMipMapLvl);

		glGenerateMipmap(GL_TEXTURE_2D);
		LogGlError();
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT);
	LogGlError();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter);
	LogGlError();

	glFinish();
	LogGlError();

	glBindTexture(GL_TEXTURE_2D, 0);
	LogGlError();
}

void FloatBuffer::DestroyTexture()
{
	GuiBackend::MakeContextCurrent(puWindow);

	if (glIsTexture(texture) == GL_TRUE)
	{
		LogGlError();

		// on detruit la texture
		glDeleteTextures(1, &texture);
		LogGlError();

		glFinish();
		LogGlError();
	}
}

void FloatBuffer::ChangeTexParameters(TextureParamsStruct *vTexParams)
{
	if (texture > 0)
	{
		GuiBackend::MakeContextCurrent(puWindow);

		if (glIsTexture(texture) == GL_TRUE)
		{
			texParams = *vTexParams;

			glBindTexture(GL_TEXTURE_2D, texture);
			LogGlError();

			/*//glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // from disk to opengl
			glTexImage2D(GL_TEXTURE_2D,
			0,
			texture.glinternalformat,
			(GLsizei)texture.w,
			(GLsizei)texture.h,
			0,
			texture.glformat,
			texture.gldatatype, 0);
			LogGlError();*/

			if (texParams.useMipMap)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, texParams.maxMipMapLvl);

				glGenerateMipmap(GL_TEXTURE_2D);
				LogGlError();
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texParams.wrapS);
			LogGlError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texParams.wrapT);
			LogGlError();

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParams.minFilter);
			LogGlError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParams.magFilter);
			LogGlError();

			glFinish();
			LogGlError();

			glBindTexture(GL_TEXTURE_2D, 0);
			LogGlError();
		}
	}
}
