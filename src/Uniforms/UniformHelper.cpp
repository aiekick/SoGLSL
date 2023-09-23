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

#include "UniformHelper.h"

#include <ctools/cTools.h>
#include <Uniforms/UniformVariant.h>
#include <Buffer/FrameBuffer.h>
#include <Buffer/FrameBuffersPipeLine.h>
#include <CodeTree/ShaderKey.h>
#include <ctools/FileHelper.h>
#include <Systems/GizmoSystem.h>
#include <Systems/GamePadSystem.h>
#include <Systems/MidiSystem.h>
#include <Systems/SoundSystem.h>
#include <Texture/Texture2D.h>
#include <Texture/Texture3D.h>
#include <Texture/TextureSound.h>
#include <ctools/Logger.h>
#include <Profiler/TracyProfiler.h>
#include <ImGuiPack.h>
#include <VR/Backend/VRBackend.h>

ct::fvec2 UniformHelper::FBOSizeForMouseUniformNormalization = 0.0f;
ct::fvec2 UniformHelper::FBOSize = 0.0f;

int UniformHelper::UploadUniformForGlslType(const GuiBackend_Window& vWin, UniformVariantPtr vUniform, int vTextureSlotId, bool vIsCompute) {
    GuiBackend::MakeContextCurrent(vWin);

    TracyGpuZone("UniformHelper::UploadUniformForGlslType");

    static Texture2DPtr empty_texture_ptr = Texture2D::createEmpty();

    if (vUniform) {
        if (vUniform->pipe) {  // size uniforms
            switch (vUniform->glslType) {
                case uType::uTypeEnum::U_VEC2:
                case uType::uTypeEnum::U_VEC3:
                case uType::uTypeEnum::U_IVEC2:
                case uType::uTypeEnum::U_IVEC3:
                case uType::uTypeEnum::U_UVEC2:
                case uType::uTypeEnum::U_UVEC3:
                    if (vUniform->pipe->getBackBuffer()) {
                        vUniform->ix = vUniform->pipe->getBackBuffer()->getSize().x;
                        vUniform->iy = vUniform->pipe->getBackBuffer()->getSize().y;
                        vUniform->x = (float)vUniform->ix;
                        vUniform->y = (float)vUniform->iy;
                        vUniform->ux = (uint32_t)vUniform->ix;
                        vUniform->uy = (uint32_t)vUniform->iy;
                    } else {
                        vUniform->x = 0.0f;
                        vUniform->y = 0.0f;
                        vUniform->ix = 0;
                        vUniform->iy = 0;
                        vUniform->ux = 0U;
                        vUniform->uy = 0U;
                    }
                    vUniform->z = 0.0f;
                    vUniform->iz = 0;
                    vUniform->uz = 0U;

#ifdef USE_VR
                    if (VRBackend::Instance()->IsInRendering()) {
                        auto& size = VRBackend::Instance()->GetFBOSize();

                        vUniform->ix = size.x;
                        vUniform->iy = size.y;
                        vUniform->x = (float)vUniform->ix;
                        vUniform->y = (float)vUniform->iy;
                        vUniform->ux = (uint32_t)vUniform->ix;
                        vUniform->uy = (uint32_t)vUniform->iy;
                    }
#endif

                    break;
                default: break;
            }
        }

        // samplers
        switch (vUniform->glslType) {
            case uType::uTypeEnum::U_SAMPLER2D:
                if (vUniform->pipe != nullptr && !vUniform->uSampler2DFromThread) {
                    if (!vUniform->sound_histo_ptr) {
                        if (vUniform->pipe->getBackBuffer())
                            vUniform->uSampler2D = vUniform->pipe->getBackBuffer()->getTextureID(vUniform->attachment);
                    }
                }
                if (vIsCompute && vUniform->uImage2D > -1) {
                    glBindImageTexture(vUniform->slot, vUniform->uImage2D, 0, GL_FALSE, 0, GL_READ_WRITE, vUniform->computeTextureFormat);
                }
                if (vUniform->loc > -1) {
                    if (vUniform->texture_ptr) {
                        if (vUniform->texture_ptr->getBack()) {
                            vUniform->uSampler2D = vUniform->texture_ptr->getBack()->glTex;
                        }
                    }

                    if (vUniform->uSampler2D < 0 && empty_texture_ptr && empty_texture_ptr->getBack()) {
                        vUniform->uSampler2D = empty_texture_ptr->getBack()->glTex;
                    }

                    if (vUniform->uSampler2D > -1) {
                        glActiveTexture(GL_TEXTURE0 + vTextureSlotId);
                        glBindTexture(GL_TEXTURE_2D, vUniform->uSampler2D);
                        glUniform1i(vUniform->loc, vTextureSlotId);
                        ++vTextureSlotId;
                    }
                }
                break;
            case uType::uTypeEnum::U_SAMPLER3D:
                if (vIsCompute && vUniform->uImage3D > -1)
                    glBindImageTexture(vUniform->slot, vUniform->uImage3D, 0, GL_FALSE, 0, GL_READ_WRITE, vUniform->computeTextureFormat);
                if (vUniform->uSampler3D > -1 && vUniform->loc > -1) {
                    if (vUniform->volume_ptr)
                        if (vUniform->volume_ptr->getBack())
                            vUniform->uSampler3D = vUniform->volume_ptr->getBack()->glTex;

                    glActiveTexture(GL_TEXTURE0 + vTextureSlotId);
                    glBindTexture(GL_TEXTURE_3D, vUniform->uSampler3D);
                    glUniform1i(vUniform->loc, vTextureSlotId);
                    ++vTextureSlotId;
                }
                break;
            case uType::uTypeEnum::U_SAMPLERCUBE:
                if (vUniform->uSamplerCube > -1 && vUniform->loc > -1) {
                    glActiveTexture(GL_TEXTURE0 + vTextureSlotId);
                    glBindTexture(GL_TEXTURE_CUBE_MAP, vUniform->uSamplerCube);
                    glUniform1i(vUniform->loc, vTextureSlotId);
                    ++vTextureSlotId;
                }
                break;
            case uType::uTypeEnum::U_SAMPLER1D:
            default: break;
        }

        if (vUniform->loc > -1) {
            switch (vUniform->glslType) {
                case uType::uTypeEnum::U_FLOAT: glUniform1f(vUniform->loc, vUniform->x); break;
                case uType::uTypeEnum::U_VEC2: glUniform2f(vUniform->loc, vUniform->x, vUniform->y); break;
                case uType::uTypeEnum::U_VEC3: glUniform3f(vUniform->loc, vUniform->x, vUniform->y, vUniform->z); break;
                case uType::uTypeEnum::U_VEC4: glUniform4f(vUniform->loc, vUniform->x, vUniform->y, vUniform->z, vUniform->w); break;
                case uType::uTypeEnum::U_INT: glUniform1i(vUniform->loc, vUniform->ix); break;
                case uType::uTypeEnum::U_IVEC2: glUniform2i(vUniform->loc, vUniform->ix, vUniform->iy); break;
                case uType::uTypeEnum::U_IVEC3: glUniform3i(vUniform->loc, vUniform->ix, vUniform->iy, vUniform->iz); break;
                case uType::uTypeEnum::U_IVEC4: glUniform4i(vUniform->loc, vUniform->ix, vUniform->iy, vUniform->iz, vUniform->iw); break;
                case uType::uTypeEnum::U_UINT: glUniform1ui(vUniform->loc, vUniform->ux); break;
                case uType::uTypeEnum::U_UVEC2: glUniform2ui(vUniform->loc, vUniform->ux, vUniform->uy); break;
                case uType::uTypeEnum::U_UVEC3: glUniform3ui(vUniform->loc, vUniform->ux, vUniform->uy, vUniform->uz); break;
                case uType::uTypeEnum::U_UVEC4: glUniform4ui(vUniform->loc, vUniform->ux, vUniform->uy, vUniform->uz, vUniform->uw); break;
                case uType::uTypeEnum::U_BOOL: glUniform1i(vUniform->loc, vUniform->bx); break;
                case uType::uTypeEnum::U_BVEC2: glUniform2i(vUniform->loc, vUniform->bx, vUniform->by); break;
                case uType::uTypeEnum::U_BVEC3: glUniform3i(vUniform->loc, vUniform->bx, vUniform->by, vUniform->bz); break;
                case uType::uTypeEnum::U_BVEC4: glUniform4i(vUniform->loc, vUniform->bx, vUniform->by, vUniform->bz, vUniform->bw); break;
                default: break;
            }

            if (vUniform->uFloatArr) {
                switch (vUniform->glslType) {
                    case uType::uTypeEnum::U_MAT2: glUniformMatrix2fv(vUniform->loc, 1, GL_FALSE, (GLfloat*)(vUniform->uFloatArr)); break;
                    case uType::uTypeEnum::U_MAT3: glUniformMatrix3fv(vUniform->loc, 1, GL_FALSE, (GLfloat*)(vUniform->uFloatArr)); break;
                    case uType::uTypeEnum::U_MAT4: glUniformMatrix4fv(vUniform->loc, 1, GL_FALSE, (GLfloat*)(vUniform->uFloatArr)); break;
                    default: break;
                }
            }

            if (vUniform->count > 0) {
                switch (vUniform->glslType) {
                    case uType::uTypeEnum::U_FLOAT_ARRAY:
                        if (vUniform->uFloatArr)
                            glUniform1fv(vUniform->loc, vUniform->count, vUniform->uFloatArr);
                        break;
                    case uType::uTypeEnum::U_VEC2_ARRAY:
                        if (vUniform->uVec2Arr)
                            glUniform2fv(vUniform->loc, vUniform->count, (GLfloat*)(vUniform->uVec2Arr));
                        break;
                    case uType::uTypeEnum::U_VEC3_ARRAY:
                        if (vUniform->uVec3Arr)
                            glUniform3fv(vUniform->loc, vUniform->count, (GLfloat*)(vUniform->uVec2Arr));
                        break;
                    case uType::uTypeEnum::U_VEC4_ARRAY:
                        if (vUniform->uVec4Arr)
                            glUniform4fv(vUniform->loc, vUniform->count, (GLfloat*)(vUniform->uVec4Arr));
                        break;
                    default: break;
                }
            }
        }
    }

    return vTextureSlotId;
}

int UniformHelper::UpdateMipMap(const GuiBackend_Window& vWin, UniformVariantPtr vUniform, int vTextureSlotId, bool vForce) {
    GuiBackend::MakeContextCurrent(vWin);
    TracyGpuZone("UniformHelper::UpdateMipMap");
    if (vUniform->mipmap || vForce) {
        if (vUniform && vUniform->loc > -1) {
            switch (vUniform->glslType) {
                case uType::uTypeEnum::U_SAMPLER1D:
                    if (vUniform->uSampler1D > -1) {
                        AIGPScoped("U_SAMPLER1D", "UpdateMipMap %s", vUniform->name.c_str());
                        glBindTexture(GL_TEXTURE_1D, vUniform->uSampler1D);
                        glGenerateMipmap(GL_TEXTURE_1D);
                        glBindTexture(GL_TEXTURE_1D, 0);
                        ++vTextureSlotId;
                    }
                    break;
                case uType::uTypeEnum::U_SAMPLER2D:
                    if (vUniform->uSampler2D > -1) {
                        AIGPScoped("U_SAMPLER2D", "UpdateMipMap %s", vUniform->name.c_str());
                        glBindTexture(GL_TEXTURE_2D, vUniform->uSampler2D);
                        glGenerateMipmap(GL_TEXTURE_2D);
                        glBindTexture(GL_TEXTURE_2D, 0);
                        ++vTextureSlotId;
                    }
                    break;
                case uType::uTypeEnum::U_SAMPLERCUBE:
                    if (vUniform->uSamplerCube > -1) {
                        AIGPScoped("U_SAMPLERCUBE", "UpdateMipMap %s", vUniform->name.c_str());
                        glBindTexture(GL_TEXTURE_CUBE_MAP, vUniform->uSamplerCube);
                        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
                        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                        ++vTextureSlotId;
                    }
                    break;
                case uType::uTypeEnum::U_SAMPLER3D:
                    if (vUniform->uSampler3D > -1) {
                        AIGPScoped("U_SAMPLER3D", "UpdateMipMap %s", vUniform->name.c_str());
                        glBindTexture(GL_TEXTURE_3D, vUniform->uSampler3D);
                        glGenerateMipmap(GL_TEXTURE_3D);
                        glBindTexture(GL_TEXTURE_3D, 0);
                        ++vTextureSlotId;
                    }
                    break;
                default: break;
            }
        }
    }

    return vTextureSlotId;
}

std::string UniformHelper::SerializeUniform(UniformVariantPtr vUniform) {
    ZoneScoped;
    std::string str;
    if (vUniform != nullptr) {
        if (vUniform->canWeSave) {
            if (vUniform->widget == "time") {
                str += vUniform->name + ":" + ct::toStr(vUniform->bx ? "true" : "false") + ":" + ct::toStr(vUniform->x) + "\n";
            } else if (vUniform->widget == "checkbox") {
                str += vUniform->name + ":";
                if (vUniform->count > 0)
                    str += ct::toStr(vUniform->bx ? "true" : "false");
                if (vUniform->count > 1)
                    str += ";" + ct::toStr(vUniform->by ? "true" : "false");
                if (vUniform->count > 2)
                    str += ";" + ct::toStr(vUniform->bz ? "true" : "false");
                if (vUniform->count > 3)
                    str += ";" + ct::toStr(vUniform->bw ? "true" : "false");
                str += "\n";
            } else if (GizmoSystem::Instance()->SerializeUniform(vUniform, &str)) {
            } else if (GamePadSystem::Instance()->SerializeUniform(vUniform, &str)) {
            } else if (SoundSystem::Instance()->SerializeUniform(vUniform, &str)) {
            } else if (MidiSystem::Instance()->SerializeUniform(vUniform, &str)) {
            } else if (vUniform->widget == "picture") {
                if (!vUniform->target.empty()) {
                    str = vUniform->name + ":target:" + vUniform->target + "\n";
                } else if (vUniform->textureFileChoosebox || vUniform->textureChoiceActivated) {
                    if (vUniform->filePathNames.size() == 1) {
                        str = vUniform->name + ":choose:" + FileHelper::Instance()->GetPathRelativeToApp(vUniform->filePathNames[0]);
                        if (vUniform->textureFlipChoosebox)
                            str += ":flip:" + ct::toStr(vUniform->flip ? "true" : "false");
                        if (vUniform->textureMipmapChoosebox)
                            str += ":mipmap:" + ct::toStr(vUniform->mipmap ? "true" : "false");
                        if (vUniform->textureWrapChoosebox)
                            str += ":wrap:" + ct::toStr(vUniform->wrap);
                        if (vUniform->textureFilterChoosebox)
                            str += ":filter:" + ct::toStr(vUniform->filter);
                        str += "\n";
                    }
                }
            } else if (vUniform->widget == "mouse:2pos_2click" || vUniform->widget == "mouse:2press_2click" ||
                       vUniform->widget == "mouse:shadertoy") {
                if (vUniform->glslType == uType::uTypeEnum::U_VEC4) {
                    if (!vUniform->constant) {
                        str = vUniform->name + ":" + ct::toStr(vUniform->x / UniformHelper::FBOSizeForMouseUniformNormalization.x) + ";" +
                            ct::toStr(vUniform->y / UniformHelper::FBOSizeForMouseUniformNormalization.y) + ";" +
                            ct::toStr(vUniform->z / UniformHelper::FBOSizeForMouseUniformNormalization.x) + ";" +
                            ct::toStr(vUniform->w / UniformHelper::FBOSizeForMouseUniformNormalization.y) + ":" +
                            UniformHelper::FBOSizeForMouseUniformNormalization.string() + "\n";
                    }
                }
            } else {
                if (!vUniform->constant) {
                    switch (vUniform->glslType) {
                        case uType::uTypeEnum::U_FLOAT: str = vUniform->name + ":" + ct::toStr(vUniform->x) + "\n"; break;
                        case uType::uTypeEnum::U_VEC2:
                            str = vUniform->name + ":" + ct::toStr(vUniform->x) + ";" + ct::toStr(vUniform->y) + "\n";
                            break;
                        case uType::uTypeEnum::U_VEC3:
                            str = vUniform->name + ":" + ct::toStr(vUniform->x) + ";" + ct::toStr(vUniform->y) + ";" + ct::toStr(vUniform->z) + "\n";
                        case uType::uTypeEnum::U_VEC4:
                            str = vUniform->name + ":" + ct::toStr(vUniform->x) + ";" + ct::toStr(vUniform->y) + ";" + ct::toStr(vUniform->z) + ";" +
                                ct::toStr(vUniform->w) + "\n";
                            break;
                        case uType::uTypeEnum::U_INT: str = vUniform->name + ":" + ct::toStr(vUniform->ix) + "\n"; break;
                        case uType::uTypeEnum::U_IVEC2:
                            str = vUniform->name + ":" + ct::toStr(vUniform->ix) + ";" + ct::toStr(vUniform->iy) + "\n";
                            break;
                        case uType::uTypeEnum::U_IVEC3:
                            str =
                                vUniform->name + ":" + ct::toStr(vUniform->ix) + ";" + ct::toStr(vUniform->iy) + ";" + ct::toStr(vUniform->iz) + "\n";
                            break;
                        case uType::uTypeEnum::U_IVEC4:
                            str = vUniform->name + ":" + ct::toStr(vUniform->ix) + ";" + ct::toStr(vUniform->iy) + ";" + ct::toStr(vUniform->iz) +
                                ";" + ct::toStr(vUniform->iw) + "\n";
                            break;
                        case uType::uTypeEnum::U_UINT: str = vUniform->name + ":" + ct::toStr(vUniform->ux) + "\n"; break;
                        case uType::uTypeEnum::U_UVEC2:
                            str = vUniform->name + ":" + ct::toStr(vUniform->ux) + ";" + ct::toStr(vUniform->uy) + "\n";
                            break;
                        case uType::uTypeEnum::U_UVEC3:
                            str =
                                vUniform->name + ":" + ct::toStr(vUniform->ux) + ";" + ct::toStr(vUniform->uy) + ";" + ct::toStr(vUniform->uz) + "\n";
                            break;
                        case uType::uTypeEnum::U_UVEC4:
                            str = vUniform->name + ":" + ct::toStr(vUniform->ux) + ";" + ct::toStr(vUniform->uy) + ";" + ct::toStr(vUniform->uz) +
                                ";" + ct::toStr(vUniform->uw) + "\n";
                            break;
                        case uType::uTypeEnum::U_BOOL: str = vUniform->name + ":" + ct::toStr(vUniform->bx) + "\n"; break;
                        case uType::uTypeEnum::U_BVEC2:
                            str = vUniform->name + ":" + ct::toStr(vUniform->bx) + ";" + ct::toStr(vUniform->by) + "\n";
                            break;
                        case uType::uTypeEnum::U_BVEC3:
                            str =
                                vUniform->name + ":" + ct::toStr(vUniform->bx) + ";" + ct::toStr(vUniform->by) + ";" + ct::toStr(vUniform->bz) + "\n";
                            break;
                        case uType::uTypeEnum::U_BVEC4:
                            str = vUniform->name + ":" + ct::toStr(vUniform->bx) + ";" + ct::toStr(vUniform->by) + ";" + ct::toStr(vUniform->bz) +
                                ";" + ct::toStr(vUniform->bw) + "\n";
                            break;
                        default: break;
                    }
                }
            }
        }
    }

    return str;
}

std::string UniformHelper::SerializeUniformAsConst(UniformVariantPtr vUniform) {
    ZoneScoped;

    std::string str;

    if (vUniform != nullptr) {
        if (!vUniform->constant && !vUniform->noExport) {
            if (vUniform->glslType == uType::uTypeEnum::U_FLOAT) {
                str = "const float " + vUniform->name + " = ";
                float v = 0.0f;
                if (vUniform->widget == "checkbox")
                    v = (vUniform->bx ? 1.0f : 0.0f);
                else
                    v = vUniform->x;
                if (IS_FLOAT_EQUAL(ct::floor(v), v)) {
                    str += ct::toStr(v) + ".0;\n";
                } else {
                    str += ct::toStr(v) + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC2) {
                if (vUniform->widget == "checkbox") {
                    str = "const vec2 " + vUniform->name + " = vec2(" + ct::toStr((vUniform->bx ? 1.0f : 0.0f)) + "," +
                        ct::toStr((vUniform->by ? 1.0f : 0.0f)) + ");\n";
                } else {
                    str = "const vec2 " + vUniform->name + " = vec2(" + ct::toStr(vUniform->x) + "," + ct::toStr(vUniform->y) + ");\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC3) {
                if (vUniform->widget == "checkbox") {
                    str = "const vec3 " + vUniform->name + " = vec3(" + ct::toStr((vUniform->bx ? 1.0f : 0.0f)) + "," +
                        ct::toStr((vUniform->by ? 1.0f : 0.0f)) + "," + ct::toStr((vUniform->bz ? 1.0f : 0.0f)) + ");\n";
                } else {
                    str = "const vec3 " + vUniform->name + " = vec3(" + ct::toStr(vUniform->x) + "," + ct::toStr(vUniform->y) + "," +
                        ct::toStr(vUniform->z) + ");\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC4) {
                if (vUniform->widget == "checkbox") {
                    str = "const vec4" + vUniform->name + " = vec4(" + ct::toStr((vUniform->bx ? 1.0f : 0.0f)) + "," +
                        ct::toStr((vUniform->by ? 1.0f : 0.0f)) + "," + ct::toStr((vUniform->bz ? 1.0f : 0.0f)) + "," +
                        ct::toStr((vUniform->bw ? 1.0f : 0.0f)) + ");\n";
                } else {
                    str = "const vec4 " + vUniform->name + " = vec4(" + ct::toStr(vUniform->x) + "," + ct::toStr(vUniform->y) + "," +
                        ct::toStr(vUniform->z) + "," + ct::toStr(vUniform->w) + ");\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_INT)
                str = "const int " + vUniform->name + " = " + ct::toStr(vUniform->ix) + ";\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_IVEC2)
                str = "const ivec2 " + vUniform->name + " = ivec2(" + ct::toStr(vUniform->ix) + "," + ct::toStr(vUniform->iy) + ");\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_IVEC3)
                str = "const ivec3 " + vUniform->name + " = ivec3(" + ct::toStr(vUniform->ix) + "," + ct::toStr(vUniform->iy) + "," +
                    ct::toStr(vUniform->iz) + ");\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_IVEC4)
                str = "const ivec4 " + vUniform->name + " = ivec4(" + ct::toStr(vUniform->ix) + "," + ct::toStr(vUniform->iy) + "," +
                    ct::toStr(vUniform->iz) + "," + ct::toStr(vUniform->iw) + ");\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_UINT)
                str = "const uint " + vUniform->name + " = " + ct::toStr(vUniform->ux) + ";\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_UVEC2)
                str = "const uvec2 " + vUniform->name + " = uvec2(" + ct::toStr(vUniform->ux) + "," + ct::toStr(vUniform->uy) + ");\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_UVEC3)
                str = "const uvec3 " + vUniform->name + " = uvec3(" + ct::toStr(vUniform->ux) + "," + ct::toStr(vUniform->uy) + "," +
                    ct::toStr(vUniform->uz) + ");\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_UVEC4)
                str = "const uvec4 " + vUniform->name + " = uvec4(" + ct::toStr(vUniform->ux) + "," + ct::toStr(vUniform->uy) + "," +
                    ct::toStr(vUniform->uz) + "," + ct::toStr(vUniform->uw) + ");\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_BOOL)
                str = "const bool " + vUniform->name + " = " + (vUniform->bx ? "true" : "false") + ";\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_BVEC2)
                str = "const bvec2 " + vUniform->name + " = bvec2(" + (vUniform->bx ? "true" : "false") + "," + (vUniform->by ? "true" : "false") +
                    ");\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_BVEC3)
                str = "const bvec3 " + vUniform->name + " = bvec3(" + (vUniform->bx ? "true" : "false") + "," + (vUniform->by ? "true" : "false") +
                    "," + (vUniform->bz ? "true" : "false") + ");\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_BVEC4)
                str = "const bvec4 " + vUniform->name + " = bvec4(" + (vUniform->bx ? "true" : "false") + "," + (vUniform->by ? "true" : "false") +
                    "," + (vUniform->bz ? "true" : "false") + "," + (vUniform->bw ? "true" : "false") + ");\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_SAMPLER2D)
                str = "//uniform sampler2D " + vUniform->name + ";\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_SAMPLER3D)
                str = "//uniform sampler3D " + vUniform->name + ";\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_SAMPLERCUBE)
                str = "//uniform samplerCube " + vUniform->name + ";\n";
            else if (vUniform->glslType == uType::uTypeEnum::U_MAT4) {
                if (vUniform->uFloatArr) {
                    str = "const mat4 " + vUniform->name + " = mat4(";
                    for (int i = 0; i < 16; i++) {
                        if (i > 0)
                            str += ',';
                        str += ct::toStr(vUniform->uFloatArr[i]);
                    }
                    str += ");\n";
                }
            }
        }
    }

    return str;
}

std::string UniformHelper::SerializeUniformAsWidget(UniformVariantPtr vUniform) {
    ZoneScoped;

    std::string str;

    if (vUniform != nullptr) {
        if (!vUniform->constant) {
            if (vUniform->glslType == uType::uTypeEnum::U_FLOAT) {
                if (vUniform->widget.empty()) {
                    str = "uniform float(";
                    str += ct::toStr(vUniform->inf.x) + ":";
                    str += ct::toStr(vUniform->sup.x) + ":";
                    str += ct::toStr(vUniform->def.x);
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f))
                        str += ":" + ct::toStr(vUniform->step.x);
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC2) {
                if (vUniform->widget.empty()) {
                    str = "uniform vec2(";
                    str += vUniform->inf.xy().string(',') + ":";
                    str += vUniform->sup.xy().string(',') + ":";
                    str += vUniform->def.xy().string(',');
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.y, 0.0f))
                        str += ":" + vUniform->step.xy().string(',');
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC3) {
                if (vUniform->widget.empty()) {
                    str = "uniform float(";
                    str += vUniform->inf.xyz().string(',') + ":";
                    str += vUniform->sup.xyz().string(',') + ":";
                    str += vUniform->def.xyz().string(',');
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.y, 0.0f) ||
                        IS_FLOAT_DIFFERENT(vUniform->step.z, 0.0f))
                        str += ":" + vUniform->step.xyz().string(',');
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC4) {
                if (vUniform->widget.empty()) {
                    str = "uniform float(";
                    str += vUniform->inf.string(',') + ":";
                    str += vUniform->sup.string(',') + ":";
                    str += vUniform->def.string(',');
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.y, 0.0f) ||
                        IS_FLOAT_DIFFERENT(vUniform->step.z, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.w, 0.0f))
                        str += ":" + vUniform->step.string(',');
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_INT) {
                if (vUniform->widget.empty()) {
                    str = "uniform int(";
                    str += ct::toStr((int)(vUniform->inf.x)) + ":";
                    str += ct::toStr((int)(vUniform->sup.x)) + ":";
                    str += ct::toStr((int)(vUniform->def.x));
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f))
                        str += ":" + ct::toStr((int)(vUniform->step.x));
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_IVEC2) {
                if (vUniform->widget.empty()) {
                    str = "uniform ivec2(";
                    str += vUniform->inf.xy().string(',') + ":";
                    str += vUniform->sup.xy().string(',') + ":";
                    str += vUniform->def.xy().string(',');
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.y, 0.0f))
                        str += ":" + vUniform->step.xy().string(',');
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_IVEC3) {
                if (vUniform->widget.empty()) {
                    str = "uniform ivec3(";
                    str += vUniform->inf.xyz().string(',') + ":";
                    str += vUniform->sup.xyz().string(',') + ":";
                    str += vUniform->def.xyz().string(',');
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.y, 0.0f) ||
                        IS_FLOAT_DIFFERENT(vUniform->step.z, 0.0f))
                        str += ":" + vUniform->step.xyz().string(',');
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_IVEC4) {
                if (vUniform->widget.empty()) {
                    str = "uniform ivec4(";
                    str += vUniform->inf.string(',') + ":";
                    str += vUniform->sup.string(',') + ":";
                    str += vUniform->def.string(',');
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.y, 0.0f) ||
                        IS_FLOAT_DIFFERENT(vUniform->step.z, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.w, 0.0f))
                        str += ":" + vUniform->step.string(',');
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_UINT) {
                if (vUniform->widget.empty()) {
                    str = "uniform uint(";
                    str += ct::toStr((int)(vUniform->inf.x)) + ":";
                    str += ct::toStr((int)(vUniform->sup.x)) + ":";
                    str += ct::toStr((int)(vUniform->def.x));
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f))
                        str += ":" + ct::toStr((int)(vUniform->step.x));
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_UVEC2) {
                if (vUniform->widget.empty()) {
                    str = "uniform uvec2(";
                    str += vUniform->inf.xy().string(',') + ":";
                    str += vUniform->sup.xy().string(',') + ":";
                    str += vUniform->def.xy().string(',');
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.y, 0.0f))
                        str += ":" + vUniform->step.xy().string(',');
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_UVEC3) {
                if (vUniform->widget.empty()) {
                    str = "uniform uvec3(";
                    str += vUniform->inf.xyz().string(',') + ":";
                    str += vUniform->sup.xyz().string(',') + ":";
                    str += vUniform->def.xyz().string(',');
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.y, 0.0f) ||
                        IS_FLOAT_DIFFERENT(vUniform->step.z, 0.0f))
                        str += ":" + vUniform->step.xyz().string(',');
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_UVEC4) {
                if (vUniform->widget.empty()) {
                    str = "uniform uvec4(";
                    str += vUniform->inf.string(',') + ":";
                    str += vUniform->sup.string(',') + ":";
                    str += vUniform->def.string(',');
                    if (IS_FLOAT_DIFFERENT(vUniform->step.x, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.y, 0.0f) ||
                        IS_FLOAT_DIFFERENT(vUniform->step.z, 0.0f) || IS_FLOAT_DIFFERENT(vUniform->step.w, 0.0f))
                        str += ":" + vUniform->step.string(',');
                    str += ") " + vUniform->name + ";\n";
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_MAT4) {
            } else if (vUniform->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                if (vUniform->widget == "picture") {
                    std::string file = (!vUniform->filePathNames.empty()) ? vUniform->filePathNames[0] : "";
                    str = "uniform sampler2D(picture:choice=" + file;
                    str += ":flip=" + ct::toStr(vUniform->flip ? "true" : "false");
                    str += ":mipmap=" + ct::toStr(vUniform->mipmap ? "true" : "false");
                    str += ":wrap=" + ct::toStr(vUniform->wrap);
                    str += ":filter=" + ct::toStr(vUniform->filter);
                    str += ") " + vUniform->name + ";\n";
                }
            }
        }
    }

    return str;
}

void UniformHelper::DeSerializeUniform(ShaderKeyPtr vShaderKey, UniformVariantPtr vUniform, const std::vector<std::string>& vParams) {
    ZoneScoped;

    if (vUniform != nullptr)  // two fields mini
    {
        if (vUniform->lockedAgainstConfigLoading)
            return;

        if (vUniform->widget == "time") {
            if (vParams.size() == 3) {
                vUniform->bx = ct::ivariant(vParams[1]).GetB();
                vUniform->x = ct::fvariant(vParams[2]).GetF();
            }
        } else if (vUniform->widget == "checkbox") {
            if (vParams.size() == 2) {
                auto vec = ct::splitStringToVector(vParams[1], ";", true);
                if (vec.size() > 0)
                    vUniform->bx = ct::ivariant(vec[0]).GetB();
                if (vec.size() > 1)
                    vUniform->by = ct::ivariant(vec[1]).GetB();
                if (vec.size() > 2)
                    vUniform->bz = ct::ivariant(vec[2]).GetB();
                if (vec.size() > 3)
                    vUniform->bw = ct::ivariant(vec[3]).GetB();
                vUniform->x = (vUniform->bx ? 1.0f : 0.0f);
                vUniform->y = (vUniform->by ? 1.0f : 0.0f);
                vUniform->z = (vUniform->bz ? 1.0f : 0.0f);
                vUniform->w = (vUniform->bw ? 1.0f : 0.0f);
            }
        } else if (GizmoSystem::Instance()->DeSerializeUniform(vShaderKey, vUniform, vParams)) {
            return;
        } else if (GamePadSystem::Instance()->DeSerializeUniform(vShaderKey, vUniform, vParams)) {
            return;
        } else if (SoundSystem::Instance()->DeSerializeUniform(vShaderKey, vUniform, vParams)) {
            return;
        } else if (MidiSystem::Instance()->DeSerializeUniform(vShaderKey, vUniform, vParams)) {
            return;
        } else if (vUniform->widget == "picture") {
            if (vParams.size() >= 3) {
                if (vParams[1] == "target") {
                    vUniform->target = vParams[2];
                } else if (vParams[1] == "choose" && (vUniform->textureFileChoosebox || vUniform->textureChoiceActivated)) {
                    if (vParams.size() > 3) {
                        size_t idx = 0;
                        for (auto it = vParams.begin(); it != vParams.end(); ++it) {
                            std::string word = *it;

                            if (word == "flip" && (vUniform->textureFlipChoosebox || vUniform->textureChoiceActivated)) {
                                if (vParams.size() > idx + 1)
                                    vUniform->flip = ct::ivariant(vParams[idx + 1]).GetB();
                            } else if (word == "mipmap" && (vUniform->textureMipmapChoosebox || vUniform->textureChoiceActivated)) {
                                if (vParams.size() > idx + 1)
                                    vUniform->mipmap = ct::ivariant(vParams[idx + 1]).GetB();
                            } else if (word == "wrap" && (vUniform->textureWrapChoosebox || vUniform->textureChoiceActivated)) {
                                if (vParams.size() > idx + 1)
                                    vUniform->wrap = vParams[idx + 1];
                            } else if (word == "filter" && (vUniform->textureFilterChoosebox || vUniform->textureChoiceActivated)) {
                                if (vParams.size() > idx + 1)
                                    vUniform->filter = vParams[idx + 1];
                            }

                            idx++;
                        }
                    }

                    vUniform->filePathNames.clear();
                    vUniform->filePathNames.emplace_back(vParams[2]);
                    vUniform->textureFileChoosebox = true;
                    if (vShaderKey) {
                        vShaderKey->Complete_Uniform_Picture_With_Texture(vUniform);
                    }
                }
            }
        } else if (vUniform->widget == "mouse:2pos_2click" || vUniform->widget == "mouse:2press_2click" || vUniform->widget == "mouse:shadertoy") {
            if (vUniform->glslType == uType::uTypeEnum::U_VEC4) {
                if (!vUniform->constant) {
                    ct::fvec4 def(vUniform->x, vUniform->y, vUniform->z, vUniform->w);
                    const ct::fvec4 xyzw = ct::fvec4(vParams[1], ';', &def);
                    if (vParams.size() > 2) {
                        UniformHelper::FBOSizeForMouseUniformNormalization = ct::fvec2(vParams[2], ';');
                    }
                    vUniform->x = xyzw.x * UniformHelper::FBOSizeForMouseUniformNormalization.x;
                    vUniform->y = xyzw.y * UniformHelper::FBOSizeForMouseUniformNormalization.y;
                    vUniform->z = xyzw.z * UniformHelper::FBOSizeForMouseUniformNormalization.x;
                    vUniform->w = xyzw.w * UniformHelper::FBOSizeForMouseUniformNormalization.y;
                }
            }
        } else if (vUniform->widget == "mouse:normalized_2pos_2click" || vUniform->widget == "mouse:normalized_2press_2click" ||
                   vUniform->widget == "mouse:normalized_shadertoy") {
            if (vUniform->glslType == uType::uTypeEnum::U_VEC4) {
                if (!vUniform->constant) {
                    ct::fvec4 def(vUniform->x, vUniform->y, vUniform->z, vUniform->w);
                    const ct::fvec4 xyzw = ct::fvec4(vParams[1], ';', &def);
                    vUniform->x = xyzw.x;
                    vUniform->y = xyzw.y;
                    vUniform->z = xyzw.z;
                    vUniform->w = xyzw.w;
                }
            }
        } else if (vParams.size() == 2) {
            if (vUniform->glslType == uType::uTypeEnum::U_INT) {
                if (!vUniform->constant) {
                    const int x = ct::ivariant(vParams[1]).GetI();
                    vUniform->ix = x;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_IVEC2) {
                if (!vUniform->constant) {
                    ct::ivec2 def(vUniform->ix, vUniform->iy);
                    const ct::ivec2 xy = ct::ivec2(vParams[1], ';', &def);
                    vUniform->ix = xy.x;
                    vUniform->iy = xy.y;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_IVEC3) {
                if (!vUniform->constant) {
                    ct::ivec3 def(vUniform->ix, vUniform->iy, vUniform->iz);
                    const ct::ivec3 xyz = ct::ivec3(vParams[1], ';', &def);
                    vUniform->ix = xyz.x;
                    vUniform->iy = xyz.y;
                    vUniform->iz = xyz.z;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_IVEC4) {
                if (!vUniform->constant) {
                    ct::ivec4 def(vUniform->ix, vUniform->iy, vUniform->iz, vUniform->iw);
                    const ct::ivec4 xyzw = ct::ivec4(vParams[1], ';', &def);
                    vUniform->ix = xyzw.x;
                    vUniform->iy = xyzw.y;
                    vUniform->iz = xyzw.z;
                    vUniform->iw = xyzw.w;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_UINT) {
                if (!vUniform->constant) {
                    const uint32_t x = ct::ivariant(vParams[1]).GetU();
                    vUniform->ux = x;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_UVEC2) {
                if (!vUniform->constant) {
                    ct::uvec2 def(vUniform->ux, vUniform->uy);
                    const ct::uvec2 xy = ct::uvec2(vParams[1], ';', &def);
                    vUniform->ux = xy.x;
                    vUniform->uy = xy.y;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_UVEC3) {
                if (!vUniform->constant) {
                    ct::uvec3 def(vUniform->ux, vUniform->uy, vUniform->uz);
                    const ct::uvec3 xyz = ct::uvec3(vParams[1], ';', &def);
                    vUniform->ux = xyz.x;
                    vUniform->uy = xyz.y;
                    vUniform->uz = xyz.z;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_UVEC4) {
                if (!vUniform->constant) {
                    ct::uvec4 def(vUniform->ux, vUniform->uy, vUniform->uz, vUniform->uw);
                    const ct::uvec4 xyzw = ct::uvec4(vParams[1], ';', &def);
                    vUniform->ux = xyzw.x;
                    vUniform->uy = xyzw.y;
                    vUniform->uz = xyzw.z;
                    vUniform->uw = xyzw.w;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_FLOAT) {
                if (!vUniform->constant) {
                    const float x = ct::fvariant(vParams[1]).GetF();
                    vUniform->x = x;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC2) {
                if (!vUniform->constant) {
                    ct::fvec2 def(vUniform->x, vUniform->y);
                    const ct::fvec2 xy = ct::fvec2(vParams[1], ';', &def);
                    vUniform->x = xy.x;
                    vUniform->y = xy.y;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC3) {
                if (!vUniform->constant) {
                    ct::fvec3 def(vUniform->x, vUniform->y, vUniform->z);
                    const ct::fvec3 xyz = ct::fvec3(vParams[1], ';', &def);
                    vUniform->x = xyz.x;
                    vUniform->y = xyz.y;
                    vUniform->z = xyz.z;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC4) {
                if (!vUniform->constant) {
                    ct::fvec4 def(vUniform->x, vUniform->y, vUniform->z, vUniform->w);
                    const ct::fvec4 xyzw = ct::fvec4(vParams[1], ';', &def);
                    vUniform->x = xyzw.x;
                    vUniform->y = xyzw.y;
                    vUniform->z = xyzw.z;
                    vUniform->w = xyzw.w;
                }
            } else if (vUniform->glslType == uType::uTypeEnum::U_MAT2) {
                /*if (vUniform->constant == false)
                {
                    auto vec = ct::fvariant(vParams[1]).GetVectorFloat(',');
                    if (vec.size() == 4)
                    {
                        vUniform->mat4 = glm::mat2(vec[0], vec[1], vec[2], vec[3]);
                    }
                }*/
            } else if (vUniform->glslType == uType::uTypeEnum::U_MAT3) {
                /*if (vUniform->constant == false)
                {
                    auto vec = ct::fvariant(vParams[1]).GetVectorFloat(',');
                    if (vec.size() == 9)
                    {
                        vUniform->mat4 = glm::mat3(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5], vec[6], vec[7], vec[8]);
                    }
                }*/
            } else if (vUniform->glslType == uType::uTypeEnum::U_MAT4) {
                if (!vUniform->constant) {
                    auto vec = ct::fvariant(vParams[1]).GetVectorFloat(',');
                    if (vec.size() == 16) {
                        vUniform->mat4 = glm::mat4(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5], vec[6], vec[7], vec[8], vec[9], vec[10], vec[11],
                                                   vec[12], vec[13], vec[14], vec[15]);
                    }
                }
            }
        }
    }
}
