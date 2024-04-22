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

#include <glad/glad.h>
#include <ctools/cTools.h>
#include <string>
#include <Buffer/FloatBuffer.h>
#include <Headers/RenderPackHeaders.h>

enum FRAMEBUFFER_ATTACHMENT_TYPE_ENUM {
    FRAMEBUFFER_ATTACHMENT_TYPE_NONE = 0,
    FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_1D,
    FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D,
    FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D,
    FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D_LAYERED,
    FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER,
    FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_RENDER_BUFFER,
    FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_TEXTURE,
    FRAMEBUFFER_ATTACHMENT_TYPE_Count
};

class FloatBuffer;

struct TextureParamsStruct;
class FrameBufferAttachment {
public:
    static std::shared_ptr<FrameBufferAttachment> Create(const GuiBackend_Window& vWin);

public:
    std::string name;
    FRAMEBUFFER_ATTACHMENT_TYPE_ENUM type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_NONE;
    int attachmentId = 0;
    ct::ivec3 size;
    GLuint depthBufferId = 0;
    GLuint colorBufferId = 0;
    ctTexturePtr texture;
    bool puUseFloatBuffer = false;
    bool puUseFXAA = false;
    int puCountFXAASamples;
    bool puLoaded = false;
    int puDepthMipMapLvl = 0;
    // bool puUseMipMap = false;

public:  // if float type
    std::shared_ptr<FloatBuffer> puFrontBuffer = nullptr;
    std::shared_ptr<FloatBuffer> puBackBuffer = nullptr;
    void SwitchBuffers();

public:  // if float type
    std::shared_ptr<FloatBuffer> CreateFloatBuffer(const GuiBackend_Window& vWin);
    GuiBackend_Window puWindow;

private:
    void DestroyTexture(const GLuint& vId);
    void DestroyRenderBuffer(const GLuint& vId);

private:
    bool LoadColorTexture1DAttachment();
    bool LoadColorTexture2DAttachment();
    bool LoadColorTexture3DAttachment();
    bool LoadColorTexture3DAttachmentLayered();
    bool LoadColorRenderBufferAttachment();
    bool LoadDepthTextureAttachment();
    bool LoadDepthRenderBufferAttachment();

public:
    FrameBufferAttachment(const GuiBackend_Window& vWin);
    ~FrameBufferAttachment();
    bool Load(const bool& vUseFloatBuffer);
    bool ReLoad();
    bool ReSize(const ct::ivec2& vNewSize);  // 2d texture
    bool ReSize(const ct::ivec3& vNewSize);  // 3d texture
    void ClearAttachment();
    void AttachToFbo(const GLuint& vFboId);
    bool IsOk();
    void Destroy();
    void UpdateMipMaping(const std::string& vName);
    bool ChangeTexParameters(TextureParamsStruct* vTexParam);
    void GetTexParameters(TextureParamsStruct* vTexParam);
    std::shared_ptr<FloatBuffer> GetFloatBuffer();
};
