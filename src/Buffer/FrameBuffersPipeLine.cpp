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

#include <Buffer/FrameBuffersPipeLine.h>
#include <Buffer/FrameBuffer.h>
#include <ctools/Logger.h>

FrameBuffersPipeLinePtr FrameBuffersPipeLine::Create(
	const GuiBackend_Window& vWin, 
	const ct::ivec3& vSize,
	const std::string& vDataType,
	const bool& vUseFXAA,
	const int& vCountFXAASamples,
	const bool& vUseZBuffer,
	const int& vCountTextures,
	const bool& vUseRenderBuffer,
	const bool& vUseFloatBuffer)
{
	FrameBuffersPipeLinePtr pipeline = nullptr;

	if (!vSize.emptyAND() && vCountTextures > 0)
	{
		pipeline = std::make_shared<FrameBuffersPipeLine>(vWin, vSize, vDataType, vUseFXAA, vCountFXAASamples, vUseZBuffer, vCountTextures, vUseRenderBuffer, vUseFloatBuffer);

		if (pipeline->puBackBuffer == nullptr || pipeline->puFrontBuffer == nullptr)
		{
			pipeline.reset();
		}
	}

	return pipeline;
}

FrameBuffersPipeLine::FrameBuffersPipeLine(
	const GuiBackend_Window& vWin, 
	const ct::ivec3& vSize,
	const std::string& vDataType,
	const bool& vUseFXAA,
	const int& vCountFXAASamples,
	const bool& vUseZBuffer,
	const int& vCountTextures,
	const bool& vUseRenderBuffer,
	const bool& vUseFloatBuffer)
{
	puWindow = vWin;

	size = vSize;
	puCountAttachments = vCountTextures;
	//puDataType = vDataType;
	puUseFXAA = vUseFXAA;
	puCountFXAASamples = vCountFXAASamples;
	puUseZBuffer = vUseZBuffer;
	puUseRenderBuffer = vUseRenderBuffer;
	puUseFloatBuffer = vUseFloatBuffer;

	puFrontBuffer = FrameBuffer::Create(vWin, size, vDataType, puUseFXAA, puCountFXAASamples, puUseZBuffer, puCountAttachments, puUseRenderBuffer, puUseFloatBuffer);
	puBackBuffer = FrameBuffer::Create(vWin, size, vDataType, puUseFXAA, puCountFXAASamples, puUseZBuffer, puCountAttachments, puUseRenderBuffer, puUseFloatBuffer);
}

FrameBuffersPipeLine::~FrameBuffersPipeLine()
{
	puFrontBuffer.reset();
	puBackBuffer.reset();
}

std::string FrameBuffersPipeLine::GetParamsToXMLString()
{
	return puFrontBuffer->GetParamsToXMLString();
}

bool FrameBuffersPipeLine::load()
{
	const bool loaded = puFrontBuffer->load() && puBackBuffer->load();

	if (loaded)
	{
		puAttachments.clear();
		for (auto it = puFrontBuffer->GetAttachments()->begin(); it != puFrontBuffer->GetAttachments()->end(); ++it)
		{
			puAttachments.emplace_back(*it);
		}

		puCountAttachments = puFrontBuffer->getCountAttachements();
		puCountTextures = puFrontBuffer->getCountTextures2D();
	}

	return loaded;
}

bool FrameBuffersPipeLine::IsOK()
{
	return puFrontBuffer->IsOK() && puBackBuffer->IsOK();
}

bool FrameBuffersPipeLine::AddColorAttachment(TextureParamsStruct *vTexParam)
{
	const bool res =
		puFrontBuffer->AddColorAttachment(vTexParam) &&
		puBackBuffer->AddColorAttachment(vTexParam);
	glFinish();
	LogGlError();
	UpdatePipeline();
	glFinish();
	LogGlError();
	return res;
}

bool FrameBuffersPipeLine::RemoveColorAttachment(const size_t& vId)
{
	const bool res =
		puFrontBuffer->RemoveColorAttachment(vId) &&
		puBackBuffer->RemoveColorAttachment(vId);
	glFinish();
	LogGlError();
	UpdatePipeline();
	glFinish();
	LogGlError();
	return res;
}

bool FrameBuffersPipeLine::AddColorAttachmentWithoutReLoad(TextureParamsStruct *vTexParam)
{
	const bool res =
		puFrontBuffer->AddColorAttachment(vTexParam) &&
		puBackBuffer->AddColorAttachment(vTexParam);
	glFinish();
	LogGlError();
	return res;
}

bool FrameBuffersPipeLine::Resize(const ct::ivec2& vNewSize)
{
	bool res = false;

	if (puFrontBuffer && puBackBuffer)
	{
		res = puFrontBuffer->Resize(vNewSize) &&
			puBackBuffer->Resize(vNewSize);
	}
	
	if (res)
	{
		size = ct::ivec3(vNewSize, 0);
	}

	return res;
}

bool FrameBuffersPipeLine::Resize(const ct::ivec3& vNewSize)
{
	bool res = false;

	if (puFrontBuffer && puBackBuffer)
	{
		res = puFrontBuffer->Resize(vNewSize) &&
			puBackBuffer->Resize(vNewSize);
	}

	if (res)
	{
		size = vNewSize;
	}

	return res;
}

void FrameBuffersPipeLine::SetZBufferUse(const bool& vUseZBuffer)
{
	puUseZBuffer = vUseZBuffer;

	puFrontBuffer->SetZBufferUse(vUseZBuffer);
	puBackBuffer->SetZBufferUse(vUseZBuffer);
}

void FrameBuffersPipeLine::SetFXAAUse(const bool& vUseFXAA, const int& vCountFXAASamples)
{
	puUseFXAA = vUseFXAA;
	puCountFXAASamples = vCountFXAASamples;

	puFrontBuffer->SetFXAAUse(puUseFXAA, puCountFXAASamples);
	puBackBuffer->SetFXAAUse(puUseFXAA, puCountFXAASamples);

	UpdatePipeline();
}

void FrameBuffersPipeLine::ClearBuffer(const ct::fColor& vColor)
{
	puFrontBuffer->ClearBuffer(vColor);
	puBackBuffer->ClearBuffer(vColor);
}

GLuint FrameBuffersPipeLine::getFrontTextureID(const int& i)
{
	return puFrontBuffer->getTextureID(i);
}

GLuint FrameBuffersPipeLine::getBackTextureID(const int& i)
{
	return puBackBuffer->getTextureID(i);
}

ctTexturePtr FrameBuffersPipeLine::getBackTexture(const int& i)
{
	return puBackBuffer->getTexture(i);
}

ctTexturePtr FrameBuffersPipeLine::getFrontTexture(const int& i)
{
	return puFrontBuffer->getTexture(i);
}

ctTexturePtr FrameBuffersPipeLine::getBackDepthTexture()
{
	return puBackBuffer->getDepthTexture();
}

ctTexturePtr FrameBuffersPipeLine::getFrontDepthTexture()
{
	return puFrontBuffer->getDepthTexture();
}

GLuint FrameBuffersPipeLine::getFrontRenderBufferID(const int& i)
{
	return puFrontBuffer->getRenderBufferID(i);
}

GLuint FrameBuffersPipeLine::getBackRenderBufferID(const int& i)
{
	return puBackBuffer->getRenderBufferID(i);
}

GLuint FrameBuffersPipeLine::getFrontDepthTextureID()
{
	return puFrontBuffer->getDepthTextureID();
}

GLuint FrameBuffersPipeLine::getBackDepthTextureID()
{
	return puBackBuffer->getDepthTextureID();
}

GLuint FrameBuffersPipeLine::getFrontFboID()
{
	return puFrontBuffer->getFboID();
}

GLuint FrameBuffersPipeLine::getBackFboID()
{
	return puBackBuffer->getFboID();
}

FrameBufferPtr FrameBuffersPipeLine::getFrontBuffer()
{
	return puFrontBuffer;
}

FrameBufferPtr FrameBuffersPipeLine::getBackBuffer()
{
	return puBackBuffer;
}

void FrameBuffersPipeLine::UpdatePipeline()
{
	if (puFrontBuffer != nullptr)
	{
		puFrontBuffer->UpdateFrameBuffer();
		puCountAttachments = puFrontBuffer->getCountAttachements();
		puCountTextures = puFrontBuffer->getCountTextures2D();
	}
	if (puBackBuffer != nullptr)
	{
		puBackBuffer->UpdateFrameBuffer();
		puCountAttachments = puBackBuffer->getCountAttachements();
		puCountTextures = puBackBuffer->getCountTextures2D();
	}
}

int FrameBuffersPipeLine::getCountAttachments()
{
	return puCountAttachments;
}

int FrameBuffersPipeLine::getCountTextures()
{
	return puCountTextures;
}

bool FrameBuffersPipeLine::bind()
{
	return puFrontBuffer->bind();
}

void FrameBuffersPipeLine::SelectBuffersTarget()
{
	return puFrontBuffer->SelectBuffersTarget();
}

void FrameBuffersPipeLine::unbind()
{
	puFrontBuffer->unbind();

	//switchBuffers();
}

void FrameBuffersPipeLine::switchBuffers()
{
	FrameBufferPtr tmp = puBackBuffer;
	
	/*LogVarLightInfo("%i : B = B%uT%u & F = B B%uT%u",
		(intptr_t)this,
		puFrontBuffer->getFboID(), puFrontBuffer->getTextureID(0),
		puBackBuffer->getFboID(), puBackBuffer->getTextureID(0));*/

	puBackBuffer = puFrontBuffer;
	
	puFrontBuffer = tmp;
}

void FrameBuffersPipeLine::ClearColor()
{
	if (puFrontBuffer != nullptr)
	{
		puFrontBuffer->ClearBuffer(ct::fColor(0.0f, 0.0f, 0.0f, 0.0f));
	}
	if (puBackBuffer != nullptr)
	{
		puBackBuffer->ClearBuffer(ct::fColor(0.0f, 0.0f, 0.0f, 0.0f));
	}
}

void FrameBuffersPipeLine::UpdateMipMaping(const std::string& vName)
{
	if (puFrontBuffer)
	{
		puFrontBuffer->UpdateMipMaping(vName);
	}
	if (puBackBuffer)
	{
		puBackBuffer->UpdateMipMaping(vName);
	}
}

void FrameBuffersPipeLine::ChangeTexParameters(TextureParamsStruct *vTexParam)
{
	if (vTexParam)
	{
		puTexParams = *vTexParam;
		
		if (puFrontBuffer)
		{
			puFrontBuffer->ChangeTexParameters(vTexParam);
		}

		if (puBackBuffer)
		{
			puBackBuffer->ChangeTexParameters(vTexParam);
		}
	}
}

void FrameBuffersPipeLine::GetTexParameters(TextureParamsStruct *vTexParam)
{
	if (vTexParam)
	{
		if (puFrontBuffer)
		{
			puFrontBuffer->GetTexParameters(vTexParam);
		}
		else if (puBackBuffer)
		{
			puBackBuffer->GetTexParameters(vTexParam);
		}
	}
}

bool FrameBuffersPipeLine::UpdateTexParameters(const std::string& vName, TextureParamsStruct *vTexParam)
{
	bool res = true;

	if (vTexParam)
	{
		puTexParams = *vTexParam;

		if (puFrontBuffer)
		{
			res &= puFrontBuffer->UpdateTexParameters(vName, vTexParam);
		}

		if (puBackBuffer)
		{
			res &= puBackBuffer->UpdateTexParameters(vName, vTexParam);
		}
	}

	return res;
}