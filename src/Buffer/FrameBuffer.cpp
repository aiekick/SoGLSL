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

#include "FrameBuffer.h"
#include "FrameBufferAttachment.h"

#include <cstring>  // memcpy

#include <ctools/Logger.h>
#include <Profiler/TracyProfiler.h>

// #define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
// #define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#define USE_GET_TEX_IMAGE

FrameBufferPtr FrameBuffer::Create(const GuiBackend_Window& vWin,
                                   const ct::ivec3& vSize,
                                   const std::string& vDataType,
                                   const bool& vUseFXAA,
                                   const int& vCountFXAASamples,
                                   const bool& vUseZBuffer,
                                   const int& vCountTextures,
                                   const bool& vUseRenderBuffer,
                                   const bool& vUseFloatBuffer) {
    FrameBufferPtr fbo = nullptr;

    if (!vSize.emptyAND() && vCountTextures > 0) {
        fbo = std::make_shared<FrameBuffer>(vWin, vSize, vDataType, vUseFXAA, vCountFXAASamples, vUseZBuffer, vCountTextures, vUseRenderBuffer, vUseFloatBuffer);
    }

    return fbo;
}

///////////////////////////////////////////////////

std::string FrameBuffer::GetParamsToXMLString() {
    std::string code;
    /*code += " SIZE=\"" + puSize.string() + "\"";
    code += " ZBUFFER=\"" + ct::fvariant(puUseZBuffer).GetS() + "\"";
    code += " ATTACHMENTS=\"" + ct::toStr(puCountAttachments) + "\"";
    code += " INTERNAL=\"" + ct::toStr(puInternalFormat == GL_RGBA32F ? "RGBA32F" : "RGBA") + "\"";
    code += " FORMAT=\"" + ct::toStr(puFormat == GL_RGBA ? "RGBA" : "RGB") + "\"";
    code += " DATATYPE=\"" + ct::toStr(puTexDataType == GL_FLOAT ? "FLOAT" : "uint8_t") + "\"";*/
    return code;
}

///////////////////////////////////////////////////

void FrameBuffer::SetFormatFromString(const std::string& vDataTypeString, GLenum* vInternalFormat, GLenum* VFormat, GLenum* vDataType) {
    if (vDataTypeString == "float") {
        if (vInternalFormat)
            *vInternalFormat = GL_RGBA32F;
        if (VFormat)
            *VFormat = GL_RGBA;
        if (vDataType)
            *vDataType = GL_FLOAT;
    } else if (vDataTypeString == "byte") {
        if (vInternalFormat)
            *vInternalFormat = GL_RGBA;
        if (VFormat)
            *VFormat = GL_RGBA;
        if (vDataType)
            *vDataType = GL_UNSIGNED_BYTE;
    }
}

FrameBuffer::FrameBuffer(const GuiBackend_Window& vWin,
                         const ct::ivec3& vSize,
                         const std::string& vDataType,
                         const bool& vUseFXAA,
                         const int& vCountFXAASamples,
                         const bool& vUseZBuffer,
                         const int& vCountTextures,
                         const bool& vUseRenderBuffer,
                         const bool& vUseFloatBuffer)
    : puWindow(vWin) {
    TracyGpuZone("FrameBuffer::FrameBuffer");

    puFBOId = 0;
    puDataType = vDataType;
    puUseFXAA = vUseFXAA;
    puCountFXAASamples = vCountFXAASamples;
    puLoaded = false;
    puCountRenderBuffers = 0;
    puColorDrawBuffers = nullptr;
    puRenderDrawBuffers = nullptr;

    puTextures3D = nullptr;
    puSize = vSize;
    puCountTextures = vCountTextures;
    puUseZBuffer = vUseZBuffer;
    puUseRenderBuffer = vUseRenderBuffer;
    puUseFloatBuffer = vUseFloatBuffer;

    for (int i = 0; i < vCountTextures; i++) {
        auto at = FrameBufferAttachment::Create(vWin);
        SetFormatFromString(vDataType, &at->texture->glinternalformat, &at->texture->glformat, &at->texture->gldatatype);
        at->size = puSize;
        at->puUseFloatBuffer = puUseFloatBuffer;
        at->puUseFXAA = puUseFXAA;
        at->puCountFXAASamples = puCountFXAASamples;
        puSize.z = 0;

        //	at->type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_3D_LAYERED;
        // else
        at->type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D;
        puAttachmentsToLoad.emplace_back(at);

        if (vUseRenderBuffer) {
            at = FrameBufferAttachment::Create(vWin);
            SetFormatFromString(vDataType, &at->texture->glinternalformat, &at->texture->glformat, &at->texture->gldatatype);
            at->size = puSize;
            at->puUseFXAA = puUseFXAA;
            at->puCountFXAASamples = puCountFXAASamples;
            at->type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER;
            puAttachmentsToLoad.emplace_back(at);
        }
    }

    if (puUseZBuffer) {
        // depth render buffer
        auto at = FrameBufferAttachment::Create(vWin);
        at->size = puSize;
        at->puUseFXAA = puUseFXAA;
        at->puCountFXAASamples = puCountFXAASamples;
        at->type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_RENDER_BUFFER;
        puAttachmentsToLoad.emplace_back(at);

        // depth texture
        /*auto at = FrameBufferAttachment::Create(vWin);
        at->size = puSize;
        at->puUseFXAA = puUseFXAA;
        at->puCountFXAASamples = puCountFXAASamples;
        at->type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_TEXTURE;
        puAttachmentsToLoad.emplace_back(at);*/
    }
}

///////////////////////////////////////////////////

FrameBuffer::~FrameBuffer() {
    DestroyFrameBuffer();

    SAFE_DELETE_ARRAY(puColorDrawBuffers);
}

void FrameBuffer::DestroyFrameBuffer() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBuffer::DestroyFrameBuffer");

    for (auto& att : puAttachments) {
        att.reset();
    }

    puAttachments.clear();
    puTextures1D.clear();
    puTextures2D.clear();
    puTextures3D.reset();
    puRenderBuffers.clear();
    puDepthTexture2D.reset();

    // Destruction des buffers
    glDeleteFramebuffers(1, &puFBOId);
    LogGlError();
}

///////////////////////////////////////////////////

bool FrameBuffer::AddColorAttachment(TextureParamsStruct* vTexParam) {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBuffer::AddColorAttachment");

    auto at = FrameBufferAttachment::Create(puWindow);
    at->ChangeTexParameters(vTexParam);
    at->size = puSize;
    at->puUseFXAA = puUseFXAA;
    at->puCountFXAASamples = puCountFXAASamples;
    at->puUseFloatBuffer = puUseFloatBuffer;
    at->type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D;
    at->attachmentId = (int)puTextures2D.size();

    if (vTexParam != nullptr) {
        at->ChangeTexParameters(vTexParam);
    }

    puTextures2D.emplace_back(at);
    puAttachments.emplace_back(at);
    puCountTextures = (int)puTextures2D.size();

    if (puUseRenderBuffer) {
        at = FrameBufferAttachment::Create(puWindow);
        at->ChangeTexParameters(vTexParam);
        at->size = puSize;
        at->puUseFXAA = puUseFXAA;
        at->puCountFXAASamples = puCountFXAASamples;
        at->type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER;
        at->attachmentId = (int)puTextures2D.size();
        puRenderBuffers.emplace_back(at);
        puAttachments.emplace_back(at);
        puCountRenderBuffers = (int)puRenderBuffers.size();
    }

    return true;
}

bool FrameBuffer::RemoveColorAttachment(const size_t& vId) {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBuffer::RemoveColorAttachment");

    if (puTextures2D.size() > vId) {
        if (puTextures2D[vId]) {
            size_t atId = 0;
            for (auto att : puAttachments) {
                if (att == puTextures2D[vId]) {
                    break;
                }
                atId++;
            }

            puTextures2D[vId].reset();
            puTextures2D.erase(puTextures2D.begin() + vId);

            if (!puAttachments.empty())
                puAttachments.erase(puAttachments.begin() + atId);

            puCountTextures = (int)puTextures2D.size();
        }
    }

    return true;
}

void FrameBuffer::UpdateFrameBuffer() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBuffer::UpdateFrameBuffer");

    glFinish();

    // Verrouillage du Frame Buffer
    glBindFramebuffer(GL_FRAMEBUFFER, puFBOId);
    LogGlError();

    for (auto it = puAttachments.begin(); it != puAttachments.end(); ++it) {
        (*it)->ReLoad();
        (*it)->AttachToFbo(puFBOId);
    }

    SAFE_DELETE(puColorDrawBuffers);
    puColorDrawBuffers = new GLenum[puTextures2D.size()];
    for (size_t i = 0; i < puTextures2D.size(); i++) {
        puColorDrawBuffers[i] = GL_COLOR_ATTACHMENT0 + (GLenum)i;

        /*FrameBufferAttachment *at = puTextures2D[i];
        if (at != 0)
        {
            if (at->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D)
                LogVar("puColorDrawBuffers[" + ct::toStr(i) + "] = GL_COLOR_ATTACHMENT" + ct::toStr(i));
        }*/
    }

    // D�verrouillage du Frame Buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    LogGlError();

    // V�rification de l'int�grit� du FBO
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LogGlError();
    }
}

///////////////////////////////////////////////////

void FrameBuffer::ClearBuffer(const ct::fColor& vColor)  // ca marche pas ca, need un mesh
{
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBuffer::ClearBuffer");

    glFinish();

    glBindFramebuffer(GL_FRAMEBUFFER, puFBOId);
    LogGlError();

    glClearColor(vColor.r, vColor.g, vColor.b, vColor.a);  // couleur d'effacement du fond
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    LogGlError();

    for (auto at : puAttachments) {
        at->ClearAttachment();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    LogGlError();
}

void FrameBuffer::SetZBufferUse(const bool& vUseZBuffer) {
    if (!puUseZBuffer && vUseZBuffer) {
        puUseZBuffer = true;

        auto at = FrameBufferAttachment::Create(puWindow);
        at->size = puSize;
        at->puUseFXAA = puUseFXAA;
        at->puCountFXAASamples = puCountFXAASamples;
        at->type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_RENDER_BUFFER;
        puAttachmentsToLoad.emplace_back(at);

        /*auto at = FrameBufferAttachment::Create(puWindow);
        at->size = puSize;
        at->puUseFXAA = puUseFXAA;
        at->puCountFXAASamples = puCountFXAASamples;
        at->type = FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_TEXTURE;
        puAttachmentsToLoad.emplace_back(at);*/
    }
}

void FrameBuffer::SetFXAAUse(const bool& vUseFXAA, const int& vCountFXAASamples) {
    if (puUseFXAA != vUseFXAA) {
        puUseFXAA = vUseFXAA;
        puCountFXAASamples = vCountFXAASamples;

        // il va falloir recreer les attahcment de type color tecture2d avec le FXAA set � vUseFXAA
        for (auto it = puAttachments.begin(); it != puAttachments.end(); ++it) {
            if ((*it)->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D ||
                (*it)->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER ||
                (*it)->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_RENDER_BUFFER ||
                (*it)->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_TEXTURE) {
                (*it)->puUseFXAA = puUseFXAA;
                (*it)->puCountFXAASamples = puCountFXAASamples;
            }
        }
    }
}

bool FrameBuffer::IsOK() {
    if (!puLoaded)
        return false;

    return puLoaded;
}

///////////////////////////////////////////////////

bool FrameBuffer::load() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBuffer::load");

    bool res = true;

    // V�rification d'un �ventuel ancien FBO
    if (glIsFramebuffer(puFBOId) == GL_TRUE) {
        LogGlError();
        DestroyFrameBuffer();
    }

    glFinish();

    // G�n�ration d'un id
    glGenFramebuffers(1, &puFBOId);
    LogGlError();

    // Verrouillage du Frame Buffer
    glBindFramebuffer(GL_FRAMEBUFFER, puFBOId);
    LogGlError();

    int AttachId = 0;
    for (auto it = puAttachmentsToLoad.begin(); it != puAttachmentsToLoad.end(); ++it) {
        res &= (*it)->Load(puUseFloatBuffer);
        puAttachments.emplace_back(*it);
        if ((*it)->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D) {
            (*it)->attachmentId = AttachId;
            puTextures2D.emplace_back(*it);
            AttachId++;
        } else if ((*it)->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER ||
                   (*it)->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_RENDER_BUFFER) {
            (*it)->attachmentId = AttachId;
            puRenderBuffers.emplace_back(*it);
            AttachId++;
        } else if ((*it)->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_DEPTH_TEXTURE) {
            (*it)->attachmentId = AttachId;
            puDepthTexture2D = *it;
            AttachId++;
        }
        (*it)->AttachToFbo(puFBOId);
    }
    puAttachmentsToLoad.clear();

    puColorDrawBuffers = new GLenum[puTextures2D.size()];
    for (size_t i = 0; i < puTextures2D.size(); i++) {
        puColorDrawBuffers[i] = GL_COLOR_ATTACHMENT0 + (GLenum)i;

        /*FrameBufferAttachment *at = puTextures2D[i];
        if (at != 0)
        {
            if (at->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D)
                LogVar("puColorDrawBuffers[" + ct::toStr(i) + "] = GL_COLOR_ATTACHMENT" + ct::toStr(i));
        }*/
    }

    if (glIsFramebuffer(puFBOId) != GL_TRUE)
        LogVarError("FBO %u is bad", puFBOId);

    // V�rification de l'int�grit� du FBO
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LogGlError();

        DestroyFrameBuffer();

        // Affichage d'un message d'erreur et retour de la valeur false
        LogVarError("Erreur : le FBO est mal construit");

        // D�verrouillage du Frame Buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        LogGlError();

        return false;
    }

    // D�verrouillage du Frame Buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    LogGlError();

    puLoaded = true;

    return IsOK();
}

///////////////////////////////////////////////////

bool FrameBuffer::bind() {
    if (IsOK()) {
        GuiBackend::MakeContextCurrent(puWindow);

        TracyGpuZone("FrameBuffer::bind");

        glBindFramebuffer(GL_FRAMEBUFFER, puFBOId);
        LogGlError();

        // Redimensionnement de la zone d'affichage
        glViewport(0, 0, (GLsizei)puSize.x, (GLsizei)puSize.y);
        LogGlError();

        return true;
    }
    return false;
}

void FrameBuffer::Select3DLayer(const int& vLayer) {
    if (puTextures3D) {
        GuiBackend::MakeContextCurrent(puWindow);

        TracyGpuZone("FrameBuffer::Select3DLayer");

        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, puTextures3D->texture->glTex, 0, vLayer);
        LogGlError();
    }
}

void FrameBuffer::SelectBuffersTarget() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBuffer::SelectBuffersTarget");

    glDrawBuffers(puCountTextures, puColorDrawBuffers);
    if (LogGlError()) {
        if (glIsFramebuffer(puFBOId) == GL_TRUE) {
            LogVarDebugError("bad FBO %u", puFBOId);
        }
    }
}

void FrameBuffer::unbind() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBuffer::unbind");

    // screen buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    LogGlError();
}

bool FrameBuffer::Resize(const ct::ivec2& vNewSize) {
    bool res = true;

    if (vNewSize.x > 0 && vNewSize.y > 0) {
        GuiBackend::MakeContextCurrent(puWindow);

        TracyGpuZone("FrameBuffer::Resize");

        puSize = ct::ivec3(vNewSize, 0);

        glFinish();

        // Verrouillage du Frame Buffer
        glBindFramebuffer(GL_FRAMEBUFFER, puFBOId);
        LogGlError();

        for (auto it = puAttachments.begin(); it != puAttachments.end(); ++it) {
            res &= (*it)->ReSize(vNewSize);
            (*it)->AttachToFbo(puFBOId);
        }

        if (glIsFramebuffer(puFBOId) != GL_TRUE)
            LogVarError("FBO %u is bad", puFBOId);
        LogGlError();

        // V�rification de l'int�grit� du FBO
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LogGlError();

            DestroyFrameBuffer();

            // Affichage d'un message d'erreur et retour de la valeur false
            LogVarError("Erreur : le FBO est mal construit");

            return false;
        }

        // D�verrouillage du Frame Buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        LogGlError();
    }

    return res;
}

bool FrameBuffer::Resize(const ct::ivec3& vNewSize) {
    bool res = true;

    if (vNewSize.x > 0 && vNewSize.y > 0) {
        GuiBackend::MakeContextCurrent(puWindow);

        TracyGpuZone("FrameBuffer::Resize");

        puSize = vNewSize;

        glFinish();

        // Verrouillage du Frame Buffer
        glBindFramebuffer(GL_FRAMEBUFFER, puFBOId);
        LogGlError();

        for (auto it = puAttachments.begin(); it != puAttachments.end(); ++it) {
            res &= (*it)->ReSize(vNewSize);
            (*it)->AttachToFbo(puFBOId);
        }

        if (glIsFramebuffer(puFBOId) != GL_TRUE)
            LogVarError("FBO %u is bad", puFBOId);
        LogGlError();

        // V�rification de l'int�grit� du FBO
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LogGlError();

            DestroyFrameBuffer();

            // Affichage d'un message d'erreur et retour de la valeur false
            LogVarError("Erreur : le FBO est mal construit");

            return false;
        }

        // D�verrouillage du Frame Buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        LogGlError();
    }

    if (!res) {
        LogVarError("Fail to Resize FBO to Resolution %i,%i,%i", vNewSize.x, vNewSize.y, vNewSize.z);
    }

    return res;
}

///////////////////////////////////////////////////

std::shared_ptr<FloatBuffer> FrameBuffer::GetFloatBufferFromColorAttachment_4_Chan(const int& vAttachmentId,
                                                                                   const bool& vReadFBO,
                                                                                   const int& vMipMapLvl,
                                                                                   const bool& vCreateNewBuffer) {
    std::shared_ptr<FloatBuffer> fBuffer = nullptr;

    TracyGpuZone("FrameBuffer::GetFloatBufferFromColorAttachment_4_Chan");

    if (puTextures2D.size() > (size_t)vAttachmentId) {
        auto at = puTextures2D[vAttachmentId];
        if (at) {
            if (vCreateNewBuffer) {
                fBuffer = at->GetFloatBuffer();
            } else {
                fBuffer = at->puFrontBuffer;
                at->SwitchBuffers();
            }

            if (fBuffer != nullptr) {
                if (vReadFBO) {
                    GuiBackend::MakeContextCurrent(puWindow);

                    if (fBuffer->usePBO) {
                        bind();

                        /*glBindBuffer(GL_PIXEL_PACK_BUFFER, fBuffer->pbo);
                        ptr = (unsigned char*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
                        if (NULL != ptr) {
                            memcpy(pixels, ptr, nbytes);
                            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                        }
                        glReadPixels(0, 0, fBuffer->w, fBuffer->h, GL_RGBA, GL_FLOAT, 0);*/

                        unbind();
                    } else {
#ifdef USE_GET_TEX_IMAGE
                        auto mipmaplvl = vMipMapLvl;
                        GLenum texType = GL_TEXTURE_2D;
                        if (puUseFXAA) {
                            texType = GL_TEXTURE_2D_MULTISAMPLE;
                            mipmaplvl = 0;
                        }

                        glBindTexture(texType, at->texture->glTex);
                        LogGlError();
                        glGetTexImage(texType, mipmaplvl, GL_RGBA, GL_FLOAT, fBuffer->buf);
                        LogGlError();
                        glBindTexture(texType, 0);
                        LogGlError();
#else
                        bind();
                        glReadBuffer(GL_COLOR_ATTACHMENT0 + vAttachmentId);
                        LogGlError();
                        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // from disk to opengl
                        LogGlError();
                        glPixelStorei(GL_PACK_ALIGNMENT, 1);  // from opengl to disk
                        LogGlError();
                        glReadPixels(0, 0, fBuffer->w, fBuffer->h, GL_RGBA, GL_FLOAT, fBuffer->buf);
                        LogGlError();
                        unbind();
#endif
                    }
                }
            } else {
                LogVarError("at->puFrontBuffer is empty...");
            }
        }
    } else {
        LogVarError("puTextures2D.size() <= %i", vAttachmentId);
    }

    return fBuffer;
}

std::shared_ptr<FloatBuffer> FrameBuffer::GetFloatBufferFromColorAttachment_1_Chan(const int& vAttachmentId,
                                                                                   const bool& vReadFBO,
                                                                                   const int& vMipMapLvl,
                                                                                   const bool& vCreateNewBuffer) {
    std::shared_ptr<FloatBuffer> fBuffer = nullptr;

    TracyGpuZone("FrameBuffer::GetFloatBufferFromColorAttachment_1_Chan");

    if (puTextures2D.size() > (size_t)vAttachmentId) {
        auto at = puTextures2D[vAttachmentId];
        if (at) {
            if (vCreateNewBuffer) {
                fBuffer = at->GetFloatBuffer();
            } else {
                fBuffer = at->puFrontBuffer;
                at->SwitchBuffers();
            }

            if (fBuffer != nullptr && vReadFBO) {
                GuiBackend::MakeContextCurrent(puWindow);

#ifdef USE_GET_TEX_IMAGE
                auto mipmaplvl = vMipMapLvl;
                GLenum texType = GL_TEXTURE_2D;
                if (puUseFXAA) {
                    texType = GL_TEXTURE_2D_MULTISAMPLE;
                    mipmaplvl = 0;
                }

                glBindTexture(texType, at->texture->glTex);
                LogGlError();
                glGetTexImage(texType, mipmaplvl, GL_RED, GL_FLOAT, fBuffer->buf);
                LogGlError();
                glBindTexture(texType, 0);
                LogGlError();
#else
                bind();
                glReadBuffer(GL_COLOR_ATTACHMENT0 + vAttachmentId);
                LogGlError();
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // from disk to opengl
                LogGlError();
                glPixelStorei(GL_PACK_ALIGNMENT, 1);  // from opengl to disk
                LogGlError();
                glReadPixels(0, 0, fBuffer->w, fBuffer->h, GL_RED, GL_FLOAT, fBuffer->buf);
                LogGlError();
                unbind();
#endif
            }
        }
    }

    return fBuffer;
}

uint8_t* FrameBuffer::GetRGBBytesFromFrameBuffer(int* vWidth, int* vHeight, int* vBufSize, const int& vAttachmentId) {
    if (vWidth && vHeight && vBufSize) {
        GuiBackend::MakeContextCurrent(puWindow);

        TracyGpuZone("FrameBuffer::GetRGBBytesFromFrameBuffer");

        *vWidth = puSize.x;
        *vHeight = puSize.y;
        *vBufSize = 3 * (*vWidth) * (*vHeight);  // 3 car RGB

        uint8_t* bmBytes = new uint8_t[*vBufSize];

#ifdef USE_GET_TEX_IMAGE
        auto at = puTextures2D[vAttachmentId];
        if (at) {
            GLenum texType = GL_TEXTURE_2D;
            if (puUseFXAA)
                texType = GL_TEXTURE_2D_MULTISAMPLE;

            glBindTexture(texType, at->texture->glTex);
            LogGlError();
            glGetTexImage(texType, 0, GL_RGB, GL_UNSIGNED_BYTE, bmBytes);
            LogGlError();
            glBindTexture(texType, 0);
            LogGlError();
        }
#else
        bind();
        glReadBuffer(GL_COLOR_ATTACHMENT0 + vAttachmentId);
        LogGlError();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // from disk to opengl
        LogGlError();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);  // from opengl to disk
        LogGlError();
        glReadPixels(0, 0, *vWidth, *vHeight, GL_RGB, GL_UNSIGNED_BYTE, bmBytes);
        LogGlError();
        unbind();
#endif

        return (uint8_t*)bmBytes;
    }
    return nullptr;
}

void FrameBuffer::FillRGBBytesBufferWithFrameBuffer(const int& vWidth, const int& vHeight, uint8_t* vBytesBuffer, const int& vAttachmentId) {
    if (vBytesBuffer) {
        GuiBackend::MakeContextCurrent(puWindow);

        TracyGpuZone("FrameBuffer::FillRGBBytesBufferWithFrameBuffer");

#ifdef USE_GET_TEX_IMAGE
        UNUSED(vWidth);
        UNUSED(vHeight);
        auto at = puTextures2D[vAttachmentId];
        if (at) {
            GLenum texType = GL_TEXTURE_2D;
            if (puUseFXAA)
                texType = GL_TEXTURE_2D_MULTISAMPLE;

            glBindTexture(texType, at->texture->glTex);
            LogGlError();
            glGetTexImage(texType, 0, GL_RGB, GL_UNSIGNED_BYTE, vBytesBuffer);
            LogGlError();
            glBindTexture(texType, 0);
            LogGlError();
        }
#else
        bind();
        glReadBuffer(GL_COLOR_ATTACHMENT0 + vAttachmentId);
        LogGlError();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // from disk to opengl
        LogGlError();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);  // from opengl to disk
        LogGlError();
        glReadPixels(0, 0, vWidth, vHeight, GL_RGB, GL_UNSIGNED_BYTE, vBytesBuffer);
        LogGlError();
        unbind();
#endif
    }
}

void FrameBuffer::GetRGBValueAtPos(const ct::ivec2& vPos, const int& vAttachmentId, ct::fvec3* vValue) {
    if (vValue) {
        GuiBackend::MakeContextCurrent(puWindow);

        TracyGpuZone("FrameBuffer::GetRGBValueAtPos");

        bind();

        glReadBuffer(GL_COLOR_ATTACHMENT0 + vAttachmentId);
        LogGlError();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // from disk to opengl
        LogGlError();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);  // from opengl to disk
        LogGlError();
        glReadPixels(vPos.x, vPos.y, 1, 1, GL_RGB, GL_FLOAT, vValue);
        LogGlError();

        unbind();
    }
}

uint8_t* FrameBuffer::GetRGBABytesFromFrameBuffer(int* vWidth, int* vHeight, int* vBufSize, const int& vAttachmentId) {
    if (vWidth && vHeight && vBufSize) {
        GuiBackend::MakeContextCurrent(puWindow);

        TracyGpuZone("FrameBuffer::GetRGBABytesFromFrameBuffer");

        *vWidth = puSize.x;
        *vHeight = puSize.y;
        *vBufSize = 4 * (*vWidth) * (*vHeight);  // 4 car RGBA

        uint8_t* bmBytes = new uint8_t[*vBufSize];

#ifdef USE_GET_TEX_IMAGE
        auto at = puTextures2D[vAttachmentId];
        if (at) {
            GLenum texType = GL_TEXTURE_2D;
            if (puUseFXAA)
                texType = GL_TEXTURE_2D_MULTISAMPLE;

            glBindTexture(texType, at->texture->glTex);
            LogGlError();
            glGetTexImage(texType, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmBytes);
            LogGlError();
            glBindTexture(texType, 0);
            LogGlError();
        }
#else
        bind();
        glReadBuffer(GL_COLOR_ATTACHMENT0 + vAttachmentId);
        LogGlError();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // from disk to opengl
        LogGlError();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);  // from opengl to disk
        LogGlError();
        glReadPixels(0, 0, *vWidth, *vHeight, GL_RGBA, GL_UNSIGNED_BYTE, bmBytes);
        LogGlError();
        unbind();
#endif
        return (uint8_t*)bmBytes;
    }

    return nullptr;
}

void FrameBuffer::FillRGBABytesBufferWithFrameBuffer(const int& vWidth, const int& vHeight, uint8_t* vBytesBuffer, const int& vAttachmentId) {
    if (vBytesBuffer) {
        GuiBackend::MakeContextCurrent(puWindow);

        TracyGpuZone("FrameBuffer::FillRGBABytesBufferWithFrameBuffer");

#ifdef USE_GET_TEX_IMAGE
        UNUSED(vWidth);
        UNUSED(vHeight);
        auto at = puTextures2D[vAttachmentId];
        if (at) {
            glBindTexture(GL_TEXTURE_2D, at->texture->glTex);
            LogGlError();
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, vBytesBuffer);
            LogGlError();
            glBindTexture(GL_TEXTURE_2D, 0);
            LogGlError();
        }
#else
        bind();
        glReadBuffer(GL_COLOR_ATTACHMENT0 + vAttachmentId);
        LogGlError();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // from disk to opengl
        LogGlError();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);  // from opengl to disk
        LogGlError();
        glReadPixels(0, 0, vWidth, vHeight, GL_RGBA, GL_UNSIGNED_BYTE, vBytesBuffer);
        LogGlError();
        unbind();
#endif
    }
}

void FrameBuffer::GetRGBAValueAtPos(const ct::ivec2& vPos, const int& vAttachmentId, ct::fvec4* vValue) {
    if (vValue) {
        GuiBackend::MakeContextCurrent(puWindow);

        TracyGpuZone("FrameBuffer::GetRGBAValueAtPos");

        bind();

        glReadBuffer(GL_COLOR_ATTACHMENT0 + vAttachmentId);
        LogGlError();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // from disk to opengl
        LogGlError();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);  // from opengl to disk
        LogGlError();
        glReadPixels(vPos.x, vPos.y, 1, 1, GL_RGBA, GL_FLOAT, vValue);
        LogGlError();

        unbind();
    }
}

GLfloat FrameBuffer::GetOneChannelValueAtPos(const ct::ivec2& vPos, const int& vChannel, const int& vAttachmentId) {
    if (vChannel < 0 || vChannel > 3) {
        LogVarError("vChannel must be in range [0-3] but here is : %i", vChannel);
        return 0.0f;
    }

    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("FrameBuffer::GetOneChannelValueAtPos");

    bind();

    GLfloat value;

    glReadBuffer(GL_COLOR_ATTACHMENT0 + vAttachmentId);
    LogGlError();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // from disk to opengl
    LogGlError();
    glPixelStorei(GL_PACK_ALIGNMENT, 1);  // from opengl to disk
    LogGlError();
    glReadPixels(vPos.x, vPos.y, 1, 1, GL_RED + vChannel, GL_FLOAT, &value);
    LogGlError();

    unbind();

    return value;
}

GLuint FrameBuffer::getTextureID(const size_t& i) {
    if (i >= 0 && i < puTextures2D.size())
        return puTextures2D[i]->texture->glTex;
    return 0;
}

ctTexturePtr FrameBuffer::getTexture(const size_t& i) {
    if (i >= 0 && i < puTextures2D.size())
        return puTextures2D[i]->texture;
    return nullptr;
}

GLuint FrameBuffer::getRenderBufferID(const size_t& i) {
    if (i >= 0 && i < puRenderBuffers.size())
        return puRenderBuffers[i]->colorBufferId;
    return 0;
}

GLuint FrameBuffer::getDepthTextureID() {
    if (puDepthTexture2D)
        return puDepthTexture2D->colorBufferId;
    return 0;
}

ctTexturePtr FrameBuffer::getDepthTexture() {
    if (puDepthTexture2D)
        return puDepthTexture2D->texture;
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// SAVE TO PICTURE FILES (PBG, BMP, TGA, HDR) SO STB EXPORT FILES //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FrameBuffer::SaveToPng(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::ivec2& vNewSize, const int& vAttachmentId) {
    bool res = false;

    int width;
    int height;
    int bufSize;
    uint8_t* bmBytesRGBA = GetRGBABytesFromFrameBuffer(&width, &height, &bufSize, vAttachmentId);

    const int bytesPerPixel = 4;

    const int ss = vSubSamplesCount;

    // Sub Sampling
    if (ss > 0) {
        uint8_t* tempdata = new uint8_t[bufSize];
        memcpy(tempdata, bmBytesRGBA, bufSize);

        const unsigned indexMaxSize = width * height;
        for (unsigned int index = 0; index < indexMaxSize; ++index) {
            int count = 0;
            int r = 0, g = 0, b = 0, a = 0;

            for (int i = -ss; i <= ss; i += ss) {
                for (int j = -ss; j <= ss; j += ss) {
                    const int ssIndex = index + width * j + i;
                    if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize) {
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
    if ((vNewSize.x != width) || (vNewSize.y != height)) {
        // resize
        const int newWidth = vNewSize.x;
        const int newHeight = vNewSize.y;
        const unsigned int newBufSize = newWidth * newHeight * bytesPerPixel;
        uint8_t* resizedData = new uint8_t[newBufSize];

        const uint8_t* resizeRes = stbir_resize_uint8_linear(
            bmBytesRGBA, width, height, width * bytesPerPixel, resizedData, newWidth, newHeight, newWidth * bytesPerPixel, (stbir_pixel_layout)bytesPerPixel);

        if (resizeRes) {
            if (vFlipY)
                stbi_flip_vertically_on_write(1);
            const int resWrite = stbi_write_png(vFilePathName.c_str(), newWidth, newHeight, bytesPerPixel, resizedData, newWidth * bytesPerPixel);

            if (resWrite)
                res = true;
        }

        SAFE_DELETE_ARRAY(resizedData);
    } else {
        if (vFlipY)
            stbi_flip_vertically_on_write(1);
        const int resWrite = stbi_write_png(vFilePathName.c_str(), width, height, bytesPerPixel, bmBytesRGBA, width * bytesPerPixel);

        if (resWrite)
            res = true;
    }

    SAFE_DELETE_ARRAY(bmBytesRGBA);

    LogVarInfo("Png picture file saved : %s", vFilePathName.c_str());

    return res;
}

bool FrameBuffer::SaveToBmp(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::ivec2& vNewSize, const int& vAttachmentId) {
    bool res = false;

    int width;
    int height;
    int bufSize;
    uint8_t* bmBytesRGB = GetRGBBytesFromFrameBuffer(&width, &height, &bufSize, vAttachmentId);

    const int bytesPerPixel = 3;

    const int ss = vSubSamplesCount;

    // Sub Sampling
    if (ss > 0) {
        uint8_t* tempdata = new uint8_t[bufSize];
        memcpy(tempdata, bmBytesRGB, bufSize);

        const unsigned indexMaxSize = width * height;
        for (unsigned int index = 0; index < indexMaxSize; ++index) {
            int count = 0;
            int r = 0, g = 0, b = 0;  // , a = 0;

            for (int i = -ss; i <= ss; i += ss) {
                for (int j = -ss; j <= ss; j += ss) {
                    const int ssIndex = index + width * j + i;
                    if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize) {
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
    if ((vNewSize.x != width) || (vNewSize.y != height)) {
        // resize
        const int newWidth = vNewSize.x;
        const int newHeight = vNewSize.y;
        const unsigned int newBufSize = newWidth * newHeight * bytesPerPixel;
        uint8_t* resizedData = new uint8_t[newBufSize];

        const uint8_t* resizeRes = stbir_resize_uint8_linear(  //
            bmBytesRGB,
            width,
            height,
            width * bytesPerPixel,  //
            resizedData,
            newWidth,
            newHeight,
            newWidth * bytesPerPixel,            //
            (stbir_pixel_layout)bytesPerPixel);  //

        if (resizeRes) {
            if (vFlipY)
                stbi_flip_vertically_on_write(1);
            const int resWrite = stbi_write_bmp(vFilePathName.c_str(), newWidth, newHeight, bytesPerPixel, resizedData);

            if (resWrite)
                res = true;
        }

        SAFE_DELETE_ARRAY(resizedData);
    } else {
        if (vFlipY)
            stbi_flip_vertically_on_write(1);
        const int resWrite = stbi_write_bmp(vFilePathName.c_str(), width, height, bytesPerPixel, bmBytesRGB);

        if (resWrite)
            res = true;
    }

    SAFE_DELETE_ARRAY(bmBytesRGB);

    LogVarInfo("bmp picture file saved : %s", vFilePathName.c_str());

    return res;
}

bool FrameBuffer::SaveToJpg(const std::string& vFilePathName,
                            const bool& vFlipY,
                            const int& vSubSamplesCount,
                            const int& vQualityFrom0To100,
                            const ct::ivec2& vNewSize,
                            const int& vAttachmentId) {
    bool res = false;

    int width;
    int height;
    int bufSize;
    const int bytesPerPixel = 3;
    uint8_t* bmBytesRGB = GetRGBBytesFromFrameBuffer(&width, &height, &bufSize, vAttachmentId);

    const int ss = vSubSamplesCount;

    // Sub Sampling
    if (ss > 0) {
        uint8_t* tempdata = new uint8_t[bufSize];
        memcpy(tempdata, bmBytesRGB, bufSize);

        const unsigned indexMaxSize = width * height;
        for (unsigned int index = 0; index < indexMaxSize; ++index) {
            int count = 0;
            int r = 0, g = 0, b = 0;  // , a = 0;

            for (int i = -ss; i <= ss; i += ss) {
                for (int j = -ss; j <= ss; j += ss) {
                    const int ssIndex = index + width * j + i;
                    if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize) {
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
    if ((vNewSize.x != width) || (vNewSize.y != height)) {
        // resize
        const int newWidth = vNewSize.x;
        const int newHeight = vNewSize.y;
        const unsigned int newBufSize = newWidth * newHeight * bytesPerPixel;
        uint8_t* resizedData = new uint8_t[newBufSize];

        const uint8_t* resizeRes = stbir_resize_uint8_linear(  //
            bmBytesRGB,
            width,
            height,
            width * bytesPerPixel,  //
            resizedData,
            newWidth,
            newHeight,
            newWidth * bytesPerPixel,            //
            (stbir_pixel_layout)bytesPerPixel);  //

        if (resizeRes) {
            if (vFlipY)
                stbi_flip_vertically_on_write(1);
            const int resWrite = stbi_write_jpg(vFilePathName.c_str(), newWidth, newHeight, bytesPerPixel, resizedData, vQualityFrom0To100);

            if (resWrite)
                res = true;
        }

        SAFE_DELETE_ARRAY(resizedData);
    } else {
        if (vFlipY)
            stbi_flip_vertically_on_write(1);
        const int resWrite = stbi_write_jpg(vFilePathName.c_str(), width, height, bytesPerPixel, bmBytesRGB, vQualityFrom0To100);

        if (resWrite)
            res = true;
    }

    SAFE_DELETE_ARRAY(bmBytesRGB);

    LogVarInfo("Jpg picture file saved : %s", vFilePathName.c_str());

    return res;
}

bool FrameBuffer::SaveToHdr(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::ivec2& vNewSize, const int& vAttachmentId) {
    bool res = false;

    // on force la creation d'un nouveau FloatBuffer, donc il faudra qu'on le detruise nous meme
    std::shared_ptr<FloatBuffer> bmFloatRGBA = GetFloatBufferFromColorAttachment_4_Chan(vAttachmentId, true, 0, true);
    if (bmFloatRGBA) {
        const int width = bmFloatRGBA->w;
        const int height = bmFloatRGBA->h;
        const int bytesPerPixel = bmFloatRGBA->bytesPerPixel;
        // int bufSize = (int)bmFloatRGBA->size;

        const int ss = vSubSamplesCount;

        // Sub Sampling
        if (ss > 0) {
            std::shared_ptr<FloatBuffer> tempdata = GetFloatBufferFromColorAttachment_4_Chan(vAttachmentId, true, 0, true);
            if (tempdata) {
                const unsigned indexMaxSize = width * height;
                for (unsigned int index = 0; index < indexMaxSize; ++index) {
                    float count = 0;
                    float r = 0, g = 0, b = 0, a = 0;

                    for (int i = -ss; i <= ss; i += ss) {
                        for (int j = -ss; j <= ss; j += ss) {
                            const int ssIndex = index + width * j + i;
                            if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize) {
                                r += tempdata->buf[ssIndex * bytesPerPixel + 0];
                                g += tempdata->buf[ssIndex * bytesPerPixel + 1];
                                b += tempdata->buf[ssIndex * bytesPerPixel + 2];
                                if (bytesPerPixel > 3)
                                    a += tempdata->buf[ssIndex * bytesPerPixel + 3];

                                count++;
                            }
                        }
                    }

                    bmFloatRGBA->buf[index * bytesPerPixel + 0] = r / count;
                    bmFloatRGBA->buf[index * bytesPerPixel + 1] = g / count;
                    bmFloatRGBA->buf[index * bytesPerPixel + 2] = b / count;
                    if (bytesPerPixel > 3)
                        bmFloatRGBA->buf[index * bytesPerPixel + 3] = a / count;
                }

                tempdata.reset();
            }
        }

        // resize
        if ((vNewSize.x != width) || (vNewSize.y != height)) {
            // resize
            const int newWidth = vNewSize.x;
            const int newHeight = vNewSize.y;
            const unsigned int newBufSize = newWidth * newHeight * bytesPerPixel;
            float* resizedData = new float[newBufSize];

            const float* resizeRes = stbir_resize_float_linear(  //
                bmFloatRGBA->buf,
                width,
                height,
                width * bytesPerPixel,  //
                resizedData,
                newWidth,
                newHeight,
                newWidth * bytesPerPixel,            //
                (stbir_pixel_layout)bytesPerPixel);  //

            if (resizeRes) {
                if (vFlipY)
                    stbi_flip_vertically_on_write(1);
                const int resWrite = stbi_write_hdr(vFilePathName.c_str(), newWidth, newHeight, bytesPerPixel, resizedData);

                if (resWrite)
                    res = true;
            }

            SAFE_DELETE_ARRAY(resizedData);
        } else {
            if (vFlipY)
                stbi_flip_vertically_on_write(1);
            const int resWrite = stbi_write_hdr(vFilePathName.c_str(), width, height, bytesPerPixel, bmFloatRGBA->buf);

            if (resWrite)
                res = true;
        }

        bmFloatRGBA.reset();
    }

    LogVarInfo("Hdr picture file saved : %s", vFilePathName.c_str());

    return res;
}

bool FrameBuffer::SaveToTga(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::ivec2& vNewSize, const int& vAttachmentId) {
    bool res = false;

    int width;
    int height;
    int bufSize;
    uint8_t* bmBytesRGBA = GetRGBABytesFromFrameBuffer(&width, &height, &bufSize, vAttachmentId);

    const int bytesPerPixel = 4;

    const int ss = vSubSamplesCount;

    // Sub Sampling
    if (ss > 0) {
        uint8_t* tempdata = new uint8_t[bufSize];
        memcpy(tempdata, bmBytesRGBA, bufSize);

        const unsigned indexMaxSize = width * height;
        for (unsigned int index = 0; index < indexMaxSize; ++index) {
            int count = 0;
            int r = 0, g = 0, b = 0, a = 0;

            for (int i = -ss; i <= ss; i += ss) {
                for (int j = -ss; j <= ss; j += ss) {
                    const int ssIndex = index + width * j + i;
                    if (ssIndex > 0 && (unsigned int)ssIndex < indexMaxSize) {
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
    if ((vNewSize.x != width) || (vNewSize.y != height)) {
        // resize
        const int newWidth = vNewSize.x;
        const int newHeight = vNewSize.y;
        const unsigned int newBufSize = newWidth * newHeight * bytesPerPixel;
        uint8_t* resizedData = new uint8_t[newBufSize];

        const uint8_t* resizeRes = stbir_resize_uint8_linear(  //
            bmBytesRGBA,
            width,
            height,
            width * bytesPerPixel,  //
            resizedData,
            newWidth,
            newHeight,
            newWidth * bytesPerPixel,            //
            (stbir_pixel_layout)bytesPerPixel);  //

        if (resizeRes) {
            if (vFlipY)
                stbi_flip_vertically_on_write(1);
            const int resWrite = stbi_write_tga(vFilePathName.c_str(), newWidth, newHeight, bytesPerPixel, resizedData);

            if (resWrite)
                res = true;
        }

        SAFE_DELETE_ARRAY(resizedData);
    } else {
        if (vFlipY)
            stbi_flip_vertically_on_write(1);
        const int resWrite = stbi_write_tga(vFilePathName.c_str(), width, height, bytesPerPixel, bmBytesRGBA);

        if (resWrite)
            res = true;
    }

    SAFE_DELETE_ARRAY(bmBytesRGBA);

    LogVarInfo("Tga picture file saved : %s", vFilePathName.c_str());

    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameBuffer::UpdateMipMaping(const std::string& vName) {
    if (puLoaded) {
        for (auto at : puAttachments) {
            if (at) {
                at->UpdateMipMaping(vName);
            }
        }
    }
}

void FrameBuffer::ChangeTexParameters(TextureParamsStruct* vTexParam) {
    if (vTexParam) {
        puTexParams = *vTexParam;

        if (!puLoaded)  // avant l'appel de load()
        {
            for (auto it = puAttachmentsToLoad.begin(); it != puAttachmentsToLoad.end(); ++it) {
                auto at = *it;
                if (at) {
                    if (at->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER ||
                        at->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D) {
                        at->ChangeTexParameters(vTexParam);
                    }
                }
            }
        } else {
            for (auto it = puAttachments.begin(); it != puAttachments.end(); ++it) {
                auto at = *it;
                if (at) {
                    if (at->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER ||
                        at->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D) {
                        at->ChangeTexParameters(vTexParam);
                    }
                }
            }
        }
    }
}

void FrameBuffer::GetTexParameters(TextureParamsStruct* vTexParam) {
    if (vTexParam) {
        if (!puTextures2D.empty()) {
            puTextures2D[0]->GetTexParameters(vTexParam);
        } else {
            *vTexParam = puTexParams;
        }
    }
}

bool FrameBuffer::UpdateTexParameters(const std::string& vName, TextureParamsStruct* vTexParam) {
    bool res = true;

    if (vTexParam) {
        GuiBackend::MakeContextCurrent(puWindow);

        TracyGpuZone("FrameBuffer::UpdateTexParameters");

        glFinish();

        // Verrouillage du Frame Buffer
        glBindFramebuffer(GL_FRAMEBUFFER, puFBOId);
        LogGlError();

        // int AttachId = 0;
        for (auto it = puAttachments.begin(); it != puAttachments.end(); ++it) {
            if ((*it)->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_RENDER_BUFFER ||
                (*it)->type == FRAMEBUFFER_ATTACHMENT_TYPE_ENUM::FRAMEBUFFER_ATTACHMENT_TYPE_COLOR_TEXTURE_2D) {
                const bool needReload = (*it)->ChangeTexParameters(vTexParam);
                if (needReload) {
                    res &= (*it)->ReLoad();
                    (*it)->AttachToFbo(puFBOId);
                }
            }
        }

        if (glIsFramebuffer(puFBOId) != GL_TRUE)
            LogVarError("FBO %u is bad", puFBOId);
        LogGlError();

        // V�rification de l'int�grit� du FBO
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LogGlError();

            DestroyFrameBuffer();

            // Affichage d'un message d'erreur et retour de la valeur false
            LogVarError("Erreur : le FBO est mal construit");

            return false;
        }

        // D�verrouillage du Frame Buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        LogGlError();

        puTexParams = *vTexParam;

        if (vTexParam->useMipMap) {
            UpdateMipMaping(vName);
        }
    }

    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////