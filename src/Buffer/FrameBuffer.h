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

#include <Headers/RenderPackHeaders.h>

#include <glad/glad.h>
#include <ctools/cTools.h>

#include <string>
#include <vector>
#include <memory>

class FrameBufferAttachment;
class FloatBuffer;
struct TextureParamsStruct;
class FrameBuffer {
private:
    GLuint puFBOId = 0;
    ct::ivec3 puSize;
    std::string puDataType;
    bool puUseFXAA = false;
    int puCountFXAASamples = 0;
    bool puUseZBuffer = false;
    bool puUseRenderBuffer = false;
    bool puUseFloatBuffer = false;
    bool puLoaded = false;
    int puCountTextures = 0;
    int puCountRenderBuffers = 0;
    GLenum* puColorDrawBuffers = nullptr;
    GLenum* puRenderDrawBuffers = nullptr;
    std::vector<std::shared_ptr<FrameBufferAttachment>> puAttachmentsToLoad;  // la totalita des attachments
    std::vector<std::shared_ptr<FrameBufferAttachment>> puAttachments;        // la totalita des attachments
    std::vector<std::shared_ptr<FrameBufferAttachment>> puTextures1D;         // tout puAttachments de type texture2d sans les renderbuffers
    std::vector<std::shared_ptr<FrameBufferAttachment>> puTextures2D;         // tout puAttachments de type texture2d sans les renderbuffers
    std::shared_ptr<FrameBufferAttachment> puDepthTexture2D = nullptr;        // une seule depth texture par fbo possible
    std::shared_ptr<FrameBufferAttachment> puTextures3D = nullptr;            // une seule texture 3d par fbo possible
    std::vector<std::shared_ptr<FrameBufferAttachment>> puRenderBuffers;      // tout les buffers sans les textures
    GuiBackend_Window puWindow;

public:
    TextureParamsStruct puTexParams;

public:
    GLuint getFboID() {
        return puFBOId;
    }
    ct::ivec3 getSize() {
        return puSize;
    }
    int getCountTextures1D() {
        return (int)puTextures1D.size();
    }
    int getCountTextures2D() {
        return (int)puTextures2D.size();
    }
    int getCountRenderBuffers() {
        return (int)puRenderBuffers.size();
    }
    int getCountAttachements() {
        return (int)puAttachments.size();
    }
    std::vector<std::shared_ptr<FrameBufferAttachment>>* GetAttachments() {
        return &puAttachments;
    }

public:
    static FrameBufferPtr Create(const GuiBackend_Window& vWin,
                                 const ct::ivec3& vSize,
                                 const std::string& vDataType = "float",
                                 const bool& vUseFXAA = false,
                                 const int& vCountFXAASamples = 2,
                                 const bool& vUseZBuffer = false,
                                 const int& vCountTextures = 1,
                                 const bool& vUseRenderBuffer = false,
                                 const bool& vUseFloatBuffer = false);
    static void SetFormatFromString(const std::string& vDataTypeString, GLenum* vInternalFormat, GLenum* VFormat, GLenum* vDataType);

public:
    std::string GetParamsToXMLString();

public:
    FrameBuffer(const GuiBackend_Window& vWin,
                const ct::ivec3& vSize,
                const std::string& vDataType,
                const bool& vUseFXAA,
                const int& vCountFXAASamples,
                const bool& vUseZBuffer,
                const int& vCountTextures,
                const bool& vUseRenderBuffer,
                const bool& vUseFloatBuffer);

    ~FrameBuffer();

    bool AddColorAttachment(TextureParamsStruct* vTexParam = nullptr);
    bool RemoveColorAttachment(const size_t& vId);
    void DestroyFrameBuffer();
    void UpdateFrameBuffer();
    void SetZBufferUse(const bool& vUseZBuffer);
    void SetFXAAUse(const bool& vUseFXAA, const int& vCountFXAASamples);
    void ClearBuffer(const ct::fColor& vColor);
    bool IsOK();
    bool load();
    bool bind();
    void Select3DLayer(const int& vLayer);
    void SelectBuffersTarget();
    void unbind();
    bool Resize(const ct::ivec2& vNewSize);
    bool Resize(const ct::ivec3& vNewSize);

    // new version for attahcment id
    std::shared_ptr<FloatBuffer> GetFloatBufferFromColorAttachment_4_Chan(const int& vAttachmentId,
                                                                          const bool& vReadFBO,
                                                                          const int& vMipMapLvl = 0,
                                                                          const bool& vCreateNewBuffer = false);
    std::shared_ptr<FloatBuffer> GetFloatBufferFromColorAttachment_1_Chan(const int& vAttachmentId,
                                                                          const bool& vReadFBO,
                                                                          const int& vMipMapLvl = 0,
                                                                          const bool& vCreateNewBuffer = false);

    uint8_t* GetRGBBytesFromFrameBuffer(int* vWidth, int* vHeight, int* vBufSize, const int& vAttachmentId);
    void FillRGBBytesBufferWithFrameBuffer(const int& vWidth, const int& vHeight, uint8_t* vBytesBuffer, const int& vAttachmentId);
    void GetRGBValueAtPos(const ct::ivec2& vPos, const int& vAttachmentId = 0, ct::fvec3* vValue = nullptr);

    uint8_t* GetRGBABytesFromFrameBuffer(int* vWidth, int* vHeight, int* vBufSize, const int& vAttachmentId);
    void FillRGBABytesBufferWithFrameBuffer(const int& vWidth, const int& vHeight, uint8_t* vBytesBuffer, const int& vAttachmentId);
    void GetRGBAValueAtPos(const ct::ivec2& vPos, const int& vAttachmentId = 0, ct::fvec4* vValue = nullptr);
    GLfloat GetOneChannelValueAtPos(const ct::ivec2& vPos, const int& vChannel = 0, const int& vAttachmentId = 0);  // vChannel => R/G/B/A = 0/1/2/3

    GLuint getTextureID(const size_t& i = 0);
    ctTexturePtr getTexture(const size_t& i = 0);
    GLuint getRenderBufferID(const size_t& i = 0);
    GLuint getDepthTextureID();
    ctTexturePtr getDepthTexture();

    bool SaveToPng(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::ivec2& vNewSize, const int& vAttachmentId);
    bool SaveToBmp(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::ivec2& vNewSize, const int& vAttachmentId);
    bool SaveToJpg(const std::string& vFilePathName,
                   const bool& vFlipY,
                   const int& vSubSamplesCount,
                   const int& vQualityFrom0To100,
                   const ct::ivec2& vNewSize,
                   const int& vAttachmentId);
    bool SaveToHdr(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::ivec2& vNewSize, const int& vAttachmentId);
    bool SaveToTga(const std::string& vFilePathName, const bool& vFlipY, const int& vSubSamplesCount, const ct::ivec2& vNewSize, const int& vAttachmentId);

    void UpdateMipMaping(const std::string& vName);

    void ChangeTexParameters(TextureParamsStruct* vTexParam);
    void GetTexParameters(TextureParamsStruct* vTexParam);
    bool UpdateTexParameters(const std::string& vName, TextureParamsStruct* vTexParam);
};
