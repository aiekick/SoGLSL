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
#include <vector>

struct FrameBufferInfosStruct
{
	ct::ivec3 size;
	std::string puDataType; // float, byte (c'est different du RGBA32f ou RGBA pour ca on verra plus tard)
	bool puUseFXAA = false;
	int puCountFXAASamples = 0;
	bool puUseZBuffer = false;
	bool puUseRenderBuffer = false;
	bool puUseFloatBuffer = false;
	int puCountAttachments = 0;
	int puCountTextures = 0;
	std::string format;
	std::string filter;
	bool mipmap = false;
	std::string wrap;
};

class FrameBuffer;
struct TextureParamsStruct;
class FrameBufferAttachment;
class FrameBuffersPipeLine
{
public:
	TextureParamsStruct puTexParams;
	FrameBufferPtr puFrontBuffer = nullptr;
	FrameBufferPtr puBackBuffer = nullptr;
	ct::ivec3 size;
	//std::string puDataType; // float, byte (c'est different du RGBA32f ou RGBA pour ca on verra plus tard)
	bool puUseFXAA = false;
	int puCountFXAASamples = 0;
	bool puUseZBuffer = false;
	bool puUseRenderBuffer = false;
	bool puUseFloatBuffer = false;
	int puCountAttachments = 0;
	int puCountTextures = 0;
	std::vector< std::shared_ptr<FrameBufferAttachment>> puAttachments;
	GuiBackend_Window puWindow;

public:
	static FrameBuffersPipeLinePtr Create(
		const GuiBackend_Window& vWin, 
		const ct::ivec3& vSize, 
		const std::string& vDataType,
		const bool& vUseFXAA, 
		const int& vCountFXAASamples,
		const bool& vUseZBuffer = false,
		const int& vCountTextures = 1,
		const bool& vUseRenderBuffer = false,
		const bool& vUseFloatBuffer = false);
	
public:
	std::vector<std::shared_ptr<FrameBufferAttachment>>* GetAttachments() { return &puAttachments; }

public:
	FrameBuffersPipeLine(
		const GuiBackend_Window& vWin, 
		const ct::ivec3& vSize,
		const std::string& vDataType,
		const bool& vUseFXAA,
		const int& vCountFXAASamples,
		const bool& vUseZBuffer,
		const int& vCountTextures,
		const bool& vUseRenderBuffer,
		const bool& vUseFloatBuffer);
	~FrameBuffersPipeLine();

	std::string GetParamsToXMLString();
	bool load();
	bool IsOK();
	bool AddColorAttachment(TextureParamsStruct *vTexParam = nullptr);
	bool RemoveColorAttachment(const size_t& vId);
	bool AddColorAttachmentWithoutReLoad(TextureParamsStruct *vTexParam = nullptr);
	bool Resize(const ct::ivec2& vNewSize);
	bool Resize(const ct::ivec3& vNewSize);
	void SetZBufferUse(const bool& vUseZBuffer);
	void SetFXAAUse(const bool& vUseFXAA, const int& vCountFXAASamples);
	void ClearBuffer(const ct::fColor& vColor);
	GLuint getFrontTextureID(const int& i = 0);
	GLuint getBackTextureID(const int& i = 0);
	ctTexturePtr getBackTexture(const int& i = 0);
	ctTexturePtr getFrontTexture(const int& i = 0);
	ctTexturePtr getBackDepthTexture();
	ctTexturePtr getFrontDepthTexture();
	GLuint getFrontRenderBufferID(const int& i = 0);
	GLuint getBackRenderBufferID(const int& i = 0);
	GLuint getFrontDepthTextureID();
	GLuint getBackDepthTextureID();
	GLuint getFrontFboID();
	GLuint getBackFboID();
	FrameBufferPtr getFrontBuffer();
	FrameBufferPtr getBackBuffer();
	void UpdatePipeline();
	int getCountAttachments();
	int getCountTextures();
	bool bind();
	void SelectBuffersTarget();
	void unbind();
	void switchBuffers();
	void ClearColor();

	void UpdateMipMaping(const std::string& vName);

	void ChangeTexParameters(TextureParamsStruct *vTexParam);
	void GetTexParameters(TextureParamsStruct *vTexParam);
	bool UpdateTexParameters(const std::string& vName, TextureParamsStruct *vTexParam);
};

