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

#pragma warning(disable : 4312)

#include "RenderPack.h"
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <ctools/Logger.h>
#include <ctools/GLVersionChecker.h>
#include <Gui/CustomGuiWidgets.h>
#include <Mesh/Operations/MeshLoader.h>
#include <Mesh/Operations/MeshSaver.h>
#include <Buffer/FrameBuffer.h>
#include <CodeTree/CodeTree.h>
#include <Systems/CameraSystem.h>
#include <Systems/GizmoSystem.h>
#include <Systems/GamePadSystem.h>
#include <Systems/MidiSystem.h>
#include <Systems/SoundSystem.h>
#include <Systems/TimeLineSystem.h>
#include <Texture/Texture2D.h>
#include <Texture/Texture3D.h>
#include <Texture/TextureSound.h>
#include <Profiler/TracyProfiler.h>
#include <ImGuiPack.h>
#include <Uniforms/UniformHelper.h>
#include <CodeTree/Parsing/UniformParsing.h>
#include <VR/Backend/VRBackend.h>
#include <Res/CustomFont.h>

#include <Mesh/Model/QuadModel.h>
#include <Mesh/Model/PointModel.h>
#include <Mesh/Model/PNTBTCModel.h>
#include <Mesh/Model/PNCModel.h>

#include <imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS

#endif
#include <imgui_internal.h>

#include <chrono>
#include <ctime>
// typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::system_clock Clock;

#ifdef WIN32
// #include "Shlwapi.h"
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

RenderPackPtr RenderPack::Create(const GuiBackend_Window& vWin) {
    auto res = std::make_shared<RenderPack>(vWin);
    res->m_This = res;
    return res;
}

//////////////////////////////////////////////////////////////////////
//// COMPUTE STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

RenderPackPtr RenderPack::createComputeWithFileWithoutLoading(const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize,
                                                              ShaderKeyPtr vShaderKey) {
    ZoneScoped;

    auto res = RenderPack::Create(vWin);

    if (vShaderKey) {
        res->SetComputeDefaultParams(vName, vSize, vShaderKey);
    } else {
        res.reset();
    }

    return res;
}

RenderPackPtr RenderPack::createComputeWithFile(const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey) {
    ZoneScoped;

    auto res = RenderPack::Create(vWin);

    if (vShaderKey && res->InitComputeWithFile(vWin, vName, vSize, vShaderKey)) {
    } else {
        res.reset();
    }

    return res;
}

//////////////////////////////////////////////////////////////////////
//// BUFFER STATIC ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

RenderPackPtr RenderPack::createBufferWithFileWithoutLoading(const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize,
                                                             ShaderKeyPtr vShaderKey, bool vUseZBuffer, bool vUseFBO, bool vUseRenderBuffer,
                                                             bool vUseFloatBuffer) {
    ZoneScoped;

    auto res = RenderPack::Create(vWin);
    res->SetBufferDefaultParams(vName, vSize, vShaderKey, vUseZBuffer, vUseFBO, vUseRenderBuffer, vUseFloatBuffer);

    return res;
}

RenderPackPtr RenderPack::createBufferWithFile(const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey,
                                               bool vUseZBuffer, bool vUseFBO, bool vUseRenderBuffer, TextureParamsStruct* vTexParam,
                                               bool vUseFloatBuffer) {
    ZoneScoped;

    auto res = RenderPack::Create(vWin);

    if (vShaderKey && res->InitBufferWithFile(vWin, vName, vSize, vShaderKey, vUseZBuffer, vUseFBO, vUseRenderBuffer, vTexParam, vUseFloatBuffer)) {
    } else {
        res.reset();
    }

    return res;
}

ct::frect RenderPack::GetScreenRectWithSize(ct::ivec2 vItemSize, ct::ivec2 vMaxSize) {
    ZoneScoped;

    ct::frect rc;

    const ct::fvec2 visibleSize = ct::fvec2((float)vMaxSize.x, (float)vMaxSize.y);
    if (visibleSize.x > 0.0f && visibleSize.y > 0.0f) {
        const ct::fvec2 visibleOrigin;
        const ct::fvec2 texSize = ct::fvec2((float)vItemSize.x, (float)vItemSize.y);

        // float visibleRatio = visibleSize.x / visibleSize.y;
        const float refRatio = texSize.x / texSize.y;

        const float newX = visibleSize.y * refRatio;
        const float newY = visibleSize.x / refRatio;

        if (newX < visibleSize.x) {
            // pos
            rc.x = visibleOrigin.x + (visibleSize.x - newX) * 0.5f;
            rc.y = visibleOrigin.y;

            // size
            rc.w = newX;
            rc.h = visibleSize.y;
        } else {
            // pos
            rc.x = visibleOrigin.x;
            rc.y = visibleOrigin.y + (visibleSize.y - newY) * 0.5f;

            // size
            rc.w = visibleSize.x;
            rc.h = newY;
        }

        rc = ct::floor(rc);

        const float newRatio = rc.w / rc.h;
        if (IS_FLOAT_DIFFERENT(newRatio, refRatio)) {
            LogVarError("the new ratio is bad");
        }
    }

    return rc;
}

ct::frect RenderPack::GetScreenRectWithRatio(float vRatio, ct::ivec2 vMaxSize) {
    ZoneScoped;

    ct::frect rc;

    const ct::fvec2 visibleSize = ct::fvec2((float)vMaxSize.x, (float)vMaxSize.y);
    if (visibleSize.x > 0.0f && visibleSize.y > 0.0f) {
        const ct::fvec2 visibleOrigin;

        const float refRatio = vRatio;

        const float newX = visibleSize.y * refRatio;
        const float newY = visibleSize.x / refRatio;

        if (newX < visibleSize.x) {
            // pos
            rc.x = visibleOrigin.x + (visibleSize.x - newX) * 0.5f;
            rc.y = visibleOrigin.y;

            // size
            rc.w = newX;
            rc.h = visibleSize.y;
        } else {
            // pos
            rc.x = visibleOrigin.x;
            rc.y = visibleOrigin.y + (visibleSize.y - newY) * 0.5f;

            // size
            rc.w = visibleSize.x;
            rc.h = newY;
        }

        rc = ct::floor(rc);

        const float newRatio = rc.w / rc.h;
        if (IS_FLOAT_DIFFERENT(newRatio, refRatio)) {
            LogVarError("the new ratio is bad");
        }
    }

    return rc;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

std::list<RenderPackWeak>::iterator BufferCallMap::begin() {
    return iterable_buffers.begin();
}

std::list<RenderPackWeak>::iterator BufferCallMap::end() {
    return iterable_buffers.end();
}

size_t BufferCallMap::size() {
    return iterable_buffers.size();
}

bool BufferCallMap::empty() {
    return iterable_buffers.empty();
}

void BufferCallMap::clear() {
    dico.clear();
    iterable_buffers.clear();
}

bool BufferCallMap::exist(const std::string& vName) {
    return dico.find(vName) != dico.end();
}

void BufferCallMap::erase(const std::string& vName) {
    if (exist(vName)) {
        RenderPackWeak p = dico[vName];
        if (!p.expired()) {
            for (auto it = iterable_buffers.begin(); it != iterable_buffers.end(); ++it) {
                if ((*it).lock() == p.lock()) {
                    iterable_buffers.erase(it);
                    break;
                }
            }

            dico.erase(vName);
        }
    }
}

void BufferCallMap::add(const std::string& vName, RenderPackPtr vBuffer) {
    if (dico.find(vName) == dico.end()) {
        dico[vName] = vBuffer;
        iterable_buffers.push_back(vBuffer);
    }
}

RenderPackWeak BufferCallMap::get(const std::string& vName) {
    if (exist(vName))
        return dico[vName];
    return RenderPackWeak();
}

inline bool BufferCallMap_Sort(const RenderPackWeak& a, const RenderPackWeak& b) {
    const auto& aPrr = a.lock();
    const auto& bPtr = b.lock();
    if (aPrr && bPtr)
        return (aPrr->puName < bPtr->puName);
    return false;
}

void BufferCallMap::finalize() {
    if (!iterable_buffers.empty()) {
#if 0
		iterable_buffers.reverse();
#else
        iterable_buffers.sort(BufferCallMap_Sort);
#endif
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

RenderPack::RenderPack(const GuiBackend_Window& vWin) {
    ZoneScoped;

    puWindow = vWin;

    puLoaded = false;

    puShaderKey = nullptr;

    puCanWeTuneMouse = true;
    puCanWeTuneCamera = true;

    puFrameBuffer = nullptr;
    puShader = nullptr;
    puModel_Render = nullptr;
    puRecordBuffer = nullptr;

    puRenderPackType = RenderPack_Type::RENDERPACK_TYPE_BUFFER;

    puSomeErrorsFromLastShader = false;
    puSomeWarningsFromLastShader = false;
    puNewCompilationNeeded = true;

    puShowZBufferButton = false;
    puShowTransparentButton = false;
    puShowCullingButton = false;
    puShowBlendingButton = false;

    puShowOpenModelButton = false;

    puCanWeRender = true;

    puCountAttachments = 1;

    puLastRenderTime = 0.0f;
    puFrameIdx = 0;

    // puSliceBufferId = 0;
    puZeroBasedMaxSliceBufferId = 0;

    puGeometryOutputRenderMode = GL_TRIANGLES;
    puGeometryInputRenderMode = GL_TRIANGLES;

    puTexParamsAlreadyInit = false;
    puTexParamCustomized = false;
    memset(puBufferIdsToShow, true, 8);

    puMeshType = BaseMeshEnum::PRIMITIVE_TYPE_QUAD;

    puMainRenderPack = nullptr;

    // utile pour le thread
    // car pendant que le thread travaille les parametres ssont tous fige dans le thread
    // il peut arriver qu'on veuille continuer a les tuner pendant ce tmeps
    // et quand on arrete la thread, la destruction des deux renderpack du thread ecrasera nos changement
    // avec cette options active pour les deux du thread, pas de souci
    puDontSaveConfigFiles = false;

    puScreenRectAlreadyCalculated = false;

    // settings
    InitCountPatchVertices();
    puLastRenderMode = 0;
    puLastLastRenderMode = 0;
    puCreateZBuffer = false;
    puUseFXAA = false;
    puCountFXAASamples = 2;
    puUseFloatBuffer = false;
    puUseFBO = false;
    puUseRBuffer = false;
    puUsePointSize = true;

    Init_CullFace();
    Init_Blending();
    Init_Depth();

    _CurrentSelectedBufferidToTune = 0;
    _BufferWrap = 0;
    _BufferFilter = 0;
    _BufferFormat = 0;

    puDisplayModeArrayIndex = 0;
}

RenderPack::~RenderPack() {
    ZoneScoped;

    Finish();
}

void RenderPack::Clear() {
    ZoneScoped;
}

bool RenderPack::Load(const std::string& vInFileBufferName, bool vReplaceRenderPackName) {
    UNUSED(vReplaceRenderPackName);

    ZoneScoped;

    bool res = false;

    GuiBackend::Instance()->MakeContextCurrent(puWindow);

    // il faut faire mieux, le but est juste de detruire les objets avant d'en recreer d'autres
    // normalement load ne devrait etre appel� qu'une seule fois
    // Finish(false);

    puLastRenderMode = (GLenum)ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES;

    if (puShaderKey) {
        UpdateInfileBufferName(vInFileBufferName);

        UpdateSectionConfig(puShaderKey->puInFileBufferName);

        puCountIterations = puSectionConfig.framebufferConfig.countIterations;
        puShaderKey->puShaderGlobalSettings.countIterations = puCountIterations.w;

        // todo : a simplifier car les 3 cas on 80% des etapes en commun maintenant
        if (puRenderPackType == RenderPack_Type::RENDERPACK_TYPE_BUFFER) {
            puMeshType = puSectionConfig.vertexConfig.meshType;

            puCountVertices = puSectionConfig.vertexConfig.countVertexs;
            puShaderKey->puShaderGlobalSettings.countVertices = puCountVertices.w;

            puCountInstances = puSectionConfig.vertexConfig.countInstances;
            puShaderKey->puShaderGlobalSettings.countInstances = puCountInstances.w;

            // pas touche a puShaderKey->puShaderGlobalSettings.countInstances.w c'est la valeur
            puLastRenderMode = (GLenum)puSectionConfig.vertexConfig.displayMode[puSectionConfig.vertexConfig.defaultDisplayMode];

            if (puMeshType == BaseMeshEnum::PRIMITIVE_TYPE_QUAD) {
                puMeshTypeString = "Quad";

                puCountAttachments = 1;

                if (puUseFBO) {
                    CreateOrUpdatePipe(puSectionConfig.framebufferConfig);

                    // change tex parameters si il y a une config de charg�e
                    if (puTexParamCustomized) {
                        ChangeTexParameters(&puTexParams);
                    }

                    if (puFrameBuffer && puFrameBuffer->load()) {
                        ConfigureBufferParams(puSectionConfig.framebufferConfig.format, puSectionConfig.framebufferConfig.mipmap, 1000,
                                              puSectionConfig.framebufferConfig.wrap, puSectionConfig.framebufferConfig.filter);

                        puFrameBuffer->GetTexParameters(&puTexParams);
                    }
                } else {
                    puMeshRect.setRect(0.0f, 0.0f, (float)puMaxScreenSize.x, (float)puMaxScreenSize.y);
                }

                puModel_Render = QuadModel::Create(puWindow);
                UpdateCountInstances((uint32_t)puShaderKey->puShaderGlobalSettings.countInstances);
                UpdateCountPatchVertices((uint32_t)puPatchsCountVertices.w);
                if (puModel_Render->IsValid()) {
                    res = true;
                    ParseAndCompilShader(puShaderKey->puInFileBufferName);
                }

                puShowBlendingButton = true;
                puShowCullingButton = false;
                puShowZBufferButton = true;
                puShowTransparentButton = false;
                puShowOpenModelButton = false;
            } else if (puMeshType == BaseMeshEnum::PRIMITIVE_TYPE_POINTS) {
                puMeshTypeString = "Points";
                puUsePointSize = true;

                puMeshRect.setRect(0.0f, 0.0f, (float)puMaxScreenSize.x, (float)puMaxScreenSize.y);

                if (puUseFBO) {
                    puFrameBuffer = FrameBuffersPipeLine::Create(puWindow, puMaxScreenSize, puSectionConfig.framebufferConfig.format, puUseFXAA,
                                                                 puCountFXAASamples, puCreateZBuffer);

                    if (puFrameBuffer && puFrameBuffer->load()) {
                        ConfigureBufferParams(puSectionConfig.framebufferConfig.format, puSectionConfig.framebufferConfig.mipmap, 1000,
                                              puSectionConfig.framebufferConfig.wrap, puSectionConfig.framebufferConfig.filter);

                        puFrameBuffer->GetTexParameters(&puTexParams);
                    }
                }

                puModel_Render = PointModel::Create(puWindow, puShaderKey->puShaderGlobalSettings.countVertices);
                UpdateCountVertex((uint32_t)puShaderKey->puShaderGlobalSettings.countVertices);
                UpdateCountInstances((uint32_t)puShaderKey->puShaderGlobalSettings.countInstances);
                UpdateCountPatchVertices(puPatchsCountVertices.w);
                if (puModel_Render->IsValid()) {
                    res = true;
                    ParseAndCompilShader(puShaderKey->puInFileBufferName);
                }

                puShowBlendingButton = true;
                puShowCullingButton = true;
                puShowZBufferButton = true;
                puShowTransparentButton = true;
                puShowOpenModelButton = false;
            } else if (puMeshType == BaseMeshEnum::PRIMITIVE_TYPE_MESH) {
                puMeshTypeString = "Mesh";
                puUsePointSize = true;

                puMeshRect.setRect(0.0f, 0.0f, (float)puMaxScreenSize.x, (float)puMaxScreenSize.y);

                if (puUseFBO) {
                    puFrameBuffer = FrameBuffersPipeLine::Create(puWindow, puMaxScreenSize, puSectionConfig.framebufferConfig.format, puUseFXAA,
                                                                 puCountFXAASamples, puCreateZBuffer);

                    if (puFrameBuffer && puFrameBuffer->load()) {
                        ConfigureBufferParams(puSectionConfig.framebufferConfig.format, puSectionConfig.framebufferConfig.mipmap, 1000,
                                              puSectionConfig.framebufferConfig.wrap, puSectionConfig.framebufferConfig.filter);

                        puFrameBuffer->GetTexParameters(&puTexParams);
                    }
                }

                puModel_Render = PNTBTCModel::Create(puWindow);
                UpdateCountVertex((uint32_t)puShaderKey->puShaderGlobalSettings.countVertices);
                UpdateCountInstances((uint32_t)puShaderKey->puShaderGlobalSettings.countInstances);
                UpdateCountPatchVertices(puPatchsCountVertices.w);
                if (puModel_Render) {
                    res = true;
                    LoadMeshFileFromVertexSection();
                    ParseAndCompilShader(puShaderKey->puInFileBufferName);
                }

                puShowBlendingButton = true;
                puShowCullingButton = true;
                puShowZBufferButton = true;
                puShowTransparentButton = true;
                puShowOpenModelButton = true;
            }
        } else if (puRenderPackType == RenderPack_Type::RENDERPACK_TYPE_COMPUTE) {
            res = ParseAndCompilShader(puShaderKey->puInFileBufferName);

            puShowBlendingButton = false;
            puShowCullingButton = false;
            puShowZBufferButton = false;
            puShowTransparentButton = false;
            puShowOpenModelButton = false;
        }

        UpdateCountInstances((uint32_t)puShaderKey->puShaderGlobalSettings.countInstances);
    } else {
        res = true;
    }

    puLoaded = res;

    puTracyRenderShaderName = "RenderShader " + puName;

    // on charge les autre pack de la scene
    for (auto& sceneShaderFile : puSectionConfig.sceneConfig.shaders) {
        CreateSceneBuffer(sceneShaderFile, sceneShaderFile, "");
    }

    return res;
}

void RenderPack::LoadMeshFileFromVertexSection() {
    ZoneScoped;

    if (puShaderKey) {
        if (!puSectionConfig.vertexConfig.modelFileToLoad.empty()) {
            const std::string pathToFile =
                FileHelper::Instance()->GetRelativePathToParent(puSectionConfig.vertexConfig.modelFileToLoad, puShaderKey->GetPath(), true);
            if (FileHelper::Instance()->IsFileExist(pathToFile, true)) {
                MeshLoader::Instance()->LoadFile(m_This, pathToFile);
                MeshLoader::Instance()->puFilePathName = pathToFile;
            } else {
                std::string fromFile = puShaderKey->puKey;
                if (!puShaderKey->puInFileBufferFromKey.empty())
                    fromFile = puShaderKey->puInFileBufferFromKey;
                LogVarLightWarning("Mesh Obj File %s not found.", pathToFile.c_str());
            }
        }
    }
}

void RenderPack::UpdateInfileBufferName(const std::string& vInFileBufferName) {
    UNUSED(vInFileBufferName);

    ZoneScoped;

    if (puShaderKey) {
        if (!puMainRenderPack && puShaderKey->puInFileBufferName.empty() && !puShaderKey->puBufferNames.empty()) {
            if (puShaderKey->puBufferNames.find("MAIN") != puShaderKey->puBufferNames.end())  // trouvé
            {
                puShaderKey->puInFileBufferName = "MAIN";
            }
        }

        puShaderKey->PrepareConfigsComboBox(puShaderKey->puInFileBufferName);
    }
}

void RenderPack::SetBufferDefaultParams(const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey, bool vUseZBuffer, bool vUseFBO,
                                        bool vUseRenderBuffer, bool vUseFloatBuffer) {
    ZoneScoped;

    puRenderPackType = RenderPack_Type::RENDERPACK_TYPE_BUFFER;
    puMeshType = BaseMeshEnum::PRIMITIVE_TYPE_QUAD;
    puName = vName;
    puMaxScreenSize = vSize;
    puShaderKey = vShaderKey;
    puCreateZBuffer = vUseZBuffer;
    puUseRBuffer = vUseRenderBuffer;
    puUseFBO = vUseFBO;
    puUseFloatBuffer = vUseFloatBuffer;
}

void RenderPack::SetShaderKey(ShaderKeyPtr vShaderKey, bool vReplaceRenderPackName) {
    ZoneScoped;

    if (puShaderKey && vShaderKey) {
        vShaderKey->puCustomWidgets = puShaderKey->puCustomWidgets;
    }

    puShaderKey = vShaderKey;

    if (puShaderKey && vReplaceRenderPackName) {
        const auto ps = FileHelper::Instance()->ParsePathFileName(puShaderKey->puKey);
        if (ps.isOk) {
            puName = ps.name;
        }
    }
}

bool RenderPack::InitBufferWithFile(const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey,
                                    bool vUseZBuffer, bool vUseFBO, bool vUseRenderBuffer, TextureParamsStruct* vTexParam, bool vUseFloatBuffer) {
    ZoneScoped;

    bool res = false;

    puWindow = vWin;
    GuiBackend::Instance()->MakeContextCurrent(puWindow);

    puCountAttachments = 1;

    puUseFloatBuffer = vUseFloatBuffer;
    puCreateZBuffer = vUseZBuffer;

    puName = vName;
    puMaxScreenSize = vSize;
    puShaderKey = vShaderKey;
    // puCurrentFullShaderCode = LoadFile(puShaderFilePathName, FILE_LOCATION_SCRIPT);

    // load shader conf
    // LoadConfigShaderFile(puShaderFilePathName, CONFIG_TYPE_Enum::CONFIG_TYPE_SHADER);

    // CodeTree::Instance()->AddFile(vName, puShaderFilePathName);

    // ParseCurrentFullShaderCodeFile();

    if (vUseFBO) {
        puFrameBuffer = FrameBuffersPipeLine::Create(vWin, puMaxScreenSize, puSectionConfig.framebufferConfig.format, puUseFXAA, puCountFXAASamples,
                                                     puCreateZBuffer, 1, vUseRenderBuffer, puUseFloatBuffer);

        if (vTexParam != nullptr) {
            puTexParams = *vTexParam;

            // change tex parameters si il y a une config de charg�e
            ChangeTexParameters(&puTexParams);
            puTexParamCustomized = true;
        }

        if (puFrameBuffer && puFrameBuffer->load()) {
            puModel_Render = QuadModel::Create(vWin);
            UpdateCountInstances(0);
            UpdateCountPatchVertices(0);

            if (puModel_Render->IsValid()) {
                res = true;
                ParseAndCompilShader();
            }
        }
    } else {
        puModel_Render = QuadModel::Create(vWin);
        UpdateCountInstances(0);
        UpdateCountPatchVertices(0);

        if (puModel_Render->IsValid()) {
            res = true;
            ParseAndCompilShader();
        }
    }

    return res;
}

void RenderPack::SetComputeDefaultParams(const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey) {
    ZoneScoped;

    puRenderPackType = RenderPack_Type::RENDERPACK_TYPE_COMPUTE;
    puMeshType = BaseMeshEnum::PRIMITIVE_TYPE_QUAD;
    puName = vName;
    puMaxScreenSize = vSize;
    puShaderKey = vShaderKey;
    puCreateZBuffer = false;
    puUseRBuffer = false;
    puUseFBO = false;
    puUseFloatBuffer = false;
}

bool RenderPack::InitComputeWithFile(const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey) {
    ZoneScoped;

    bool res = false;

    puWindow = vWin;
    GuiBackend::Instance()->MakeContextCurrent(puWindow);

    puCountAttachments = 1;

    puUseFloatBuffer = false;
    puCreateZBuffer = false;

    puName = vName;
    puMaxScreenSize = vSize;
    puShaderKey = vShaderKey;

    res = ParseAndCompilShader();

    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RenderPack::DrawTextureParamComboBox(bool vMipMap, bool vS, bool vT, bool vMin, bool vMag) {
    ZoneScoped;

    bool change = false;

    if (!puTexParamsAlreadyInit) {
        GetTexParameters(&puTexParams);

        if (puTexParams.wrapS == (GLenum)UniformTextureWrapEnum::wrapREPEAT)
            TexParamsArrayIndex[0] = 0;
        if (puTexParams.wrapS == (GLenum)UniformTextureWrapEnum::wrapCLAMP_TO_EDGE)
            TexParamsArrayIndex[0] = 1;
        if (puTexParams.wrapS == (GLenum)UniformTextureWrapEnum::wrapMIRRORED_REPEAT)
            TexParamsArrayIndex[0] = 2;

        if (puTexParams.wrapT == (GLenum)UniformTextureWrapEnum::wrapREPEAT)
            TexParamsArrayIndex[1] = 0;
        if (puTexParams.wrapT == (GLenum)UniformTextureWrapEnum::wrapCLAMP_TO_EDGE)
            TexParamsArrayIndex[1] = 1;
        if (puTexParams.wrapT == (GLenum)UniformTextureWrapEnum::wrapMIRRORED_REPEAT)
            TexParamsArrayIndex[1] = 2;

        if (puTexParams.minFilter == (GLenum)UniformTextureMinEnum::minNEAREST)
            TexParamsArrayIndex[2] = 0;
        if (puTexParams.minFilter == (GLenum)UniformTextureMinEnum::minNEAREST_MIPMAP_NEAREST)
            TexParamsArrayIndex[2] = 1;
        if (puTexParams.minFilter == (GLenum)UniformTextureMinEnum::minLINEAR_MIPMAP_NEAREST)
            TexParamsArrayIndex[2] = 2;
        if (puTexParams.minFilter == (GLenum)UniformTextureMinEnum::minNEAREST_MIPMAP_LINEAR)
            TexParamsArrayIndex[2] = 3;
        if (puTexParams.minFilter == (GLenum)UniformTextureMinEnum::minLINEAR_MIPMAP_LINEAR)
            TexParamsArrayIndex[2] = 4;

        if (puTexParams.magFilter == (GLenum)UniformTextureMagEnum::magNEAREST)
            TexParamsArrayIndex[3] = 0;
        if (puTexParams.magFilter == (GLenum)UniformTextureMagEnum::magLINEAR)
            TexParamsArrayIndex[3] = 1;

        puTexParamsAlreadyInit = true;
    }

    if (vMipMap) {
        change |= ImGui::Checkbox("Use MipMap ?", &puTexParams.useMipMap);
    }

    if (vS) {
        change |= ImGui::ContrastedCombo(180, "S", &TexParamsArrayIndex[0], TexParamsComboString[0], -1);
    }

    if (vT) {
        change |= ImGui::ContrastedCombo(180, "T", &TexParamsArrayIndex[1], TexParamsComboString[0], -1);
    }

    if (vMin) {
        change |= ImGui::ContrastedCombo(180, "Min", &TexParamsArrayIndex[2], TexParamsComboString[1], -1);
    }

    if (vMag) {
        change |= ImGui::ContrastedCombo(150, "Mag", &TexParamsArrayIndex[3], TexParamsComboString[2], -1);
    }

    if (change) {
        if (vS) {
            if (TexParamsArrayIndex[0] == 0)
                puTexParams.wrapS = (GLenum)UniformTextureWrapEnum::wrapREPEAT;
            else if (TexParamsArrayIndex[0] == 1)
                puTexParams.wrapS = (GLenum)UniformTextureWrapEnum::wrapCLAMP_TO_EDGE;
            else if (TexParamsArrayIndex[0] == 2)
                puTexParams.wrapS = (GLenum)UniformTextureWrapEnum::wrapMIRRORED_REPEAT;
        }

        if (vT) {
            if (TexParamsArrayIndex[1] == 0)
                puTexParams.wrapT = (GLenum)UniformTextureWrapEnum::wrapREPEAT;
            else if (TexParamsArrayIndex[1] == 1)
                puTexParams.wrapT = (GLenum)UniformTextureWrapEnum::wrapCLAMP_TO_EDGE;
            else if (TexParamsArrayIndex[1] == 2)
                puTexParams.wrapT = (GLenum)UniformTextureWrapEnum::wrapMIRRORED_REPEAT;
        }

        if (vMin) {
            if (TexParamsArrayIndex[2] == 0)
                puTexParams.minFilter = (GLenum)UniformTextureMinEnum::minNEAREST;
            else if (TexParamsArrayIndex[2] == 1)
                puTexParams.minFilter = (GLenum)UniformTextureMinEnum::minNEAREST_MIPMAP_NEAREST;
            else if (TexParamsArrayIndex[2] == 2)
                puTexParams.minFilter = (GLenum)UniformTextureMinEnum::minLINEAR_MIPMAP_NEAREST;
            else if (TexParamsArrayIndex[2] == 3)
                puTexParams.minFilter = (GLenum)UniformTextureMinEnum::minNEAREST_MIPMAP_LINEAR;
            else if (TexParamsArrayIndex[2] == 4)
                puTexParams.minFilter = (GLenum)UniformTextureMinEnum::minLINEAR_MIPMAP_LINEAR;
        }

        if (vMag) {
            if (TexParamsArrayIndex[3] == 0)
                puTexParams.magFilter = (GLenum)UniformTextureMagEnum::magNEAREST;
            else if (TexParamsArrayIndex[3] == 1)
                puTexParams.magFilter = (GLenum)UniformTextureMagEnum::magLINEAR;
        }

        ChangeTexParameters(&puTexParams);
    }

    return change;
}

void RenderPack::ChangeTexParameters(TextureParamsStruct* vTexParams) {
    ZoneScoped;

    if (puFrameBuffer) {
        puFrameBuffer->ChangeTexParameters(vTexParams);
    }
}

void RenderPack::GetTexParameters(TextureParamsStruct* vTexParams) {
    ZoneScoped;

    if (puFrameBuffer) {
        puFrameBuffer->GetTexParameters(vTexParams);
    }
}

void RenderPack::ConfigureBufferParams(std::string vFormat, bool vMipMap, int vMaxMipMaplvl, std::string vWrap, std::string vFilter,
                                       bool vReloadFBO) {
    ZoneScoped;

    puTexParamCustomized = true;

    // WRAP
    GLenum wrap = GL_CLAMP_TO_EDGE;
    if (vWrap == "clamp")
        wrap = GL_CLAMP_TO_EDGE;
    else if (vWrap == "repeat")
        wrap = GL_REPEAT;
    else if (vWrap == "mirror")
        wrap = GL_MIRRORED_REPEAT;

    // MIPMAP
    GLenum min = GL_LINEAR;
    if (vMipMap) {
        if (vFilter == "linear")
            min = GL_LINEAR_MIPMAP_LINEAR;
        if (vFilter == "nearest")
            min = GL_NEAREST_MIPMAP_NEAREST;
        puTexParams.useMipMap = true;
        puTexParams.maxMipMapLvl = vMaxMipMaplvl;
    } else {
        if (vFilter == "linear")
            min = GL_LINEAR;
        else if (vFilter == "nearest")
            min = GL_NEAREST;
        puTexParams.useMipMap = false;
        puTexParams.maxMipMapLvl = 0;
    }

    // FILTER
    GLenum mag = GL_LINEAR;
    if (vFilter == "linear")
        mag = GL_LINEAR;
    if (vFilter == "nearest")
        mag = GL_NEAREST;

    // FORMAT
    GLenum _internalFormat = GL_RGBA32F;
    GLenum _format = GL_RGBA;
    GLenum _dataType = GL_FLOAT;
    if (vFormat == "float") {
        _internalFormat = GL_RGBA32F;  // si le format interne est GL_RGBA, le driver metra datatype a byte
        _format = GL_RGBA;
        _dataType = GL_FLOAT;
    } else if (vFormat == "byte") {
        _internalFormat = GL_RGBA;  // si le format interne est GL_RGBA32F, le driver metrta datatype a float
        _format = GL_RGBA;
        _dataType = GL_UNSIGNED_BYTE;
    }

    puTexParams.dataType = _dataType;
    puTexParams.format = _format;
    puTexParams.internalFormat = _internalFormat;

    puTexParams.wrapS = wrap;
    puTexParams.wrapT = wrap;
    puTexParams.minFilter = min;
    puTexParams.magFilter = mag;

    // LogVarInfo(puName + " : " + (vMipMap ? "MipMap(true)" : "MipMap(false)") + " : " + vWrap + " : " + vFilter);

    if (vReloadFBO && puFrameBuffer) {
        puFrameBuffer->UpdateTexParameters(puName, &puTexParams);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RenderPack::RenderNode(GLenum* vRenderMode, FrameBuffersPipeLinePtr vPipe, std::atomic<size_t>* vCurrentIteration, std::atomic<bool>* vWorking,
                            const bool& vDontUseAnyFBO)  // pour les threads
{
    bool res = false;

    if (puCanWeRender && puShaderKey) {
        prCountIterations = ct::maxi((uint32_t)puShaderKey->puShaderGlobalSettings.countIterations, 1U);
        prCountFramesToJump = ct::maxi((uint32_t)puShaderKey->puShaderGlobalSettings.countFramesToJump, 0U) + 1U;

        if (puFrameIdx % prCountFramesToJump == 0) {
            m_CommandBuffer.Begin(puWindow);

            TracyGpuZone("RenderNode");
            AIGPScoped(puName, "Render Node %s", puName.c_str());

            {
                if (puUseFXAA) {
                    m_CommandBuffer.BeginMultiSampling();
                }

                res = true;

                for (prCurrentIteration = 0; prCurrentIteration < prCountIterations; prCurrentIteration++) {
                    AIGPScoped(puName, "Iteration %u", prCurrentIteration);

                    if (vCurrentIteration)
                        *vCurrentIteration = (size_t)prCurrentIteration;

                    if (vWorking && !(*vWorking))
                        break;

                    if (!puBuffers.empty()) {
                        TracyGpuZone("Childs");
                        AIGPScoped(puName, "Childs");
                        for (auto it : puBuffers) {
                            if (vWorking && !(*vWorking))
                                break;

                            auto itPtr = it.lock();
                            if (itPtr) {
                                itPtr->RenderNode(&itPtr->puLastRenderMode, vPipe);
                            }
                        }
                    }

                    if (BindFBO(vPipe, vDontUseAnyFBO)) {
                        if (!vDontUseAnyFBO && ((vPipe && !puCanBeIntegratedInExternalPipeline) || !vPipe)) {
                            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        }

                        if (RenderShader(vRenderMode, vPipe)) {
                            res &= true;
                        }

                        UnBindFBO(true, true, vPipe, vDontUseAnyFBO);
                    }
                }

                m_CommandBuffer.EndMultiSampling();
            }

            m_CommandBuffer.End();
        } else {
            puFrameIdx++;
        }
    }

    return res;
}

bool RenderPack::BindFBO(FrameBuffersPipeLinePtr vPipe, const bool& vDontUseAnyFBO) {
    ZoneScoped;

    bool res = false;

    GuiBackend::Instance()->MakeContextCurrent(puWindow);

    if (puCanWeRender) {
        if (puRenderPackType != RenderPack_Type::RENDERPACK_TYPE_COMPUTE) {
            if (vDontUseAnyFBO)
                return true;

            if (vPipe && puCanBeIntegratedInExternalPipeline) {
                res = vPipe->bind();
            } else if (puFrameBuffer != nullptr) {
                res = puFrameBuffer->bind();
            }
        } else {
            res = true;
        }
    }

    return res;
}

bool RenderPack::RenderShader(GLenum* vRenderMode, FrameBuffersPipeLinePtr vPipe) {
    GuiBackend::Instance()->MakeContextCurrent(puWindow);

    TracyGpuZoneTransient(___tracy_gpu_zone, puTracyRenderShaderName.c_str(), true);
    AIGPScoped(puName, "Render Shader %s", puName.c_str());

    if (vRenderMode)
        puLastRenderMode = *vRenderMode;

    if (puCanWeRender) {
        if (puShader && puShader->IsValid()) {
            ShaderKeyPtr key = puShaderKey;
            if (key) {
                if (puExportBuffer && puRenderPackType == RenderPack_Type::RENDERPACK_TYPE_BUFFER) {
                    TransformFeedbackShader();
                }

                {
                    const int64 firstTimeMark =
                        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                    if (puRenderPackType == RenderPack_Type::RENDERPACK_TYPE_COMPUTE) {
                        ComputeShader();
                    } else {
                        PixelShader(vPipe);
                    }

                    const int64 secondTimeMark =
                        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

                    puLastRenderTime = (secondTimeMark - firstTimeMark) / 1000.0f;
                    puFrameIdx++;
                }

                return true;
            }
        }
    }

    return false;
}

void RenderPack::UploadUniforms() {
    if (puShaderKey) {
        TracyGpuZone("UploadUniforms");
        AIGPScoped(puName, "UploadUniforms");

        const bool isCompute = (puRenderPackType == RenderPack_Type::RENDERPACK_TYPE_COMPUTE);

        int textureSlotId = 0;
        for (auto it = puShaderKey->puUniformsDataBase.begin(); it != puShaderKey->puUniformsDataBase.end(); ++it) {
            UniformVariantPtr v = it->second;

            RenderPackWeak otherBuffer;

            // on choisi le buffer associé
            if (puMainRenderPack && (v->widget == "buffer" || v->widget == "compute")) {
                if (!v->bufferShaderName.empty()) {
                    otherBuffer = puMainRenderPack->puBuffers.get(v->bufferShaderName);
                } else if (!v->computeShaderName.empty()) {
                    otherBuffer = puMainRenderPack->puBuffers.get(v->computeShaderName);
                }
            }

            // on demarre le binding des uniforms avec leur valeurs
            if (v->widget == "deltatime") {
                v->x = puLastRenderTime;
            } else if (v->widget == "frame") {
                v->ix = puFrameIdx;
            } else if (v->widget == "record") {
                v->record = puRecordBuffer;
            } else if (v->widget == "depth" && puFrameBuffer) {
                v->uSampler2D = puFrameBuffer->getBackDepthTextureID();
            } else if (v->widget == "buffer") {
                if (v->bufferShaderName.empty()) {
                    v->pipe = puFrameBuffer;

                    /*if (v->glslType == uType::uTypeEnum::U_SAMPLER2D)
                    {
                        LogVarLightInfo("%i : B %s B%uT%u -> F %s B%uT%u",
                            (intptr_t)v->pipe,
                            puName.c_str(),
                            v->pipe->getBackFboID(),
                            v->pipe->getBackBuffer()->getTextureID(0),
                            puName.c_str(),
                            puFrameBuffer->getFrontFboID(),
                            puFrameBuffer->getFrontBuffer()->getTextureID(0));
                    }*/
                } else {
                    if (!otherBuffer.expired()) {
                        auto otherPtr = otherBuffer.lock();
                        if (otherPtr) {
                            v->pipe = otherPtr->GetPipe();

                            /*if (v->glslType == uType::uTypeEnum::U_SAMPLER2D)
                            {
                                LogVarLightInfo("%i : B %s B%uT%u -> F %s B%uT%u",
                                    (intptr_t)v->pipe,
                                    otherBuffer->puName.c_str(),
                                    v->pipe->getBackFboID(),
                                    v->pipe->getBackBuffer()->getTextureID(0),
                                    puName.c_str(),
                                    puFrameBuffer->getFrontFboID(),
                                    puFrameBuffer->getFrontBuffer()->getTextureID(0));
                            }*/
                        }
                    } else {
                        v->pipe = nullptr;
                    }
                }
            } else if (v->widget == "compute") {
                /*if (isCompute)
                {
                    if (v->texture_ptr)
                        v->texture_ptr->Swap();
                    if (v->volume_ptr)
                        v->volume_ptr->Swap();
                }*/

                if (v->computeShaderName.empty()) {
                    if (v->glslType == uType::uTypeEnum::U_VEC3) {
                        v->x = (float)puSectionConfig.computeConfig.size.x;
                        v->y = (float)puSectionConfig.computeConfig.size.y;
                        v->z = (float)puSectionConfig.computeConfig.size.z;
                    } else if (v->glslType == uType::uTypeEnum::U_VEC2) {
                        v->x = (float)puSectionConfig.computeConfig.size.x;
                        v->y = (float)puSectionConfig.computeConfig.size.y;
                    }
                } else {
                    if (!otherBuffer.expired()) {
                        auto otherPtr = otherBuffer.lock();
                        if (otherPtr) {
                            if (v->glslType == uType::uTypeEnum::U_VEC3) {
                                v->x = (float)otherPtr->puSectionConfig.computeConfig.size.x;
                                v->y = (float)otherPtr->puSectionConfig.computeConfig.size.y;
                                v->z = (float)otherPtr->puSectionConfig.computeConfig.size.z;
                            } else if (v->glslType == uType::uTypeEnum::U_VEC2) {
                                v->x = (float)otherPtr->puSectionConfig.computeConfig.size.x;
                                v->y = (float)otherPtr->puSectionConfig.computeConfig.size.y;
                            } else if (v->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                                if (otherPtr->GetShaderKey() && !v->target.empty()) {
                                    auto uni = otherPtr->GetShaderKey()->GetUniformByName(v->target);
                                    if (uni) {
                                        // vUniform->volume_ptr = uni->volume;
                                        // vUniform->ownVolume = false;
                                        if (uni->texture_ptr)
                                            v->uSampler2D = uni->texture_ptr->getBack()->glTex;
                                    }
                                }
                            } else if (v->glslType == uType::uTypeEnum::U_SAMPLER3D) {
                                if (otherPtr->GetShaderKey() && !v->target.empty()) {
                                    auto uni = otherPtr->GetShaderKey()->GetUniformByName(v->target);
                                    if (uni) {
                                        // vUniform->volume_ptr = uni->volume;
                                        // vUniform->ownVolume = false;
                                        if (uni->volume_ptr)
                                            v->uSampler3D = uni->volume_ptr->getBack()->glTex;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            textureSlotId = UniformHelper::UploadUniformForGlslType(puWindow, v, textureSlotId, isCompute);
            textureSlotId = SoundSystem::Instance()->UploadUniformForGlslType(puWindow, v, textureSlotId, isCompute);
        }
    }
}

bool RenderPack::TransformFeedbackShader() {
    if (puExportBuffer->puNeedCapture) {
        TracyGpuZone("Transform Feedback Shader");
        AIGPScoped(puName, "Transform Feedback Shader");

        puExportBuffer->BeforeCapture();

        puShader->Use();

        UploadUniforms();

        if (puShaderKey->puIsGeometryShaderPresent && puShaderKey->puShaderGlobalSettings.useGeometryShaderIfPresent) {
            puLastRenderMode = puGeometryInputRenderMode;
        }

        if (puShaderKey->puShaderGlobalSettings.useTesselationShaderIfPresent) {
            if (puShaderKey->puIsTesselationControlShaderPresent || puShaderKey->puIsTesselationEvalShaderPresent) {
                puLastRenderMode = GL_PATCHES;
            }
        }

        puExportBuffer->Capture(puModel_Render, puLastRenderMode);

        puShader->UnUse();

        puExportBuffer->AfterCapture(puLastRenderMode);

        return true;
    }

    return false;
}

bool RenderPack::PixelShader(FrameBuffersPipeLinePtr vPipe) {
    TracyGpuZone("Pixel Shader");
    AIGPScoped(puName, "Pixel Shader");

    puShader->Use();

    UploadUniforms();

    if (vPipe)
        vPipe->SelectBuffersTarget();
    else if (puFrameBuffer)
        puFrameBuffer->SelectBuffersTarget();

    if (puShowZBufferButton && puShaderKey->puShaderGlobalSettings.useZBuffer)
        m_CommandBuffer.BeginDepth(0.0f, 1.0f, puDepthFunc);

    if (puShowCullingButton && puShaderKey->puShaderGlobalSettings.useCulling)
        m_CommandBuffer.BeginCulling(puCullFace, puFrontFace);

    if (puShowBlendingButton && puShaderKey->puShaderGlobalSettings.useBlending)
        m_CommandBuffer.BeginBlending(puBlendSFactor, puBlendDFactor, puBlendEquation);

    if (IsUsingSmoothLine())
        m_CommandBuffer.BeginLineSmoothing(puShaderKey->puShaderGlobalSettings.lineWidth);

    if (puUsePointSize)
        m_CommandBuffer.BeginPointSize();

    if (puShaderKey->puIsGeometryShaderPresent && puShaderKey->puShaderGlobalSettings.useGeometryShaderIfPresent)
        puLastRenderMode = puGeometryInputRenderMode;

    if (puShaderKey->puShaderGlobalSettings.useTesselationShaderIfPresent)
        if (puShaderKey->puIsTesselationControlShaderPresent || puShaderKey->puIsTesselationEvalShaderPresent)
            puLastRenderMode = GL_PATCHES;

    if (puShaderKey->puShaderGlobalSettings.useTransparent)
        m_CommandBuffer.BeginTransparency();

    puModel_Render->DrawModel(puName, puLastRenderMode, puShaderKey->puShaderGlobalSettings.useTesselationShaderIfPresent);

    UpdateMipMap();

    m_CommandBuffer.EndTransparency();
    m_CommandBuffer.EndPointSize();
    m_CommandBuffer.EndLineSmoothing();
    m_CommandBuffer.EndBlending();
    m_CommandBuffer.EndCulling();
    m_CommandBuffer.EndDepth();

    puShader->UnUse();

    return true;
}

// https://stackoverflow.com/questions/37136813/what-is-the-difference-between-glbindimagetexture-and-glbindtexture
// https://zestedesavoir.com/tutoriels/1554/introduction-aux-compute-shaders/
bool RenderPack::ComputeShader() {
    TracyGpuZone("Compute Shader");
    AIGPScoped(puName, "Compute Shader");

    puShader->Use();

    UploadUniforms();

    glDispatchCompute(puSectionConfig.computeConfig.size.x, puSectionConfig.computeConfig.size.y, puSectionConfig.computeConfig.size.z);
    LogGlError();
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    LogGlError();

    // texture swap
    if (puShaderKey) {
        const bool isCompute = (puRenderPackType == RenderPack_Type::RENDERPACK_TYPE_COMPUTE);

        for (auto it = puShaderKey->puUniformsDataBase.begin(); it != puShaderKey->puUniformsDataBase.end(); ++it) {
            UniformVariantPtr v = it->second;

            if (v->widget == "compute") {
                if (isCompute) {
                    if (v->texture_ptr) {
                        v->texture_ptr->Swap();

                        if (v->glslType == uType::uTypeEnum::U_SAMPLER1D) {
                            v->uSampler1D = v->texture_ptr->getBack()->glTex;
                            v->uImage1D = v->texture_ptr->getFront()->glTex;
                        } else if (v->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                            v->uSampler2D = v->texture_ptr->getBack()->glTex;
                            v->uImage2D = v->texture_ptr->getFront()->glTex;
                        } else if (v->glslType == uType::uTypeEnum::U_SAMPLER3D) {
                            v->uSampler3D = v->texture_ptr->getBack()->glTex;
                            v->uImage3D = v->texture_ptr->getFront()->glTex;
                        }
                    }

                    if (v->volume_ptr) {
                        v->volume_ptr->Swap();

                        if (v->glslType == uType::uTypeEnum::U_SAMPLER1D) {
                            v->uSampler1D = v->volume_ptr->getBack()->glTex;
                            v->uImage1D = v->volume_ptr->getFront()->glTex;
                        } else if (v->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                            v->uSampler2D = v->volume_ptr->getBack()->glTex;
                            v->uImage2D = v->volume_ptr->getFront()->glTex;
                        } else if (v->glslType == uType::uTypeEnum::U_SAMPLER3D) {
                            v->uSampler3D = v->volume_ptr->getBack()->glTex;
                            v->uImage3D = v->volume_ptr->getFront()->glTex;
                        }
                    }

                    // unbind
                    glBindImageTexture(v->slot, 0, 0, GL_FALSE, 0, GL_READ_WRITE, v->computeTextureFormat);
                    LogGlError();

                    if (v->glslType == uType::uTypeEnum::U_SAMPLER1D) {
                        glBindTexture(GL_TEXTURE_1D, 0);
                        LogGlError();
                    } else if (v->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                        glBindTexture(GL_TEXTURE_2D, 0);
                        LogGlError();
                    } else if (v->glslType == uType::uTypeEnum::U_SAMPLER3D) {
                        glBindTexture(GL_TEXTURE_3D, 0);
                        LogGlError();
                    }
                }
            }
        }
    }

    UpdateMipMap();

    return true;
}

void RenderPack::UpdateMipMap() {
    if (puShaderKey && puFrameBuffer) {
        // puFrameBuffer->UpdateMipMaping();
    }
}

void RenderPack::UnBindFBO(const bool& vUpdateMipMap, const bool& vSwitchBuffers, FrameBuffersPipeLinePtr vPipe, const bool& vDontUseAnyFBO) {
    ZoneScoped;

    GuiBackend::Instance()->MakeContextCurrent(puWindow);

    if (puCanWeRender && puRenderPackType != RenderPack_Type::RENDERPACK_TYPE_COMPUTE) {
        if (vDontUseAnyFBO)
            return;

        if (vPipe && puCanBeIntegratedInExternalPipeline) {
            vPipe->unbind();
        } else if (puFrameBuffer) {
            if (vUpdateMipMap) {
                if (puFrameBuffer->puFrontBuffer) {
                    puFrameBuffer->puFrontBuffer->UpdateMipMaping(puName);
                }

                // puFrameBuffer->UpdateMipMaping();
            }

            puFrameBuffer->unbind();

            if (vSwitchBuffers) {
                // printf("Buffer %s : Swap front %u to back %u\n", puName.c_str(), puFrameBuffer->getFrontFboID(), puFrameBuffer->getBackFboID());

                puFrameBuffer->switchBuffers();
            }
        }
    }
}

void RenderPack::UpdatePreDefinedUniforms(std::list<UniformVariantPtr> vUniforms) {
    ZoneScoped;

    if (puCanWeRender) {
        ShaderKeyPtr key = puShaderKey;
        if (key) {
            for (auto unis = vUniforms.begin(); unis != vUniforms.end(); ++unis) {
                UniformVariantPtr uni = *unis;
                if (uni != nullptr) {
                    if (key->puUniformsDataBase.find(uni->name) != key->puUniformsDataBase.end()) {
                        UniformVariantPtr v = key->puUniformsDataBase[uni->name];
                        if (v != nullptr) {
                            if (uni->pipe)
                                v->pipe = uni->pipe;
                            v->copyValues(uni);
                        }
                    }
                }
            }

            for (auto it : puBuffers) {
                auto itPtr = it.lock();
                if (itPtr) {
                    itPtr->UpdatePreDefinedUniforms(vUniforms);
                }
            }
        }
    }
}

void RenderPack::UpdateUniforms(UpdateUniformFuncSignature vSpecificUniformsFunc, DisplayQualityType vDisplayQuality, MouseInterface* vMouse,
                                CameraInterface* vCamera, ct::ivec2 vScreenSize, MeshRectType* vMouseRect) {
    ZoneScoped;

    if (puShaderKey) {
        // if (/*vMouse->canUpdateMouse &&*/ !puBuffers.empty())
        //	printf("---------------\n");

        for (auto uni = puShaderKey->puUniformsDataBase.begin(); uni != puShaderKey->puUniformsDataBase.end(); ++uni) {
            UpdateUniform(uni->second, vDisplayQuality, vMouse, vCamera, vMouseRect);
            if (vSpecificUniformsFunc)
                vSpecificUniformsFunc(m_This, uni->second, vDisplayQuality, vMouse, vCamera, vMouseRect);
        }

        // update cam
        // if (vCamera)
        //	vCamera->UpdateIfNeeded(/*puShaderKey, */vScreenSize);

        for (auto it : puBuffers) {
            auto itPtr = it.lock();
            if (itPtr) {
                itPtr->UpdateUniforms(vSpecificUniformsFunc, vDisplayQuality, vMouse, vCamera, vScreenSize, vMouseRect);
            }
        }
    }
}

void RenderPack::UpdateUniform(UniformVariantPtr vUniPtr, DisplayQualityType vDisplayQuality, MouseInterface* vMouse, CameraInterface* vCamera,
                               MeshRectType* vMouseRect) {
    ZoneScoped;

    if (vUniPtr && vDisplayQuality > 0.0f) {
        UpdateMouseWidgets(vUniPtr, vDisplayQuality, vMouse, vCamera, vMouseRect);

        bool used = GizmoSystem::Instance()->UpdateUniforms(vUniPtr);
        used |= GamePadSystem::Instance()->UpdateUniforms(vUniPtr);
        used |= SoundSystem::Instance()->UpdateUniforms(vUniPtr);
        used |= MidiSystem::Instance()->UpdateUniforms(vUniPtr);
#ifdef USE_VR
        used |= VRBackend::Instance()->UpdateUniforms(vUniPtr);
#endif
        if (!used) {
            if (vUniPtr->widget == "checkbox" || vUniPtr->widget == "radio" || vUniPtr->widget == "button") {
                vUniPtr->x = (vUniPtr->bx ? 1.0f : 0.0f);
                vUniPtr->y = (vUniPtr->by ? 1.0f : 0.0f);
                vUniPtr->z = (vUniPtr->bz ? 1.0f : 0.0f);
                vUniPtr->w = (vUniPtr->bw ? 1.0f : 0.0f);
            } else if (vUniPtr->widget == "usegeometry")  // use geometry
            {
                if (puShaderKey) {
                    vUniPtr->bx = puShaderKey->puShaderGlobalSettings.useGeometryShaderIfPresent && puShaderKey->puIsGeometryShaderPresent;
                    vUniPtr->x = (vUniPtr->bx ? 1.0f : 0.0f);
                }
            } else if (vUniPtr->widget == "usetesscontrol")  // use tesselation control
            {
                if (puShaderKey) {
                    vUniPtr->bx =
                        puShaderKey->puShaderGlobalSettings.useTesselationShaderIfPresent && puShaderKey->puIsTesselationControlShaderPresent;
                    vUniPtr->x = (vUniPtr->bx ? 1.0f : 0.0f);
                }
            } else if (vUniPtr->widget == "usetesseval")  // use tesselation eval
            {
                if (puShaderKey) {
                    vUniPtr->bx = puShaderKey->puShaderGlobalSettings.useTesselationShaderIfPresent && puShaderKey->puIsTesselationEvalShaderPresent;
                    vUniPtr->x = (vUniPtr->bx ? 1.0f : 0.0f);
                }
            } else if (vUniPtr->widget == "maxpoints")  // free selection inside and outside thread
            {
                vUniPtr->x = 0.0f;

                if (puModel_Render != nullptr) {
                    if (puModel_Render->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_POINTS) {
                        vUniPtr->x = (float)puModel_Render->GetVerticesCount();
                    }
                }
            } else if (vUniPtr->widget == "maxinstances")  // free selection inside and outside thread
            {
                vUniPtr->x = 0.0f;

                if (puModel_Render != nullptr) {
                    vUniPtr->x = (float)puModel_Render->GetInstancesCount();
                }
            } else if (vUniPtr->widget == "picture") {
                if (vUniPtr->glslType == uType::uTypeEnum::U_VEC2) {
                    if (!vUniPtr->target.empty()) {
                        if (CodeTree::puPictureChooseUniform)  // ca veut dire que la boite de selction est ouverte donc on reset les coords
                        {
                            vUniPtr->x = 0.0f;
                            vUniPtr->y = 0.0f;
                        } else if (IS_FLOAT_EQUAL(vUniPtr->x, 0.0f) || IS_FLOAT_EQUAL(vUniPtr->y, 0.0f)) {
                            UniformVariantPtr var = puShaderKey->GetUniformByName(vUniPtr->target);
                            if (var) {
                                if (var->texture_ptr) {
                                    vUniPtr->texture_ptr = var->texture_ptr;
                                    vUniPtr->x = (float)vUniPtr->texture_ptr->getSize().x;
                                    vUniPtr->y = (float)vUniPtr->texture_ptr->getSize().y;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (vCamera
#ifdef USE_VR
            && !VRBackend::Instance()->IsInRendering()
#endif
        ) {
            if (vUniPtr->widget == "camera:mvp") {
                vUniPtr->uFloatArr = glm::value_ptr(vCamera->uCam[0]);
                vUniPtr->ownFloatArr = false;
            } else if (vUniPtr->widget == "camera:imvp") {
                vUniPtr->uFloatArr = glm::value_ptr(vCamera->uInvCam[0]);
                vUniPtr->ownFloatArr = false;
            } else if (vUniPtr->widget == "camera:m") {
                // le model contient zoom et rotation
                vUniPtr->uFloatArr = glm::value_ptr(vCamera->uModel[0]);
                vUniPtr->ownFloatArr = false;
            } else if (vUniPtr->widget == "camera:v") {
                vUniPtr->uFloatArr = glm::value_ptr(vCamera->uView[0]);
                vUniPtr->ownFloatArr = false;
            } else if (vUniPtr->widget == "camera:p") {
                vUniPtr->uFloatArr = glm::value_ptr(vCamera->uProj[0]);
                vUniPtr->ownFloatArr = false;
            } else if (vUniPtr->widget == "camera:nm") {
                vUniPtr->uFloatArr = glm::value_ptr(vCamera->uNormalMatrix[0]);
                vUniPtr->ownFloatArr = false;
            }
        }

        TimeLineSystem::Instance()->UpdateUniforms(puShaderKey, vUniPtr);
    }
}

void RenderPack::UpdateMouseWidgets(UniformVariantPtr vUniPtr, DisplayQualityType vDisplayQuality, MouseInterface* vMouse, CameraInterface* vCamera,
                                    MeshRectType* vMouseRect) {
    UNUSED(vCamera);

    ZoneScoped;

    if (vUniPtr && vDisplayQuality > 0.0f && vMouse) {
        const ct::fvec2 pos = ct::fvec2(vMouse->px, vMouse->py) / vDisplayQuality;
        const bool meshRectCatched = puMeshRect.IsContainPoint(pos);

        if (puFrameBuffer) {
            const ct::fvec2 fboSize = ct::fvec2((float)puFrameBuffer->size.x, (float)puFrameBuffer->size.y);

            if (vUniPtr->widget == "mouse:shadertoy")  // free selection inside and outside thread
            {
                // xy position de la souris
                // zw position pendant le click
                // donc quand on a le click, xy ne change pas

                // https://www.shadertoy.com/view/Mss3zH
                // mouse.xy  = mouse position during last button down
                // abs(mouse.zw) = mouse position during last button click
                // sign(mouze.z) = button is down
                // sign(mouze.w) = button is clicked

                //      mouse.xy  = mouse position during last button down
                //  abs(mouse.zw) = mouse position during last button click
                // sign(mouze.z)  = button is down
                // sign(mouze.w)  = button is clicked
                ct::fvec2 mousePos;

                if (vMouse->canUpdateMouse) {
                    if (vMouseRect) {
                        mousePos.x = vMouseRect->x * fboSize.x / vMouseRect->w;
                        mousePos.y = fboSize.y - vMouseRect->y * fboSize.y / vMouseRect->h;
                    } else if (meshRectCatched) {
                        mousePos.x = (pos.x - puMeshRect.x) * fboSize.x / puMeshRect.w;
                        mousePos.y = fboSize.y - (pos.y - puMeshRect.y) * fboSize.y / puMeshRect.h;
                    }

                    vUniPtr->sup = ct::fvec4(fboSize.x, fboSize.y, fboSize.x, fboSize.y);
                }

                if (vMouse->canUpdateMouse) {
                    if (vMouse->buttonDown[0])  // on va sauver la position du click
                    {
                        if (!vMouse->buttonDownLastFrame[0]) {
                            vUniPtr->z = mousePos.x;  // down
                            vUniPtr->w = mousePos.y;  // clicked
                        } else {
                            vUniPtr->w = -std::abs(vUniPtr->w);  // not clicked
                        }

                        vUniPtr->x = mousePos.x;
                        vUniPtr->y = mousePos.y;
                    }
                } else {
                    if (vMouse->buttonDownLastFrame[0]) {
                        vUniPtr->z = -std::abs(vUniPtr->z);
                    }
                }
            }
            if (vUniPtr->widget == "mouse:2pos_2click")  // free selection inside and outside thread
            {
                // xy position de la souris
                // zw position pendant le click
                // donc xy et zw sont pareil pendant le click

                ct::fvec2 mousePos;

                if (vMouse->canUpdateMouse) {
                    if (vMouseRect) {
                        mousePos.x = vMouseRect->x * fboSize.x / vMouseRect->w;
                        mousePos.y = fboSize.y - vMouseRect->y * fboSize.y / vMouseRect->h;
                    } else if (meshRectCatched) {
                        mousePos.x = (pos.x - puMeshRect.x) * fboSize.x / puMeshRect.w;
                        mousePos.y = fboSize.y - (pos.y - puMeshRect.y) * fboSize.y / puMeshRect.h;
                    }

                    vUniPtr->sup = ct::fvec4(fboSize.x, fboSize.y, fboSize.x, fboSize.y);
                }

                if (vMouse->buttonDown[0])  // on va sauver la position du click
                {
                    if (vMouse->canUpdateMouse) {
                        vUniPtr->x = mousePos.x;
                        vUniPtr->y = mousePos.y;
                        vUniPtr->z = mousePos.x;
                        vUniPtr->w = mousePos.y;
                    }
                } else {
                    vUniPtr->z = 0.0f;
                    vUniPtr->w = 0.0f;
                }
            }
        }

        if (vUniPtr->widget == "mouse:normalized")  // free selection inside and outside thread
        {
            if (vMouse->canUpdateMouse) {
                if (meshRectCatched) {
                    vUniPtr->x = (pos.x - puMeshRect.x) / puMeshRect.w;
                    vUniPtr->y = (puMeshRect.h - pos.y - puMeshRect.y) / puMeshRect.h;
                    vUniPtr->sup = ct::fvec4(1.0f);
                }
            }
        } else if (vUniPtr->widget == "mouse:normalized_2pos_2click")  // free selection inside and outside thread
        {
            if (vMouseRect) {
                if (vMouse->canUpdateMouse) {
                    vUniPtr->x = vMouseRect->x / vMouseRect->w;
                    vUniPtr->y = vMouseRect->y / vMouseRect->h;
                    vUniPtr->sup = ct::fvec4(1.0f);
                }

                if (vMouse->buttonDown[0])  // on va sauver la position du click
                {
                    if (vMouse->canUpdateMouse) {
                        vUniPtr->z = vUniPtr->x;
                        vUniPtr->w = vUniPtr->y;
                    }
                } else  // if (vMouse->buttonReleased[0])
                {
                    vUniPtr->z = 0.0f;
                    vUniPtr->w = 0.0f;
                }
            } else if (meshRectCatched) {
                if (vMouse->canUpdateMouse) {
                    vUniPtr->x = (pos.x - puMeshRect.x) / puMeshRect.w;
                    vUniPtr->y = (puMeshRect.h - pos.y - puMeshRect.y) / puMeshRect.h;
                    vUniPtr->sup = ct::fvec4(1.0f);
                }

                if (vMouse->buttonDown[0])  // on va sauver la position du click
                {
                    if (vMouse->canUpdateMouse) {
                        vUniPtr->z = vUniPtr->x;
                        vUniPtr->w = vUniPtr->y;
                    }
                } else  // if (vMouse->buttonReleased[0])
                {
                    vUniPtr->z = 0.0f;
                    vUniPtr->w = 0.0f;
                }
            }
        } else if (vUniPtr->widget == "mouse:normalized_2press_2click")  // free selection inside and outside thread
        {
            if (meshRectCatched) {
                if (vMouse->buttonDown[0])  // on va sauver la position du click
                {
                    if (vMouse->canUpdateMouse) {
                        vUniPtr->x = (pos.x - puMeshRect.x) / puMeshRect.w;
                        vUniPtr->y = (puMeshRect.h - pos.y - puMeshRect.y) / puMeshRect.h;
                        vUniPtr->z = vUniPtr->x;
                        vUniPtr->w = vUniPtr->y;
                        vUniPtr->sup = ct::fvec4(1.0f);
                    }
                } else  // if (vMouse->buttonReleased[0])
                {
                    vUniPtr->z = 0.0f;
                    vUniPtr->w = 0.0f;
                }
            }
        }
    }
}

void RenderPack::UpdateTimeWidgets(float vDeltaTime) {
    ZoneScoped;

    GuiBackend::Instance()->MakeContextCurrent(puWindow);

    if (puCanWeRender) {
        ShaderKeyPtr key = puShaderKey;
        if (key) {
            for (auto unis = key->puUniformsDataBase.begin(); unis != key->puUniformsDataBase.end(); ++unis) {
                auto uni = unis->second;
                if (uni.use_count()) {
                    if (uni->widget == "time") {
                        if (uni->bx) {
                            uni->x += vDeltaTime;
                            if (uni->def.y > 0.0f)
                                uni->x = fmod(uni->x, uni->def.y);
                        }
                    } else if (uni->widget == "sound") {
                        if (uni->bx) {
                            uni->x += vDeltaTime;
                        }
                    } else if (uni->widget == "date") {
                        auto now = Clock::now();
                        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
                        std::time_t now_c = Clock::to_time_t(now);
                        struct tm* parts = std::localtime(&now_c);

                        const float time_seconds =
                            (float)(parts->tm_hour * 3600 + parts->tm_min * 60 + parts->tm_sec + (float)(ms.count() % 1000) * 0.001f);

                        uni->x = (float)(1900 + parts->tm_year);
                        uni->y = (float)(1 + parts->tm_mon);
                        uni->z = (float)(parts->tm_mday);
                        uni->w = time_seconds;
                    }
                }
            }

            for (auto it : puBuffers) {
                auto itPtr = it.lock();
                if (itPtr) {
                    itPtr->UpdateTimeWidgets(vDeltaTime);
                }
            }
        }
    }
}

bool RenderPack::Resize(ct::ivec3 vNewSize, bool vForceResize) {
    ZoneScoped;

    std::string inFile;
    if (puShaderKey)
        inFile = puShaderKey->puInFileBufferName;
    UpdateSectionConfig(inFile);

    const bool res = ResizeWithFramebufferConfig(vNewSize, vForceResize, puSectionConfig.framebufferConfig);

    return res;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

bool RenderPack::GetFBOValuesAtPixelPos(ct::ivec2 vPos, ct::fvec4* vArr, int vAttachmentCount, bool vIfPosChanged) {
    ZoneScoped;

    bool res = false;

    if (puFrameBuffer) {
        if (vArr && !vPos.emptyAND()) {
            if (!vIfPosChanged || (vIfPosChanged && puLastFBOValuePosRead != vPos)) {
                vAttachmentCount = ct::mini<int>(vAttachmentCount, puZeroBasedMaxSliceBufferId + 1);
                for (int i = 0; i < vAttachmentCount; i++) {
                    puFrameBuffer->getBackBuffer()->GetRGBAValueAtPos(vPos, i, &vArr[i]);
                }

                res = true;

                puLastFBOValuePosRead = vPos;
            }
        }
    }

    return res;
}

bool RenderPack::GetFBOValuesAtNormalizedPos(ct::fvec2 vPos, ct::fvec4* vArr, int vAttachmentCount, bool vIfPosChanged) {
    ZoneScoped;

    bool res = false;

    if (puFrameBuffer) {
        if (vArr && vPos.x >= 0.0f && vPos.y >= 0.0f) {
            const ct::fvec2 p(vPos.x * (float)puFrameBuffer->size.x, vPos.y * (float)puFrameBuffer->size.y);
            res = GetFBOValuesAtPixelPos(ct::ivec2((int)p.x, (int)p.y), vArr, vAttachmentCount, vIfPosChanged);
        }
    }

    return res;
}

void RenderPack::UpdateAroundModelChange()  // when the model is change
{
    ZoneScoped;

    if (puModel_Render) {
        InitCountMeshIndicesFromModel();
    }
}

void RenderPack::SaveRenderPackConfig(CONFIG_TYPE_Enum vConfigType) {
    ZoneScoped;

    if (puShaderKey) {
        puShaderKey->SaveRenderPackConfig(vConfigType);
    }

    for (auto rp : puBuffers) {
        auto rpPtr = rp.lock();
        if (rpPtr && rpPtr->GetShaderKey()) {
            rpPtr->GetShaderKey()->SaveRenderPackConfig(vConfigType);
        }
    }
}

//////////////////////////////////////////////////

void RenderPack::Finish(bool vSaveConfig) {
    ZoneScoped;

    if (!puLoaded)
        return;

    GuiBackend::Instance()->MakeContextCurrent(puWindow);

    if (!puDontSaveConfigFiles) {
        if (vSaveConfig) {
            if (puShaderKey)
                puShaderKey->SaveRenderPackConfig(CONFIG_TYPE_Enum::CONFIG_TYPE_ALL);
        }
    }

    if (puShaderKey) {
        auto _codeTree = puShaderKey->puParentCodeTree;
        if (_codeTree && vSaveConfig) {
            _codeTree->RemoveKey(puShaderKey->puKey);
        }
    }

    puShaderKey = nullptr;

    ClearBuffers(vSaveConfig);

    puModel_Render.reset();
    puShader.reset();
    puFrameBuffer.reset();

    puLoaded = false;
}

bool RenderPack::ParseAndCompilShader(const std::string& vInFileBufferName, const GuiBackend_Window& vWin) {
    GuiBackend_Window window = vWin;
    if (!window.win) {
        window = puWindow;
    }

    GuiBackend::Instance()->MakeContextCurrent(window);

    TracyGpuZone("RenderPack::ParseAndCompilShader");

    bool res = false;

    if (puShaderKey) {
        if (!vInFileBufferName.empty())
            puShaderKey->puInFileBufferName = vInFileBufferName;

        // save shader params
        if (!puDontSaveConfigFiles) {
            if (puShader ||                                         // si il y a pas eu de shader avant on charge pas
                puShaderKey->puSyntaxErrors.puIsThereSomeErrors ||  // si un shader a été null a cause d'erreurs alors on charge la conf
                puShaderKey->puSyntaxErrors.puIsThereSomeWarnings)  // pareil pour les warnings
            {
                puShaderKey->SaveRenderPackConfig(CONFIG_TYPE_Enum::CONFIG_TYPE_ALL);
            }
        }

        // on charge la section
        puShaderKey->LoadConfigShaderFile(puShaderKey->puKey, CONFIG_TYPE_Enum::CONFIG_TYPE_SHADER, "");

        puShaderKey->GetSyntaxErrors()->clear("Compilation error :");

        const bool isCompute = (puRenderPackType == RenderPack_Type::RENDERPACK_TYPE_COMPUTE);

        if (isCompute) {
            puLastShaderCode = puShaderKey->GetComputeShader(puShaderKey->puInFileBufferName);
        } else {
            puLastShaderCode =
                puShaderKey->GetFullShader(puShaderKey->puInFileBufferName, puShaderKey->puShaderGlobalSettings.useGeometryShaderIfPresent,
                                           puShaderKey->puShaderGlobalSettings.useTesselationShaderIfPresent);
        }

        puLastShaderNote = puLastShaderCode.GetNote();

        std::string shaderName = puName;
        const auto pPath = FileHelper::Instance()->ParsePathFileName(puShaderKey->puKey);
        if (pPath.isOk) {
            shaderName = pPath.name;
        }

        ShaderPtr newPossibleShader = Shader::Create(window, shaderName);

        ProgramMsg shaderResultMsg = newPossibleShader->InitAndLinkShaderProgram(isCompute, puLastShaderCode, "", "4.3");

        puShaderKey->puSyntaxErrors.puParentKeyName = puShaderKey->puKey;
        puShaderKey->puSyntaxErrors.CompleteWithShader(puShaderKey, newPossibleShader);

        puSomeErrorsFromLastShader = puShaderKey->puSyntaxErrors.puIsThereSomeErrors || puShaderKey->puParseSuccess == ShaderMsg::SHADER_MSG_ERROR;
        puSomeWarningsFromLastShader =
            puShaderKey->puSyntaxErrors.puIsThereSomeWarnings || puShaderKey->puParseSuccess == ShaderMsg::SHADER_MSG_WARNING;

        if (!puSomeErrorsFromLastShader) {
            // onc ree les uniforms et ca va d'abord detruire les actuels qui seront donc les anciens
            // fallait etre sur que la compilation soit ok avant de faire ca
            puShaderKey->CreateUniforms(m_This);

            if (puFrameBuffer) {
                puZeroBasedMaxSliceBufferId = puSectionConfig.framebufferConfig.count - 1;
                int countBuffers = puFrameBuffer->getCountTextures();

                // on va ajouter des buffers s'il en manque
                if (puZeroBasedMaxSliceBufferId + 1 > countBuffers) {
                    bool SomeBufferAdded = false;
                    const int countBuffersToAdd = puZeroBasedMaxSliceBufferId + 1 - countBuffers;
                    for (int i = 0; i < countBuffersToAdd; i++) {
                        const int id = countBuffers + i;

                        if (id >= 0 && id < 8) {
                            puFrameBuffer->AddColorAttachmentWithoutReLoad(&puTexParams);
                            SomeBufferAdded = true;
                        }
                    }

                    if (SomeBufferAdded) {
                        puFrameBuffer->UpdatePipeline();
                    }
                }

                // on va suppprimer les buffers s'il y en a trop
                countBuffers = puFrameBuffer->getCountTextures();
                if (countBuffers > puZeroBasedMaxSliceBufferId + 1) {
                    bool SomeBufferRemoved = false;
                    const int countBuffersToRemove = countBuffers - (puZeroBasedMaxSliceBufferId + 1);
                    for (int i = 0; i < countBuffersToRemove; i++) {
                        const int id = countBuffers - i - 1;

                        if (id >= 0 && id < 8) {
                            puFrameBuffer->RemoveColorAttachment(id);
                            SomeBufferRemoved = true;
                        }
                    }

                    if (SomeBufferRemoved) {
                        puFrameBuffer->UpdatePipeline();
                    }
                }

                puFragColorNames.clear();
                for (int i = 0; i <= puZeroBasedMaxSliceBufferId; ++i) {
                    puFragColorNames.push_back(puShaderKey->GetFragColorName(i));
                }
            }

            // uniforms visibilty by sections parsing
            for (auto it = puShaderKey->puUniformsDataBase.begin(); it != puShaderKey->puUniformsDataBase.end(); ++it) {
                UniformVariantPtr v = it->second;
                if (v) {
                    v->loc = newPossibleShader->getUniformLocationForName(v->name);
                    puShaderKey->FinaliseUniformSectionParsing(v);
                } else {
                    LogVarError("Uniform = NULL");
                }
            }

            // re check les warnings evneutles apres la finalisation des uniforms
            // si ya des bug d'ecriture sur els widgets ca ne crahs pas le sahder, mais ca ne marcherai pas comme voulu, du
            // coup on genere des warnings
            puSomeWarningsFromLastShader =
                puShaderKey->puSyntaxErrors.puIsThereSomeWarnings || puShaderKey->puParseSuccess == ShaderMsg::SHADER_MSG_WARNING;

            puShader.reset();
            puShader = newPossibleShader;
            puLastCompiledShaderName = puCurrentShaderName;
            puShaderKey->LoadRenderPackConfig(CONFIG_TYPE_Enum::CONFIG_TYPE_UNIFORM);
            // puShaderKey->PrepareConfigsComboBox();
            if (!puDontSaveConfigFiles) {
                puShaderKey->SaveConfigShaderFile(puShaderKey->puKey, CONFIG_TYPE_Enum::CONFIG_TYPE_ALL, "");
            }

            res = true;
        } else {
            res = false;
        }
    }

    // erreurs ou pas on met a false, sinon le thread va valoir tester non stop
    puNewCompilationNeeded = false;

    if (res) {
        if (puFrameBuffer) {
            puFrameBuffer->GetTexParameters(&puTexParams);
        }

        if (puMainRenderPack == m_This.lock())  // so m_This is the root
        {
            puBuffers.finalize();  // on inverse la list
                                   // puSceneBuffers // les scenebuffer n'ont aps de liens, donc pas besoin de reverser
        }
    }
    return res;
}

// si on force l'update, vForceUpdateIfReplaceCodeKeyIsPresent sert a ne updater que les renderpack dont le code conteint la clef
// qui est dans vForceUpdateIfReplaceCodeKeyIsPresent, pour l'inserttion de code
bool RenderPack::UpdateShaderChanges(bool vForceUpdate, std::string /*vForceUpdateIfReplaceCodeKeyIsPresent*/) {
    ZoneScoped;

    bool res = false;

    if (puShaderKey) {
        bool IsThereSomeChange = false;

        // pour aller le plus vite possible
        // on va tester plusieurs niveau de condtions
        // du plus rapide au plus lent
        // des que le changement est confirm� on confirme directement le changement

        // auto lstChanges = puShaderKey->puParentCodeTree->CheckChanges(puShaderKey->puKey, false);
        if (puShaderKey->puCodeUpdated || vForceUpdate) {
            IsThereSomeChange = true;

            const BaseMeshEnum oldMeshType = puSectionConfig.vertexConfig.meshType;

            UpdateInfileBufferName(puShaderKey->puInFileBufferName);

            UpdateSectionConfig(puShaderKey->puInFileBufferName);

            CreateOrUpdatePipe(puSectionConfig.framebufferConfig);

            if (puSectionConfig.vertexConfig.needCountPointUpdate) {
                puCountVertices = puSectionConfig.vertexConfig.countVertexs;
                puShaderKey->puShaderGlobalSettings.countVertices = puCountVertices.w;

                puCountInstances = puSectionConfig.vertexConfig.countInstances;
                puShaderKey->puShaderGlobalSettings.countInstances = puCountInstances.w;

                puCountIterations = puSectionConfig.framebufferConfig.countIterations;
                puShaderKey->puShaderGlobalSettings.countIterations = puCountIterations.w;

                // UpdateCountVertex((uint32_t)puShaderKey->puShaderGlobalSettings.countVertices);
                // UpdateCountInstances((uint32_t)puShaderKey->puShaderGlobalSettings.countInstances);
            }

            if (puSectionConfig.vertexConfig.needModelUpdate) {
                puMeshType = puSectionConfig.vertexConfig.meshType;

                if (puModel_Render) {
                    ReloadModelIfNeeded(oldMeshType == puMeshType);
                    UpdateCountInstances((uint32_t)puShaderKey->puShaderGlobalSettings.countInstances);
                    LoadMeshFileFromVertexSection();
                }

                puSectionConfig.vertexConfig.needModelUpdate = false;
            }

            if (puSectionConfig.vertexConfig.needDisplayModeUpdate) {
                puLastRenderMode = (GLenum)puSectionConfig.vertexConfig.displayMode[puSectionConfig.vertexConfig.defaultDisplayMode];
            }
        }

        if (vForceUpdate) {
            IsThereSomeChange = true;
        }

        res = IsThereSomeChange;

        if (IsThereSomeChange) {
            PropagateCustomWidgetNamesToChilds();

            if (ParseAndCompilShader(puShaderKey->puInFileBufferName)) {
                res = true;
            }
        }

        // update childs
        for (auto it : puBuffers) {
            auto itPtr = it.lock();
            if (itPtr) {
                res |= itPtr->UpdateShaderChanges(IsThereSomeChange);
            }
        }
    }

    return res;
}

void RenderPack::ReloadModelIfNeeded(bool vNeedUpdate) {
    if (vNeedUpdate) {
        if (puModel_Render->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_POINTS) {
            puModel_Render->SetVerticesCount(puShaderKey->puShaderGlobalSettings.countVertices);
        } else if (puModel_Render->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_MESH) {
            puModel_Render->ReLoadModel();
        }
    } else {
        puModel_Render.reset();
        switch (puMeshType) {
            case BaseMeshEnum::PRIMITIVE_TYPE_POINTS:
                puModel_Render = PointModel::Create(puWindow, puShaderKey->puShaderGlobalSettings.countVertices);
                break;
            case BaseMeshEnum::PRIMITIVE_TYPE_QUAD: puModel_Render = QuadModel::Create(puWindow); break;
            case BaseMeshEnum::PRIMITIVE_TYPE_MESH: puModel_Render = PNTBTCModel::Create(puWindow); break;
            case BaseMeshEnum::PRIMITIVE_TYPE_NONE:
            default: break;
        }
    }
}

void RenderPack::CreateOrUpdatePipe(FrameBufferShaderConfigStruct vFrameBufferShaderConfigStruct) {
    if (puFrameBuffer) {
        TracyGpuZone("RenderPack::CreateOrUpdatePipe");

        if (vFrameBufferShaderConfigStruct.needSizeUpdate) {
            Resize(puMaxScreenSize, true);
            vFrameBufferShaderConfigStruct.needSizeUpdate = false;
        }
        if (vFrameBufferShaderConfigStruct.needFBOUpdate) {
            ConfigureBufferParams(vFrameBufferShaderConfigStruct.format, vFrameBufferShaderConfigStruct.mipmap, 1000,
                                  vFrameBufferShaderConfigStruct.wrap, vFrameBufferShaderConfigStruct.filter);

            vFrameBufferShaderConfigStruct.needFBOUpdate = false;
        }

        puFrameBuffer->GetTexParameters(&puTexParams);
    } else {
        if (vFrameBufferShaderConfigStruct.countChanges > 0) {
            ct::ivec3 maxSize;

            if (vFrameBufferShaderConfigStruct.size.z >
                0)  // pas normal, normalment ca devrait pas marcher le z avec un fbo, depuis la supression du layered texture mode
            {
                maxSize = ct::ivec3(GLVersionChecker::Instance()->m_OpenglInfosStruct.max3DTextureSize);
            } else {
                maxSize = ct::ivec3(GLVersionChecker::Instance()->m_OpenglInfosStruct.maxTextureSize);
            }

            if (vFrameBufferShaderConfigStruct.size.xy() > 0 && vFrameBufferShaderConfigStruct.size.xy() < maxSize.xy()) {
                puMeshRect = GetScreenRectWithSize(vFrameBufferShaderConfigStruct.size.xy(), puMaxScreenSize.xy());
                puFrameBuffer =
                    FrameBuffersPipeLine::Create(puWindow, vFrameBufferShaderConfigStruct.size, vFrameBufferShaderConfigStruct.format, puUseFXAA,
                                                 puCountFXAASamples, puCreateZBuffer, puCountAttachments, puUseRBuffer, puUseFloatBuffer);
            } else if (vFrameBufferShaderConfigStruct.ratio > 0.0f) {
                puMeshRect = GetScreenRectWithRatio(vFrameBufferShaderConfigStruct.ratio, puMaxScreenSize.xy());
                puFrameBuffer =
                    FrameBuffersPipeLine::Create(puWindow, ct::ivec3((int)puMeshRect.w, (int)puMeshRect.h, 0), vFrameBufferShaderConfigStruct.format,
                                                 puUseFXAA, puCountFXAASamples, puCreateZBuffer, puCountAttachments, puUseRBuffer, puUseFloatBuffer);
            } else {
                puMeshRect.setRect(0.0f, 0.0f, (float)puMaxScreenSize.x, (float)puMaxScreenSize.y);
                puFrameBuffer = FrameBuffersPipeLine::Create(puWindow, puMaxScreenSize, vFrameBufferShaderConfigStruct.format, puUseFXAA,
                                                             puCountFXAASamples, puCreateZBuffer, puCountAttachments, puUseRBuffer, puUseFloatBuffer);
            }
        } else {
            puMeshRect.setRect(0.0f, 0.0f, (float)puMaxScreenSize.x, (float)puMaxScreenSize.y);
            puFrameBuffer = FrameBuffersPipeLine::Create(puWindow, puMaxScreenSize, vFrameBufferShaderConfigStruct.format, puUseFXAA,
                                                         puCountFXAASamples, puCreateZBuffer, puCountAttachments, puUseRBuffer, puUseFloatBuffer);
        }
    }
}

bool RenderPack::ResizeWithFramebufferConfig(ct::ivec3 vNewSize, bool vForceResize, FrameBufferShaderConfigStruct vFrameBufferShaderConfigStruct) {
    bool res = true;

    if (puMaxScreenSize != vNewSize || vForceResize) {
        puMaxScreenSize = vNewSize;

        GuiBackend::Instance()->MakeContextCurrent(puWindow);

        TracyGpuZone("RenderPack::ResizeWithFramebufferConfig");

        if (!vNewSize.emptyAND()) {
            if (puFrameBuffer) {
                if (puFrameBuffer->size.xy() != vNewSize.xy() || vForceResize) {
                    ShaderKeyPtr key = puShaderKey;
                    if (key) {
                        if (vFrameBufferShaderConfigStruct.countChanges > 0) {
                            ct::ivec3 maxSize;

                            if (vFrameBufferShaderConfigStruct.size.z >
                                0)  // pas normal, normalment ca devrait pas marcher le z avec un fbo, depuis la supression du layered texture mode
                            {
                                maxSize = ct::ivec3(GLVersionChecker::Instance()->m_OpenglInfosStruct.max3DTextureSize);
                            } else {
                                maxSize = ct::ivec3(GLVersionChecker::Instance()->m_OpenglInfosStruct.maxTextureSize);
                            }

                            if (vFrameBufferShaderConfigStruct.size.xy() > 0 && vFrameBufferShaderConfigStruct.size.xy() < maxSize.xy()) {
                                if (puFrameBuffer->size != vFrameBufferShaderConfigStruct.size) {
                                    puMeshRect = GetScreenRectWithSize(vFrameBufferShaderConfigStruct.size.xy(), vNewSize.xy());
                                    res = puFrameBuffer->Resize(vFrameBufferShaderConfigStruct.size);
                                } else {
                                    puMeshRect = GetScreenRectWithSize(puFrameBuffer->size.xy(), vNewSize.xy());
                                }
                            } else if (vFrameBufferShaderConfigStruct.ratio > 0.0f) {
                                if (puFrameBuffer->size.xy().ratioXY<float>() != vFrameBufferShaderConfigStruct.ratio) {
                                    puMeshRect = GetScreenRectWithRatio(vFrameBufferShaderConfigStruct.ratio, vNewSize.xy());
                                    res = puFrameBuffer->Resize(ct::ivec2((int)puMeshRect.w, (int)puMeshRect.h));
                                } else {
                                    puMeshRect = GetScreenRectWithSize(puFrameBuffer->size.xy(), vNewSize.xy());
                                }
                            } else {
                                // puMeshRect = GetScreenRectWithSize(puFrameBuffer->size.xy(), vNewSize.xy());
                                // res = puFrameBuffer->Resize(vNewSize);
                                puMeshRect.setRect(0.0f, 0.0f, (float)vNewSize.x, (float)vNewSize.y);
                                res = puFrameBuffer->Resize(vNewSize);
                            }
                        } else {
                            puMeshRect.setRect(0.0f, 0.0f, (float)vNewSize.x, (float)vNewSize.y);
                            res = puFrameBuffer->Resize(vNewSize);
                        }
                    }
                }
            } else {
                puMeshRect.setRect(0.0f, 0.0f, (float)vNewSize.x, (float)vNewSize.y);
            }
        }

        for (auto it : puBuffers) {
            auto itPtr = it.lock();
            if (itPtr) {
                res &= itPtr->Resize(vNewSize, vForceResize);
            }
        }

        for (auto it : puSceneBuffers) {
            CTOOL_DEBUG_BREAK;

            auto itPtr = it.lock();
            if (itPtr) {
                res &= itPtr->Resize(vNewSize, vForceResize);
            }
        }
    }

    return res;
}

void RenderPack::ClearColor() {
    if (puFrameBuffer) {
        puFrameBuffer->UpdatePipeline();  // recreel pipeline avec les meme params
    }

    for (auto it : puBuffers) {
        auto itPtr = it.lock();
        if (itPtr) {
            itPtr->ClearColor();
        }
    }

    for (auto it : puSceneBuffers) {
        auto itPtr = it.lock();
        if (itPtr) {
            itPtr->ClearColor();
        }
    }
}

void RenderPack::SetFXAAUse(bool vUseFXAA, int vCountFXAASamples) {
    ZoneScoped;

    if (puFrameBuffer) {
        puFrameBuffer->SetFXAAUse(vUseFXAA, vCountFXAASamples);
    }
}

void RenderPack::AddCustomWidgetNameAndPropagateToChilds(uType::uTypeEnum vGlslType, const std::string& vName, int vArrayCount) {
    ZoneScoped;

    if (puShaderKey) {
        puShaderKey->AddCustomWidgetName(vGlslType, vName, vArrayCount);

        for (auto it : puBuffers) {
            auto itPtr = it.lock();
            if (itPtr) {
                itPtr->AddCustomWidgetNameAndPropagateToChilds(vGlslType, vName, vArrayCount);
            }
        }
    }
}

void RenderPack::PropagateCustomWidgetNamesToChilds() {
    ZoneScoped;

    for (auto it : puBuffers) {
        auto itPtr = it.lock();
        if (itPtr) {
            if (itPtr && itPtr->GetShaderKey()) {
                itPtr->GetShaderKey()->CompleteWithCustomWidgetsFromKey(puShaderKey);
            }
        }
    }
}

void RenderPack::ClearBuffers(bool vSaveConfig) {
    ZoneScoped;

    for (auto it : puBuffers) {
        auto itPtr = it.lock();
        if (itPtr) {
            itPtr->Finish(vSaveConfig);
            itPtr.reset();
        }
    }
    puBuffers.clear();

    for (auto it : puSceneBuffers) {
        auto itPtr = it.lock();
        if (itPtr) {
            itPtr->Finish(vSaveConfig);
            itPtr.reset();
        }
    }
    puSceneBuffers.clear();
}

bool RenderPack::DestroyChildBuffer(const std::string& vBufferId) {
    ZoneScoped;

    bool success = false;

    auto rp = puBuffers.get(vBufferId).lock();
    if (rp) {
        rp.reset();
        puBuffers.erase(vBufferId);
        success = true;
    }

    return success;
}

RenderPackPtr RenderPack::CreateChildBuffer(const std::string& vBufferId, const std::string& vBufferFileName, const std::string& vInFileBufferName) {
    UNUSED(vBufferId);

    RenderPackPtr rp = nullptr;

    TracyGpuZone("RenderPack::CreateChildBuffer");

    std::string bufferId = vBufferFileName;
    std::string bufferFilePathName;

    if (puShaderKey) {
        if (!vInFileBufferName.empty()) {
            std::string filePathName = vBufferFileName;

            if (filePathName.find(".glsl") == std::string::npos)
                filePathName += ".glsl";

            // on regarde si le buufer est pas dans le dosseir Imports
            std::string path = filePathName;
            if (!FileHelper::Instance()->IsFileExist(path, true)) {
                path = FileHelper::Instance()->GetAbsolutePathForFileLocation(filePathName, (int)FILE_LOCATION_Enum::FILE_LOCATION_IMPORT);
            }
            if (!FileHelper::Instance()->IsFileExist(path, true)) {
                path = FileHelper::Instance()->GetRelativePathToParent(filePathName, puShaderKey->GetPath(), true);
            }

            std::string newFilePath = FileHelper::Instance()->GetPathRelativeToApp(path);

            auto ps = FileHelper::Instance()->ParsePathFileName(newFilePath);
            if (ps.isOk) {
                bufferId = ps.name + "_" + vInFileBufferName;
                bufferFilePathName = ps.path + FileHelper::Instance()->puSlashType + bufferId + ".glsl";
            } else {
                bufferId = vInFileBufferName;
            }
        }

        if (bufferId != puName) {
            if (!puMainRenderPack)
                puMainRenderPack = m_This.lock();

            if (!puMainRenderPack->puBuffers.exist(bufferId)/*// non trouve
				|| !vInFileBufferName.empty()*/) // si on a un buffer in file on va recharger la key avec le code au cas ou
			{
                // probleme rel est deja une cle existante, comment pouvoir recharger avec un buffer different ?
                ShaderKeyPtr key = nullptr;

                if (!vInFileBufferName.empty()) {
                    std::string currentKeyString = puShaderKey->puMainSection->code;
                    key = puShaderKey->puParentCodeTree->AddOrUpdateFromStringAndGetKey(bufferFilePathName, currentKeyString, vBufferFileName,
                                                                                        vInFileBufferName, true, true, false);
                } else {
                    std::string filePathName = vBufferFileName;

                    if (filePathName.find(".glsl") == std::string::npos)
                        filePathName += ".glsl";

                    // on regarde si le buufer est pas dans le dosseir Imports
                    std::string path = filePathName;
                    if (!FileHelper::Instance()->IsFileExist(path, true)) {
                        path = FileHelper::Instance()->GetAbsolutePathForFileLocation(filePathName, (int)FILE_LOCATION_Enum::FILE_LOCATION_IMPORT);
                    }
                    if (!FileHelper::Instance()->IsFileExist(path, true)) {
                        path = FileHelper::Instance()->GetRelativePathToParent(filePathName, puShaderKey->GetPath(), true);
                    }

                    std::string rel = FileHelper::Instance()->GetPathRelativeToApp(path);

                    key = puShaderKey->puParentCodeTree->AddOrUpdateFromFileAndGetKey(rel, true, true, false);
                }

                if (key) {
                    key->CompleteWithCustomWidgetsFromKey(puShaderKey);

                    rp = RenderPack::createBufferWithFileWithoutLoading(
                        puWindow,         // thread
                        bufferId,         // tag
                        puMaxScreenSize,  // fbo size
                        key,              // script
                        true,             // zbuffer // pour l'opprotunite de charger du points ou du mesh il seviera pas si c'est du quads
                        true,             // fbo
                        false,            // renderbuffer
                        false             // floatbuffer
                    );

                    if (rp) {
                        rp->puShowBlendingButton = puMainRenderPack->puShowBlendingButton;
                        rp->puShowCullingButton = puMainRenderPack->puShowCullingButton;
                        rp->puShowTransparentButton = puMainRenderPack->puShowTransparentButton;
                        rp->puShowZBufferButton = puMainRenderPack->puShowZBufferButton;

                        // le sens est important

                        // en 1er
                        puMainRenderPack->puBuffers.add(bufferId, rp);

                        // en second
                        rp->puMainRenderPack = puMainRenderPack;

                        // les child n'ont pas a etre intégré avec le pipeline general, vu que ce seront des buffer et pas des points
                        rp->puCanBeIntegratedInExternalPipeline = false;

                        // on set les tex params de buffer
                        // rp->ConfigureTexParams(vAttachment, vFlip, vMipMap, vMaxMipMaplvl, vWrap, vFilter);

                        // en dernier car ca peut appeler cette fonction et creer une boucle sinon
                        rp->Load(vInFileBufferName);
                    }
                }
            } else {
                rp = puMainRenderPack->puBuffers.get(bufferId).lock();
            }
        }
    }

    return rp;
}

RenderPackPtr RenderPack::CreateChildCompute(const std::string& vBufferId, const std::string& vBufferFileName) {
    UNUSED(vBufferId);

    TracyGpuZone("RenderPack::CreateChildCompute");

    RenderPackPtr rp = nullptr;

    const std::string bufferId = vBufferFileName;

    if (puShaderKey && bufferId != puName) {
        if (puMainRenderPack == nullptr)
            puMainRenderPack = m_This.lock();

        if (!puMainRenderPack->puBuffers.exist(vBufferFileName))  // non found
        {
            std::string filePathName = vBufferFileName;

            if (filePathName.find(".glsl") == std::string::npos)
                filePathName += ".glsl";

            const std::string path = FileHelper::Instance()->GetRelativePathToParent(filePathName, puShaderKey->GetPath(), true);
            const std::string rel = FileHelper::Instance()->GetPathRelativeToApp(path);

            ShaderKeyPtr key = puShaderKey->puParentCodeTree->AddOrUpdateFromFileAndGetKey(rel, true, true, false);

            if (key) {
                key->CompleteWithCustomWidgetsFromKey(puShaderKey);

                rp = RenderPack::createComputeWithFileWithoutLoading(puWindow,         // thread
                                                                     bufferId,         // tag
                                                                     puMaxScreenSize,  // fbo size
                                                                     key);

                if (rp) {
                    rp->puShowBlendingButton = false;
                    rp->puShowCullingButton = false;
                    rp->puShowTransparentButton = false;
                    rp->puShowZBufferButton = false;

                    // le sens est important

                    // en 1er
                    puMainRenderPack->puBuffers.add(vBufferFileName, rp);

                    // les child n'ont pas a etre intégré avec le pipeline general, vu que ce seront des buffer/compute et pas des points/mesh
                    rp->puCanBeIntegratedInExternalPipeline = false;

                    // en second
                    rp->puMainRenderPack = puMainRenderPack;

                    // en dernier car ca peut appeler cette fonction et creer une boucle sinon
                    rp->Load();
                }
            }
        } else {
            rp = puMainRenderPack->puBuffers.get(vBufferFileName).lock();
        }
    }

    return rp;
}

RenderPackPtr RenderPack::CreateSceneBuffer(const std::string& vBufferId, const std::string& vBufferFileName, const std::string& vInFileBufferName) {
    UNUSED(vBufferId);

    TracyGpuZone("RenderPack::CreateSceneBuffer");

    RenderPackPtr rp = nullptr;

    std::string bufferId = vBufferFileName;
    std::string bufferFilePathName;

    if (puShaderKey) {
        if (!vInFileBufferName.empty()) {
            std::string filePathName = vBufferFileName;

            if (filePathName.find(".glsl") == std::string::npos)
                filePathName += ".glsl";

            // on regarde si le buffer est pas dans le dosseir Imports
            std::string path = filePathName;
            if (!FileHelper::Instance()->IsFileExist(path, true)) {
                path = FileHelper::Instance()->GetAbsolutePathForFileLocation(filePathName, (int)FILE_LOCATION_Enum::FILE_LOCATION_IMPORT);
            }
            if (!FileHelper::Instance()->IsFileExist(path, true)) {
                path = FileHelper::Instance()->GetRelativePathToParent(filePathName, puShaderKey->GetPath(), true);
            }

            std::string newFilePath = FileHelper::Instance()->GetPathRelativeToApp(path);

            auto ps = FileHelper::Instance()->ParsePathFileName(newFilePath);
            if (ps.isOk) {
                bufferId = ps.name + "_" + vInFileBufferName;
                bufferFilePathName = ps.path + FileHelper::Instance()->puSlashType + bufferId + ".glsl";
            } else {
                bufferId = vInFileBufferName;
            }
        }

        if (bufferId != puName) {
            if (puMainRenderPack == nullptr)
                puMainRenderPack = m_This.lock();

            if (!puMainRenderPack->puSceneBuffers.exist(bufferId)/*// non trouve
				|| !vInFileBufferName.empty()*/) // si on a un buffer in file on va recharger la key avec le code au cas ou
			{
                // probleme rel est deja une cle existante, comment pouvoir recharger avec un buffer different ?
                ShaderKeyPtr key = nullptr;

                if (!vInFileBufferName.empty()) {
                    std::string currentKeyString = puShaderKey->puMainSection->code;
                    key = puShaderKey->puParentCodeTree->AddOrUpdateFromStringAndGetKey(bufferFilePathName, currentKeyString, vBufferFileName,
                                                                                        vInFileBufferName, true, true, false);
                } else {
                    std::string filePathName = vBufferFileName;

                    if (filePathName.find(".glsl") == std::string::npos)
                        filePathName += ".glsl";

                    // on regarde si le buufer est pas dans le dosseir Imports
                    std::string path = filePathName;
                    if (!FileHelper::Instance()->IsFileExist(path, true)) {
                        path = FileHelper::Instance()->GetAbsolutePathForFileLocation(filePathName, (int)FILE_LOCATION_Enum::FILE_LOCATION_IMPORT);
                    }
                    if (!FileHelper::Instance()->IsFileExist(path, true)) {
                        path = FileHelper::Instance()->GetRelativePathToParent(filePathName, puShaderKey->GetPath(), true);
                    }

                    std::string rel = FileHelper::Instance()->GetPathRelativeToApp(path);

                    key = puShaderKey->puParentCodeTree->AddOrUpdateFromFileAndGetKey(rel, true, true, false);
                }

                if (key) {
                    key->CompleteWithCustomWidgetsFromKey(puShaderKey);

                    rp = RenderPack::createBufferWithFileWithoutLoading(
                        puWindow,         // thread
                        bufferId,         // tag
                        puMaxScreenSize,  // fbo size
                        key,              // script
                        true,             // zbuffer // pour l'opprotunite de charger du points ou du mesh il seviera pas si c'est du quads
                        true,             // fbo
                        false,            // renderbuffer
                        false             // floatbuffer
                    );

                    if (rp) {
                        rp->puShowBlendingButton = puMainRenderPack->puShowBlendingButton;
                        rp->puShowCullingButton = puMainRenderPack->puShowCullingButton;
                        rp->puShowTransparentButton = puMainRenderPack->puShowTransparentButton;
                        rp->puShowZBufferButton = puMainRenderPack->puShowZBufferButton;

                        // le sens est important

                        // en 1er
                        puMainRenderPack->puSceneBuffers.add(bufferId, rp);

                        // en secondn c'ets un buffer de scene, il n'a pas de parents
                        rp->puMainRenderPack = nullptr;

                        // on set les tex params de buffer
                        // rp->ConfigureTexParams(vAttachment, vFlip, vMipMap, vMaxMipMaplvl, vWrap, vFilter);

                        // en dernier car ca peut appeler cette fonction et creer une boucle sinon
                        rp->Load(vInFileBufferName);
                    }
                }
            } else {
                rp = puMainRenderPack->puSceneBuffers.get(bufferId).lock();
            }
        }
    }

    return rp;
}

void RenderPack::ResetFrame() {
    ZoneScoped;

    puFrameIdx = 0;

    for (auto it : puBuffers) {
        auto itPtr = it.lock();
        if (itPtr) {
            itPtr->ResetFrame();
        }
    }
}

void RenderPack::ResetTime() {
    ZoneScoped;

    ShaderKeyPtr key = puShaderKey;
    if (key) {
        std::list<UniformVariantPtr>* lst = key->GetUniformsByWidget("time");
        if (lst) {
            for (auto itLst = lst->begin(); itLst != lst->end(); ++itLst) {
                UniformVariantPtr v = *itLst;
                if (v) {
                    v->x = 0.0f;
                }
            }
        }

        for (auto it : puBuffers) {
            auto itPtr = it.lock();
            if (itPtr) {
                itPtr->ResetTime();
            }
        }
    }
}

void RenderPack::ResetSoundPlayBack() {
    ZoneScoped;

    ShaderKeyPtr key = puShaderKey;
    if (key) {
        std::list<UniformVariantPtr>* lst = key->GetUniformsByWidget("sound");
        if (lst) {
            for (auto itLst = lst->begin(); itLst != lst->end(); ++itLst) {
                UniformVariantPtr v = *itLst;
                if (v) {
                    /*if (v->sound_histo_ptr)
                    {
                        v->sound_histo_ptr->Reset();
                    }*/
                    v->x = 0.0f;
                }
            }
        }

        for (auto it : puBuffers) {
            auto itPtr = it.lock();
            if (itPtr) {
                itPtr->ResetSoundPlayBack();
            }
        }
    }
}

void RenderPack::Init_CullFace() {
    ZoneScoped;

    puMeshFrontFace = 0;
    puFrontFace = GL_CCW;

    puMeshCullFace = 0;
    puCullFace = GL_BACK;
}

bool RenderPack::DrawRenderingOptions_CullFace() {
    ZoneScoped;

    bool change = false;

    if (puShaderKey->puShaderGlobalSettings.useCulling) {
        if (ImGui::BeginFramedGroup("Culling")) {
            if (ImGui::ContrastedCombo(150, "Cull Face##MCCullFace", &puMeshCullFace, "BACK\0FRONT\0BOTH\0\0", -1)) {
                if (puMeshCullFace == 0)
                    puCullFace = GL_BACK;
                else if (puMeshCullFace == 1)
                    puCullFace = GL_FRONT;
                else if (puMeshCullFace == 2)
                    puCullFace = GL_FRONT_AND_BACK;

                change |= true;
            }
            /*}
            else
            {*/
            if (ImGui::ContrastedCombo(150, "Front Face##MCFrontFace", &puMeshFrontFace, "CCW\0CW\0\0", -1)) {
                if (puMeshFrontFace == 0)
                    puFrontFace = GL_CCW;
                else if (puMeshFrontFace == 1)
                    puFrontFace = GL_CW;

                change |= true;
            }

            ImGui::EndFramedGroup();
        }
    }

    return change;
}

bool RenderPack::DrawRenderingOptions_DepthFunc() {
    ZoneScoped;

    bool change = false;

    if (ImGui::BeginFramedGroup("Depth Buffer")) {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Depth Func");
        if (ImGui::ContrastedCombo(150, "Mode##MCDepthFunc", &puMeshDepthFunc, "LESS\0LEQUAL\0\0", -1)) {
            if (puMeshDepthFunc == 0)
                puDepthFunc = GL_LESS;
            else if (puMeshDepthFunc == 1)
                puDepthFunc = GL_LEQUAL;

            change |= true;
        }

        ImGui::EndFramedGroup();
    }

    return change;
}

void RenderPack::Init_Depth() {
    ZoneScoped;

    puMeshDepthFunc = 0;
    puDepthFunc = GL_LESS;
}

void RenderPack::Init_Blending() {
    ZoneScoped;

    puMeshBlendFuncSource = 1;
    puBlendSFactor = GL_ONE;

    puMeshBlendFuncDestination = 7;
    puBlendDFactor = GL_ONE_MINUS_SRC_ALPHA;

    puMeshBlendEquation = 0;
    puBlendEquation = GL_FUNC_ADD;
}

bool RenderPack::DrawRenderingOptions_BlendFunc_Src() {
    ZoneScoped;

    bool change = false;

    // ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Blend Src");
    if (ImGui::ContrastedCombo(
            150, "Src##MCBlendFunc", &puMeshBlendFuncSource,
            "ZERO\0ONE\0SRC_COL\0ONE_MIN_SRC_COL\0DST_COL\0ONE_MIN_DST_COL\0SRC_ALPHA\0ONE_MIN_SRC_ALPHA\0DST_ALPHA\0ONE_MIN_DST_ALPHA\0CONST_"
            "COL\0ONE_MIN_CONST_COL\0CONST_ALPHA\0ONE_MIN_CONST_ALPHA\0SRC_ALPHA_SAT\0SRC1_COL\0ONE_MIN_SRC1_COL\0SRC1_ALPHA\0ONE_MIN_SRC1_ALPHA\0\0",
            -1)) {
        if (puMeshBlendFuncSource == 0)
            puBlendSFactor = GL_ZERO;
        else if (puMeshBlendFuncSource == 1)
            puBlendSFactor = GL_ONE;
        else if (puMeshBlendFuncSource == 2)
            puBlendSFactor = GL_SRC_COLOR;
        else if (puMeshBlendFuncSource == 3)
            puBlendSFactor = GL_ONE_MINUS_SRC_COLOR;
        else if (puMeshBlendFuncSource == 4)
            puBlendSFactor = GL_DST_COLOR;
        else if (puMeshBlendFuncSource == 5)
            puBlendSFactor = GL_ONE_MINUS_DST_COLOR;
        else if (puMeshBlendFuncSource == 6)
            puBlendSFactor = GL_SRC_ALPHA;
        else if (puMeshBlendFuncSource == 7)
            puBlendSFactor = GL_ONE_MINUS_SRC_ALPHA;
        else if (puMeshBlendFuncSource == 8)
            puBlendSFactor = GL_DST_ALPHA;
        else if (puMeshBlendFuncSource == 9)
            puBlendSFactor = GL_ONE_MINUS_DST_ALPHA;
        else if (puMeshBlendFuncSource == 10)
            puBlendSFactor = GL_CONSTANT_COLOR;
        else if (puMeshBlendFuncSource == 11)
            puBlendSFactor = GL_ONE_MINUS_CONSTANT_COLOR;
        else if (puMeshBlendFuncSource == 12)
            puBlendSFactor = GL_CONSTANT_ALPHA;
        else if (puMeshBlendFuncSource == 13)
            puBlendSFactor = GL_ONE_MINUS_CONSTANT_ALPHA;
        else if (puMeshBlendFuncSource == 14)
            puBlendSFactor = GL_SRC_ALPHA_SATURATE;
        else if (puMeshBlendFuncSource == 15)
            puBlendSFactor = GL_SRC1_COLOR;
        else if (puMeshBlendFuncSource == 16)
            puBlendSFactor = GL_ONE_MINUS_SRC1_COLOR;
        else if (puMeshBlendFuncSource == 17)
            puBlendSFactor = GL_SRC1_ALPHA;
        else if (puMeshBlendFuncSource == 18)
            puBlendSFactor = GL_ONE_MINUS_SRC1_ALPHA;

        change |= true;
    }

    return change;
}

bool RenderPack::DrawRenderingOptions_BlendFunc_Dst() {
    ZoneScoped;

    bool change = false;

    // ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Blend Dst");
    if (ImGui::ContrastedCombo(
            150, "Dst##MCBlendFunc", &puMeshBlendFuncDestination,
            "ZERO\0ONE\0SRC_COL\0ONE_MIN_SRC_COL\0DST_COL\0ONE_MIN_DST_COL\0SRC_ALPHA\0ONE_MIN_SRC_ALPHA\0DST_ALPHA\0ONE_MIN_DST_ALPHA\0CONST_"
            "COL\0ONE_MIN_CONST_COL\0CONST_ALPHA\0ONE_MIN_CONST_ALPHA\0SRC_ALPHA_SAT\0SRC1_COL\0ONE_MIN_SRC1_COL\0SRC1_ALPHA\0ONE_MIN_SRC1_ALPHA\0\0",
            -1)) {
        if (puMeshBlendFuncDestination == 0)
            puBlendDFactor = GL_ZERO;
        else if (puMeshBlendFuncDestination == 1)
            puBlendDFactor = GL_ONE;
        else if (puMeshBlendFuncDestination == 2)
            puBlendDFactor = GL_SRC_COLOR;
        else if (puMeshBlendFuncDestination == 3)
            puBlendDFactor = GL_ONE_MINUS_SRC_COLOR;
        else if (puMeshBlendFuncDestination == 4)
            puBlendDFactor = GL_DST_COLOR;
        else if (puMeshBlendFuncDestination == 5)
            puBlendDFactor = GL_ONE_MINUS_DST_COLOR;
        else if (puMeshBlendFuncDestination == 6)
            puBlendDFactor = GL_SRC_ALPHA;
        else if (puMeshBlendFuncDestination == 7)
            puBlendDFactor = GL_ONE_MINUS_SRC_ALPHA;
        else if (puMeshBlendFuncDestination == 8)
            puBlendDFactor = GL_DST_ALPHA;
        else if (puMeshBlendFuncDestination == 9)
            puBlendDFactor = GL_ONE_MINUS_DST_ALPHA;
        else if (puMeshBlendFuncDestination == 10)
            puBlendDFactor = GL_CONSTANT_COLOR;
        else if (puMeshBlendFuncDestination == 11)
            puBlendDFactor = GL_ONE_MINUS_CONSTANT_COLOR;
        else if (puMeshBlendFuncDestination == 12)
            puBlendDFactor = GL_CONSTANT_ALPHA;
        else if (puMeshBlendFuncDestination == 13)
            puBlendDFactor = GL_ONE_MINUS_CONSTANT_ALPHA;
        else if (puMeshBlendFuncDestination == 14)
            puBlendDFactor = GL_SRC_ALPHA_SATURATE;
        else if (puMeshBlendFuncDestination == 15)
            puBlendDFactor = GL_SRC1_COLOR;
        else if (puMeshBlendFuncDestination == 16)
            puBlendDFactor = GL_ONE_MINUS_SRC1_COLOR;
        else if (puMeshBlendFuncDestination == 17)
            puBlendDFactor = GL_SRC1_ALPHA;
        else if (puMeshBlendFuncDestination == 18)
            puBlendDFactor = GL_ONE_MINUS_SRC1_ALPHA;

        change |= true;
    }

    return change;
}

bool RenderPack::DrawRenderingOptions_BlendFunc_Equation() {
    ZoneScoped;

    bool change = false;

    // ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Blend Equation");
    if (ImGui::ContrastedCombo(150, "Equ##MCBlendEquation", &puMeshBlendEquation, "ADD\0SUBTRACT\0REVERSE_SUBTRACT\0MIN\0MAX\0\0", -1)) {
        if (puMeshBlendEquation == 0)
            puBlendEquation = GL_FUNC_ADD;
        else if (puMeshBlendEquation == 1)
            puBlendEquation = GL_FUNC_SUBTRACT;
        else if (puMeshBlendEquation == 2)
            puBlendEquation = GL_FUNC_REVERSE_SUBTRACT;
        else if (puMeshBlendEquation == 3)
            puBlendEquation = GL_MIN;
        else if (puMeshBlendEquation == 4)
            puBlendEquation = GL_MAX;

        change |= true;
    }

    return change;
}

bool RenderPack::DrawRenderingButtons(const char* vRenderPackString) {
    ZoneScoped;

    bool configChange = false;

    if (puShaderKey) {
        if (puRenderPackType == RenderPack_Type::RENDERPACK_TYPE_COMPUTE) {
            ImGui::SameLine();
            ImGui::PushID(ImGui::IncPUSHID());
            configChange |= ImGui::Checkbox("Use ?", &puShaderKey->puShaderGlobalSettings.showFlag);
            ImGui::PopID();
        }

        if (puShaderKey->puShaderGlobalSettings.showFlag) {
            char buffer[256];

            bool _frameBegan = false;

            if (puShowBlendingButton || puShowCullingButton || puShowZBufferButton || puShowTransparentButton) {
                _frameBegan = ImGui::BeginFramedGroup("Options");
            }

            bool _buttonBefore = false;
            if (puShowBlendingButton) {
                snprintf(buffer, 256, "B##%s", vRenderPackString);
                configChange |= ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), buffer, "Blending", &puShaderKey->puShaderGlobalSettings.useBlending);
                _buttonBefore = true;
            }

            if (puShowCullingButton) {
                if (_buttonBefore)
                    ImGui::SameLine();
                snprintf(buffer, 256, "C##%s", vRenderPackString);
                configChange |= ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), buffer, "Culling", &puShaderKey->puShaderGlobalSettings.useCulling);
                _buttonBefore = true;
            }

            if (puShowZBufferButton) {
                if (_buttonBefore)
                    ImGui::SameLine();
                snprintf(buffer, 256, "Z##%s", vRenderPackString);
                configChange |= ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), buffer, "ZBuffer", &puShaderKey->puShaderGlobalSettings.useZBuffer);
                _buttonBefore = true;
            }

            if (puShowTransparentButton) {
                if (_buttonBefore)
                    ImGui::SameLine();
                snprintf(buffer, 256, "T##%s", vRenderPackString);
                configChange |=
                    ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), buffer, "Transparent", &puShaderKey->puShaderGlobalSettings.useTransparent);
                _buttonBefore = true;
            }

            if (_frameBegan) {
                ImGui::EndFramedGroup();
            }

            configChange |= DrawCountSliders();

            configChange |= DrawComboBoxs();

            configChange |= DrawBufferParams(true);

            configChange |= DrawFeedBackBufferParams();

            configChange |= DrawRenderingOptions(true);
        }
    }

    return configChange;
}

bool RenderPack::DrawCountSliders() {
    ZoneScoped;

    bool change = false;

    if (ImGui::BeginFramedGroup("Control Sliders")) {
        // iterations count
        change |= DrawCountRenderingIterationsSlider(-1.0f);

        // count frames to jump
        change |= DrawCountFramesToJumpSlider(-1.0f);

        // points count
        change |= DrawCountMeshPointsSlider(-1.0f);

        // instances count
        change |= DrawCountMeshInstancesSlider(-1.0f);

        // patch vertices count
        change |= DrawCountPatchVerticesSlider(-1.0f);

        // count indices to show
        change |= DrawCountMeshIndicesToShowSlider(-1.0f);

        ImGui::EndFramedGroup();
    }

    return change;  // le bool de retour n'est pas utilis� pour le moment
}

bool RenderPack::DrawComboBoxs() {
    ZoneScoped;

    bool configChange = false;

    char buffer[256];

    const bool _ConfigsToShow = puShaderKey->IsThereSomeConfigs() || puShaderKey->IsThereSomeSectionConfigs() ||
        puShaderKey->puIsGeometryShaderPresent || puShaderKey->puIsTesselationControlShaderPresent || puShaderKey->puIsTesselationEvalShaderPresent;

    if (_ConfigsToShow) {
        if (ImGui::BeginFramedGroup("Conifg Boxs")) {
            configChange |= puShaderKey->DrawSectionComboBox();

            // configChange |= puShaderKey->DrawConfigComboBox("UNIFORMS");

            if (puShaderKey->puIsVertexShaderPresent) {
                configChange |= puShaderKey->DrawConfigComboBox("VERTEX", puShaderKey->puInFileBufferName);
            }

            if (puShaderKey->puIsGeometryShaderPresent) {
                snprintf(buffer, 256, "Use Geometry Shader##%s", puName.c_str());
                if (ImGui::Checkbox(buffer, &puShaderKey->puShaderGlobalSettings.useGeometryShaderIfPresent)) {
                    if (puShaderKey->puShaderGlobalSettings.useGeometryShaderIfPresent) {
                        puShaderKey->SelectConfigName("GEOMETRY", puShaderKey->puInFileBufferName,
                                                      puShaderKey->GetSelectedConfigName("GEOMETRY", puShaderKey->puInFileBufferName));
                    } else {
                        puLastRenderMode = (GLenum)GetLastRenderMode_NoGeopuNoTess();
                    }
                    configChange |= true;
                }

                ImGui::Indent();

                if (puShaderKey->puShaderGlobalSettings.useGeometryShaderIfPresent) {
                    configChange |= puShaderKey->DrawConfigComboBox("GEOMETRY", puShaderKey->puInFileBufferName);
                }

                ImGui::Unindent();
            }

            if (puShaderKey->puIsTesselationControlShaderPresent || puShaderKey->puIsTesselationEvalShaderPresent) {
                snprintf(buffer, 256, "Use Tesselation Shader##%s", puName.c_str());
                if (ImGui::Checkbox(buffer, &puShaderKey->puShaderGlobalSettings.useTesselationShaderIfPresent)) {
                    if (puShaderKey->puShaderGlobalSettings.useTesselationShaderIfPresent) {
                        if (puShaderKey->puIsTesselationControlShaderPresent)
                            puShaderKey->SelectConfigName("TESSCONTROL", puShaderKey->puInFileBufferName,
                                                          puShaderKey->GetSelectedConfigName("TESSCONTROL", puShaderKey->puInFileBufferName));
                        if (puShaderKey->puIsTesselationEvalShaderPresent)
                            puShaderKey->SelectConfigName("TESSEVAL", puShaderKey->puInFileBufferName,
                                                          puShaderKey->GetSelectedConfigName("TESSEVAL", puShaderKey->puInFileBufferName));
                    } else {
                        puLastRenderMode = (GLenum)GetLastRenderMode_NoGeopuNoTess();
                    }

                    configChange |= true;
                }

                ImGui::Indent();

                if (puShaderKey->puShaderGlobalSettings.useTesselationShaderIfPresent) {
                    configChange |= puShaderKey->DrawConfigComboBox("TESSCONTROL", puShaderKey->puInFileBufferName);
                    configChange |= puShaderKey->DrawConfigComboBox("TESSEVAL", puShaderKey->puInFileBufferName);
                }

                ImGui::Unindent();
            }

            configChange |= puShaderKey->DrawConfigComboBox("FRAGMENT", puShaderKey->puInFileBufferName);

            configChange |= puShaderKey->DrawConfigComboBox("COMPUTE", puShaderKey->puInFileBufferName);

            if (configChange) {
                UpdateShaderChanges(true);

                puShaderKey->puParentCodeTree->FillIncludeFileList();
            }

            ImGui::EndFramedGroup();
        }
    }

    return configChange;
}

static std::string _bufferWrapString = "";
static std::string _bufferFilterString = "";
static std::string _bufferFormatString = "";

bool RenderPack::DrawBufferParams(bool vUseMipMapChecking) {
    ZoneScoped;

    bool configChange = false;

    if (puFrameBuffer) {
        if (ImGui::BeginFramedGroup("FrameBuffer Settings")) {
            if (vUseMipMapChecking)
                configChange |= ImGui::CheckBoxBoolDefault("Mipmap", &puTexParams.useMipMap, false);
            else
                puTexParams.useMipMap = false;

            if (puTexParams.wrapS == (GLenum)UniformTextureWrapEnum::wrapCLAMP_TO_EDGE)
                _BufferWrap = 0;
            else if (puTexParams.wrapS == (GLenum)UniformTextureWrapEnum::wrapREPEAT)
                _BufferWrap = 1;
            else if (puTexParams.wrapS == (GLenum)UniformTextureWrapEnum::wrapMIRRORED_REPEAT)
                _BufferWrap = 2;
            configChange |= ImGui::ContrastedCombo(150, "Wrap", &_BufferWrap, "clamp\0repeat\0mirror\0\0");

            if (puTexParams.minFilter == (GLenum)UniformTextureMinEnum::minLINEAR_MIPMAP_LINEAR)
                _BufferFilter = 0;
            else if (puTexParams.minFilter == (GLenum)UniformTextureMinEnum::minLINEAR)
                _BufferFilter = 0;
            else if (puTexParams.minFilter == (GLenum)UniformTextureMinEnum::minNEAREST)
                _BufferFilter = 1;
            configChange |= ImGui::ContrastedCombo(150, "Filter", &_BufferFilter, "linear\0nearest\0\0");

            if (puTexParams.dataType == (GLenum)UniformTextureFormatEnum::formatFLOAT)
                _BufferFormat = 0;
            else if (puTexParams.dataType == (GLenum)UniformTextureFormatEnum::formatBYTE)
                _BufferFormat = 1;
            configChange |= ImGui::ContrastedCombo(150, "Format", &_BufferFormat, "float\0byte\0\0");

            if (configChange) {
                puTexParamCustomized = true;

                if (_BufferWrap == 0)
                    _bufferWrapString = "clamp";
                else if (_BufferWrap == 1)
                    _bufferWrapString = "repeat";
                else if (_BufferWrap == 2)
                    _bufferWrapString = "mirror";

                if (_BufferFilter == 0)
                    _bufferFilterString = "linear";
                else if (_BufferFilter == 1)
                    _bufferFilterString = "nearest";

                if (_BufferFormat == 0)
                    _bufferFormatString = "float";
                else if (_BufferFormat == 1)
                    _bufferFormatString = "byte";

                ConfigureBufferParams(_bufferFormatString, puTexParams.useMipMap, 1000, _bufferWrapString, _bufferFilterString, true);

                puFrameBuffer->GetTexParameters(&puTexParams);
            }

            ImGui::EndFramedGroup();
        }
    }

    return configChange;
}

bool RenderPack::DrawFeedBackBufferParams() {
    ZoneScoped;

    const bool configChange = false;

    if (puRenderPackType == RenderPack_Type::RENDERPACK_TYPE_BUFFER) {
        if (ImGui::BeginFramedGroup("Transform Feedback")) {
            if (puExportBuffer) {
                puExportBuffer->DrawImGui(puWindow, puModel_Render);

                if (ImGui::ContrastedButton("Remove")) {
                    puExportBuffer.reset();
                }
            } else {
                if (ImGui::ContrastedButton("Add")) {
                    puExportBuffer = std::make_shared<ExportBuffer>();
                    puExportBuffer->InitBuffer(puModel_Render);
                }
            }

            ImGui::EndFramedGroup();
        }
    }

    return configChange;
}

ModelRenderModeEnum RenderPack::GetLastRenderMode_NoGeopuNoTess() {
    ZoneScoped;

    const auto renderMode = puSectionConfig.vertexConfig.displayMode[puDisplayModeArray[puDisplayModeArrayIndex]];
    auto dico = &puSectionConfig.vertexConfig.displayMode;
    if (dico->find(puShaderKey->puShaderGlobalSettings.displayMode) == dico->end())  // not found, we redefine it
    {
        puShaderKey->puShaderGlobalSettings.displayMode = puDisplayModeArray[puDisplayModeArrayIndex];
    }
    return renderMode;
}

bool RenderPack::DrawRenderingOptions_Model() {
    ZoneScoped;

    bool change = false;

    if (puShaderKey && puModel_Render) {
        const bool displayCond = (puDisplayModeArray.size() > 1U &&
                                  !(puShaderKey->puShaderGlobalSettings.useGeometryShaderIfPresent && puShaderKey->puIsGeometryShaderPresent) &&
                                  !(puShaderKey->puShaderGlobalSettings.useTesselationShaderIfPresent &&
                                    (puShaderKey->puIsTesselationControlShaderPresent || puShaderKey->puIsTesselationEvalShaderPresent)));
        if (puShowOpenModelButton || displayCond) {
            if (ImGui::BeginFramedGroup("Mesh Settings")) {
                if (displayCond) {
                    if (ImGui::ContrastedComboVectorDefault(150, "Display mode", &puDisplayModeArrayIndex, puDisplayModeArray,
                                                            (int)puDisplayModeArray.size(), 0)) {
                        puLastRenderMode = (GLenum)puSectionConfig.vertexConfig.displayMode[puDisplayModeArray[puDisplayModeArrayIndex]];
                        puShaderKey->puShaderGlobalSettings.displayMode = puDisplayModeArray[puDisplayModeArrayIndex];
                        change = true;
                    }
                }

                change |= m_GuiModel.DrawGui(m_This, puModel_Render, puShowOpenModelButton);

                ImGui::EndFramedGroup();
            }
        }
    }

    return change;
}

bool RenderPack::DrawRenderingOptions_FrameBuffer() {
    ZoneScoped;

    const bool change = false;

    if (puFrameBuffer) {
        if (puFrameBuffer->getCountTextures()) {
            if (ImGui::BeginFramedGroup("FrameBuffer Settings")) {
                for (int i = 0; i < puFrameBuffer->getCountTextures(); i++) {
                    if (i > 0)
                        ImGui::SameLine();
                    ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), ct::toStr(i).c_str(), "show/hide", &puBufferIdsToShow[i]);
                }

                for (int i = 0; i < puFrameBuffer->getCountTextures(); i++) {
                    if (puBufferIdsToShow[i]) {
                        ImGui::TextureOverLay(100, puFrameBuffer->getBackTexture(i).get(), ImVec4(0, 0, 0, 0), ct::toStr(i).c_str(),
                                              ImVec4(0, 0, 0, 1), ImVec4(0, 0, 0, 0));
                    }
                }

                ImGui::EndFramedGroup();
            }
        }
    }

    return change;
}

bool RenderPack::DrawRenderingOptions(bool vExpertMode) {
    ZoneScoped;

    bool change = false;

    if (puShaderKey) {
        if (puShaderKey->puShaderGlobalSettings.showFlag) {
            if (ImGui::BeginFramedGroup("Rendering")) {
                change |= ImGui::ToggleContrastedButton(ICON_NDP_PAUSE, ICON_NDP_PLAY, &puCanWeRender);
                ImGui::SameLine();
                change |=
                    ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "ext pipeline", "Can be merged in space3d", &puCanBeIntegratedInExternalPipeline);
                ImGui::EndFramedGroup();
            }

            if (puCanWeRender) {
                ImGui::Text("Shader Type : %s", puMeshTypeString.c_str());

                if (IsUsingSmoothLine()) {
                    if (vExpertMode) {
                        if (ImGui::BeginFramedGroup("Line ThickNess")) {
                            char buffer[256];
                            snprintf(buffer, 256, "Line Width##%s", puName.c_str());
                            change |= ImGui::SliderFloatDefault(150, buffer, &puShaderKey->puShaderGlobalSettings.lineWidth, 0.0f, 10.0f, 1.0f, 0.01f,
                                                                "%.2f");

                            ImGui::EndFramedGroup();
                        }
                    }
                }

                if (puShaderKey->puShaderGlobalSettings.useBlending) {
                    if (ImGui::BeginFramedGroup("Blending")) {
                        change |= DrawRenderingOptions_BlendFunc_Src();
                        change |= DrawRenderingOptions_BlendFunc_Dst();
                        change |= DrawRenderingOptions_BlendFunc_Equation();

                        ImGui::EndFramedGroup();
                    }
                }

                if (puShowCullingButton) {
                    change |= DrawRenderingOptions_CullFace();
                }

                if (puShaderKey->puShaderGlobalSettings.useZBuffer) {
                    change |= DrawRenderingOptions_DepthFunc();
                }

                change |= DrawRenderingOptions_Model();

                change |= DrawRenderingOptions_FrameBuffer();
            }
        }
    }

    return change;
}

void RenderPack::UpdateSectionConfig(const std::string& vInFileBufferName) {
    ZoneScoped;

    if (puShaderKey) {
        std::string currentConf;
        if (!puDisplayModeArray.empty() && puDisplayModeArrayIndex < (int)puDisplayModeArray.size())
            currentConf = puDisplayModeArray[puDisplayModeArrayIndex];

        puSectionConfig = puShaderKey->GetSectionConfig(vInFileBufferName);

        if (currentConf.empty())
            currentConf = puSectionConfig.vertexConfig.defaultDisplayMode;

        puDisplayModeArray.clear();
        for (const auto& mode : puSectionConfig.vertexConfig.displayMode) {
            if (mode.first == currentConf) {
                puDisplayModeArrayIndex = (int)puDisplayModeArray.size();
            }
            puDisplayModeArray.push_back(mode.first);
        }

        puDisplayModeArrayIndex = ct::mini(puDisplayModeArrayIndex, (int)puDisplayModeArray.size() - 1);
    }
}

bool RenderPack::DrawImGuiUniformWidget(float vFirstColumnWidth) {
    ZoneScoped;

    bool change = false;

    GuiBackend::Instance()->MakeContextCurrent(puWindow);

    if (puShaderKey && puCanWeRender) {
        if (puShader) {
            change |= puShaderKey->puParentCodeTree->DrawImGuiUniformWidget(puShaderKey, vFirstColumnWidth, m_This,
                                                                            puShaderKey->puShaderGlobalSettings.showUnUsedUniforms,
                                                                            puShaderKey->puShaderGlobalSettings.showCustomUniforms);
        }
    }

    return change;
}

bool RenderPack::DrawDialogsAndPopups(ct::ivec2 vScreenSize) {
    ZoneScoped;

    bool change = false;

    MeshLoader::Instance()->ShowDialog(vScreenSize);
    MeshSaver::Instance()->ShowDialog(vScreenSize);

    if (puShaderKey) {
        change |= puShaderKey->puParentCodeTree->DrawPopups(m_This);
        change |= puShaderKey->puParentCodeTree->DrawDialogs(m_This, vScreenSize);
    }

    return change;
}

bool RenderPack::CollapsingHeader(const char* vLabel, bool vForceExpand, bool vShowEditButton, bool* vEditCatched) {
    ZoneScoped;

    bool res = false;

    if (puShaderKey) {
        if (puSomeErrorsFromLastShader) {
            res = puShaderKey->puSyntaxErrors.CollapsingHeaderError(vLabel, vForceExpand, vShowEditButton, vEditCatched);
        } else if (puSomeWarningsFromLastShader) {
            res = puShaderKey->puSyntaxErrors.CollapsingHeaderWarnings(vLabel, vForceExpand, vShowEditButton, vEditCatched);
        } else {
            res = ImGui::CollapsingHeader_Button(vLabel, -1, vForceExpand, ICON_NDP2_PENCIL, vShowEditButton, vEditCatched);
        }
    }

    return res;
}

std::shared_ptr<FloatBuffer> RenderPack::GetFloatBuffer(int vAttachmentId, bool vReadFBO, int vMipMapLvl) {
    ZoneScoped;

    if (puFrameBuffer && puFrameBuffer->getBackBuffer()) {
        return puFrameBuffer->getBackBuffer()->GetFloatBufferFromColorAttachment_4_Chan(vAttachmentId, vReadFBO, vMipMapLvl);
    }

    return nullptr;
}

void RenderPack::UploadMesh(VertexStruct::P3_N3_T2_C4* vPoints, int vCountPoints, VertexStruct::I1* vIndices, int vCountIndices) {
    ZoneScoped;

    /*if (puModel_Render && puModel_Render->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_MESH)
    {
        puModel_Render->LoadModelPNTC(vPoints, vCountPoints, vIndices, vCountIndices, true);
    }*/
}

void RenderPack::UpdateCountVertex(uint32_t vCountVertexs) {
    ZoneScoped;

    if (puModel_Render && puModel_Render->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_MESH) {
        puModel_Render->SetVerticesCount(vCountVertexs);
    }
}

void RenderPack::UpdateCountIndices(uint32_t vCountIndexs) {
    ZoneScoped;

    if (puModel_Render && puModel_Render->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_MESH) {
        puModel_Render->SetIndicesCount(vCountIndexs);
    }
}

void RenderPack::UpdateCountInstances(uint32_t vCountInstances) {
    ZoneScoped;

    if (puModel_Render)
        puModel_Render->SetInstancesCount(vCountInstances);
}

void RenderPack::UpdateCountPatchVertices(uint32_t vCountPatchCountVertices) {
    ZoneScoped;

    if (puModel_Render)
        puModel_Render->SetPatchVerticesCount(vCountPatchCountVertices);
}

bool RenderPack::SaveFBOToPng(const std::string& vFilePathName, bool vFlipY, int vSubSamplesCount, ct::ivec2 vNewSize, int vAttachmentId) {
    ZoneScoped;

    if (puFrameBuffer) {
        if (vNewSize.emptyOR()) {
            return puFrameBuffer->getBackBuffer()->SaveToPng(vFilePathName, vFlipY, vSubSamplesCount, puFrameBuffer->size.xy(), vAttachmentId);
        } else {
            return puFrameBuffer->getBackBuffer()->SaveToPng(vFilePathName, vFlipY, vSubSamplesCount, vNewSize, vAttachmentId);
        }
    }

    return false;
}

bool RenderPack::SaveFBOToBmp(const std::string& vFilePathName, bool vFlipY, int vSubSamplesCount, ct::ivec2 vNewSize, int vAttachmentId) {
    ZoneScoped;

    if (puFrameBuffer) {
        if (vNewSize.emptyOR()) {
            return puFrameBuffer->getBackBuffer()->SaveToBmp(vFilePathName, vFlipY, vSubSamplesCount, puFrameBuffer->size.xy(), vAttachmentId);
        } else {
            return puFrameBuffer->getBackBuffer()->SaveToBmp(vFilePathName, vFlipY, vSubSamplesCount, vNewSize, vAttachmentId);
        }
    }

    return false;
}

bool RenderPack::SaveFBOToHdr(const std::string& vFilePathName, bool vFlipY, int vSubSamplesCount, ct::ivec2 vNewSize, int vAttachmentId) {
    ZoneScoped;

    if (puFrameBuffer) {
        if (vNewSize.emptyOR()) {
            return puFrameBuffer->getBackBuffer()->SaveToHdr(vFilePathName, vFlipY, vSubSamplesCount, puFrameBuffer->size.xy(), vAttachmentId);
        } else {
            return puFrameBuffer->getBackBuffer()->SaveToHdr(vFilePathName, vFlipY, vSubSamplesCount, vNewSize, vAttachmentId);
        }
    }

    return false;
}

bool RenderPack::SaveFBOToJpg(const std::string& vFilePathName, bool vFlipY, int vSubSamplesCount, int vQualityFrom0To100, ct::ivec2 vNewSize,
                              int vAttachmentId) {
    ZoneScoped;

    if (puFrameBuffer) {
        if (vNewSize.emptyOR()) {
            return puFrameBuffer->getBackBuffer()->SaveToJpg(vFilePathName, vFlipY, vSubSamplesCount, vQualityFrom0To100, puFrameBuffer->size.xy(),
                                                             vAttachmentId);
        } else {
            return puFrameBuffer->getBackBuffer()->SaveToJpg(vFilePathName, vFlipY, vSubSamplesCount, vQualityFrom0To100, vNewSize, vAttachmentId);
        }
    }

    return false;
}

bool RenderPack::SaveFBOToTga(const std::string& vFilePathName, bool vFlipY, int vSubSamplesCount, ct::ivec2 vNewSize, int vAttachmentId) {
    ZoneScoped;

    if (puFrameBuffer) {
        if (vNewSize.emptyOR()) {
            return puFrameBuffer->getBackBuffer()->SaveToTga(vFilePathName, vFlipY, vSubSamplesCount, puFrameBuffer->size.xy(), vAttachmentId);
        } else {
            return puFrameBuffer->getBackBuffer()->SaveToTga(vFilePathName, vFlipY, vSubSamplesCount, vNewSize, vAttachmentId);
        }
    }

    return false;
}

void RenderPack::InitCountPatchVertices() {
    ZoneScoped;

    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glPatchParameter.xhtml
    /*
    When pname is GL_PATCH_VERTICES, value specifies the number of vertices that will be used to make up a single patch primitive.
    Patch primitives are consumed by the tessellation control shader(if present) and subsequently used for tessellation.When primitives
    are specified using glDrawArrays or a similar function, each patch will be made from parameter control points, each represented by a
    vertex taken from the enabeld vertex arrays.parameter must be greater than zero, and less than or equal to the value of GL_MAX_PATCH_VERTICES.

    When pname is GL_PATCH_DEFAULT_OUTER_LEVEL or GL_PATCH_DEFAULT_INNER_LEVEL, values contains the address of an array contiaining the default
    outer or inner tessellation levels, respectively, to be used when no tessellation control shader is present.
    */
    puPatchsCountVertices = ct::uvec4(0, GLVersionChecker::Instance()->m_OpenglInfosStruct.maxPatchVertices, 3, 3);
    ChangeCountPatchVertices(puPatchsCountVertices.w);
}

void RenderPack::ChangeCountPatchVertices(uint32_t vNewvPatchCountVertices) {
    ZoneScoped;

    puPatchsCountVertices.w = ct::clamp(vNewvPatchCountVertices, puPatchsCountVertices.x, puPatchsCountVertices.y);
    if (puModel_Render) {
        puModel_Render->SetPatchVerticesCount(puPatchsCountVertices.w);
    }
}

bool RenderPack::DrawCountPatchVerticesSlider(float vWidth) {
    ZoneScoped;

    bool change = false;

    if (puShaderKey) {
        if (puShaderKey->puShaderGlobalSettings.useTesselationShaderIfPresent) {
            if (puShaderKey->puIsTesselationControlShaderPresent || puShaderKey->puIsTesselationEvalShaderPresent) {
                if (puModel_Render && puPatchsCountVertices.y > 0) {
                    ImGui::PushID(m_This.lock().get());
                    if (ImGui::SliderUIntDefaultCompact(vWidth, "Patch Vertices Count", &puPatchsCountVertices.w, puPatchsCountVertices.x,
                                                        puPatchsCountVertices.y, puPatchsCountVertices.z)) {
                        ChangeCountPatchVertices(puPatchsCountVertices.w);
                        change = true;
                    }
                    ImGui::PopID();
                }
            }
        }
    }

    return change;
}

void RenderPack::InitCountMeshIndicesFromModel() {
    ZoneScoped;

    if (puModel_Render && puModel_Render->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_MESH) {
        puCountIndicesToShow.x = 0;
        puCountIndicesToShow.y = (uint32_t)puModel_Render->GetIndicesCount();
        puCountIndicesToShow.z = puCountIndicesToShow.y;
        puCountIndicesToShow.w = puCountIndicesToShow.y;
    }
}

void RenderPack::ChangeCountMeshIndicesToShow(uint32_t vCountMeshIndicesToShow) {
    ZoneScoped;

    if (puModel_Render && puModel_Render->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_MESH && vCountMeshIndicesToShow >= 0) {
        puCountIndicesToShow.w = vCountMeshIndicesToShow;
        puModel_Render->SetIndicesCountToShow(puCountIndicesToShow.w);
    }
}

bool RenderPack::DrawCountMeshIndicesToShowSlider(float vWidth) {
    ZoneScoped;

    bool change = false;

    if (puCountIndicesToShow.y > 0) {
        ImGui::PushID(m_This.lock().get());
        if (ImGui::SliderUIntDefaultCompact(vWidth, "Indices", &puCountIndicesToShow.w, puCountIndicesToShow.x, puCountIndicesToShow.y,
                                            puCountIndicesToShow.z)) {
            puCountIndicesToShow.w = ct::clamp(puCountIndicesToShow.w, puCountIndicesToShow.x, puCountIndicesToShow.y);
            ChangeCountMeshIndicesToShow(puCountIndicesToShow.w);
            change = true;
        }
        ImGui::PopID();
    }

    return change;
}

void RenderPack::ChangeCountMeshPoints(uint32_t vNewCountPoints) {
    ZoneScoped;

    if (puModel_Render && puModel_Render->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_POINTS && vNewCountPoints > 0) {
        puShaderKey->puShaderGlobalSettings.countVertices = vNewCountPoints;
        puModel_Render->SetVerticesCount(puShaderKey->puShaderGlobalSettings.countVertices);
    }
}

bool RenderPack::DrawCountMeshPointsSlider(float vWidth) {
    ZoneScoped;

    bool change = false;

    auto& globalSettings = puShaderKey->puShaderGlobalSettings;

    if (puCountVertices.y > 0) {
        ImGui::PushID(m_This.lock().get());
        if (ImGui::SliderSizeTDefaultCompact(vWidth, "Points", &globalSettings.countVertices, puCountVertices.x, puCountVertices.y,
                                             puCountVertices.z)) {
            globalSettings.countVertices = ct::clamp((uint32_t)globalSettings.countVertices, puCountVertices.x, puCountVertices.y);
            ChangeCountMeshPoints((uint32_t)globalSettings.countVertices);
            change = true;
        }
        ImGui::PopID();
    } else {
        if (puMeshType == BaseMeshEnum::PRIMITIVE_TYPE_POINTS) {
            ImGui::Text("Count Points : %i", globalSettings.countVertices);
        }
    }

    return change;
}

void RenderPack::ChangeCountMeshInstances(uint32_t vNewCountInstances) {
    ZoneScoped;

    puShaderKey->puShaderGlobalSettings.countInstances = ct::clamp(vNewCountInstances, puCountInstances.x, puCountInstances.y);
    if (puModel_Render) {
        puModel_Render->SetInstancesCount(puShaderKey->puShaderGlobalSettings.countInstances);
    }
}

bool RenderPack::DrawCountMeshInstancesSlider(float vWidth) {
    ZoneScoped;

    bool change = false;

    if (puShaderKey && puModel_Render) {
        if (puCountInstances.y > 0) {
            ImGui::PushID(m_This.lock().get());
            if (ImGui::SliderSizeTDefaultCompact(vWidth, "Instances", &puShaderKey->puShaderGlobalSettings.countInstances, puCountInstances.x,
                                                 puCountInstances.y, puCountInstances.z)) {
                ChangeCountMeshInstances((uint32_t)puShaderKey->puShaderGlobalSettings.countInstances);
                change = true;
            }
            ImGui::PopID();
        } else {
            ImGui::Text("Count Instances : %i", puShaderKey->puShaderGlobalSettings.countInstances);
        }
    }

    return change;
}

void RenderPack::ChangeCountRenderingIterations(uint32_t vNewCountRenderingIterations) {
    ZoneScoped;

    if (puShaderKey) {
        puShaderKey->puShaderGlobalSettings.countIterations = ct::clamp(vNewCountRenderingIterations, puCountIterations.x, puCountIterations.y);
    }
}

bool RenderPack::DrawCountRenderingIterationsSlider(float vWidth) {
    ZoneScoped;

    bool change = false;

    if (puShaderKey) {
        ImGui::PushID(m_This.lock().get());
        if (ImGui::SliderSizeTDefaultCompact(vWidth, "Iterations", &puShaderKey->puShaderGlobalSettings.countIterations, puCountIterations.x,
                                             puCountIterations.y, puCountIterations.z)) {
            puShaderKey->puShaderGlobalSettings.countIterations =
                ct::clamp((uint32_t)puShaderKey->puShaderGlobalSettings.countIterations, puCountIterations.x, puCountIterations.y);
            ChangeCountRenderingIterations((uint32_t)puShaderKey->puShaderGlobalSettings.countIterations);
            change = true;
        }
        ImGui::PopID();
    }

    return change;
}

void RenderPack::ChangeCountFramesToJump(uint32_t vNewCountFramesToJump) {
    ZoneScoped;

    if (puShaderKey) {
        puShaderKey->puShaderGlobalSettings.countFramesToJump = ct::clamp(vNewCountFramesToJump, puCountFrameToJump.x, puCountFrameToJump.y);
    }
}

bool RenderPack::DrawCountFramesToJumpSlider(float vWidth) {
    ZoneScoped;

    bool change = false;

    if (puShaderKey) {
        ImGui::PushID(m_This.lock().get());
        if (ImGui::SliderSizeTDefaultCompact(vWidth, "Count Frames to Jump", &puShaderKey->puShaderGlobalSettings.countFramesToJump,
                                             puCountFrameToJump.x, puCountFrameToJump.y, puCountFrameToJump.z)) {
            puShaderKey->puShaderGlobalSettings.countFramesToJump =
                ct::clamp((uint32_t)puShaderKey->puShaderGlobalSettings.countFramesToJump, puCountFrameToJump.x, puCountFrameToJump.y);
            ChangeCountFramesToJump((uint32_t)puShaderKey->puShaderGlobalSettings.countFramesToJump);
            change = true;
        }
        ImGui::PopID();
    }

    return change;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

void RenderPack::SetLineWidth(float vThick) {
    ZoneScoped;

    puShaderKey->puShaderGlobalSettings.lineWidth = vThick;
}

float RenderPack::GetLineWidth() {
    ZoneScoped;

    return puShaderKey->puShaderGlobalSettings.lineWidth;
}

bool RenderPack::IsUsingSmoothLine() {
    ZoneScoped;

    return puGeometryOutputRenderMode == GL_LINES || puLastRenderMode == GL_LINES || puLastRenderMode == GL_LINE_LOOP ||
        puLastRenderMode == GL_LINE_STRIP;
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string RenderPack::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    ZoneScoped;

    std::string str;

    if (puModel_Render != nullptr) {
        str += vOffset + "<name value=\"" + puName + "\"/>\n";
        str += vOffset + "<size value=\"" + puMaxScreenSize.string() + "\"/>\n";

        /*for (int i = 0; i < 8; i++)
        {
            std::string texParamString;

            if (puTexParamsAlreadyInit[i])
            {
                texParamString += vOffset + "<TexParams value=\"" + ct::toStr(i) + ";";

                for (int k = 0; k < 4; k++)
                {
                    if (k > 0)
                        texParamString += ";";

                    texParamString += ct::toStr(puTexParams[i][k]);
                }

                texParamString += "\"/>\n";
            }

            if (texParamString.size())
            {
                str += texParamString;
            }
        }*/
    }

    return str;
}

bool RenderPack::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    ZoneScoped;

    // The value of m_This child identifies the name of m_This element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (!strName.empty()) {
        for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
            std::string attName = attr->Name();
            std::string attValue = attr->Value();

            if (strName == "name" && attName == "value")
                puName = attValue;
            if (strName == "size" && attName == "value")
                puMaxScreenSize = ct::ivariant(attValue).GetV3();

            /*if (strName == "TexParams" && attName == "value")
            {
                auto arr = splitStringToVector(attValue, ';');
                if (arr.size() == 5)
                {
                    int bufferId = ct::fvariant(arr[0]).GetI();
                    if (bufferId < 8)
                    {
                        puTexParamCustomized[bufferId] = true;
                        puTexParams[bufferId][0] = (GLenum)ct::fvariant(arr[1]).GetI();
                        puTexParams[bufferId][1] = (GLenum)ct::fvariant(arr[2]).GetI();
                        puTexParams[bufferId][2] = (GLenum)ct::fvariant(arr[3]).GetI();
                        puTexParams[bufferId][3] = (GLenum)ct::fvariant(arr[4]).GetI();
                    }
                }
            }*/
        }
    }

    return false;
}

///////////////////////////////////////////////////////
//// MESSAGES /////////////////////////////////////////
///////////////////////////////////////////////////////

void RenderPack::DisplayMessageOfRenderPack(bool vHideWarnings) {
    ZoneScoped;

    bool show = false;

    if (puShaderKey) {
        if (GetSyntaxErrors()) {
            show = GetSyntaxErrors()->puIsThereSomeErrors;

            if (!vHideWarnings) {
                show |= GetSyntaxErrors()->puIsThereSomeWarnings;
            }

            if (show) {
                bool editCatched = false;
                GetSyntaxErrors()->ImGui_DisplayMessages(puShaderKey, puName.c_str(), false, true, &editCatched);
                if (editCatched) {
                    puShaderKey->OpenFileKey();
                }
            }

            // buffers
            for (auto it : puBuffers) {
                auto itPtr = it.lock();
                if (itPtr) {
                    itPtr->DisplayMessageOfRenderPack(vHideWarnings);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////
//// ERRORS / WARNINGS ////////////////////////////////
///////////////////////////////////////////////////////

bool RenderPack::IsTheseSomeWarnings() {
    ZoneScoped;

    bool res = puSomeWarningsFromLastShader;

    for (auto it : puBuffers) {
        auto itPtr = it.lock();
        if (itPtr) {
            res |= itPtr->IsTheseSomeWarnings();
        }
    }

    for (auto it : puSceneBuffers) {
        auto itPtr = it.lock();
        if (itPtr) {
            res |= itPtr->IsTheseSomeErrors();
        }
    }

    return res;
}

bool RenderPack::IsTheseSomeErrors() {
    ZoneScoped;

    bool res = puSomeErrorsFromLastShader;

    for (auto it : puBuffers) {
        auto itPtr = it.lock();
        if (itPtr) {
            res |= itPtr->IsTheseSomeErrors();
        }
    }

    for (auto it : puSceneBuffers) {
        auto itPtr = it.lock();
        if (itPtr) {
            res |= itPtr->IsTheseSomeErrors();
        }
    }

    return res;
}

bool RenderPack::IsTheseSomeErrorsOrWarnings() {
    ZoneScoped;

    bool res = (puSomeErrorsFromLastShader || puSomeWarningsFromLastShader);

    for (auto it : puBuffers) {
        auto itPtr = it.lock();
        if (itPtr) {
            res |= itPtr->IsTheseSomeErrorsOrWarnings();
        }
    }

    return res;
}
