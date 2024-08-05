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

#include "FrameBufferAttachment.h"
#include <ctools/Logger.h>
#include <Profiler/TracyProfiler.h>
#include <ImGuiPack.h>
#include <iagp/iagp.h>
#include <ctools/GLVersionChecker.h>

//////////////////////////////////////////////////////////////

std::shared_ptr<FrameBufferAttachment> FrameBufferAttachment::Create(const GuiBackend_Window& vWin) {
    return std::make_shared<FrameBufferAttachment>(vWin);
}

//////////////////////////////////////////////////////////////

FrameBufferAttachment::FrameBufferAttachment(const GuiBackend_Window& vWin) {
    puLoaded = false;

    texture = std::make_shared<ct::texture>();
    puWindow = vWin;
    name.clear();
    size = 0;
    attachmentId = -1;
    texture->glinternalformat = 0;
    puUseFloatBuffer = false;
    puUseFXAA = false;
    puCountFXAASamples = 2;
    depthBufferId = 0;
    colorBufferId = 0;
    texture->glTex = 0;
    type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_NONE;
    puFrontBuffer = nullptr;
    puBackBuffer = nullptr;
    puDepthMipMapLvl = 0;
    // puUseMipMap = false;

    texture->glWrapS = GL_CLAMP_TO_EDGE;
    texture->glWrapT = GL_CLAMP_TO_EDGE;
    texture->glWrapR = GL_CLAMP_TO_EDGE;
    texture->glMinFilter = GL_LINEAR;
    texture->glMagFilter = GL_LINEAR;
    texture->useMipMap = false;
}

//////////////////////////////////////////////////////////////

void FrameBufferAttachment::DestroyTexture(const GLuint& vId) {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBufferAttachment::DestroyTexture");

    if (glIsTexture(vId) == GL_TRUE) {
        LogGlError();

        // on detruit la texture
        glDeleteTextures(1, &vId);
        LogGlError();

        glFinish();
        LogGlError();
    }
}

void FrameBufferAttachment::DestroyRenderBuffer(const GLuint& vId) {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBufferAttachment::DestroyRenderBuffer");

    if (glIsRenderbuffer(vId) == GL_TRUE) {
        LogGlError();

        // destroy renderbuffer
        glDeleteRenderbuffers(1, &vId);
        LogGlError();

        glFinish();
        LogGlError();
    }
}

void FrameBufferAttachment::Destroy() {
    TracyGpuZone("FrameBufferAttachment::Destroy");

    if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_RENDER_BUFFER) {
        DestroyRenderBuffer(depthBufferId);
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER) {
        DestroyRenderBuffer(colorBufferId);
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_1D ||
               type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D ||
               type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D ||
               type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_TEXTURE ||
               type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D_LAYERED) {
        DestroyTexture(texture->glTex);
    }

    puFrontBuffer.reset();
    puBackBuffer.reset();
}

FrameBufferAttachment::~FrameBufferAttachment() {
    Destroy();

    texture.reset();
}

//////////////////////////////////////////////////////////////

bool FrameBufferAttachment::LoadColorTexture1DAttachment() {
    TracyGpuZone("FrameBufferAttachment::LoadColorTexture1DAttachment");

    /*GuiBackend::MakeContextCurrent(puWindow);

    type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_1D;

    if (texture->glTex > 0)
    {
        if (glIsTexture(texture->glTex) == GL_TRUE)
        {
            LogGlError();

            // on detruit la texture
            glDeleteTextures(1, &texture->glTex);
        }
    }

    GLuint texId = 0;
    glGenTextures(1, &texId);
    LogGlError();

    texture->glTex = texId;
    texture->w = size.x;
    texture->h = size.y;
    texture->glformat = puFormat;
    texture->glinternalformat = texture->glinternalformat;
    texture->gldatatype = puTexDataType;

    texture->glWrapS = GL_CLAMP_TO_EDGE;
    texture->glWrapT = GL_CLAMP_TO_EDGE;
    texture->glMinFilter = GL_LINEAR;
    texture->glMagFilter = GL_LINEAR;

    glBindTexture(GL_TEXTURE_2D, texture->glTex);
    LogGlError();

    glGenerateMipmap(GL_TEXTURE_2D);
    LogGlError();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->glWrapS);
    LogGlError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture->glWrapT);
    LogGlError();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->glMinFilter);
    LogGlError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture->glMagFilter);
    LogGlError();

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // from disk to opengl
    glTexImage2D(GL_TEXTURE_2D,
        0,
        texture->glinternalformat,
        (GLsizei)texture->w,
        (GLsizei)texture->h,
        0,
        texture->glformat,
        texture->gldatatype, 0);
    LogGlError();

    glBindTexture(GL_TEXTURE_2D, 0);
    LogGlError();

    if (glIsTexture(texture->glTex) != GL_TRUE)
        LogVarError("Texture %u is bad", texture->glTex);

    if (glIsTexture(texture->glTex) == GL_FALSE)
    {
        return false;
    }*/

    return false;  // false pour le moment en attendant l'implementation
}

bool FrameBufferAttachment::LoadColorTexture2DAttachment() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBufferAttachment::LoadColorTexture2DAttachment");

    type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D;

    if (texture->glTex > 0) {
        if (glIsTexture(texture->glTex) == GL_TRUE) {
            // on detruit la texture
            glDeleteTextures(1, &texture->glTex);
            LogGlError();

            glFinish();
            LogGlError();
        }
    }

    GLuint texId = 0;
    glGenTextures(1, &texId);
    LogGlError();

    texture->glTex = texId;
    texture->w = (size_t)size.x;
    texture->h = (size_t)size.y;

    // texture->glWrapS = GL_CLAMP_TO_EDGE;
    // texture->glWrapT = GL_CLAMP_TO_EDGE;
    // texture->glMinFilter = GL_LINEAR;
    // texture->glMagFilter = GL_LINEAR;

    // normal ou FXAA
    GLenum textureTypeEnum = GL_TEXTURE_2D;
    if (puUseFXAA)
        textureTypeEnum = GL_TEXTURE_2D_MULTISAMPLE;

    glBindTexture(textureTypeEnum, texture->glTex);
    LogGlError();

    // glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // from disk to opengl
    if (puUseFXAA) {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, puCountFXAASamples, texture->glinternalformat, (GLsizei)texture->w, (GLsizei)texture->h, GL_TRUE);
        LogGlError();
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, texture->glinternalformat, (GLsizei)texture->w, (GLsizei)texture->h, 0, texture->glformat, texture->gldatatype, nullptr);
        LogGlError();

        if (texture->useMipMap) {
            glTexParameteri(textureTypeEnum, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(textureTypeEnum, GL_TEXTURE_MAX_LEVEL, texture->maxMipMapLvl);

            glGenerateMipmap(textureTypeEnum);
            LogGlError();
        }

        glTexParameteri(textureTypeEnum, GL_TEXTURE_WRAP_S, texture->glWrapS);
        LogGlError();
        glTexParameteri(textureTypeEnum, GL_TEXTURE_WRAP_T, texture->glWrapT);
        LogGlError();

        glTexParameteri(textureTypeEnum, GL_TEXTURE_MIN_FILTER, texture->glMinFilter);
        LogGlError();
        glTexParameteri(textureTypeEnum, GL_TEXTURE_MAG_FILTER, texture->glMagFilter);
        LogGlError();
    }

    glFinish();
    LogGlError();

    glBindTexture(textureTypeEnum, 0);
    LogGlError();

    if (glIsTexture(texture->glTex) != GL_TRUE)
        LogVarError("Texture %u is bad", texture->glTex);

    if (glIsTexture(texture->glTex) == GL_FALSE) {
        return false;
    }

    return true;
}

bool FrameBufferAttachment::LoadColorTexture3DAttachment() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBufferAttachment::LoadColorTexture3DAttachment");

    type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D;

    if (texture->glTex > 0) {
        if (glIsTexture(texture->glTex) == GL_TRUE) {
            LogGlError();

            // on detruit la texture
            glDeleteTextures(1, &texture->glTex);
        }
    }

    GLuint texId = 0;
    glGenTextures(1, &texId);
    LogGlError();

    texture->glTex = texId;
    texture->w = (size_t)size.x;
    texture->h = (size_t)size.y;
    texture->d = (size_t)size.z;

    // texture->glWrapS = GL_CLAMP_TO_EDGE;
    // texture->glWrapT = GL_CLAMP_TO_EDGE;
    // texture->glWrapR = GL_CLAMP_TO_EDGE;
    // texture->glMinFilter = GL_LINEAR;
    // texture->glMagFilter = GL_LINEAR;

    glBindTexture(GL_TEXTURE_3D, texture->glTex);
    LogGlError();

    glTexImage3D(
        GL_TEXTURE_3D, 0, texture->glinternalformat, (GLsizei)texture->w, (GLsizei)texture->h, (GLsizei)texture->d, 0, texture->glformat, texture->gldatatype, nullptr);
    LogGlError();

    if (texture->useMipMap) {
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, texture->maxMipMapLvl);

        glGenerateMipmap(GL_TEXTURE_3D);
        LogGlError();
    }

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, texture->glWrapS);
    LogGlError();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, texture->glWrapT);
    LogGlError();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, texture->glWrapR);
    LogGlError();

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, texture->glMinFilter);
    LogGlError();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, texture->glMagFilter);
    LogGlError();

    glBindTexture(GL_TEXTURE_3D, 0);
    LogGlError();

    if (glIsTexture(texture->glTex) != GL_TRUE)
        LogVarError("Texture %u is bad", texture->glTex);

    if (glIsTexture(texture->glTex) == GL_FALSE) {
        return false;
    }

    return true;
}

// https://gist.github.com/tylercamp/90a59104e439835f48554b1a7236f6a3
bool FrameBufferAttachment::LoadColorTexture3DAttachmentLayered() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBufferAttachment::LoadColorTexture3DAttachmentLayered");

    type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D_LAYERED;

    if (texture->glTex > 0) {
        if (glIsTexture(texture->glTex) == GL_TRUE) {
            LogGlError();

            // on detruit la texture
            glDeleteTextures(1, &texture->glTex);
        }
    }

    GLuint texId = 0;
    glGenTextures(1, &texId);
    LogGlError();

    texture->glTex = texId;
    texture->w = (size_t)size.x;
    texture->h = (size_t)size.y;
    texture->d = (size_t)size.z;

    // texture->glWrapS = GL_CLAMP_TO_EDGE;
    // texture->glWrapT = GL_CLAMP_TO_EDGE;
    // texture->glWrapR = GL_CLAMP_TO_EDGE;
    // texture->glMinFilter = GL_LINEAR;
    // texture->glMagFilter = GL_LINEAR;

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture->glTex);
    LogGlError();

    glTexImage3D(GL_TEXTURE_2D_ARRAY,
                 0,
                 texture->glinternalformat,
                 (GLsizei)texture->w,
                 (GLsizei)texture->h,
                 (GLsizei)texture->d,
                 0,
                 texture->glformat,
                 texture->gldatatype,
                 nullptr);
    LogGlError();

    if (texture->useMipMap) {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, texture->maxMipMapLvl);

        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        LogGlError();
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, texture->glWrapS);
    LogGlError();
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, texture->glWrapT);
    LogGlError();
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, texture->glWrapR);
    LogGlError();

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, texture->glMinFilter);
    LogGlError();
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, texture->glMagFilter);
    LogGlError();

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    LogGlError();

    if (glIsTexture(texture->glTex) != GL_TRUE)
        LogVarError("Texture %u is bad", texture->glTex);

    if (glIsTexture(texture->glTex) == GL_FALSE) {
        return false;
    }

    return true;
}

bool FrameBufferAttachment::LoadDepthTextureAttachment() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBufferAttachment::LoadDepthTextureAttachment");

    type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_TEXTURE;

    if (texture->glTex > 0) {
        if (glIsTexture(texture->glTex) == GL_TRUE) {
            LogGlError();

            // on detruit la texture
            glDeleteTextures(1, &texture->glTex);
        }
    }

    glGenTextures(1, &texture->glTex);
    LogGlError();

    texture->gldatatype = GL_FLOAT;
    texture->glformat = GL_DEPTH_COMPONENT;
    texture->glinternalformat = GL_DEPTH_COMPONENT32;
    texture->w = (size_t)size.x;
    texture->h = (size_t)size.y;

    texture->glWrapS = GL_REPEAT;
    texture->glWrapT = GL_REPEAT;
    texture->glMinFilter = GL_NEAREST;
    texture->glMagFilter = GL_NEAREST;

    // normal ou FXAA
    GLenum textureTypeEnum = GL_TEXTURE_2D;
    if (puUseFXAA)
        textureTypeEnum = GL_TEXTURE_2D_MULTISAMPLE;

    glBindTexture(textureTypeEnum, texture->glTex);
    LogGlError();

    if (puUseFXAA) {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, puCountFXAASamples, texture->glinternalformat, (GLsizei)texture->w, (GLsizei)texture->h, GL_TRUE);
        LogGlError();
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, texture->glinternalformat, (GLsizei)texture->w, (GLsizei)texture->h, 0, texture->glformat, texture->gldatatype, nullptr);
        LogGlError();

        if (texture->useMipMap) {
            glTexParameteri(textureTypeEnum, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(textureTypeEnum, GL_TEXTURE_MAX_LEVEL, texture->maxMipMapLvl);

            glGenerateMipmap(textureTypeEnum);
            LogGlError();
        }
    }

    glTexParameteri(textureTypeEnum, GL_TEXTURE_WRAP_S, texture->glWrapS);
    LogGlError();
    glTexParameteri(textureTypeEnum, GL_TEXTURE_WRAP_T, texture->glWrapT);
    LogGlError();

    glTexParameteri(textureTypeEnum, GL_TEXTURE_MIN_FILTER, texture->glMinFilter);
    LogGlError();
    glTexParameteri(textureTypeEnum, GL_TEXTURE_MAG_FILTER, texture->glMagFilter);
    LogGlError();

    if (GLVersion.major == 4 && GLVersion.minor >= 3) {
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/
        glTexParameteri(textureTypeEnum, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
        LogGlError();
    } else {
        glTexParameteri(textureTypeEnum, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
        LogGlError();
    }

    glTexParameteri(textureTypeEnum, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    LogGlError();
    glTexParameteri(textureTypeEnum, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    LogGlError();

    glFinish();
    LogGlError();

    glBindTexture(textureTypeEnum, 0);
    LogGlError();

    if (glIsTexture(texture->glTex) != GL_TRUE)
        LogVarError("Texture %u is bad", texture->glTex);

    if (glIsTexture(texture->glTex) == GL_FALSE) {
        return false;
    }

    return true;
}

bool FrameBufferAttachment::LoadDepthRenderBufferAttachment() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBufferAttachment::LoadDepthRenderBufferAttachment");

    type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_RENDER_BUFFER;

    // Destruction d'un �ventuel ancien Render Buffer
    if (depthBufferId > 0) {
        if (glIsRenderbuffer(depthBufferId) == GL_TRUE) {
            LogGlError();

            // destroy renderbuffer
            glDeleteRenderbuffers(1, &depthBufferId);
            LogGlError();
        }
    }

    // G�n�ration de l'identifiant
    GLuint renderBufferId = 0;
    glGenRenderbuffers(1, &renderBufferId);
    LogGlError();

    depthBufferId = renderBufferId;

    // Verrouillage
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferId);
    LogGlError();

    texture->glinternalformat = GL_DEPTH_COMPONENT32F;

    // Configuration du Render Buffer
    if (puUseFXAA) {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, puCountFXAASamples, texture->glinternalformat, (GLsizei)size.x, (GLsizei)size.y);
        LogGlError();
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, texture->glinternalformat, (GLsizei)size.x, (GLsizei)size.y);
        LogGlError();
    }

    glFinish();
    LogGlError();

    // D�verrouillage
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    LogGlError();

    if (glIsRenderbuffer(depthBufferId) != GL_TRUE)
        LogVarError("RenderBuffer %u is bad", depthBufferId);

    if (!(glIsRenderbuffer(depthBufferId) == GL_TRUE)) {
        return false;
    }

    return true;
}

bool FrameBufferAttachment::LoadColorRenderBufferAttachment() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBufferAttachment::LoadColorRenderBufferAttachment");

    type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER;

    // Destruction d'un �ventuel ancien Render Buffer
    if (colorBufferId > 0) {
        if (glIsRenderbuffer(colorBufferId) == GL_TRUE) {
            LogGlError();

            // destroy renderbuffer
            glDeleteRenderbuffers(1, &colorBufferId);
            LogGlError();
        }
    }

    // G�n�ration de l'identifiant
    GLuint renderBufferId = 0;
    glGenRenderbuffers(1, &renderBufferId);
    LogGlError();

    colorBufferId = renderBufferId;

    // Verrouillage
    glBindRenderbuffer(GL_RENDERBUFFER, colorBufferId);
    LogGlError();

    // Configuration du Render Buffer
    if (puUseFXAA) {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, puCountFXAASamples, texture->glinternalformat, (GLsizei)size.x, (GLsizei)size.y);
        LogGlError();
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, texture->glinternalformat, (GLsizei)size.x, (GLsizei)size.y);
        LogGlError();
    }

    glFinish();
    LogGlError();

    // D�verrouillage
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    LogGlError();

    if (glIsRenderbuffer(colorBufferId) != GL_TRUE)
        LogVarError("RenderBuffer %u is bad", colorBufferId);

    if (!(glIsRenderbuffer(colorBufferId) == GL_TRUE)) {
        return false;
    }

    return true;
}

bool FrameBufferAttachment::Load(const bool& vUseFloatBuffer) {
    puLoaded = false;

    TracyGpuZone("FrameBufferAttachment::Load");

    if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_RENDER_BUFFER) {
        puLoaded = LoadDepthRenderBufferAttachment();
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER) {
        puLoaded = LoadColorRenderBufferAttachment();
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_1D) {
        puLoaded = LoadColorTexture1DAttachment();

        if (vUseFloatBuffer) {
            puFrontBuffer = CreateFloatBuffer(puWindow);
            puBackBuffer = CreateFloatBuffer(puWindow);
        }
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D) {
        puLoaded = LoadColorTexture2DAttachment();

        if (vUseFloatBuffer) {
            puFrontBuffer = CreateFloatBuffer(puWindow);
            puBackBuffer = CreateFloatBuffer(puWindow);
        }
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D) {
        puLoaded = LoadColorTexture3DAttachment();

        if (vUseFloatBuffer) {
            puFrontBuffer = CreateFloatBuffer(puWindow);
            puBackBuffer = CreateFloatBuffer(puWindow);
        }
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D_LAYERED) {
        puLoaded = LoadColorTexture3DAttachmentLayered();

        if (vUseFloatBuffer) {
            puFrontBuffer = CreateFloatBuffer(puWindow);
            puBackBuffer = CreateFloatBuffer(puWindow);
        }
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_TEXTURE) {
        puLoaded = LoadDepthTextureAttachment();
    }

    if (puLoaded) {
        puUseFloatBuffer = vUseFloatBuffer;
    }

    return puLoaded;
}

bool FrameBufferAttachment::ReLoad() {
    TracyGpuZone("FrameBufferAttachment::ReLoad");

    Destroy();
    return Load(puUseFloatBuffer);
}

bool FrameBufferAttachment::ReSize(const ct::ivec2& vNewSize) {
    if (vNewSize.x > 0 && vNewSize.y > 0) {
        TracyGpuZone("FrameBufferAttachment::ReSize");

        Destroy();
        size = ct::ivec3(vNewSize, 0);
        return Load(puUseFloatBuffer);
    }
    return false;
}

bool FrameBufferAttachment::ReSize(const ct::ivec3& vNewSize) {
    if (vNewSize.x > 0 && vNewSize.y > 0) {
        TracyGpuZone("FrameBufferAttachment::ReSize");

        Destroy();
        size = vNewSize;
        return Load(puUseFloatBuffer);
    }
    return false;
}

std::shared_ptr<FloatBuffer> FrameBufferAttachment::CreateFloatBuffer(const GuiBackend_Window& vWin) {
    GuiBackend::MakeContextCurrent(puWindow);

    if (texture->gldatatype == GL_FLOAT) {
        std::shared_ptr<FloatBuffer> buffer = std::make_shared<FloatBuffer>(vWin);

        buffer->w = ct::mini(size.x, 4096);
        buffer->h = ct::mini(size.y, 4096);

        if (texture->glformat == GL_RGBA)
            buffer->bytesPerPixel = 4;  // 4 car RGBA
        else if (texture->glformat == GL_RGB)
            buffer->bytesPerPixel = 3;  // 3 car RGB

        buffer->size = (size_t)((double)buffer->bytesPerPixel * (double)(size.x) * (double)(size.y));

        float* buf = nullptr;

        try {
            buf = new float[buffer->size];
        } catch (std::exception /*e*/) {
            return nullptr;
        }

        buffer->buf = buf;

        // buffer->InitPBO();

        return buffer;
    }

    return nullptr;
}

void FrameBufferAttachment::SwitchBuffers() {
    std::shared_ptr<FloatBuffer> tmp = puFrontBuffer;
    puFrontBuffer = puBackBuffer;
    puBackBuffer = tmp;
}

//////////////////////////////////////////////////////////////

void FrameBufferAttachment::AttachToFbo(const GLuint& vFboId) {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBufferAttachment::AttachToFbo");

    if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_RENDER_BUFFER) {
        glBindFramebuffer(GL_FRAMEBUFFER, vFboId);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferId);
        LogGlError();
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER) {
        glBindFramebuffer(GL_FRAMEBUFFER, vFboId);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentId, GL_RENDERBUFFER, colorBufferId);
        LogGlError();
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_1D) {
        glBindFramebuffer(GL_FRAMEBUFFER, vFboId);
        glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentId, GL_TEXTURE_1D, texture->glTex, 0);
        LogGlError();
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D) {
        // normal ou FXAA
        if (puUseFXAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, vFboId);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentId, GL_TEXTURE_2D_MULTISAMPLE, texture->glTex, 0);
            LogGlError();
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, vFboId);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentId, GL_TEXTURE_2D, texture->glTex, 0);
            LogGlError();
        }
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D) {
        glBindFramebuffer(GL_FRAMEBUFFER, vFboId);
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentId, GL_TEXTURE_3D, texture->glTex, 0, 0);
        LogGlError();
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D_LAYERED) {
        glBindFramebuffer(GL_FRAMEBUFFER, vFboId);
        for (unsigned int i = 0; i < (unsigned int)size.z; i++) {
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->glTex, 0, i);
            LogGlError();
        }
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_TEXTURE) {
        // normal ou FXAA
        if (puUseFXAA) {
            glBindFramebuffer(GL_FRAMEBUFFER, vFboId);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, texture->glTex, 0);
            LogGlError();
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, vFboId);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->glTex, 0);
            LogGlError();
        }
    }

    glFinish();
    LogGlError();
}

//////////////////////////////////////////////////////////////

void FrameBufferAttachment::ClearAttachment() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBufferAttachment::ClearAttachment");

    if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_RENDER_BUFFER) {
        glClear(GL_DEPTH_BUFFER_BIT);
        LogGlError();
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER) {
        glClear(GL_COLOR_BUFFER_BIT);
        LogGlError();
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_1D) {
        glClear(GL_COLOR_BUFFER_BIT);
        LogGlError();
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D) {
        glClear(GL_COLOR_BUFFER_BIT);
        LogGlError();
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D ||
               type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D_LAYERED) {
        glClear(GL_COLOR_BUFFER_BIT);
        LogGlError();
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_TEXTURE) {
        glClear(GL_DEPTH_BUFFER_BIT);
        LogGlError();
    }

    glFinish();
    LogGlError();
}

//////////////////////////////////////////////////////////////

bool FrameBufferAttachment::IsOk() {
    GuiBackend::MakeContextCurrent(puWindow);

    if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_RENDER_BUFFER) {
        const bool isRend = (glIsRenderbuffer(depthBufferId) == GL_TRUE);
        if (!isRend)
            LogVarError("FBO::IsOK() => Depth RenderBuffer %u is bad", depthBufferId);
        return isRend;
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER) {
        const bool isRend = (glIsRenderbuffer(colorBufferId) == GL_TRUE);
        if (!isRend)
            LogVarError("FBO::IsOK() => Color RenderBuffer %u is bad", colorBufferId);
        return isRend;
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_1D) {
        const bool isTex = (glIsTexture(texture->glTex) == GL_TRUE);
        if (!isTex)
            LogVarError("FBO::IsOK() => Color Texture 1D %u is bad", texture->glTex);
        return isTex;
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D) {
        const bool isTex = (glIsTexture(texture->glTex) == GL_TRUE);
        if (!isTex)
            LogVarError("FBO::IsOK() => Color Texture 2D %u is bad", texture->glTex);
        return isTex;
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D) {
        const bool isTex = (glIsTexture(texture->glTex) == GL_TRUE);
        if (!isTex)
            LogVarError("FBO::IsOK() => Color Texture 3D %u is bad", texture->glTex);
        return isTex;
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D_LAYERED) {
        bool isTex = true;
        for (size_t layer = 0; layer < (size_t)size.z; layer++) {
            if (layer < texture->glTexLayered.size()) {
                isTex &= (glIsTexture(texture->glTexLayered[layer]) == GL_TRUE);
            }
        }
        if (!isTex)
            LogVarError("FBO::IsOK() => Color Texture 3D %u is bad", texture->glTex);
        return isTex;
    } else if (type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_TEXTURE) {
        const bool isTex = (glIsTexture(texture->glTex) == GL_TRUE);
        if (!isTex)
            LogVarError("FBO::IsOK() => Depth Texture %u is bad", texture->glTex);
        return isTex;
    }

    return false;
}

void FrameBufferAttachment::UpdateMipMaping(const std::string& vName) {
    if (!puLoaded)  // avant l'appel de load()
    {
    } else {
        GuiBackend::MakeContextCurrent(puWindow);

        if (texture->useMipMap /*puUseMipMap*/) {
            TracyGpuZone("FrameBufferAttachment::UpdateMipMaping");
            AIGPScoped(vName, "MipMaping");

            glBindTexture(GL_TEXTURE_2D, texture->glTex);
            LogGlError();

            glGenerateMipmap(GL_TEXTURE_2D);
            LogGlError();

            glFinish();
            LogGlError();

            glBindTexture(GL_TEXTURE_2D, 0);
            LogGlError();
        }
    }
}

bool FrameBufferAttachment::ChangeTexParameters(TextureParamsStruct* vTexParam) {
    bool needReload = false;

    if (vTexParam) {
        TracyGpuZone("FrameBufferAttachment::ChangeTexParameters");

        if (!puLoaded)  // avant l'appel de load()
        {
            texture->glformat = vTexParam->format;
            texture->glinternalformat = vTexParam->internalFormat;
            texture->gldatatype = vTexParam->dataType;

            texture->glWrapS = vTexParam->wrapS;
            texture->glWrapT = vTexParam->wrapT;
            texture->glMinFilter = vTexParam->minFilter;
            texture->glMagFilter = vTexParam->magFilter;
            texture->useMipMap = vTexParam->useMipMap;
            texture->maxMipMapLvl = vTexParam->maxMipMapLvl;
        } else {
            if (texture->glTex > 0) {
                GuiBackend::MakeContextCurrent(puWindow);

                if (glIsTexture(texture->glTex) == GL_TRUE) {
                    if (texture->glformat != vTexParam->format || texture->glinternalformat != vTexParam->internalFormat || texture->gldatatype != vTexParam->dataType)
                        needReload = true;

                    texture->glformat = vTexParam->format;
                    texture->glinternalformat = vTexParam->internalFormat;
                    texture->gldatatype = vTexParam->dataType;

                    texture->glWrapS = vTexParam->wrapS;
                    texture->glWrapT = vTexParam->wrapT;
                    texture->glMinFilter = vTexParam->minFilter;
                    texture->glMagFilter = vTexParam->magFilter;
                    texture->useMipMap = vTexParam->useMipMap;
                    texture->maxMipMapLvl = vTexParam->maxMipMapLvl;

                    glBindTexture(GL_TEXTURE_2D, texture->glTex);
                    LogGlError();

                    // si on fait pas ca on peut pas desactiver le mipmpa
                    // quoi qu'il arrive il sera reuplaod� avec le fbo qui l'utilise
                    /*glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // from disk to opengl
                    glTexImage2D(GL_TEXTURE_2D,
                        0,
                        texture->glinternalformat,
                        (GLsizei)texture->w,
                        (GLsizei)texture->h,
                        0,
                        texture->glformat,
                        texture->gldatatype, 0);
                    LogGlError();*/

                    if (texture->useMipMap) {
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, texture->maxMipMapLvl);
                        LogGlError();
                        // glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
                        glGenerateMipmap(GL_TEXTURE_2D);  // fonction en asynchorne d'ou le call a glFinish
                        LogGlError();
                    } else {
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
                        LogGlError();
                        // glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
                        // LogGlError();
                    }

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->glWrapS);
                    LogGlError();
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture->glWrapT);
                    LogGlError();

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->glMinFilter);
                    LogGlError();
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture->glMagFilter);
                    LogGlError();

                    glFinish();
                    LogGlError();

                    glBindTexture(GL_TEXTURE_2D, 0);
                    LogGlError();

                    if (puFrontBuffer)
                        puFrontBuffer->ChangeTexParameters(vTexParam);
                    if (puBackBuffer)
                        puBackBuffer->ChangeTexParameters(vTexParam);
                }
            }
        }
    }

    return needReload;
}

void FrameBufferAttachment::GetTexParameters(TextureParamsStruct* vTexParam) {
    if (vTexParam) {
        vTexParam->format = texture->glformat;
        vTexParam->internalFormat = texture->glinternalformat;
        vTexParam->dataType = texture->gldatatype;

        vTexParam->useMipMap = texture->useMipMap;
        vTexParam->maxMipMapLvl = texture->maxMipMapLvl;
        vTexParam->wrapS = texture->glWrapS;
        vTexParam->wrapT = texture->glWrapT;
        vTexParam->minFilter = texture->glMinFilter;
        vTexParam->magFilter = texture->glMagFilter;
    }
}

std::shared_ptr<FloatBuffer> FrameBufferAttachment::GetFloatBuffer() {
    return CreateFloatBuffer(puWindow);
}
