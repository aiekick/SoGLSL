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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include "ScreenGrabber.h"
#include <ctools/Logger.h>

ScreenGrabber::ScreenGrabber()
{

}

ScreenGrabber::~ScreenGrabber()
{

}

bool ScreenGrabber::SaveToPng(const GuiBackend_Window& vWin, const std::string& vFilePathName, int vSubSamplesCount, ct::fvec2
                              /*vNewSize*/)
{
	bool res = false;

	int width;
	int height;
	int bufSize;
	uint8_t *bmBytesRGBA = GetRGBABytesFromWindow(vWin, &width, &height, &bufSize);

	const int bytesPerPixel = 4;

	const int ss = vSubSamplesCount;

	// Sub Sampling
	if (ss > 0)
	{
		uint8_t* tempdata = new uint8_t[bufSize];
		memcpy(tempdata, bmBytesRGBA, bufSize);

		const unsigned indexMaxSize = width * height;
		for (unsigned int index = 0; index < indexMaxSize; ++index)
		{
			int count = 0;
			int r = 0, g = 0, b = 0, a = 0;

			for (int i = -ss; i <= ss; i += ss)
			{
				for (int j = -ss; j <= ss; j += ss)
				{
					const int ssIndex = index + width * j + i;
					if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize)
					{
						r += tempdata[ssIndex * bytesPerPixel + 0];
						g += tempdata[ssIndex * bytesPerPixel + 1];
						b += tempdata[ssIndex * bytesPerPixel + 2];
						a += tempdata[ssIndex * bytesPerPixel + 3];

						count++;
					}
				}
			}

			bmBytesRGBA[index * bytesPerPixel + 0] = (uint8_t)((float)r / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 1] = (uint8_t)((float)g / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 2] = (uint8_t)((float)b / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 3] = (uint8_t)((float)a / (float)count);
		}

		SAFE_DELETE_ARRAY(tempdata);
	}

	// resize
	/*if (!vNewSize.emptyAND() && (IS_FLOAT_DIFFERENT(vNewSize.x, width) || IS_FLOAT_DIFFERENT(vNewSize.y, height)))
	{
		// resize
		int newWidth = vNewSize.x;
		int newHeight = vNewSize.y;
		unsigned int newBufSize = newWidth * newHeight * bytesPerPixel;
		uint8_t* resizedData = new uint8_t[newBufSize];

		int resizeRes = stbir_resize_uint8(bmBytesRGBA, width, height, width * bytesPerPixel,
			resizedData, newWidth, newHeight, newWidth * bytesPerPixel,
			bytesPerPixel);

		if (resizeRes)
		{
			stbi_flip_vertically_on_write(1);
			int resWrite = stbi_write_png(vFilePathName.c_str(),
				newWidth,
				newHeight,
				bytesPerPixel,
				resizedData,
				newWidth * bytesPerPixel);

			if (resWrite)
				res = true;
		}

		SAFE_DELETE_ARRAY(resizedData);
	}
	else*/
	{
		stbi_flip_vertically_on_write(1);
		const int resWrite = stbi_write_png(vFilePathName.c_str(),
		                                    width,
		                                    height,
		                                    bytesPerPixel,
		                                    bmBytesRGBA,
		                                    width * bytesPerPixel);

		if (resWrite)
			res = true;
	}

	SAFE_DELETE_ARRAY(bmBytesRGBA);

	return res;
}

bool ScreenGrabber::SaveToBmp(const GuiBackend_Window& vWin, const std::string& vFilePathName, int vSubSamplesCount, ct::fvec2 vNewSize)
{
	bool res = false;

	int width;
	int height;
	int bufSize;
	uint8_t *bmBytesRGB = GetRGBABytesFromWindow(vWin, &width, &height, &bufSize);

	const int bytesPerPixel = 3;

	const int ss = vSubSamplesCount;

	// Sub Sampling
	if (ss > 0)
	{
		uint8_t* tempdata = new uint8_t[bufSize];
		memcpy(tempdata, bmBytesRGB, bufSize);

		const unsigned indexMaxSize = width * height;
		for (unsigned int index = 0; index < indexMaxSize; ++index)
		{
			int count = 0;
			int r = 0, g = 0, b = 0;// , a = 0;

			for (int i = -ss; i <= ss; i += ss)
			{
				for (int j = -ss; j <= ss; j += ss)
				{
					const int ssIndex = index + width * j + i;
					if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize)
					{
						r += tempdata[ssIndex * bytesPerPixel + 0];
						g += tempdata[ssIndex * bytesPerPixel + 1];
						b += tempdata[ssIndex * bytesPerPixel + 2];

						count++;
					}
				}
			}

			bmBytesRGB[index * bytesPerPixel + 0] = (uint8_t)((float)r / (float)count);
			bmBytesRGB[index * bytesPerPixel + 1] = (uint8_t)((float)g / (float)count);
			bmBytesRGB[index * bytesPerPixel + 2] = (uint8_t)((float)b / (float)count);
		}

		SAFE_DELETE_ARRAY(tempdata);
	}

	// resize
	if (IS_FLOAT_DIFFERENT(vNewSize.x, width) || IS_FLOAT_DIFFERENT(vNewSize.y, height))
	{
		// resize
		const int newWidth = (int)vNewSize.x;
		const int newHeight = (int)vNewSize.y;
		const unsigned int newBufSize = newWidth * newHeight * bytesPerPixel;
		uint8_t* resizedData = new uint8_t[newBufSize];

		const auto* resizeRes =
            stbir_resize_uint8_linear(bmBytesRGB, width, height, width * bytesPerPixel,
		        resizedData, newWidth, newHeight, newWidth * bytesPerPixel,
		        (stbir_pixel_layout)bytesPerPixel);

		if (resizeRes)
		{
			stbi_flip_vertically_on_write(1);
			const int resWrite = stbi_write_bmp(vFilePathName.c_str(),
			                                    newWidth,
			                                    newHeight,
			                                    bytesPerPixel,
			                                    resizedData);

			if (resWrite)
				res = true;
		}

		SAFE_DELETE_ARRAY(resizedData);
	}
	else
	{
		stbi_flip_vertically_on_write(1);
		const int resWrite = stbi_write_bmp(vFilePathName.c_str(),
		                                    width,
		                                    height,
		                                    bytesPerPixel,
		                                    bmBytesRGB);

		if (resWrite)
			res = true;
	}

	SAFE_DELETE_ARRAY(bmBytesRGB);

	return res;
}

bool ScreenGrabber::SaveToJpg(const GuiBackend_Window& vWin, const std::string& vFilePathName, int vSubSamplesCount, int vQualityFrom0To100, ct::fvec2 vNewSize)
{
	bool res = false;

	int width;
	int height;
	int bufSize;
	const int bytesPerPixel = 3;
	uint8_t *bmBytesRGB = GetRGBABytesFromWindow(vWin, &width, &height, &bufSize);

	const int ss = vSubSamplesCount;

	// Sub Sampling
	if (ss > 0)
	{
		uint8_t* tempdata = new uint8_t[bufSize];
		memcpy(tempdata, bmBytesRGB, bufSize);

		const unsigned indexMaxSize = width * height;
		for (unsigned int index = 0; index < indexMaxSize; ++index)
		{
			int count = 0;
			int r = 0, g = 0, b = 0;// , a = 0;

			for (int i = -ss; i <= ss; i += ss)
			{
				for (int j = -ss; j <= ss; j += ss)
				{
					const int ssIndex = index + width * j + i;
					if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize)
					{
						r += tempdata[ssIndex * bytesPerPixel + 0];
						g += tempdata[ssIndex * bytesPerPixel + 1];
						b += tempdata[ssIndex * bytesPerPixel + 2];

						count++;
					}
				}
			}

			bmBytesRGB[index * bytesPerPixel + 0] = (uint8_t)((float)r / (float)count);
			bmBytesRGB[index * bytesPerPixel + 1] = (uint8_t)((float)g / (float)count);
			bmBytesRGB[index * bytesPerPixel + 2] = (uint8_t)((float)b / (float)count);
		}

		SAFE_DELETE_ARRAY(tempdata);
	}

	// resize
	if (IS_FLOAT_DIFFERENT(vNewSize.x, width) || IS_FLOAT_DIFFERENT(vNewSize.y, height))
	{
		// resize
		const int newWidth = (int)vNewSize.x;
		const int newHeight = (int)vNewSize.y;
		const unsigned int newBufSize = newWidth * newHeight * bytesPerPixel;
		uint8_t* resizedData = new uint8_t[newBufSize];

		const auto* resizeRes =
            stbir_resize_uint8_linear(bmBytesRGB, width, height, width * bytesPerPixel,
		        resizedData, newWidth, newHeight, newWidth * bytesPerPixel,
		        (stbir_pixel_layout)bytesPerPixel);

		if (resizeRes)
		{
			stbi_flip_vertically_on_write(1);
			const int resWrite = stbi_write_jpg(vFilePathName.c_str(),
			                                    newWidth,
			                                    newHeight,
			                                    bytesPerPixel,
			                                    resizedData,
			                                    vQualityFrom0To100);

			if (resWrite)
				res = true;
		}

		SAFE_DELETE_ARRAY(resizedData);
	}
	else
	{
		stbi_flip_vertically_on_write(1);
		const int resWrite = stbi_write_jpg(vFilePathName.c_str(),
		                                    width,
		                                    height,
		                                    bytesPerPixel,
		                                    bmBytesRGB,
		                                    vQualityFrom0To100);

		if (resWrite)
			res = true;
	}

	SAFE_DELETE_ARRAY(bmBytesRGB);

	return res;
}

bool ScreenGrabber::SaveToTga(const GuiBackend_Window& vWin, const std::string& vFilePathName, int vSubSamplesCount, ct::fvec2 vNewSize)
{
	bool res = false;

	int width;
	int height;
	int bufSize;
	uint8_t *bmBytesRGBA = GetRGBABytesFromWindow(vWin, &width, &height, &bufSize);

	const int bytesPerPixel = 4;

	const int ss = vSubSamplesCount;

	// Sub Sampling
	if (ss > 0)
	{
		uint8_t* tempdata = new uint8_t[bufSize];
		memcpy(tempdata, bmBytesRGBA, bufSize);

		const unsigned indexMaxSize = width * height;
		for (unsigned int index = 0; index < indexMaxSize; ++index)
		{
			int count = 0;
			int r = 0, g = 0, b = 0, a = 0;

			for (int i = -ss; i <= ss; i += ss)
			{
				for (int j = -ss; j <= ss; j += ss)
				{
					const int ssIndex = index + width * j + i;
					if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize)
					{
						r += tempdata[ssIndex * bytesPerPixel + 0];
						g += tempdata[ssIndex * bytesPerPixel + 1];
						b += tempdata[ssIndex * bytesPerPixel + 2];
						a += tempdata[ssIndex * bytesPerPixel + 3];

						count++;
					}
				}
			}

			bmBytesRGBA[index * bytesPerPixel + 0] = (uint8_t)((float)r / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 1] = (uint8_t)((float)g / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 2] = (uint8_t)((float)b / (float)count);
			bmBytesRGBA[index * bytesPerPixel + 3] = (uint8_t)((float)a / (float)count);
		}

		SAFE_DELETE_ARRAY(tempdata);
	}

	// resize
	if (IS_FLOAT_DIFFERENT(vNewSize.x, width) || IS_FLOAT_DIFFERENT(vNewSize.y, height))
	{
		// resize
		const int newWidth = (int)vNewSize.x;
		const int newHeight = (int)vNewSize.y;
		const unsigned int newBufSize = newWidth * newHeight * bytesPerPixel;
		uint8_t* resizedData = new uint8_t[newBufSize];

		const auto* resizeRes =
            stbir_resize_uint8_linear(bmBytesRGBA, width, height, width * bytesPerPixel,
		        resizedData, newWidth, newHeight, newWidth * bytesPerPixel,
		        (stbir_pixel_layout)bytesPerPixel);

		if (resizeRes)
		{
			stbi_flip_vertically_on_write(1);
			const int resWrite = stbi_write_tga(vFilePathName.c_str(),
			                                    newWidth,
			                                    newHeight,
			                                    bytesPerPixel,
			                                    resizedData);

			if (resWrite)
				res = true;
		}

		SAFE_DELETE_ARRAY(resizedData);
	}
	else
	{
		stbi_flip_vertically_on_write(1);
		const int resWrite = stbi_write_tga(vFilePathName.c_str(),
		                                    width,
		                                    height,
		                                    bytesPerPixel,
		                                    bmBytesRGBA);

		if (resWrite)
			res = true;
	}

	SAFE_DELETE_ARRAY(bmBytesRGBA);

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t* ScreenGrabber::GetRGBABytesFromWindow(const GuiBackend_Window& vWin, int *vWidth, int *vHeight, int *vBufSize)
{
	GuiBackend::MakeContextCurrent(vWin);

	GuiBackend::Instance()->GetWindowSize(vWin, vWidth, vHeight);
	
	*vBufSize = 4 * (*vWidth) * (*vHeight); // 4 car RGBA

	uint8_t* bmBytes = new uint8_t[*vBufSize];

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	LogGlError();

	glReadBuffer(GL_BACK);
	LogGlError();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // from disk to opengl
	LogGlError();
	glPixelStorei(GL_PACK_ALIGNMENT, 1); // from opengl to disk
	LogGlError();
	glReadPixels(0, 0, *vWidth, *vHeight, GL_RGBA, GL_UNSIGNED_BYTE, bmBytes);
	LogGlError();
	
	return (uint8_t*)bmBytes;
}

std::shared_ptr<FloatBuffer> ScreenGrabber::GetFloatBufferFromWindow_4_Chan(const GuiBackend_Window& vWin)
{
	std::shared_ptr<FloatBuffer> fBuffer = nullptr;

	GuiBackend::MakeContextCurrent(vWin);

	/*glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // from disk to opengl
	LogGlError();
	glPixelStorei(GL_PACK_ALIGNMENT, 1); // from opengl to disk
	LogGlError();
	glReadPixels(0, 0, fBuffer->w, fBuffer->h, GL_RGBA, GL_FLOAT, fBuffer->buf);
	LogGlError();*/

	return fBuffer;
}