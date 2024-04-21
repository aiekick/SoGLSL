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

#include "Texture2D.h"
#include <ctools/Logger.h>
#include <Profiler/TracyProfiler.h>
#include <stb/stb_image.h>
#include <Headers/RenderPackHeaders.h>

Texture2DPtr Texture2D::createEmpty(GLenum vTexType)
{
	auto res = std::make_shared<Texture2D>();

	if (!res->InitEmpty(vTexType))
	{
		res.reset();
	}

	return res;
}

Texture2DPtr Texture2D::createFromFile(const char* vFilePathName, GLenum vTexType, bool vInvertY, bool vGenMipMap, std::string vWrap, std::string vFilter)
{
	auto res = std::make_shared<Texture2D>();
	
	if (!res->InitFromFile(vFilePathName, vTexType, vInvertY, vGenMipMap, vWrap, vFilter))
	{
		res.reset();
	}
	
	return res;
}

Texture2DPtr Texture2D::createComputeTexture(std::string vFormat, ct::ivec2 vSize, bool vGenMipMap, std::string vWrap, std::string vFilter)
{
	auto res = std::make_shared<Texture2D>();

	if (!res->InitComputeTexture(vFormat, vSize, vGenMipMap, vWrap, vFilter))
	{
		res.reset();
	}

	return res;
}

Texture2DPtr Texture2D::createFromBuffer(unsigned char const *buffer, int len, GLenum vTexType, bool vInvertY, bool vGenMipMap)
{
	auto res = std::make_shared<Texture2D>();

	if (!res->InitFromBuffer(buffer, len, vTexType, vInvertY, vGenMipMap))
	{
		res.reset();
	}
	
	return res;
}

Texture2D::Texture2D() : PingPong()
{

}

Texture2D::~Texture2D()
{
	clean();
}

bool Texture2D::InitEmpty(GLenum vTexType)
{
	auto res = false;

	puBackTex = PrepareEmpty(vTexType);

	if (puBackTex)
	{
		res = true;
	}

	return res;
}

bool Texture2D::InitFromBuffer(unsigned char const * /*buffer*/, int /*len*/, GLenum /*vTexType*/, bool /*vInvertY*/, bool /*vGenMipMap*/)
{
	auto res = false;

	CTOOL_DEBUG_BREAK;

	puBackTex = nullptr;// PrepareFromBuffer(int vCountChannels, float *vBuffer, size_t vCount, ct::ivec2 vTextureSize, bool vGenMipMap, std::string vWrap, std::string vFilter);
	
	if (puBackTex)
	{
		res = true;
	}

	return res;
}

// sprite normal
bool Texture2D::InitFromFile(const char* vFilePathName, GLenum vTexType, bool vInvertY, bool vGenMipMap, std::string vWrap, std::string vMode)
{
	auto res = false;

	puBackTex = PrepareFromFile(vFilePathName, vTexType, vInvertY, vGenMipMap, vWrap, vMode);
	
	if (puBackTex)
	{
		res = true;
	}

	return res;
}

bool Texture2D::InitComputeTexture(std::string vFormat, ct::ivec2 vTextureSize, bool vGenMipMap, std::string vWrap, std::string vFilter)
{
	auto res = false;

	puBackTex = PrepareComputeTexture(vFormat, vTextureSize, vGenMipMap, vWrap, vFilter);
	puFrontTex = PrepareComputeTexture(vFormat, vTextureSize, vGenMipMap, vWrap, vFilter);

	if (puBackTex && puFrontTex)
	{
		res = true;
	}

	return res;
}

ctTexturePtr Texture2D::PrepareEmpty(GLenum vTexType)
{
	ctTexturePtr res = nullptr;

	TracyGpuZone("Texture2D::PrepareEmpty");

	res = std::make_shared<ct::texture>();

	res->w = 1;
	res->h = 1;

	if (vTexType == GL_TEXTURE_2D || vTexType == GL_TEXTURE_1D)
	{
		glGenTextures(1, &res->glTex);
		LogGlError();

		glBindTexture(vTexType, res->glTex);
		LogGlError();

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		LogGlError();

		res->glWrapS = GL_CLAMP_TO_EDGE;
		res->glWrapT = GL_CLAMP_TO_EDGE;

		glTexParameteri(vTexType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		LogGlError();
		if (vTexType == GL_TEXTURE_2D)
		{
			glTexParameteri(vTexType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			LogGlError();
		}
		
		res->glMinFilter = GL_LINEAR;
		res->glMagFilter = GL_LINEAR;

		glTexParameteri(vTexType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		LogGlError();
		if (vTexType == GL_TEXTURE_2D)
		{
			glTexParameteri(vTexType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			LogGlError();
		}
		
		res->gldatatype = GL_UNSIGNED_BYTE;

		//auto comp = 4;

		/*if (comp == 1)
		{
			res->glformat = GL_RED;
			res->glinternalformat = GL_RED;
		}
		else if (comp == 2)
		{
			res->glformat = GL_RG;
			res->glinternalformat = GL_RG;
		}
		else if (comp == 3)
		{
			res->glformat = GL_RGB;
			res->glinternalformat = GL_RGB;
		}
		else *///if (comp == 4)
		{
			res->glformat = GL_RGBA;
			res->glinternalformat = GL_RGBA;
		}

		glTexImage2D(vTexType, 0, res->glinternalformat, (GLsizei)res->w, (GLsizei)res->h, 0, res->glformat, GL_UNSIGNED_BYTE, nullptr);
		LogGlError();
	}
	else
	{
		res.reset();
		LogVarError("Failed to load texture");
	}

	return res;
}

ctTexturePtr Texture2D::PrepareFromBuffer(int /*vCountChannels*/, float * /*vBuffer*/, size_t /*vCount*/, ct::ivec2 /*vTextureSize*/, bool /*vGenMipMap*/, std::string /*vWrap*/, std::string /*vFilter*/)
{
	return nullptr;
}

ctTexturePtr Texture2D::PrepareFromFile(const char* vFilePathName, GLenum vTexType, bool vInvertY, bool vGenMipMap, std::string vWrap, std::string vFilter)
{
	ctTexturePtr res = nullptr;

	TracyGpuZone("Texture2D::PrepareFromFile");
	
	//puGenMipMap = vGenMipMap;
	//puInvertY = vInvertY;
	//puRepeatMode = vRepeat;
	//puCustomSizeMode = vCustomSize;
	//puSliceMode = vSlice;
	//pu9SliceBorder = vBorder;
	//puWrap = wrap;
	//puFilter = mode;

	res = std::make_shared<ct::texture>();

	res->flipY = vInvertY;
	res->useMipMap = vGenMipMap;
	res->relativPath = vFilePathName;
	res->glTextureType = vTexType;

	stbi_set_flip_vertically_on_load(vInvertY);

	auto w = 0;
	auto h = 0;
	auto chans = 0;
	auto image = stbi_load(vFilePathName, &w, &h, &chans, 0);
	if (image)
	{
		stbi_image_free(image);
		if (chans == 4)
		{
			image = stbi_load(vFilePathName, &w, &h, &chans, STBI_rgb_alpha);
		}
		else if (chans == 3)
		{
			image = stbi_load(vFilePathName, &w, &h, &chans, STBI_rgb);
		}
		else if(chans == 2)
		{
			image = stbi_load(vFilePathName, &w, &h, &chans, STBI_grey_alpha);
		}
		else if(chans == 1)
		{
			image = stbi_load(vFilePathName, &w, &h, &chans, STBI_grey);
		}
	}
	if (image)
	{
		res->w = w;
		res->h = h;
		
		if (vTexType == GL_TEXTURE_2D)
		{
			glGenTextures(1, &res->glTex);
			LogGlError();

			glBindTexture(vTexType, res->glTex);
			LogGlError();

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			LogGlError();

			if (vWrap == "repeat")
			{
				res->glWrapS = GL_REPEAT;
				res->glWrapT = GL_REPEAT;

				glTexParameteri(vTexType, GL_TEXTURE_WRAP_S, res->glWrapS);
				glTexParameteri(vTexType, GL_TEXTURE_WRAP_T, res->glWrapT);
				LogGlError();
			}
			else if (vWrap == "mirror")
			{
				res->glWrapS = GL_MIRRORED_REPEAT;
				res->glWrapT = GL_MIRRORED_REPEAT;

				glTexParameteri(vTexType, GL_TEXTURE_WRAP_S, res->glWrapS);
				glTexParameteri(vTexType, GL_TEXTURE_WRAP_T, res->glWrapT);
				LogGlError();
			}
			else if (vWrap == "clamp")
			{
				res->glWrapS = GL_CLAMP_TO_EDGE;
				res->glWrapT = GL_CLAMP_TO_EDGE;

				glTexParameteri(vTexType, GL_TEXTURE_WRAP_S, res->glWrapS);
				glTexParameteri(vTexType, GL_TEXTURE_WRAP_T, res->glWrapT);
				LogGlError();
			}
			else
			{
				res->glWrapS = GL_CLAMP_TO_EDGE;
				res->glWrapT = GL_CLAMP_TO_EDGE;

				glTexParameteri(vTexType, GL_TEXTURE_WRAP_S, res->glWrapS);
				glTexParameteri(vTexType, GL_TEXTURE_WRAP_T, res->glWrapT);
				LogGlError();
			}

			if (vFilter == "linear")
			{
				if (vGenMipMap)
				{
					res->glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
					res->glMagFilter = GL_LINEAR;

					glTexParameteri(vTexType, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
					glTexParameteri(vTexType, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
					LogGlError();
				}
				else
				{
					res->glMinFilter = GL_LINEAR;
					res->glMagFilter = GL_LINEAR;

					glTexParameteri(vTexType, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
					glTexParameteri(vTexType, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
					LogGlError();
				}
			}
			else if (vFilter == "nearest")
			{
				if (vGenMipMap)
				{
					res->glMinFilter = GL_NEAREST_MIPMAP_NEAREST;
					res->glMagFilter = GL_NEAREST;

					glTexParameteri(vTexType, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
					glTexParameteri(vTexType, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
					LogGlError();
				}
				else
				{
					res->glMinFilter = GL_NEAREST;
					res->glMagFilter = GL_NEAREST;

					glTexParameteri(vTexType, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
					glTexParameteri(vTexType, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
					LogGlError();
				}
			}
			else
			{
				res->glMinFilter = GL_LINEAR;
				res->glMagFilter = GL_LINEAR;

				glTexParameteri(vTexType, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
				glTexParameteri(vTexType, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
				LogGlError();
			}
		}
				
		//     N=#comp     components
		//       1           red
		//       2           red, green
		//       3           red, green, blue
		//       4           red, green, blue, alpha
		
		res->gldatatype = GL_UNSIGNED_BYTE;

		if (chans == 1)
		{
			res->glformat = GL_RED;
			res->glinternalformat = GL_RED;
		}
		else if (chans == 2)
		{
			res->glformat = GL_RG;
			res->glinternalformat = GL_RG;
		}
		else if (chans == 3)
		{
			res->glformat = GL_RGB;
			res->glinternalformat = GL_RGB;
		}
		else if (chans == 4)
		{
			res->glformat = GL_RGBA;
			res->glinternalformat = GL_RGBA;
		}
				
		glTexImage2D(vTexType, 0, res->glinternalformat, (GLsizei)res->w, (GLsizei)res->h, 0, res->glformat, GL_UNSIGNED_BYTE, image);
		LogGlError();

		if (vGenMipMap)
		{
			glGenerateMipmap(vTexType);
			LogGlError();
		}
		
		glFinish();
		LogGlError();

		glBindTexture(GL_TEXTURE_2D, 0);
		LogGlError();

		stbi_image_free(image);
	}
	else
	{
		res.reset();
		LogVarError("Failed to load texture");
	}

	return res;
}
	
ctTexturePtr Texture2D::PrepareComputeTexture(std::string vFormat, ct::ivec2 vTextureSize, bool vGenMipMap, std::string vWrap, std::string vFilter)
{
	ctTexturePtr res = nullptr;

	TracyGpuZone("Texture2D::PrepareComputeTexture");

	if (!vTextureSize.emptyAND())
	{
		auto countChannels = 0;

		/* le symbol entre crochet dit que je prend en compte
		Image Unit Format 	Format Qualifier 	Image Unit Format 	Format Qualifier
		[[GL_RGBA32F]]		rgba32f 			GL_RGBA32UI 		rgba32ui
		[[GL_RGBA16F]] 		rgba16f 			GL_RGBA16UI 		rgba16ui
		[[GL_RG32F]]		rg32f 				GL_RGB10_A2UI 		rgb10_a2ui
		[[GL_RG16F]]  		rg16f 				GL_RGBA8UI 			rgba8ui
		GL_R11F_G11F_B10F 	r11f_g11f_b10f 		GL_RG32UI 			rg32ui
		[[GL_R32F]] 		r32f 				GL_RG16UI 			rg16ui
		[[GL_R16F]]  		r16f 				GL_RG8UI 			rg8ui
		GL_RGBA16 			rgba16 				GL_R32UI 			r32ui
		GL_RGB10_A2 		rgb10_a2 			GL_R16UI 			r16ui
		GL_RGBA8 			rgba8 				GL_R8UI 			r8ui
		GL_RG16 			rg16 				GL_RGBA32I 			rgba32i
		GL_RG8 				rg8 				GL_RGBA16I 			rgba16i
		GL_R16 				r16 				GL_RGBA8I 			rgba8i
		GL_R8 				r8					GL_RG32I 			rg32i
		GL_RGBA16_SNORM 	rgba16_snorm 		GL_RG16I 			rg16i
		GL_RGBA8_SNORM 		rgba8_snorm 		GL_RG8I 			rg8i
		GL_RG16_SNORM 		rg16_snorm 			GL_R32I 			r32i
		GL_RG8_SNORM 		rg8_snorm 			GL_R16I 			r16i
		GL_R16_SNORM 		r16_snorm 			GL_R8I 				r8i
		*/

		vFormat = ct::toLower(vFormat);

		if (vFormat == "r32f")
		{
			res = std::make_shared<ct::texture>();

			countChannels = 1;
			res->gldatatype = GL_FLOAT;
			res->glformat = GL_RED;
			res->glinternalformat = GL_R32F;
		}
		else if (vFormat == "r16f")
		{
			res = std::make_shared<ct::texture>();

			countChannels = 1;
			res->gldatatype = GL_FLOAT;
			res->glformat = GL_RED;
			res->glinternalformat = GL_R16F;
		}
		else if (vFormat == "rg32f")
		{
			res = std::make_shared<ct::texture>();

			countChannels = 2;
			res->gldatatype = GL_FLOAT;
			res->glformat = GL_RG;
			res->glinternalformat = GL_RG32F;
		}
		else if (vFormat == "rg16f")
		{
			res = std::make_shared<ct::texture>();

			countChannels = 2;
			res->gldatatype = GL_FLOAT;
			res->glformat = GL_RG;
			res->glinternalformat = GL_RG16F;
		}
		else if (vFormat == "rgba32f")
		{
			res = std::make_shared<ct::texture>();

			countChannels = 4;
			res->gldatatype = GL_FLOAT;
			res->glformat = GL_RGBA;
			res->glinternalformat = GL_RGBA32F;
		}
		else if (vFormat == "rgba16f")
		{
			res = std::make_shared<ct::texture>();

			countChannels = 4;
			res->gldatatype = GL_FLOAT;
			res->glformat = GL_RGBA;
			res->glinternalformat = GL_RGBA16F;
		}

		if (countChannels)
		{
			const size_t verifSize = vTextureSize.x * vTextureSize.y * countChannels;
			if (verifSize > 0)
			{
				//puGenMipMap = vGenMipMap;
				//puWrap = vWrap;
				//puFilter = vFilter;
				//puSize = vSize;
				//res->w = puSize.x;
				//res->h = puSize.y;

				res->useMipMap = vGenMipMap;
				res->glTextureType = GL_TEXTURE_2D;

				res->w = vTextureSize.x;
				res->h = vTextureSize.y;
				
				glGenTextures(1, &res->glTex);
				//LogVar("texture id = " + ct::toStr(puTexId));
				LogGlError();

				glBindTexture(GL_TEXTURE_2D, res->glTex);
				LogGlError();

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				if (vWrap == "repeat")
				{
					res->glWrapS = GL_REPEAT;
					res->glWrapT = GL_REPEAT;

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, res->glWrapS);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, res->glWrapT);
					
					LogGlError();
				}
				else if (vWrap == "mirror")
				{
					res->glWrapS = GL_MIRRORED_REPEAT;
					res->glWrapT = GL_MIRRORED_REPEAT;

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, res->glWrapS);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, res->glWrapT);

					LogGlError();
				}
				else if (vWrap == "clamp")
				{
					res->glWrapS = GL_CLAMP_TO_EDGE;
					res->glWrapT = GL_CLAMP_TO_EDGE;

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, res->glWrapS);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, res->glWrapT);

					LogGlError();
				}
				else
				{
					res->glWrapS = GL_CLAMP_TO_EDGE;
					res->glWrapT = GL_CLAMP_TO_EDGE;

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, res->glWrapS);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, res->glWrapT);
				}

				if (vFilter == "linear")
				{
					if (vGenMipMap)
					{
						res->glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
						res->glMagFilter = GL_LINEAR;

						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
					}
					else
					{
						res->glMinFilter = GL_LINEAR;
						res->glMagFilter = GL_LINEAR;

						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
					}
				}
				else if (vFilter == "nearest")
				{
					if (vGenMipMap)
					{
						res->glMinFilter = GL_NEAREST_MIPMAP_NEAREST;
						res->glMagFilter = GL_NEAREST;

						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
					}
					else
					{
						res->glMinFilter = GL_LINEAR;
						res->glMagFilter = GL_LINEAR;

						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
					}
				}
				else
				{
					res->glMinFilter = GL_LINEAR;
					res->glMagFilter = GL_LINEAR;

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
				}

				LogGlError();

				//LogVar("puTex from File Size = " + ct::toStr(puRealSize.x) + "," + ct::toStr(puRealSize.y));
				glTexImage2D(GL_TEXTURE_2D, 0, res->glinternalformat,
					(GLsizei)res->w, (GLsizei)res->h,
					0, res->glformat, res->gldatatype, nullptr);

				LogGlError();
				
				if (vGenMipMap)
				{
					glGenerateMipmap(GL_TEXTURE_2D);
					LogGlError();
				}
			}
		}
	}

	return res;
}