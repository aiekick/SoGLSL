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

#include "Texture3D.h"
#include <ctools/Logger.h>
#include <Profiler/TracyProfiler.h>
#include <stb/stb_image.h>

Texture3DPtr Texture3D::createFromFile(const char* vFilePathName, std::string vSourceFile, bool vGenMipMap, std::string vWrap, std::string vFilter) {
    auto res = std::make_shared<Texture3D>();

    if (!res->InitFromFile(vFilePathName, vSourceFile, vGenMipMap, vWrap, vFilter)) {
        res.reset();
    }

    return res;
}

Texture3DPtr Texture3D::createComputeVolume(std::string vFormat, ct::ivec3 vVolumeSize, bool vGenMipMap, std::string vWrap, std::string vFilter) {
    auto res = std::make_shared<Texture3D>();

    if (!res->InitComputeVolume(vFormat, vVolumeSize, vGenMipMap, vWrap, vFilter)) {
        res.reset();
    }

    return res;
}

Texture3DPtr
Texture3D::createFromBuffer(int vCountChannels, float* vBuffer, size_t vCount, ct::ivec3 vVolumeSize, bool vGenMipMap, std::string vWrap, std::string vFilter) {
    auto res = std::make_shared<Texture3D>();

    if (!res->InitFromBuffer(vCountChannels, vBuffer, vCount, vVolumeSize, vGenMipMap, vWrap, vFilter)) {
        res.reset();
    }

    return res;
}

Texture3D::Texture3D() : PingPong() {
}

Texture3D::~Texture3D() {
    clean();
}

bool Texture3D::InitFromFile(const char* vFilePathName, std::string vSourceFile, bool vGenMipMap, std::string vWrap, std::string vFilter) {
    auto res = false;

    puBackTex = PrepareFromFile(vFilePathName, vSourceFile, vGenMipMap, vWrap, vFilter);
    if (puBackTex) {
        res = true;
    }

    return res;
}

bool Texture3D::InitFromBuffer(int vCountChannels, float* vBuffer, size_t vCount, ct::ivec3 vVolumeSize, bool vGenMipMap, std::string vWrap, std::string vFilter) {
    auto res = false;

    puBackTex = PrepareFromBuffer(vCountChannels, vBuffer, vCount, vVolumeSize, vGenMipMap, vWrap, vFilter);
    if (puBackTex) {
        res = true;
    }

    return res;
}

bool Texture3D::InitComputeVolume(std::string vFormat, ct::ivec3 vVolumeSize, bool vGenMipMap, std::string vWrap, std::string vFilter) {
    auto res = false;

    puBackTex = PrepareComputeVolume(vFormat, vVolumeSize, vGenMipMap, vWrap, vFilter);
    puFrontTex = PrepareComputeVolume(vFormat, vVolumeSize, vGenMipMap, vWrap, vFilter);
    if (puBackTex && puFrontTex) {
        res = true;
    }

    return res;
}

ctTexturePtr Texture3D::PrepareFromFile(const char* vFilePathName, std::string vType, bool vGenMipMap, std::string vWrap, std::string vFilter) {
    ctTexturePtr res = nullptr;

    TracyGpuZone("Texture3D::PrepareFromFile");

    // puGenMipMap = vGenMipMap;
    // puWrap = vWrap;
    // puFilter = vFilter;

    uint8_t* buffer = nullptr;

    if (vType == "shadertoy") {
        buffer = GetBufferFromFile_ShaderToy(vFilePathName, res);
    }

    if (buffer && res) {
        glGenTextures(1, &res->glTex);
        // LogVar("texture id = " + ct::toStr(puTexId));
        LogGlError();

        res->useMipMap = vGenMipMap;

        res->glTextureType = GL_TEXTURE_3D;

        glBindTexture(GL_TEXTURE_3D, res->glTex);
        LogGlError();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        if (vWrap == "repeat") {
            res->glWrapS = GL_REPEAT;
            res->glWrapT = GL_REPEAT;
            res->glWrapR = GL_REPEAT;

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

            LogGlError();
        } else if (vWrap == "mirror") {
            res->glWrapS = GL_MIRRORED_REPEAT;
            res->glWrapT = GL_MIRRORED_REPEAT;
            res->glWrapR = GL_MIRRORED_REPEAT;

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

            LogGlError();
        } else if (vWrap == "clamp") {
            res->glWrapS = GL_CLAMP_TO_EDGE;
            res->glWrapT = GL_CLAMP_TO_EDGE;
            res->glWrapR = GL_CLAMP_TO_EDGE;

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            LogGlError();
        } else {
            res->glWrapS = GL_CLAMP_TO_EDGE;
            res->glWrapT = GL_CLAMP_TO_EDGE;
            res->glWrapR = GL_CLAMP_TO_EDGE;

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        }

        if (vFilter == "linear") {
            if (vGenMipMap) {
                res->glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
                res->glMagFilter = GL_LINEAR;

                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            } else {
                res->glMinFilter = GL_LINEAR;
                res->glMagFilter = GL_LINEAR;

                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
        } else if (vFilter == "nearest") {
            res->glMinFilter = GL_NEAREST;
            res->glMagFilter = GL_NEAREST;

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            res->glMinFilter = GL_LINEAR;
            res->glMagFilter = GL_LINEAR;

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        LogGlError();

        if (res->glformat != 0 && res->glinternalformat != 0 && res->gldatatype != 0 && res->w > 0 && res->h > 0 && res->d > 0) {
            // LogVar("puTex from File Size = " + ct::toStr(puRealSize.x) + "," + ct::toStr(puRealSize.y));
            glTexImage3D(GL_TEXTURE_3D, 0, res->glinternalformat, (GLsizei)res->w, (GLsizei)res->h, (GLsizei)res->d, 0, res->glformat, res->gldatatype, buffer);

            LogGlError();
        }

        if (vGenMipMap) {
            glGenerateMipmap(GL_TEXTURE_3D);
        }

        LogGlError();

        SAFE_DELETE_ARRAY(buffer);
    } else {
        LogVarError("Failed to load texture");
    }

    return res;
}

ctTexturePtr
Texture3D::PrepareFromBuffer(int vCountChannels, float* vBuffer, size_t vCount, ct::ivec3 vVolumeSize, bool vGenMipMap, std::string vWrap, std::string vFilter) {
    TracyGpuZone("Texture3D::PrepareFromBuffer");

    if (vBuffer == nullptr || vCount == 0 || vCountChannels == 0)
        return nullptr;

    if (vVolumeSize.emptyOR())
        return nullptr;

    const size_t verifSize = vVolumeSize.x * vVolumeSize.y * vVolumeSize.z * vCountChannels;
    if (verifSize != vCount)
        return nullptr;

    auto res = std::make_shared<ct::texture>();

    res->useMipMap = vGenMipMap;
    res->glTextureType = GL_TEXTURE_3D;
    res->gldatatype = GL_FLOAT;

    if (vCountChannels == 1) {
        res->glformat = GL_RED;
        res->glinternalformat = GL_R32F;
    } else if (vCountChannels == 2) {
        res->glformat = GL_RG;
        res->glinternalformat = GL_RG32F;
    } else if (vCountChannels == 3) {
        res->glformat = GL_RGB;
        res->glinternalformat = GL_RGB32F;
    } else if (vCountChannels == 4) {
        res->glformat = GL_RGBA;
        res->glinternalformat = GL_RGBA32F;
    }

    // puGenMipMap = vGenMipMap;
    // puWrap = vWrap;
    // puFilter = vFilter;
    // puSize = vVolumeSize;

    res->w = vVolumeSize.x;
    res->h = vVolumeSize.y;
    res->d = vVolumeSize.z;

    glGenTextures(1, &res->glTex);
    // LogVar("texture id = " + ct::toStr(puTexId));
    LogGlError();

    glBindTexture(GL_TEXTURE_3D, res->glTex);
    LogGlError();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (vWrap == "repeat") {
        res->glWrapS = GL_REPEAT;
        res->glWrapT = GL_REPEAT;
        res->glWrapR = GL_REPEAT;

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, res->glWrapS);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, res->glWrapT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, res->glWrapR);

        LogGlError();
    } else if (vWrap == "mirror") {
        res->glWrapS = GL_MIRRORED_REPEAT;
        res->glWrapT = GL_MIRRORED_REPEAT;
        res->glWrapR = GL_MIRRORED_REPEAT;

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, res->glWrapS);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, res->glWrapT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, res->glWrapR);

        LogGlError();
    } else if (vWrap == "clamp") {
        res->glWrapS = GL_CLAMP_TO_EDGE;
        res->glWrapT = GL_CLAMP_TO_EDGE;
        res->glWrapR = GL_CLAMP_TO_EDGE;

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, res->glWrapS);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, res->glWrapT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, res->glWrapR);

        LogGlError();
    } else {
        res->glWrapS = GL_CLAMP_TO_EDGE;
        res->glWrapT = GL_CLAMP_TO_EDGE;
        res->glWrapR = GL_CLAMP_TO_EDGE;

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, res->glWrapS);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, res->glWrapT);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, res->glWrapR);
    }

    if (vFilter == "linear") {
        if (vGenMipMap) {
            res->glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
            res->glMagFilter = GL_LINEAR;

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
        } else {
            res->glMinFilter = GL_LINEAR;
            res->glMagFilter = GL_LINEAR;

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
        }
    } else if (vFilter == "nearest") {
        if (vGenMipMap) {
            res->glMinFilter = GL_NEAREST_MIPMAP_NEAREST;
            res->glMagFilter = GL_NEAREST;

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
        } else {
            res->glMinFilter = GL_NEAREST;
            res->glMagFilter = GL_NEAREST;

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
        }
    } else {
        res->glMinFilter = GL_LINEAR;
        res->glMagFilter = GL_LINEAR;

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
    }

    LogGlError();

    // LogVar("puTex from File Size = " + ct::toStr(puRealSize.x) + "," + ct::toStr(puRealSize.y));
    glTexImage3D(GL_TEXTURE_3D, 0, res->glinternalformat, (GLsizei)res->w, (GLsizei)res->h, (GLsizei)res->d, 0, res->glformat, res->gldatatype, vBuffer);

    LogGlError();

    if (vGenMipMap) {
        glGenerateMipmap(GL_TEXTURE_3D);
        LogGlError();
    }

    return res;
}

ctTexturePtr Texture3D::PrepareComputeVolume(std::string vFormat, ct::ivec3 vVolumeSize, bool vGenMipMap, std::string vWrap, std::string vFilter) {
    ctTexturePtr res = nullptr;

    TracyGpuZone("Texture3D::PrepareComputeVolume");

    if (!vVolumeSize.emptyAND()) {
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

        if (vFormat == "r32f") {
            res = std::make_shared<ct::texture>();

            countChannels = 1;
            res->gldatatype = GL_FLOAT;
            res->glformat = GL_RED;
            res->glinternalformat = GL_R32F;
        } else if (vFormat == "r16f") {
            res = std::make_shared<ct::texture>();

            countChannels = 1;
            res->gldatatype = GL_FLOAT;
            res->glformat = GL_RED;
            res->glinternalformat = GL_R16F;
        } else if (vFormat == "rg32f") {
            res = std::make_shared<ct::texture>();

            countChannels = 2;
            res->gldatatype = GL_FLOAT;
            res->glformat = GL_RG;
            res->glinternalformat = GL_RG32F;
        } else if (vFormat == "rg16f") {
            res = std::make_shared<ct::texture>();

            countChannels = 2;
            res->gldatatype = GL_FLOAT;
            res->glformat = GL_RG;
            res->glinternalformat = GL_RG16F;
        } else if (vFormat == "rgba32f") {
            res = std::make_shared<ct::texture>();

            countChannels = 4;
            res->gldatatype = GL_FLOAT;
            res->glformat = GL_RGBA;
            res->glinternalformat = GL_RGBA32F;
        } else if (vFormat == "rgba16f") {
            res = std::make_shared<ct::texture>();

            countChannels = 4;
            res->gldatatype = GL_FLOAT;
            res->glformat = GL_RGBA;
            res->glinternalformat = GL_RGBA16F;
        }

        if (res && countChannels) {
            const size_t verifSize = vVolumeSize.x * vVolumeSize.y * vVolumeSize.z * countChannels;
            if (verifSize > 0) {
                // puGenMipMap = vGenMipMap;
                // puWrap = vWrap;
                // puFilter = vFilter;
                // puSize = vVolumeSize;

                res->w = vVolumeSize.x;
                res->h = vVolumeSize.y;
                res->d = vVolumeSize.z;

                res->useMipMap = vGenMipMap;
                res->glTextureType = GL_TEXTURE_3D;

                glGenTextures(1, &res->glTex);
                // LogVar("texture id = " + ct::toStr(puTexId));
                LogGlError();

                glBindTexture(GL_TEXTURE_3D, res->glTex);
                LogGlError();

                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                if (vWrap == "repeat") {
                    res->glWrapS = GL_REPEAT;
                    res->glWrapT = GL_REPEAT;
                    res->glWrapR = GL_REPEAT;

                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, res->glWrapS);
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, res->glWrapT);
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, res->glWrapR);

                    LogGlError();
                } else if (vWrap == "mirror") {
                    res->glWrapS = GL_MIRRORED_REPEAT;
                    res->glWrapT = GL_MIRRORED_REPEAT;
                    res->glWrapR = GL_MIRRORED_REPEAT;

                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, res->glWrapS);
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, res->glWrapT);
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, res->glWrapR);

                    LogGlError();
                } else if (vWrap == "clamp") {
                    res->glWrapS = GL_CLAMP_TO_EDGE;
                    res->glWrapT = GL_CLAMP_TO_EDGE;
                    res->glWrapR = GL_CLAMP_TO_EDGE;

                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, res->glWrapS);
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, res->glWrapT);
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, res->glWrapR);

                    LogGlError();
                } else {
                    res->glWrapS = GL_CLAMP_TO_EDGE;
                    res->glWrapT = GL_CLAMP_TO_EDGE;
                    res->glWrapR = GL_CLAMP_TO_EDGE;

                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, res->glWrapS);
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, res->glWrapT);
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, res->glWrapR);
                }

                if (vFilter == "linear") {
                    if (vGenMipMap) {
                        res->glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
                        res->glMagFilter = GL_LINEAR;

                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
                    } else {
                        res->glMinFilter = GL_LINEAR;
                        res->glMagFilter = GL_LINEAR;

                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
                    }
                } else if (vFilter == "nearest") {
                    if (vGenMipMap) {
                        res->glMinFilter = GL_NEAREST_MIPMAP_NEAREST;
                        res->glMagFilter = GL_NEAREST;

                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
                    } else {
                        res->glMinFilter = GL_NEAREST;
                        res->glMagFilter = GL_NEAREST;

                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
                        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
                    }
                } else {
                    res->glMinFilter = GL_LINEAR;
                    res->glMagFilter = GL_LINEAR;

                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);
                }

                LogGlError();

                // LogVar("puTex from File Size = " + ct::toStr(puRealSize.x) + "," + ct::toStr(puRealSize.y));
                glTexImage3D(GL_TEXTURE_3D, 0, res->glinternalformat, (GLsizei)res->w, (GLsizei)res->h, (GLsizei)res->d, 0, res->glformat, res->gldatatype, nullptr);

                LogGlError();

                if (vGenMipMap) {
                    glGenerateMipmap(GL_TEXTURE_3D);
                    LogGlError();
                }
            } else {
                res.reset();
            }
        }
    }

    return res;
}

////////////////////////////////////////////////////

uint8_t* Texture3D::GetBufferFromFile_ShaderToy(std::string vFile, ctTexturePtr vTexture) {
    /* pour le format venant de shadertoy
    var signature = file.ReadUInt32();
    texture.mImage.mXres = file.ReadUInt32();
    texture.mImage.mYres = file.ReadUInt32();
    texture.mImage.mZres = file.ReadUInt32();
    var binNumChannels = file.ReadUInt8();
    var binLayout = file.ReadUInt8();
    var binFormat = file.ReadUInt16();
    var format = renderer.TEXFMT.C1I8;
    if (binNumChannels == 1 && binFormat == 0)  format = renderer.TEXFMT.C1I8;
    else if (binNumChannels == 2 && binFormat == 0)  format = renderer.TEXFMT.C2I8;
    else if (binNumChannels == 3 && binFormat == 0)  format = renderer.TEXFMT.C3I8;
    else if (binNumChannels == 4 && binFormat == 0)  format = renderer.TEXFMT.C4I8;
    else if (binNumChannels == 1 && binFormat == 10) format = renderer.TEXFMT.C1F32;
    else if (binNumChannels == 2 && binFormat == 10) format = renderer.TEXFMT.C2F32;
    else if (binNumChannels == 3 && binFormat == 10) format = renderer.TEXFMT.C3F32;
    else if (binNumChannels == 4 && binFormat == 10) format = renderer.TEXFMT.C4F32;
    else return;
    var buffer = new Uint8Array(data, 20); // skip 16 bytes (header of .bin)
    */

    if (!vFile.empty()) {
        FILE* file = nullptr;
#ifdef MSVC
        const auto lastError = fopen_s(&file, vFile.c_str(), "rb");
        if (lastError == 0)
#else
        file = fopen(vFile.c_str(), "rb");
        if (file != NULL)
#endif
        {
            vTexture = std::make_shared<ct::texture>();

            // Header 16 bytes
            // 4 bytes
            uint32_t signature = 0;
            fread(&signature, sizeof(uint32_t), 1, file);
            // 12 bytes
            uint32_t size[3];
            fread(size, sizeof(uint32_t), 3, file);
            vTexture->w = (size_t)size[0];
            vTexture->h = (size_t)size[1];
            vTexture->d = (size_t)size[2];

            // 1 byte
            uint8_t binNumChannels = 0;  // 1 => r / 2 => rg / 3 => rgb / 4 => rgba
            fread(&binNumChannels, sizeof(uint8_t), 1, file);
            // 1 bytes
            uint8_t binLayout = 0;
            fread(&binLayout, sizeof(uint8_t), 1, file);
            // 2 bytes
            uint16_t binFormat = 0;  // 0 => bytes / 10 => float32
            fread(&binFormat, sizeof(uint16_t), 1, file);

            const auto endHeaderPos = ftell(file);

            fseek(file, 0, SEEK_END);
            const size_t fileSize = ftell(file);

            fseek(file, endHeaderPos, SEEK_SET);

            // datas reste of the file
            size_t bufferSize = 0;
            if (binFormat == 0) {
                vTexture->gldatatype = GL_UNSIGNED_BYTE;
                if (binNumChannels == 1) {
                    vTexture->glinternalformat = GL_R8;
                    vTexture->glformat = GL_RED;
                    bufferSize = size[0] * size[1] * size[2] * sizeof(uint8_t) * 1;
                } else if (binNumChannels == 2) {
                    vTexture->glinternalformat = GL_RG8;
                    vTexture->glformat = GL_RG;
                    bufferSize = size[0] * size[1] * size[2] * sizeof(uint8_t) * 2;
                } else if (binNumChannels == 3) {
                    vTexture->glinternalformat = GL_RGB8;
                    vTexture->glformat = GL_RGB;
                    bufferSize = size[0] * size[1] * size[2] * sizeof(uint8_t) * 3;
                } else if (binNumChannels == 4) {
                    vTexture->glinternalformat = GL_RGBA8;
                    vTexture->glformat = GL_RGBA;
                    bufferSize = size[0] * size[1] * size[2] * sizeof(uint8_t) * 4;
                }
            } else if (binFormat == 10) {
                vTexture->gldatatype = GL_FLOAT;
                if (binNumChannels == 1) {
                    vTexture->glinternalformat = GL_R32F;
                    vTexture->glformat = GL_RED;
                    bufferSize = size[0] * size[1] * size[2] * sizeof(float) * 1;
                } else if (binNumChannels == 2) {
                    vTexture->glinternalformat = GL_RG32F;
                    vTexture->glformat = GL_RG;
                    bufferSize = size[0] * size[1] * size[2] * sizeof(float) * 2;
                } else if (binNumChannels == 3) {
                    vTexture->glinternalformat = GL_RGB32F;
                    vTexture->glformat = GL_RGB;
                    bufferSize = size[0] * size[1] * size[2] * sizeof(float) * 3;
                } else if (binNumChannels == 4) {
                    vTexture->glinternalformat = GL_RGBA32F;
                    vTexture->glformat = GL_RGBA;
                    bufferSize = size[0] * size[1] * size[2] * sizeof(float) * 4;
                }
            }

            uint8_t* buffer = nullptr;

            if (bufferSize > 0) {
                const auto realBufferSize = fileSize - endHeaderPos;

                buffer = new uint8_t[bufferSize];
                fread(buffer, sizeof(uint8_t), realBufferSize, file);
            }

            fclose(file);

            return buffer;
        }
    }

    return nullptr;
}

uint8_t* Texture3D::GetBufferFromFile_MagicaVoxel_Vox(std::string vFile, ctTexturePtr /*vTexture*/) {
    if (!vFile.empty()) {
    }

    return nullptr;
}
