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

#include <Systems/TimeLineSystem.h>
#include <Renderer/RenderPack.h>
#include <CodeTree/ShaderKey.h>
#include <CodeTree/CodeTree.h>
#include <Gui/CustomGuiWidgets.h>
#include <ImGuiPack.h>
#include <Profiler/TracyProfiler.h>
#include <Uniforms/UniformHelper.h>
#include <Res/CustomFont.h>
#include <Res/CustomFont2.h>

// contrib
#include <imgui.h>  // https://github.com/ocornut/imgui
#ifndef IMGUI_DEFINE_MATH_OPERATORS

#endif
#include <imgui_internal.h>
// #include "IconsFontAwesome5.h"				// https://github.com/juliettef/IconFontCppHeaders

static float _PreviewRatio = 0.5f;

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

void UploadableUniform::copy(std::shared_ptr<UploadableUniform> v) {
    if (v.use_count()) {
        uniformName = v->uniformName;
        glslType    = v->glslType;

        memcpy(useFloat, v->useFloat, sizeof(float) * 4);
        memcpy(xyzw, v->xyzw, sizeof(float) * 4);
        memcpy(useBool, v->useBool, sizeof(bool) * 4);
        memcpy(useInt, v->useInt, sizeof(int) * 4);
        memcpy(ixyzw, v->ixyzw, sizeof(int) * 4);
        mat4                    = v->mat4;
        useMat                  = v->useMat;
        timeLineHandlerType     = v->timeLineHandlerType;
        bezierControlStartPoint = v->bezierControlStartPoint;
        bezierContorlEndPoint   = v->bezierContorlEndPoint;
    }
}

void UploadableUniform::copyValuesForTimeLine(UniformVariantPtr v) {
    if (v) {
        uniformName = v->name;
        glslType    = v->glslType;
        xyzw[0]     = v->x;
        xyzw[1]     = v->y;
        xyzw[2]     = v->z;
        xyzw[3]     = v->w;
        ixyzw[0]    = v->ix;
        ixyzw[1]    = v->iy;
        ixyzw[2]    = v->iz;
        ixyzw[3]    = v->iw;
        mat4        = v->mat4;
    }
}

bool UploadableUniform::GetValue(float* val) {
    bool res = false;

    if (val) {
        switch (glslType) {
            case uType::uTypeEnum::U_FLOAT:
            case uType::uTypeEnum::U_VEC2:
            case uType::uTypeEnum::U_VEC3:
            case uType::uTypeEnum::U_VEC4:
                for (size_t idx = 0U; idx < 4; ++idx) {
                    if (useFloat[idx]) {
                        *val = xyzw[idx];
                        res  = true;
                    }
                }
                break;
            case uType::uTypeEnum::U_BOOL:
            case uType::uTypeEnum::U_BVEC2:
            case uType::uTypeEnum::U_BVEC3:
            case uType::uTypeEnum::U_BVEC4:
                for (size_t idx = 0U; idx < 4; ++idx) {
                    if (useBool[idx]) {
                        *val = xyzw[idx];
                        res  = true;
                    }
                }
                break;
            case uType::uTypeEnum::U_INT:
            case uType::uTypeEnum::U_IVEC2:
            case uType::uTypeEnum::U_IVEC3:
            case uType::uTypeEnum::U_IVEC4:
                for (size_t idx = 0U; idx < 4; ++idx) {
                    if (useInt[idx]) {
                        *val = (float)ixyzw[idx];
                        res  = true;
                    }
                }
                break;
            default: break;
        }
    }
    return res;
}

bool UploadableUniform::GetValue(float* val, int vChannel) {
    bool res = false;
    if (val) {
        switch (glslType) {
            case uType::uTypeEnum::U_FLOAT:
            case uType::uTypeEnum::U_VEC2:
            case uType::uTypeEnum::U_VEC3:
            case uType::uTypeEnum::U_VEC4:
            case uType::uTypeEnum::U_BOOL:
            case uType::uTypeEnum::U_BVEC2:
            case uType::uTypeEnum::U_BVEC3:
            case uType::uTypeEnum::U_BVEC4: {
                *val = xyzw[vChannel];
                res  = true;
            } break;
            case uType::uTypeEnum::U_INT:
            case uType::uTypeEnum::U_IVEC2:
            case uType::uTypeEnum::U_IVEC3:
            case uType::uTypeEnum::U_IVEC4: {
                *val = (float)ixyzw[vChannel];
                res  = true;
            } break;
            default: break;
        }
    }
    return res;
}

bool UploadableUniform::SetValue(float val, int vChannel) {
    bool res = false;
    if (IS_FLOAT_DIFFERENT(val, 0.0f)) {
        switch (glslType) {
            case uType::uTypeEnum::U_FLOAT:
            case uType::uTypeEnum::U_VEC2:
            case uType::uTypeEnum::U_VEC3:
            case uType::uTypeEnum::U_VEC4:
            case uType::uTypeEnum::U_BOOL:
            case uType::uTypeEnum::U_BVEC2:
            case uType::uTypeEnum::U_BVEC3:
            case uType::uTypeEnum::U_BVEC4: {
                xyzw[vChannel] = val;
                res            = true;
            } break;
            case uType::uTypeEnum::U_INT:
            case uType::uTypeEnum::U_IVEC2:
            case uType::uTypeEnum::U_IVEC3:
            case uType::uTypeEnum::U_IVEC4: {
                ixyzw[vChannel] = (int)val;
                res             = true;
            } break;
            default: break;
        }
    }
    return res;
}

UploadableUniform::UploadableUniform() {
    movingFrame = 0.0f;  // sert just pour le deplecment de keys avec frame offsetés a la souris

    memset(useFloat, 0, sizeof(int) * 4);
    memset(useBool, 0, sizeof(int) * 4);
    memset(useInt, 0, sizeof(int) * 4);

    // bool selected = false;
    glslType = uType::uTypeEnum::U_VOID;
    memset(xyzw, 0, sizeof(float) * 4);
    memset(ixyzw, 0, sizeof(int) * 4);

    useMat = 0;
    mat4   = glm::mat4(1.0f);

    bezierControlStartPoint = ct::fvec2(1.0f, 0.0f);
    bezierContorlEndPoint   = ct::fvec2(1.0f, 0.0f);

    timeLineHandlerType = TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

UniformTimeKey::UniformTimeKey() {
    glslType              = uType::uTypeEnum::U_VOID;
    splineStartButtonDown = false;
    splineEndButtonDown   = false;
    currentStartButton    = 0;
    currentEndButton      = 0;
    currentAxisButton     = 0;
    // interpolationMode = TimeLineInterpolation::TIMELINE_INTERPOLATION_SPLINE;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

TimeLineSystem::TimeLineSystem() {
    ZoneScoped;

    puActive = false;

    puFrameBg         = ImVec4(0.21f, 0.29f, 0.36f, 0.0f);
    puRangeFrame      = ImVec4(0.2f, 0.2f, 0.2f, 0.4f);
    puThickLinesDark  = ImVec4(0.4f, 0.4f, 0.4f, 0.5f);
    puThickLinesLight = ImVec4(0.6f, 0.6f, 0.6f, 0.5f);

    puActiveKey = nullptr;

    puRenderingMode = RenderingModeEnum::RENDERING_MODE_PICTURES;

    // for bitmask test
    puTimeLineItemAxisMasks[0] = TimeLineItemAxis::TIMELINE_AXIS_RED;
    puTimeLineItemAxisMasks[1] = TimeLineItemAxis::TIMELINE_AXIS_GREEN;
    puTimeLineItemAxisMasks[2] = TimeLineItemAxis::TIMELINE_AXIS_BLUE;
    puTimeLineItemAxisMasks[3] = TimeLineItemAxis::TIMELINE_AXIS_ALPHA;
}

TimeLineSystem::~TimeLineSystem() {
    ZoneScoped;
}

void TimeLineSystem::ClearLocalVar() {
    ZoneScoped;

    puUseMouseSelectionType = TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_NONE;
    puMouseStartOverButton  = false;
    puMouseOverButton       = false;
    puMouseHasDragged       = false;
    puSelectedKeys.clear();
    puUniformsToEdit.clear();
}

ct::ivec4 TimeLineSystem::ShowUI(ImGuiContext* /*vContext*/, const ct::ivec4& vSize, RenderPackWeak vRenderPack, bool* vChange) {
    ZoneScoped;

    ct::ivec4 newSize = vSize;  // ct::ivec4(vSize.x, vSize.y, vSize.z, vSize.w);

    const ImVec4 color = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
    const ImGuiWindowFlags flags =
        /*ImGuiWindowFlags_NoDocking | */ ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, color);

    const auto window = ImGui::FindWindowByName("TimeLine");

    ImGui::SetNextWindowSize(ImVec2((float)newSize.z, (float)newSize.w));
    ImGui::SetNextWindowPos(ImVec2((float)newSize.x, (float)newSize.y));

    ImGui::Begin("TimeLine", (bool*)nullptr, flags);

    puFrameChanged = false;

    auto rpPtr = vRenderPack.lock();
    if (rpPtr) {
        SetActiveKey(rpPtr->GetShaderKey());
        *vChange |= DrawBar(puActiveKey, newSize.zw());
        *vChange |= DrawTimeLine("TimeLine##customui", puActiveKey);

        if (ImGui::IsKeyReleased(GuiBackend_KEY_DELETE)) {
            DelSelectedKeys(puActiveKey);
        }
    }

    if (window) {
        const float h = ImGui::GetWindowHeight();
        if (IS_FLOAT_DIFFERENT(h, (float)newSize.w)) {
            newSize.w = (int)h;
        }
    }

    ImGui::End();

    ImGui::PopStyleColor();

    return newSize;
}

void TimeLineSystem::SetActive(bool vFlag) {
    ZoneScoped;

    puActive = vFlag;
}

bool TimeLineSystem::IsActive() {
    ZoneScoped;

    return puActive;
}

bool TimeLineSystem::CanWeRecord() {
    ZoneScoped;

    return puRecord;
}

bool TimeLineSystem::IsPlaying() {
    ZoneScoped;

    return puPlayTimeLineReverse || puPlayTimeLine;
}

std::string TimeLineSystem::GetRenderingFilePathNameForCurrentFrame() {
    ZoneScoped;

    std::string file;

    if (puRenderingMode == RenderingModeEnum::RENDERING_MODE_PICTURES) {
        file = puRenderingPath + "/" + puRenderingPrefix + "_" + ct::toStr(puCurrentFrame) + ".png";
        file = FileHelper::Instance()->CorrectSlashTypeForFilePathName(file);
    }

    return file;
}

void TimeLineSystem::SetActiveKey(ShaderKeyPtr vKey) {
    ZoneScoped;

    puActiveKey = vKey;
}

ShaderKeyPtr TimeLineSystem::GetActiveKey() {
    ZoneScoped;

    return puActiveKey;
}

void TimeLineSystem::Resize(ct::ivec2 /*vNewSize*/) {
    ZoneScoped;
}

void TimeLineSystem::AddShaderKeyForCurrentFrame(ShaderKeyPtr vKey) {
    ZoneScoped;

    if (vKey) {
        // uniforms
        for (auto it = vKey->puUniformsDataBase.begin(); it != vKey->puUniformsDataBase.end(); ++it) {
            if (it->second) {
                // on vaut sauver que les uniforms qui ne sont que dans ce shader, donc pas utilis� dans des incldues
                if (vKey->puParentCodeTree->puIncludeUniformNames.find(it->second->name) == vKey->puParentCodeTree->puIncludeUniformNames.end())  // non trouvé
                {
                    UniformVariantPtr v = it->second;

                    if (v->timeLineSupported) {
                        AddKeyForCurrentFrame(vKey, v, 0);
                        if (v->count > 0) AddKeyForCurrentFrame(vKey, v, 1);
                        if (v->count > 1) AddKeyForCurrentFrame(vKey, v, 2);
                        if (v->count > 2) AddKeyForCurrentFrame(vKey, v, 3);
                    }
                }
            }
        }
    }
}

void TimeLineSystem::DelShaderKeyForCurrentFrame(ShaderKeyPtr vKey) {
    ZoneScoped;

    if (vKey) {
        // uniforms
        for (auto it = vKey->puUniformsDataBase.begin(); it != vKey->puUniformsDataBase.end(); ++it) {
            if (it->second) {
                // on vaut sauver que les uniforms qui ne sont que dans ce shader, donc pas utilis� dans des incldues
                if (vKey->puParentCodeTree->puIncludeUniformNames.find(it->second->name) == vKey->puParentCodeTree->puIncludeUniformNames.end())  // non trouvé
                {
                    UniformVariantPtr v = it->second;

                    DelKeyForCurrentFrame(vKey, v, 0);
                    if (v->count > 0) DelKeyForCurrentFrame(vKey, v, 1);
                    if (v->count > 1) DelKeyForCurrentFrame(vKey, v, 2);
                    if (v->count > 2) DelKeyForCurrentFrame(vKey, v, 3);
                }
            }
        }
    }
}

void TimeLineSystem::AddIncludeKeyForCurrentFrame(CodeTreePtr vCodeTree, std::string vIncludeKey) {
    ZoneScoped;

    if (vCodeTree && puActiveKey) {
        auto unis = vCodeTree->GetUniformsFromIncludeFileName(vIncludeKey);
        if (unis) {
            for (auto it = unis->begin(); it != unis->end(); ++it) {
                if (it->second) {
                    // on vaut sauver que les uniforms qui ne sont que dans ce shader, donc pas utilis� dans des incldues
                    UniformVariantPtr v = it->second;

                    if (v->timeLineSupported) {
                        AddKeyForCurrentFrame(puActiveKey, v, 0);
                        if (v->count > 0) AddKeyForCurrentFrame(puActiveKey, v, 1);
                        if (v->count > 1) AddKeyForCurrentFrame(puActiveKey, v, 2);
                        if (v->count > 2) AddKeyForCurrentFrame(puActiveKey, v, 3);
                    }
                }
            }
        }
    }
}

void TimeLineSystem::DelIncludeKeyForCurrentFrame(CodeTreePtr vCodeTree, std::string vIncludeKey) {
    ZoneScoped;

    if (vCodeTree && puActiveKey) {
        auto unis = vCodeTree->GetUniformsFromIncludeFileName(vIncludeKey);
        if (unis) {
            for (auto it = unis->begin(); it != unis->end(); ++it) {
                if (it->second) {
                    // on vaut sauver que les uniforms qui ne sont que dans ce shader, donc pas utilis� dans des incldues
                    UniformVariantPtr v = it->second;

                    DelKeyForCurrentFrame(puActiveKey, v, 0);
                    if (v->count > 0) DelKeyForCurrentFrame(puActiveKey, v, 1);
                    if (v->count > 1) DelKeyForCurrentFrame(puActiveKey, v, 2);
                    if (v->count > 2) DelKeyForCurrentFrame(puActiveKey, v, 3);
                }
            }
        }
    }
}

void TimeLineSystem::AddKeyForCurrentFrame(ShaderKeyPtr vKey, UniformVariantPtr v, int vComponent) {
    ZoneScoped;

    // cette focntion sert pour la creation de key
    // et la maj de key via la feature autokeying
    if (vKey && v) {
        switch (v->glslType) {
            case uType::uTypeEnum::U_BOOL:
            case uType::uTypeEnum::U_BVEC2:
            case uType::uTypeEnum::U_BVEC3:
            case uType::uTypeEnum::U_BVEC4:
            case uType::uTypeEnum::U_FLOAT:
            case uType::uTypeEnum::U_VEC2:
            case uType::uTypeEnum::U_VEC3:
            case uType::uTypeEnum::U_VEC4:
            case uType::uTypeEnum::U_INT:
            case uType::uTypeEnum::U_IVEC2:
            case uType::uTypeEnum::U_IVEC3:
            case uType::uTypeEnum::U_IVEC4:
            case uType::uTypeEnum::U_MAT2:
            case uType::uTypeEnum::U_MAT3:
            case uType::uTypeEnum::U_MAT4:
                if (v->timeLineSupported) {
                    vKey->puTimeLine.timeLine[v->name].uniformName = v->name;
                    vKey->puTimeLine.timeLine[v->name].glslType    = v->glslType;
                    vKey->puTimeLine.timeLine[v->name].widget      = v->widget;

                    const size_t countKeysBeforeInsertion = vKey->puTimeLine.timeLine[v->name].keys[vComponent].size();

                    const bool firstCreation = !IsKeyExistForFrame(vKey, v->name, vComponent, puCurrentFrame);

                    if (vKey->puTimeLine.timeLine[v->name].keys[vComponent].find(puCurrentFrame) == vKey->puTimeLine.timeLine[v->name].keys[vComponent].end()) {
                        vKey->puTimeLine.timeLine[v->name].keys[vComponent][puCurrentFrame] = std::make_shared<UploadableUniform>();
                    }

                    auto currentUploadableUniform = vKey->puTimeLine.timeLine[v->name].keys[vComponent][puCurrentFrame];

                    if (currentUploadableUniform.use_count()) {
                        currentUploadableUniform->copyValuesForTimeLine(v);
                        currentUploadableUniform->movingFrame = (float)puCurrentFrame;

                        if (firstCreation) {
                            // longueur des point de controles, fonction du scale de la vue
                            currentUploadableUniform->bezierControlStartPoint = ct::fvec2(10.0f * puTimeLineScale, 0.0f);
                            currentUploadableUniform->bezierContorlEndPoint   = ct::fvec2(10.0f * puTimeLineScale, 0.0f);

                            // on va voir si on pre calcule les tangeant du point qu'on insere
                            if (countKeysBeforeInsertion > 1) {
                                int lastFrame = -1, nextFrame = -1;
                                int _lf = 0;
                                for (auto key : vKey->puTimeLine.timeLine[v->name].keys[vComponent]) {
                                    if (key.first == puCurrentFrame) {
                                        lastFrame = _lf;
                                    }
                                    if (_lf == puCurrentFrame) {
                                        nextFrame = key.first;
                                    }

                                    _lf = key.first;
                                }

                                // on tombe entre deux point donc on va orienter le point sur les point de controle la tengeant
                                if (lastFrame > -1 && nextFrame > -1) {
                                    // ici on place la tangeante vers les point de controle d'avant et d'apres
                                    auto start = vKey->puTimeLine.timeLine[v->name].keys[vComponent][lastFrame];
                                    auto end   = vKey->puTimeLine.timeLine[v->name].keys[vComponent][nextFrame];
                                    if (start.use_count() && end.use_count()) {
                                        float sv = 0.0f;
                                        start->GetValue(&sv, vComponent);
                                        float ev = 0.0f;
                                        end->GetValue(&ev, vComponent);

                                        const ct::fvec2 p0 = ct::fvec2((float)lastFrame, sv);
                                        const ct::fvec2 p1 = p0 + start->bezierControlStartPoint;
                                        const ct::fvec2 p3 = ct::fvec2((float)nextFrame, ev);
                                        const ct::fvec2 p2 = p3 - end->bezierContorlEndPoint;

                                        const ct::fvec2 vec = (p2 - p1) * 0.3f;

                                        // redimentionnement des point de controls
                                        currentUploadableUniform->bezierControlStartPoint = vec;
                                        currentUploadableUniform->bezierContorlEndPoint   = vec;
                                    }
                                }
                            }
                        }

                        ReComputeInterpolation(vKey, v->name, vComponent);
                    }
                }
                break;
            default: break;
        }
    }
}
// void TimeLineSystem::ReSizeControlPoints(ShaderKeyPtr vKey, UniformVariantPtr v, int vComponent)

void TimeLineSystem::DelKeyForCurrentFrame(ShaderKeyPtr vKey, UniformVariantPtr v, int vComponent) {
    ZoneScoped;

    if (v) {
        DelKeyForCurrentFrame(vKey, v->name, vComponent, puCurrentFrame);
    }
}

void TimeLineSystem::DelKeyForCurrentFrame(ShaderKeyPtr vKey, std::string vUniformName, int vComponent, int vFrame) {
    ZoneScoped;

    if (vKey && !vUniformName.empty()) {
        vKey->puTimeLine.timeLine[vUniformName].keys[vComponent].erase(vFrame);

        // si pas de clef sous ce component on vire le component
        if (vKey->puTimeLine.timeLine[vUniformName].keys[vComponent].empty()) {
            DelUniformComponent(vKey, vUniformName, vComponent);
        } else {
            ReComputeInterpolation(vKey, vUniformName, vComponent);
        }

        // si plus aucune clef alors on vire l'uniform du container
        if (vKey->puTimeLine.timeLine[vUniformName].keys.empty()) {
            DelUniform(vKey, vUniformName);
        }
    }
}

void TimeLineSystem::DelUniform(ShaderKeyPtr vKey, std::string vUniformName) {
    ZoneScoped;

    if (vKey && !vUniformName.empty()) {
        vKey->puTimeLine.timeLine.erase(vUniformName);
        HideCurveForUniform(vUniformName);
    }
}

void TimeLineSystem::DelUniformComponent(ShaderKeyPtr vKey, std::string vUniformName, int vComponent) {
    ZoneScoped;

    if (vKey && !vUniformName.empty()) {
        vKey->puTimeLine.timeLine[vUniformName].keys.erase(vComponent);
        HideCurveForUniformAxis(vUniformName, vComponent);

        // si plus aucune clef alors on vire l'uniform du container
        if (vKey->puTimeLine.timeLine[vUniformName].keys.empty()) {
            DelUniform(vKey, vUniformName);
        } else {
            ReComputeInterpolation(vKey, vUniformName);
        }
    }
}

bool TimeLineSystem::IsKeyExist(ShaderKeyPtr vKey, UniformVariantPtr v, int vComponent) {
    ZoneScoped;

    bool found = false;

    if (v) {
        found = IsKeyExist(vKey, v->name, vComponent);
    }

    return found;
}

bool TimeLineSystem::IsKeyExist(ShaderKeyPtr vKey, std::string vUniformName, int vComponent) {
    ZoneScoped;

    bool found = false;

    if (vKey) {
        if (vKey->puTimeLine.timeLine.find(vUniformName) != vKey->puTimeLine.timeLine.end())  // trouvé
        {
            auto st = &vKey->puTimeLine.timeLine[vUniformName];
            if (st->keys.find(vComponent) != st->keys.end())  // trouvé
            {
                found = true;
            } else {  // non trouvé
            }
        } else {  // non trouvé
        }
    }

    return found;
}

bool TimeLineSystem::IsKeyExistForCurrentFrame(ShaderKeyPtr vKey, UniformVariantPtr v, int vComponent) {
    ZoneScoped;

    bool found = false;

    if (v) {
        found = IsKeyExistForCurrentFrame(vKey, v->name, vComponent);
    }

    return found;
}

bool TimeLineSystem::IsKeyExistForCurrentFrame(ShaderKeyPtr vKey, std::string vUniformName, int vComponent) {
    ZoneScoped;

    bool found = false;

    if (vKey) {
        if (vKey->puTimeLine.timeLine.find(vUniformName) != vKey->puTimeLine.timeLine.end())  // trouvé
        {
            auto st = &vKey->puTimeLine.timeLine[vUniformName];
            if (st->keys.find(vComponent) != st->keys.end())  // trouvé
            {
                if (st->keys[vComponent].find(puCurrentFrame) != st->keys[vComponent].end())  // trouvé
                {
                    found = true;
                } else {  // non trouvé
                }
            } else {  // non trouvé
            }
        } else {  // non trouvé
        }
    }

    return found;
}

bool TimeLineSystem::IsKeyExistForFrame(ShaderKeyPtr vKey, std::string vUniformName, int vComponent, int vFrame) {
    ZoneScoped;

    bool found = false;

    if (vKey) {
        if (vKey->puTimeLine.timeLine.find(vUniformName) != vKey->puTimeLine.timeLine.end())  // trouvé
        {
            auto st = &vKey->puTimeLine.timeLine[vUniformName];
            if (st->keys.find(vComponent) != st->keys.end())  // trouvé
            {
                if (st->keys[vComponent].find(vFrame) != st->keys[vComponent].end())  // trouvé
                {
                    found = true;
                } else {  // non trouvé
                }
            } else {  // non trouvé
            }
        } else {  // non trouvé
        }
    }

    return found;
}

void TimeLineSystem::DelSelectedKeys(ShaderKeyPtr vKey) {
    ZoneScoped;

    if (vKey) {
        std::unordered_map<std::string, std::unordered_map<int, std::list<int>>> toDelete;

        for (auto itStruct = vKey->puTimeLine.timeLine.begin(); itStruct != vKey->puTimeLine.timeLine.end(); ++itStruct) {
            std::string name = itStruct->first;
            for (auto itKey = itStruct->second.keys.begin(); itKey != itStruct->second.keys.end(); ++itKey) {
                int chan = itKey->first;
                for (auto itChan = itKey->second.begin(); itChan != itKey->second.end(); ++itChan) {
                    int frame     = itChan->first;
                    const auto st = itChan->second;
                    if (st.use_count()) {
                        if (IsKeyInSelection(st)) {
                            toDelete[name][chan].emplace_back(frame);
                        }
                    }
                }
            }
        }

        // on va virer les clefs trouvé avant
        for (auto itStruct = toDelete.begin(); itStruct != toDelete.end(); ++itStruct) {
            const std::string name = itStruct->first;
            for (auto itKey = itStruct->second.begin(); itKey != itStruct->second.end(); ++itKey) {
                const int chan = itKey->first;
                for (auto itFrame = itKey->second.begin(); itFrame != itKey->second.end(); ++itFrame) {
                    const int frame = *itFrame;
                    DelKeyForCurrentFrame(vKey, name, chan, frame);
                }
            }
        }
        toDelete.clear();
    }
}

void TimeLineSystem::DeselectAllKeys(ShaderKeyPtr vKey) {
    ZoneScoped;

    if (vKey) {
        for (auto itStruct = vKey->puTimeLine.timeLine.begin(); itStruct != vKey->puTimeLine.timeLine.end(); ++itStruct) {
            //std::string name = itStruct->first;
            for (auto itKey = itStruct->second.keys.begin(); itKey != itStruct->second.keys.end(); ++itKey) {
                // int chan = itKey->first;
                for (auto itChan = itKey->second.begin(); itChan != itKey->second.end(); ++itChan) {
                    // int frame = itChan->first;
                    const auto st = itChan->second;
                    if (st.use_count()) {
                        RemoveKeyFromSelection(st);
                    }
                }
            }
        }

        puSelectedKeys.clear();
    }
}

void TimeLineSystem::SelectAllKeysInMouseSelectionRect(ShaderKeyPtr vKey, ImRect vZone) {
    ZoneScoped;

    if (vKey) {
        ct::fAABB rc(1e5f, -1e5f);
        rc.Combine(puStartMouseClick);
        rc.Combine(puEndMouseClick);

        for (auto itStruct = vKey->puTimeLine.timeLine.begin(); itStruct != vKey->puTimeLine.timeLine.end(); ++itStruct) {
            //std::string name = itStruct->first;
            for (auto itKey = itStruct->second.keys.begin(); itKey != itStruct->second.keys.end(); ++itKey) {
                const int chan = itKey->first;
                for (auto itChan = itKey->second.begin(); itChan != itKey->second.end(); ++itChan) {
                    const int frame = itChan->first;
                    auto st         = itChan->second;
                    if (itChan->second.use_count()) {
                        // on recup la valeur
                        float v = 0.0f;
                        itChan->second->GetValue(&v, chan);

                        ct::fvec2 p = GetWorldPosFromFrameValue(ct::fvec2((float)frame, v), vZone);

                        if (rc.ContainsPoint(p)) {
                            AddKeyToSelection(itChan->second);
                        } else {
                            RemoveKeyFromSelection(itChan->second);
                        }
                    }
                }
            }
        }
    }
}

void TimeLineSystem::SelectIfContainedInMouseSelectionRect(std::shared_ptr<UploadableUniform> vStruct, ImVec2 vPoint) {
    ZoneScoped;

    if (puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_RECTANGLE) {
        ct::fAABB rc(1e5f, -1e5f);
        rc.Combine(puStartMouseClick);
        rc.Combine(puEndMouseClick);
        if (rc.ContainsPoint(ct::fvec2(vPoint.x, vPoint.y))) {
            AddKeyToSelection(vStruct);
        } else {
            RemoveKeyFromSelection(vStruct);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

TimeLineSegmentInterpolation TimeLineSystem::GetTimeLineSegmentInterpolationMode(std::shared_ptr<UploadableUniform> vStart, std::shared_ptr<UploadableUniform> vEnd) {
    // start,	end,	=> result
    // none,	none	=> linear
    // none,	left	=> quadratic
    // none,	right	=> linear
    // none,	both	=> quadratic
    // left,	none	=> linear
    // left,	left	=> quadratic
    // left,	right	=> linear
    // left,	both	=> quadratic
    // right,	none	=> quadratic
    // right,	left	=> cubic
    // right,	right	=> quadratic
    // right,	both	=> cubic

    TimeLineSegmentInterpolation interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_CUBIC;

    if (vStart && vEnd) {
        if (vStart->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_NONE) {
            if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_NONE) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_LINEAR;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_LEFT) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_QUADRATIC;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_RIGHT) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_LINEAR;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_QUADRATIC;
            }
        } else if (vStart->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_LEFT) {
            if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_NONE) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_LINEAR;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_LEFT) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_QUADRATIC;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_RIGHT) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_LINEAR;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_QUADRATIC;
            }
        } else if (vStart->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_RIGHT) {
            if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_NONE) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_QUADRATIC;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_LEFT) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_CUBIC;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_RIGHT) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_QUADRATIC;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_CUBIC;
            }
        } else if (vStart->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH) {
            if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_NONE) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_QUADRATIC;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_LEFT) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_CUBIC;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_RIGHT) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_QUADRATIC;
            } else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH) {
                interpolation_mode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_CUBIC;
            }
        }
    }

    return interpolation_mode;
}

float TimeLineSystem::Interpolate(UniformTimeKey* vTimeKey, const int& vStartFrame, const float& vStartValue, std::shared_ptr<UploadableUniform> vStart, const int& vEndFrame, const float& vEndValue, std::shared_ptr<UploadableUniform> vEnd,
                                  const float& vRatio, bool vUseDerivation) {
    ZoneScoped;

    if (vTimeKey && vStart && vEnd) {
        auto interpolation_mode = GetTimeLineSegmentInterpolationMode(vStart, vEnd);

        if (interpolation_mode == TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_LINEAR) {
            return Interpolate_Linear(vStartValue, vEndValue, vRatio);
        } else if (interpolation_mode == TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_QUADRATIC) {
            return Interpolate_Quadratic(vStartFrame, vStartValue, vStart, vEndFrame, vEndValue, vEnd, vRatio, vUseDerivation);
        } else if (interpolation_mode == TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_CUBIC) {
            return Interpolate_Bezier(vStartFrame, vStartValue, vStart, vEndFrame, vEndValue, vEnd, vRatio, vUseDerivation);
        }
    }

    return 0.0f;
}

float TimeLineSystem::Interpolate_Linear(const float& vStart, const float& vEnd, const float& vRatio) {
    ZoneScoped;

    return ct::mix(vStart, vEnd, vRatio);
}

float TimeLineSystem::Interpolate_Quadratic(const int& vStartFrame, const float& vStartValue, std::shared_ptr<UploadableUniform> vStart, const int& vEndFrame, const float& vEndValue, std::shared_ptr<UploadableUniform> vEnd, const float& vRatio,
                                            bool vUseDerivation) {
    ZoneScoped;

    if (vStart && vEnd) {
        const ct::fvec2 p0 = ct::fvec2((float)vStartFrame, (float)vStartValue);
        const ct::fvec2 p2 = ct::fvec2((float)vEndFrame, (float)vEndValue);
        ct::fvec2 p1;
        if (vStart->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_RIGHT || vStart->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH)
            p1 = p0 + vStart->bezierControlStartPoint;
        else if (vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_LEFT || vEnd->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH)
            p1 = p2 - vEnd->bezierContorlEndPoint;
        else {
            CTOOL_DEBUG_BREAK;
        }

        if (IS_FLOAT_EQUAL(vRatio, 0.0f)) return vStartValue;
        if (IS_FLOAT_EQUAL(vRatio, 1.0f)) return vEndValue;

        const float targetX = ct::mix(p0.x, p2.x, vRatio);

        // TODO: a optimiser, l'algo de recherche part du debut a chaque fois, voir a partir de ratio dans un sens puis dans l'autre ci on s'eloigne
        float _x = 1e5f, x = -1e5f;
        float w[3] = {0.0f, 0.0f, 0.0f};
        for (float t = 0.0f; t <= 1.0f; t += 0.001f) {
            if (vUseDerivation)  // derivee pour le calcul de la pente pour afficher les tangeante
            {
                // dP(t) / dt =  - 2 * (1 - t) * P0 + 2 * t * P1 + 2 * P2
                // dP(t) / dt =  -2 * u * P0 + 2 * t * P1 + P2
                const float u = 1.0f - t;
                w[0]          = -2.0f * u;
                w[1]          = 2.0f * t;
                w[2]          = 2.0f;
            } else {
                // P(t) = (1 - t) ^ 2 * P0 + 2 * t * (1 - t) * P1 + t ^ 2 * P2
                // P(t) = u ^ 2 * P0 + 2 * t * u * P1 + t * t  * P2

                const float u = 1.0f - t;
                w[0]          = u * u;
                w[1]          = 2.0f * u * t;
                w[2]          = t * t;
            }

            x = w[0] * p0.x + w[1] * p1.x + w[2] * p2.x;

            if (x > targetX && _x < targetX) {
                const float y = w[0] * p0.y + w[1] * p1.y + w[2] * p2.y;
                // LogVar("t:" + ct::toStr(t) + "\tf:" + ct::toStr(targetX) + "\ty:" + ct::toStr(y));
                return y;
            }

            _x = x;
        }

        return _x;
    }

    return 0.0f;
}

float TimeLineSystem::Interpolate_Bezier(const int& vStartFrame, const float& vStartValue, std::shared_ptr<UploadableUniform> vStart, const int& vEndFrame, const float& vEndValue, std::shared_ptr<UploadableUniform> vEnd, const float& vRatio,
                                         bool vUseDerivation) {
    ZoneScoped;

    if (vStart && vEnd) {
        const ct::fvec2 p0 = ct::fvec2((float)vStartFrame, (float)vStartValue);
        const ct::fvec2 p1 = p0 + vStart->bezierControlStartPoint;
        const ct::fvec2 p3 = ct::fvec2((float)vEndFrame, (float)vEndValue);
        const ct::fvec2 p2 = p3 - vEnd->bezierContorlEndPoint;

        if (IS_FLOAT_EQUAL(vRatio, 0.0f)) return vStartValue;
        if (IS_FLOAT_EQUAL(vRatio, 1.0f)) return vEndValue;

        const float targetX = ct::mix(p0.x, p3.x, vRatio);

        // TODO: a optimiser, l'algo de recherche part du debut a chaque fois, voir a partir de ratio dans un sens puis dans l'autre ci on s'eloigne
        float _x = 1e5f, x = -1e5f;
        float w[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        for (float t = 0.0f; t <= 1.0f; t += 0.001f) {
            if (vUseDerivation)  // derivee pour le calcul de la pente pour afficher les tangeante
            {
                // dP(t) / dt =  -3 * (1-t)^2 * P0 + 3 * (1-t)^2 * P1 - 6 * t * (1-t) * P1 - 3 * t^2 * P2 + 6 * t(1-t) * P2 + 3 * t^2 * P3
                // dP(t) / dt =  -3 * u^2 * P0 + (3 * u^2 - 6 * t * u) * P1 + (6 * t * u - 3 * t^2) * P2 + 3 * t^2 * P3
                const float u = 1.0f - t;
                w[0]          = -3.0f * u * u;
                w[1]          = 3.0f * u * u - 6.0f * t * u;
                w[2]          = 6.0f * t * u - 3.0f * t * t;
                w[3]          = 3.0f * t * t;
            } else {
                // P(t) = (1 - t) ^ 3 * P0 + 3t(1 - t) ^ 2 * P1 + 3 * t ^ 2 (1 - t) * P2 + t ^ 3 * P3
                // P(t) = u ^ 3 * P0 + 3 * t * u^2 * P1 + 3 * t^2 * u * P2 + t ^ 3 * P3

                const float u = 1.0f - t;
                w[0]          = u * u * u;
                w[1]          = 3.0f * u * u * t;
                w[2]          = 3.0f * u * t * t;
                w[3]          = t * t * t;
            }

            x = w[0] * p0.x + w[1] * p1.x + w[2] * p2.x + w[3] * p3.x;

            if (x > targetX && _x < targetX) {
                const float y = w[0] * p0.y + w[1] * p1.y + w[2] * p2.y + w[3] * p3.y;
                // LogVar("t:" + ct::toStr(t) + "\tf:" + ct::toStr(targetX) + "\ty:" + ct::toStr(y));
                return y;
            }

            _x = x;
        }

        return _x;
    }

    return 0.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void TimeLineSystem::ReComputeInterpolation(ShaderKeyPtr vKey) {
    ZoneScoped;

    if (vKey) {
        for (auto& it : vKey->puTimeLine.timeLine) {
            it.second.values.clear();
            for (auto it2 = it.second.keys.begin(); it2 != it.second.keys.end(); ++it2) {
                ReComputeInterpolation(vKey, it.first, it2->first);
            }
        }
    }
}

void TimeLineSystem::ReComputeInterpolation(ShaderKeyPtr vKey, std::string vUniformName) {
    ZoneScoped;

    // ca va remplir la map value du container
    if (vKey && !vUniformName.empty()) {
        if (vKey->puTimeLine.timeLine.find(vUniformName) != vKey->puTimeLine.timeLine.end()) {  // trouvé
            auto st = &vKey->puTimeLine.timeLine[vUniformName];
            st->values.clear();
            for (auto it = st->keys.begin(); it != st->keys.end(); ++it) {
                ReComputeInterpolation(vKey, vUniformName, it->first);
            }
        }
    }
}

void TimeLineSystem::ReComputeInterpolation(TimeLineInfos* vTimeLineInfos, std::string vUniformName) {
    ZoneScoped;

    // ca va remplir la map value du container
    if (vTimeLineInfos && !vUniformName.empty()) {
        if (vTimeLineInfos->timeLine.find(vUniformName) != vTimeLineInfos->timeLine.end()) {  // trouvé
            auto st = &vTimeLineInfos->timeLine[vUniformName];
            st->values.clear();
            for (auto it = st->keys.begin(); it != st->keys.end(); ++it) {
                ReComputeInterpolation(vTimeLineInfos, vUniformName, it->first);
            }
        }
    }
}

void TimeLineSystem::ReComputeInterpolation(ShaderKeyPtr vKey, std::string vUniformName, int vComponent) {
    ZoneScoped;

    // ca va remplir la map value du container
    if (vKey && !vUniformName.empty()) {
        ReComputeInterpolation(&(vKey->puTimeLine), vUniformName, vComponent);
    }
}

void TimeLineSystem::ReComputeInterpolation(TimeLineInfos* vTimeLineInfos, std::string vUniformName, int vComponent) {
    ZoneScoped;

    // ca va remplir la map value du container
    if (vTimeLineInfos && !vUniformName.empty()) {
        if (vTimeLineInfos->timeLine.find(vUniformName) != vTimeLineInfos->timeLine.end())  // trouvé
        {
            const auto st = &vTimeLineInfos->timeLine[vUniformName];
            ReComputeInterpolation(st, vComponent);
        } else  // non trouvé
        {
        }
    }
}

void TimeLineSystem::ReComputeInterpolation(UniformTimeKey* vUniformTimeKeyStruct, int vComponent) {
    ZoneScoped;

    // ca va remplir la map value du container
    if (vUniformTimeKeyStruct) {
        UniformTimeKey* st = vUniformTimeKeyStruct;
        if (st->keys.find(vComponent) != st->keys.end()) {  // trouvé
            if (st->keys[vComponent].size() != 1) {
                int lastFrame = st->keys[vComponent].begin()->first;
                auto last     = st->keys[vComponent].begin()->second;
                if (last.use_count()) {
                    int currentFrame = 0;
                    for (auto frame : st->keys[vComponent]) {
                        currentFrame = frame.first;
                        auto current = frame.second;
                        if (current.use_count()) {
                            int countFrameToInterpolate = currentFrame - lastFrame;
                            if (countFrameToInterpolate > 0) {
                                for (int i = lastFrame; i <= currentFrame; i++) {
                                    float ratio = (float)(i - lastFrame) / (float)(currentFrame - lastFrame);
                                    std::shared_ptr<UploadableUniform> val_ptr = nullptr;
                                    if (st->values.size() > i) {
                                        val_ptr = st->values[i];
                                    } else {
                                        val_ptr = std::make_shared<UploadableUniform>();
                                    }
                                    if (val_ptr.use_count()) {
                                        val_ptr->glslType = st->glslType;
                                        switch (st->glslType) {
                                            case uType::uTypeEnum::U_FLOAT:
                                            case uType::uTypeEnum::U_VEC2:
                                            case uType::uTypeEnum::U_VEC3:
                                            case uType::uTypeEnum::U_VEC4: {
                                                val_ptr->xyzw[vComponent]     = Interpolate(st, lastFrame, last->xyzw[vComponent], last, currentFrame, current->xyzw[vComponent], current, ratio);
                                                val_ptr->useFloat[vComponent] = 1;
                                            } break;
                                            case uType::uTypeEnum::U_BOOL:
                                            case uType::uTypeEnum::U_BVEC2:
                                            case uType::uTypeEnum::U_BVEC3:
                                            case uType::uTypeEnum::U_BVEC4: {
                                                val_ptr->xyzw[vComponent]    = Interpolate(st, lastFrame, last->xyzw[vComponent], last, currentFrame, current->xyzw[vComponent], current, ratio);
                                                val_ptr->useBool[vComponent] = 1;
                                            } break;
                                            case uType::uTypeEnum::U_INT:
                                            case uType::uTypeEnum::U_IVEC2:
                                            case uType::uTypeEnum::U_IVEC3:
                                            case uType::uTypeEnum::U_IVEC4: {
                                                val_ptr->ixyzw[vComponent]  = (int)Interpolate(st, lastFrame, (float)last->ixyzw[vComponent], last, currentFrame, (float)current->ixyzw[vComponent], current, ratio);
                                                val_ptr->useInt[vComponent] = 1;
                                            } break;
                                            case uType::uTypeEnum::U_MAT2: {
                                                float* arr     = glm::value_ptr(val_ptr->mat4[0]);
                                                float* arrLast = glm::value_ptr(last->mat4[0]);
                                                float* arrCurr = glm::value_ptr(current->mat4[0]);
                                                for (int j = 0; j < 4; j++) {
                                                    arr[j] = (float)ct::mix(arrLast[j], arrCurr[j], ratio);
                                                }
                                                val_ptr->useMat = 1;
                                            } break;
                                            case uType::uTypeEnum::U_MAT3: {
                                                float* arr     = glm::value_ptr(val_ptr->mat4[0]);
                                                float* arrLast = glm::value_ptr(last->mat4[0]);
                                                float* arrCurr = glm::value_ptr(current->mat4[0]);
                                                for (int j = 0; j < 9; j++) {
                                                    arr[j] = (float)ct::mix(arrLast[j], arrCurr[j], ratio);
                                                }
                                                val_ptr->useMat = 1;
                                            } break;
                                            case uType::uTypeEnum::U_MAT4: {
                                                float* arr     = glm::value_ptr(val_ptr->mat4[0]);
                                                float* arrLast = glm::value_ptr(last->mat4[0]);
                                                float* arrCurr = glm::value_ptr(current->mat4[0]);
                                                for (int j = 0; j < 16; j++) {
                                                    arr[j] = (float)ct::mix(arrLast[j], arrCurr[j], ratio);
                                                }
                                                val_ptr->useMat = 1;
                                            } break;
                                            default: break;
                                        }
                                        st->values[i] = val_ptr;
                                    }
                                }
                            }
                            lastFrame = currentFrame;
                            last      = current;
                        }
                    }
                }
            }
        } else {  // non trouvé
        }
        puFrameChanged = true;
    }
}

bool TimeLineSystem::UpdateUniforms(ShaderKeyPtr vKey, UniformVariantPtr vUniPtr) {
    ZoneScoped;
    const bool change = false;
    if (vKey && vUniPtr) {
        if (puUploadToGpu && puFrameChanged || puRendering) {
            if (!vKey->puTimeLine.timeLine.empty()) {
                if (vKey->puTimeLine.timeLine.find(vUniPtr->name) != vKey->puTimeLine.timeLine.end()) {  // trouvé
                    auto st = &vKey->puTimeLine.timeLine[vUniPtr->name];
                    if (st->values.find(puCurrentFrame) != st->values.end()) {  // trouvé
                        auto v = st->values[puCurrentFrame];
                        if (v.use_count()) {
                            if (v->useFloat[0]) vUniPtr->x = v->xyzw[0];
                            if (v->useFloat[1]) vUniPtr->y = v->xyzw[1];
                            if (v->useFloat[2]) vUniPtr->z = v->xyzw[2];
                            if (v->useFloat[3]) vUniPtr->w = v->xyzw[3];
                            if (v->useBool[0]) vUniPtr->bx = (v->xyzw[0] > 0.5f);
                            if (v->useBool[1]) vUniPtr->by = (v->xyzw[1] > 0.5f);
                            if (v->useBool[2]) vUniPtr->bz = (v->xyzw[2] > 0.5f);
                            if (v->useBool[3]) vUniPtr->bw = (v->xyzw[3] > 0.5f);
                            if (v->useInt[0]) vUniPtr->ix = v->ixyzw[0];
                            if (v->useInt[1]) vUniPtr->iy = v->ixyzw[1];
                            if (v->useInt[2]) vUniPtr->iz = v->ixyzw[2];
                            if (v->useInt[3]) vUniPtr->iw = v->ixyzw[3];
                            if (v->useMat) vUniPtr->mat4 = v->mat4;
                            if (vUniPtr->widget == "checkbox" || vUniPtr->widget == "radio") {
                                if (v->useFloat[0]) vUniPtr->bx = vUniPtr->x > 0.5f;
                                if (v->useFloat[1]) vUniPtr->by = vUniPtr->y > 0.5f;
                                if (v->useFloat[2]) vUniPtr->bz = vUniPtr->z > 0.5f;
                                if (v->useFloat[3]) vUniPtr->bw = vUniPtr->w > 0.5f;
                            }
                        }
                    }
                }
            }
        }
    }

    return change;
}

std::string TimeLineSystem::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    ZoneScoped;

    std::string str;

    str += vOffset + "<timelinesystem>\n";

    str += vOffset + "\t<FrameBgColor>" + ct::fvariant(ct::fvec4(puFrameBg.x,puFrameBg.y, puFrameBg.z, puFrameBg.w)).GetS() + "</FrameBgColor>\n";
    str += vOffset + "\t<RangeFrameColor>" + ct::fvariant(ct::fvec4(puRangeFrame.x, puRangeFrame.y, puRangeFrame.z, puRangeFrame.w)).GetS() + "</RangeFrameColor>\n";
    str += vOffset + "\t<ThickLinesDarkColor>" + ct::fvariant(ct::fvec4(puThickLinesDark.x, puThickLinesDark.y, puThickLinesDark.z, puThickLinesDark.w)).GetS() +
        "</ThickLinesDarkColor>\n";
    str += vOffset + "\t<ThickLinesLightColor>" + ct::fvariant(ct::fvec4(puThickLinesLight.x, puThickLinesLight.y, puThickLinesLight.z, puThickLinesLight.w)).GetS() +
        "</ThickLinesLightColor>\n";

    str += vOffset + "</timelinesystem>\n";

    return str;
}

bool TimeLineSystem::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    ZoneScoped;

    // The value of this child identifies the name of this element
    std::string strName       = "";
    std::string strValue      = "";
    std::string strParentName = "";

    strName = vElem->Value();
    if (vElem->GetText()) strValue = vElem->GetText();
    if (vParent != nullptr) strParentName = vParent->Value();

    if (strParentName == "timelinesystem") {
        if (strName == "FrameBgColor") {
            auto c = ct::fvariant(strValue).GetColor();
            puFrameBg = ImVec4(c.r, c.g, c.b, c.a);
        }
        if (strName == "RangeFrameColor") {
            auto c = ct::fvariant(strValue).GetColor();
            puRangeFrame = ImVec4(c.r, c.g, c.b, c.a);
        }
        if (strName == "ThickLinesDarkColor") {
            auto c = ct::fvariant(strValue).GetColor();
            puThickLinesDark = ImVec4(c.r, c.g, c.b, c.a);
        }
        if (strName == "ThickLinesLightColor") {
            auto c = ct::fvariant(strValue).GetColor();
            puThickLinesLight = ImVec4(c.r, c.g, c.b, c.a);
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

void TimeLineSystem::ShowHelpPopup() {
    ZoneScoped;

    ImGui::SetTooltip("Ctrl+click => multi Selection");
}

void TimeLineSystem::ShowDialog(ShaderKeyPtr vKey, ct::ivec2 vScreenSize) {
    ZoneScoped;

    if (vKey) {
        ImVec2 min = ImVec2(0, 0);
        ImVec2 max = ImVec2(FLT_MAX, FLT_MAX);
        if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) {
            max = ImVec2((float)vScreenSize.x, (float)vScreenSize.y);
            min = max * 0.5f;
        }

        if (ImGuiFileDialog::Instance()->Display("TimeLineRenderingToPictures", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                puRenderingPrefix      = ImGuiFileDialog::Instance()->GetCurrentFileName();
                const size_t lastPoint = puRenderingPrefix.find_last_of('.');
                if (lastPoint != std::string::npos) {
                    puRenderingPrefix = puRenderingPrefix.substr(0, lastPoint);
                }
                puRenderingPath = ImGuiFileDialog::Instance()->GetCurrentPath();
                puRendering     = true;
                puCurrentFrame  = vKey->puTimeLine.rangeFrames.x;
                puRenderingMode = RenderingModeEnum::RENDERING_MODE_PICTURES;
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
}

bool TimeLineSystem::DrawBar(ShaderKeyPtr vKey, ct::ivec2 vScreenSize) {
    ZoneScoped;

    bool change = false;

    if (!vKey) return false;

    ShowDialog(vKey, vScreenSize);

    const ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    static float _groupPlayWidth   = 0.0f;
    static float _groupParamsWidth = 0.0f;

    if (!IsRendering()) {
        ImGui::BeginMenuBar();

        if (ImGui::BeginMenu("Export")) {
            if (ImGui::MenuItem("To Pictures")) {
                IGFD::FileDialogConfig config;
                config.path = puRenderingPath;
                config.countSelectionMax = 1;
                config.flags = ImGuiFileDialogFlags_Modal;
                ImGuiFileDialog::Instance()->OpenDialog("TimeLineRenderingToPictures", puRenderingPrefix.c_str(), ".png", config);
            }

            /*

            if (ImGui::BeginMenu("To Video"))
            {
                if (ImGui::MenuItem("avi"))
                {
                    puRenderingMode = TimeLineRenderingModeEnum::TIMELINE_RENDERING_MODE_VIDEO;
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("To Gif"))
            {
                puRenderingMode = TimeLineRenderingModeEnum::TIMELINE_RENDERING_MODE_GIF;

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("To HEIC"))
            {
                //puRendering = true;
                puRenderingMode = TimeLineRenderingModeEnum::TIMELINE_RENDERING_MODE_HEIC;
            }

            */

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Style")) {
            ImGui::Text("Frame Color");
            ImGui::SameLine();
            change |= ImGui::ColorEdit4("##Frame Color", &puFrameBg.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB);
            ImGui::Text("Range Color");
            ImGui::SameLine();
            change |= ImGui::ColorEdit4("##Range Color", &puRangeFrame.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB);
            ImGui::Text("Light Lines");
            ImGui::SameLine();
            change |= ImGui::ColorEdit4("##Light Lines", &puThickLinesLight.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB);
            ImGui::Text("Dark Lines");
            ImGui::SameLine();
            change |= ImGui::ColorEdit4("##Dark Lines", &puThickLinesDark.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB);
            ImGui::EndMenu();
        }

#ifdef _DEBUG
        if (ImGui::BeginMenu("Interpolations")) {
            if (ImGui::MenuItem("ReCompute alls")) {
                if (puActiveKey) {
                    ReComputeInterpolation(puActiveKey);
                }
            }
            ImGui::EndMenu();
        }        

        if (ImGui::BeginMenu("Debug")) {
            ImGui::Text("Pixel Horiz Steps");
            ImGui::SameLine();
            if (ImGui::DragFloat("##Pixel Horiz Steps", &puStepSize)) {
                puStepSize = ct::maxi(puStepSize, 1.0f);
                change     = true;
            }
            ImGui::Text("Pixel Vert Steps");
            ImGui::SameLine();
            if (ImGui::DragFloat("##Pixel Vert Steps", &pucurveStepSize)) {
                pucurveStepSize = ct::maxi(pucurveStepSize, 1.0f);
                change          = true;
            }
            ImGui::Text("Preview Ratio");
            ImGui::SameLine();
            if (ImGui::SliderFloat("##Preview Ratio", &_PreviewRatio, 0.0f, 1.0f)) {
                _PreviewRatio = ct::clamp(_PreviewRatio, 0.0f, 1.0f);
                change        = true;
            }
            ImGui::Text("Curve Step Scale");
            ImGui::SameLine();
            if (ImGui::SliderFloat("##Curve Step Scale", &pucurveStepScale, 0.0001f, 50.0f)) {
                pucurveStepScale = ct::maxi(pucurveStepScale, 0.0001f);
                change           = true;
            }
            ImGui::Text("Frame Step Scale");
            ImGui::SameLine();
            if (ImGui::SliderInt("##Frame Step Scale", &puStepScale, 1, 50)) {
                puStepScale = ct::maxi(puStepScale, 1);
                change      = true;
            }

            ImGui::EndMenu();
        }
#endif
        if (ImGui::BeginMenu("Loop")) {
            if (ImGui::BeginMenu("Position and Control Points Gradient")) {
                if (ImGui::MenuItem("Loop on start key")) {
                    DoLoopLimitKeys(vKey, true, true, true, false);
                }

                if (ImGui::MenuItem("Loop on start key (Same control segment length)")) {
                    DoLoopLimitKeys(vKey, true, true, true, true);
                }

                if (ImGui::MenuItem("Loop on end key")) {
                    DoLoopLimitKeys(vKey, false, true, true, false);
                }

                if (ImGui::MenuItem("Loop on end key (Same control segment length)")) {
                    DoLoopLimitKeys(vKey, false, true, true, true);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Position")) {
                if (ImGui::MenuItem("Loop on start key")) {
                    DoLoopLimitKeys(vKey, true, true, false, false);
                }

                if (ImGui::MenuItem("Loop on end key")) {
                    DoLoopLimitKeys(vKey, false, true, false, false);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Control Points Gradient")) {
                if (ImGui::MenuItem("Loop on start key")) {
                    DoLoopLimitKeys(vKey, true, false, true, false);
                }

                if (ImGui::MenuItem("Loop on start key (Same control segment length)")) {
                    DoLoopLimitKeys(vKey, true, false, true, true);
                }

                if (ImGui::MenuItem("Loop on end key")) {
                    DoLoopLimitKeys(vKey, false, false, true, false);
                }

                if (ImGui::MenuItem("Loop on end key (Same control segment length)")) {
                    DoLoopLimitKeys(vKey, false, false, true, true);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Cleaning")) {
            if (ImGui::MenuItem("Clean whole TimeLine")) {
                change = true;
                Clean_All(vKey);
            }

            if (ImGui::MenuItem("Keep only Changed Uniforms")) {
                change = true;
                Clean_KeepOnlyChangedUniforms(vKey);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Re Scale")) {
            if (ImGui::MenuItem("re scale by 0.25")) ScaleTimeLine(vKey, 0.25f);
            if (ImGui::MenuItem("re scale by 0.5")) ScaleTimeLine(vKey, 0.5f);
            if (ImGui::MenuItem("re scale by 0.75")) ScaleTimeLine(vKey, 0.75f);
            ImGui::Separator();
            if (ImGui::MenuItem("re scale by 1.25")) ScaleTimeLine(vKey, 1.25f);
            if (ImGui::MenuItem("re scale by 1.5")) ScaleTimeLine(vKey, 1.5f);
            if (ImGui::MenuItem("re scale by 1.75")) ScaleTimeLine(vKey, 1.75f);
            if (ImGui::MenuItem("re scale by 2.0")) ScaleTimeLine(vKey, 2.0f);

            ImGui::EndMenu();
        }

        ImGui::Text("?");
        if (ImGui::IsItemHovered()) {
            ShowHelpPopup();
        }

        /*
        #ifdef _DEBUG
                ImGui::Text("|| min[x:%.2f y:%.2f] max[z:%.2f w:%.2f]",
                    puStartMouseClick.x,
                    puStartMouseClick.y,
                    puEndMouseClick.x,
                    puEndMouseClick.y);

                if (puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_NONE)
                    ImGui::Text(" || None");
                else if (puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_POINT)
                    ImGui::Text(" || Point");
                else if (puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_GRABBER_MOVING)
                    ImGui::Text(" || Grabber");
                else if (puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_MOVE_POINT)
                    ImGui::Text(" || Move Point");
                else if (puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_CONTROL_POINT)
                    ImGui::Text(" || Control Point");
                else if (puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_RECTANGLE)
                    ImGui::Text(" || Rectangle");

                ImGui::Text("StartOver:%s", puMouseStartOverButton?"true":"false");
                ImGui::Text("Over:%s", puMouseOverButton ? "true" : "false");
                ImGui::Text("Has Dragged:%s", puMouseHasDragged ? "true" : "false");
        #endif
        */

        ImGui::EndMenuBar();

        change |= ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "GPU", "Use TimeLine for define uniforms in GPU", &puUploadToGpu);

        ImGui::SameLine();

        if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "F", "Free", puBezierHandler == TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_FREE)) {
            puBezierHandler = TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_FREE;
        }

        ImGui::SameLine();

        if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "A", "Aligned", puBezierHandler == TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_ALIGNED)) {
            puBezierHandler = TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_ALIGNED;
        }

        ImGui::SameLine();

        if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "AS", "Aligned Symetric", puBezierHandler == TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_ALIGNED_SYMMETRIC)) {
            puBezierHandler = TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_ALIGNED_SYMMETRIC;
        }

        ImGui::SameLine();

        const float totalWidth = ImGui::GetContentRegionMax().x;

        ImGui::Spacing(20.0f);

        ImGui::SameLine();

        ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "UU", "Show/Hide Unused Uniforms", &puShowUnUsed);

        ImGui::SameLine();

        if (IsShowingCurves(vKey)) {
            ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "CP", "Show/Hide Control Points", &puShowControlPoints);

            ImGui::SameLine();

            ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "FP", "Show/Hide Frame Points", &puShowInterpolatedPoints);

            ImGui::SameLine();
        }

        ImGui::Spacing(20.0f);

        ImGui::SameLine();

        if (IsSelectionExist()) {
            if (ImGui::ContrastedButton("No Lin", "Make theses keys not linear")) {
                // its mean no control points on right
                ChangeHandlerTypeOfSelectedKeys(vKey, TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH);
            }

            ImGui::SameLine();

            if (ImGui::ContrastedButton("Lin", "Make theses keys linear")) {
                // its mean no control points on right
                ChangeHandlerTypeOfSelectedKeys(vKey, TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_NONE);
            }

            ImGui::SameLine();

            if (ImGui::ContrastedButton("Lin L", "Make theses keys linear on left only")) {
                // its mean no control points on right
                ChangeHandlerTypeOfSelectedKeys(vKey, TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_RIGHT);
            }

            ImGui::SameLine();

            if (ImGui::ContrastedButton("Lin R", "Make theses keys linear on right only")) {
                // its mean no control points on right
                ChangeHandlerTypeOfSelectedKeys(vKey, TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_LEFT);
            }

            ImGui::SameLine();
        }

        ImGui::SetCursorScreenPos(cursorPos + ImVec2((totalWidth - _groupPlayWidth) * 0.5f + ImGui::GetStyle().FramePadding.x, 0.0f));
        ImGui::BeginGroup();
        {
            ImGui_AutoKeyingButton("AutoKeying", &puRecord);

            ImGui::SameLine();

            ImGui::Spacing();

            ImGui::SameLine();

            if (ImGui::ContrastedButton(ICON_NDP_FAST_BACKWARD, "GoToStartFrame")) {
                change = true;
                GoToFrame(vKey->puTimeLine.rangeFrames.x);
            }

            ImGui::SameLine();

            if (ImGui::ContrastedButton(ICON_NDP_BACKWARD, "Go to previous key")) {
                change = true;
                GoToPreviousKey(vKey, puCurrentFrame);
            }

            ImGui::SameLine();

            if (puPlayTimeLineReverse || puPlayTimeLine) {
                if (puPlayTimeLineReverse) {
                    ImGui::ToggleContrastedButton(ICON_NDP_PAUSE, ICON_NDP_PAUSE, &puPlayTimeLineReverse, "Pause");
                } else if (puPlayTimeLine) {
                    ImGui::ToggleContrastedButton(ICON_NDP_PAUSE, ICON_NDP_PAUSE, &puPlayTimeLine, "Pause");
                }
            } else {
                ImGui::ToggleContrastedButton(ICON_NDP_CARET_LEFT, ICON_NDP_CARET_LEFT, &puPlayTimeLineReverse, "Reverse");

                ImGui::SameLine();

                ImGui::ToggleContrastedButton(ICON_NDP_CARET_RIGHT, ICON_NDP_CARET_RIGHT, &puPlayTimeLine, "Play");
            }

            ImGui::SameLine();

            if (ImGui::ContrastedButton(ICON_NDP_FORWARD, "Go to next key")) {
                change = true;
                GoToNextKey(vKey, puCurrentFrame);
            }

            ImGui::SameLine();

            if (ImGui::ContrastedButton(ICON_NDP_FAST_FORWARD, "Go to end frame")) {
                change = true;
                GoToFrame(vKey->puTimeLine.rangeFrames.y);
            }
        }
        ImGui::EndGroup();
        _groupPlayWidth = ImGui::GetItemRectSize().x;

        ImGui::SameLine();

        ImGui::SetCursorScreenPos(cursorPos + ImVec2((totalWidth - _groupParamsWidth) * 0.99f, 0.0f));
        ImGui::BeginGroup();
        {
            ImGui::Text("(f/s)");

            ImGui::SameLine();

            if (ImGui_DragInt(50, "##playtimefps", "Count Frame Per Second", &puFrameRate)) {
                puFrameRateInMS = 1000 / puFrameRate;
                change          = true;
            }

            ImGui::SameLine();

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

            ImGui::SameLine();

            ImGui::Text("(Frame)");

            ImGui::SameLine();

            ImGui_DragInt(50, "##playtimeframe", "Current Frame", &puCurrentFrame);

            ImGui::SameLine();

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

            ImGui::SameLine();

            ImGui::Text("(Range)");

            ImGui::SameLine();

            // dans blender le text s'affiche dans le dragInt
            ImGui_DragInt(50, "##playtimeframestart", "Start Frame", &vKey->puTimeLine.rangeFrames.x);

            ImGui::SameLine();

            ImGui_DragInt(50, "##playtimeframeend", "End Frame", &vKey->puTimeLine.rangeFrames.y);
        }
        ImGui::EndGroup();
        _groupParamsWidth = ImGui::GetItemRectSize().x;
    } else {
        const float totalWidth = ImGui::GetContentRegionMax().x;

        ImGui::SetCursorScreenPos(cursorPos + ImVec2((totalWidth - _groupPlayWidth) * 0.5f + ImGui::GetStyle().FramePadding.x, 0.0f));
        ImGui::BeginGroup();
        {
            if (ImGui_AbortButton("Abort")) {
                puRendering = false;
            }
        }
        ImGui::EndGroup();
        _groupPlayWidth = ImGui::GetItemRectSize().x;
    }

    ImGui::Separator();

    // change |= DoAnimation_WithoutUiTriggering();

    return change;
}

void TimeLineSystem::GoToNextKey(ShaderKeyPtr vKey, int vCurrentFrame) {
    ZoneScoped;

    if (vKey) {
        bool defined     = false;
        int closestFrame = vCurrentFrame;
        for (auto it = vKey->puTimeLine.timeLine.begin(); it != vKey->puTimeLine.timeLine.end(); ++it) {
            for (auto itComps = it->second.keys.begin(); itComps != it->second.keys.end(); ++itComps) {
                for (auto itKeys = itComps->second.begin(); itKeys != itComps->second.end(); ++itKeys) {
                    if (itKeys->first > vCurrentFrame) {
                        if (defined) {
                            closestFrame = ct::mini(itKeys->first, closestFrame);
                        } else {
                            closestFrame = itKeys->first;
                            defined      = true;
                        }
                        break;
                    }
                }
            }
        }

        if (closestFrame != vCurrentFrame) {
            GoToFrame(closestFrame);
        }
    }
}

void TimeLineSystem::GoToPreviousKey(ShaderKeyPtr vKey, int vCurrentFrame) {
    ZoneScoped;

    if (vKey) {
        bool defined     = false;
        int closestFrame = vCurrentFrame;
        for (auto it = vKey->puTimeLine.timeLine.begin(); it != vKey->puTimeLine.timeLine.end(); ++it) {
            for (auto itComps = it->second.keys.begin(); itComps != it->second.keys.end(); ++itComps) {
                for (auto itKeys = itComps->second.rbegin(); itKeys != itComps->second.rend(); ++itKeys) {
                    if (itKeys->first < vCurrentFrame) {
                        if (defined) {
                            closestFrame = ct::maxi(itKeys->first, closestFrame);
                        } else {
                            closestFrame = itKeys->first;
                            defined      = true;
                        }
                        break;
                    }
                }
            }
        }

        if (closestFrame != vCurrentFrame) {
            GoToFrame(closestFrame);
        }
    }
}

void TimeLineSystem::GoToFrame(int vFrame) {
    ZoneScoped;

    puCurrentFrame = vFrame;
    puFrameChanged = true;
}

void TimeLineSystem::ScaleTimeLine(ShaderKeyPtr vKey, float vScale) {
    ZoneScoped;

    if (vKey && vScale > 0.0f) {
        puTimeLineScale = vScale;

        // on va augmenter le nombre de frames
        // dont multiplier la position des key par le scale
        // et re interpoler
        std::map<std::string, UniformTimeKey> tmp = vKey->puTimeLine.timeLine;
        vKey->puTimeLine.timeLine.clear();

        bool conversioNotPermise = false;

        for (auto itStruct = tmp.begin(); itStruct != tmp.end(); ++itStruct) {
            std::string name = itStruct->first;
            for (auto itChan = itStruct->second.keys.begin(); itChan != itStruct->second.keys.end(); ++itChan) {
                int chan                                    = itChan->first;
                vKey->puTimeLine.timeLine[name].uniformName = tmp[name].uniformName;
                vKey->puTimeLine.timeLine[name].glslType    = tmp[name].glslType;
                vKey->puTimeLine.timeLine[name].widget      = tmp[name].widget;
                for (auto itFrame = itChan->second.begin(); itFrame != itChan->second.end(); ++itFrame) {
                    int frame    = itFrame->first;
                    int newFrame = (int)((float)frame * vScale);
                    if (frame != 0 && itChan->second.find(newFrame) != itChan->second.end())  // trouvé
                    {
                        conversioNotPermise = true;
                        break;
                    } else {
                        vKey->puTimeLine.timeLine[name].keys[chan][newFrame] = tmp[name].keys[chan][frame];
                    }
                }
                if (conversioNotPermise) break;
            }
            if (conversioNotPermise) break;
        }

        if (conversioNotPermise) {
            vKey->puTimeLine.timeLine.clear();
            vKey->puTimeLine.timeLine = tmp;
        } else {
            vKey->puTimeLine.rangeFrames.y = (int)((float)vKey->puTimeLine.rangeFrames.y * vScale);

            // recompute
            for (auto itStruct = tmp.begin(); itStruct != tmp.end(); ++itStruct) {
                ReComputeInterpolation(vKey, itStruct->first);
            }
        }
    }
}

void TimeLineSystem::ReArrangeForViewContent(ShaderKeyPtr vKey, ImRect vZone, float* vPaneOffsetX, float* vPaneOffsetY)  // re arrange view on content in graph mode
{
    ZoneScoped;

    if (!puUniformsToEdit.empty()) {
        if (vKey) {
            // on va modifier :
            // X : puStepScale et vPaneOffsetX
            // Y : pucurveStepScale et vPaneOffsetY

            // on doit trouver le range de frames et le range de values
            ct::fAABB aabb(1e8f, -1e8f);
            for (auto itStruct = vKey->puTimeLine.timeLine.begin(); itStruct != vKey->puTimeLine.timeLine.end(); ++itStruct) {
                std::string name = itStruct->first;
                if (puUniformsToEdit.find(name) != puUniformsToEdit.end())  // trouvé
                {
                    for (auto val : itStruct->second.values) {
                        const int frame = val.first;

                        if (val.second.use_count()) {
                            for (int i = 0; i < 4; i++) {
                                if (puUniformsToEdit[name] & puTimeLineItemAxisMasks[i]) {
                                    float v = 0.0f;
                                    if (val.second->GetValue(&v, i)) {
                                        aabb.Combine(ct::fvec2((float)frame, v));
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // on va re scale

            const ct::fvec2 countFramesValues  = aabb.Size();
            const ct::fvec2 middleFramesValues = aabb.GetCenter();

            if (vPaneOffsetX && countFramesValues.x > 0)  // test for avoid div by zero error
            {
                const float zoneMaxX = vZone.Max.x - vZone.Min.x - puPaneWidth - pucurveBarWidth;
                const int maxFrames  = (int)ct::ceil(zoneMaxX / puStepSize);  // *puStepScale;
                if (maxFrames > 0) puStepScale = (int)(countFramesValues.x / (float)maxFrames);
                puStepScale   = ct::maxi<int>(puStepScale, 1);
                *vPaneOffsetX = puPaneWidth + pucurveBarWidth + middleFramesValues.x * puStepScale / puStepSize;
            }

            if (vPaneOffsetY && countFramesValues.y > 0.0f)  // test for avoid div by zero error
            {
                const float zoneMaxY  = vZone.Max.y - vZone.Min.y - putimeBarHeight;
                const float maxValues = zoneMaxY / pucurveStepSize;
                if (IS_FLOAT_DIFFERENT(maxValues, 0.0f)) pucurveStepScale = countFramesValues.y / maxValues;
                pucurveStepScale = ct::maxi<float>(pucurveStepScale, 1e-8f);
                *vPaneOffsetY    = putimeBarHeight + (countFramesValues.y * 0.5f + middleFramesValues.y) / pucurveStepScale * pucurveStepSize;
            }
        }
    }
}

void TimeLineSystem::DoLoopLimitKeys(ShaderKeyPtr vKey, bool vStartOrEnd, bool vSamePos, bool vSameGradient, bool vSameGradientLength) {
    ZoneScoped;

    // on va mettre le end sur la valeur du start, si pas de end on va le creer
    // et si on est en spline on va aussi mettre le meme angle, en gardant la longuer actuelle
    if (vKey) {
#define existInMap(a) (chan.second.find(a) != chan.second.end())
        const ct::ivec2 range = vKey->puTimeLine.rangeFrames;
        for (auto uTimeKey : vKey->puTimeLine.timeLine) {
            const std::string name = uTimeKey.first;
            for (auto chan : uTimeKey.second.keys) {
                if (vStartOrEnd) {
                    if (existInMap(range.x))  // start trouvé
                    {
                        if (existInMap(range.y))  // end trouvé
                        {
                            if (vSamePos) {
                                // set point pos
                                chan.second[range.y]->copy(chan.second[range.x]);
                            }

                            if (vSameGradient) {
                                // set gradient
                                float len = chan.second[range.y]->bezierContorlEndPoint.length();
                                if (vSameGradientLength) len = chan.second[range.x]->bezierControlStartPoint.length();
                                chan.second[range.y]->bezierContorlEndPoint = chan.second[range.x]->bezierControlStartPoint.GetNormalized() * len;
                            }
                        }
                    }
                } else {
                    if (existInMap(range.y))  // end trouvé
                    {
                        if (existInMap(range.x))  // start trouvé
                        {
                            if (vSamePos) {
                                // set point pos
                                chan.second[range.x]->copy(chan.second[range.y]);
                            }

                            if (vSameGradient) {
                                // set gradient
                                float len = chan.second[range.x]->bezierControlStartPoint.length();
                                if (vSameGradientLength) len = chan.second[range.y]->bezierContorlEndPoint.length();
                                chan.second[range.x]->bezierControlStartPoint = chan.second[range.y]->bezierContorlEndPoint.GetNormalized() * len;
                            }
                        }
                    }
                }
            }

            ReComputeInterpolation(vKey, name);
            puFrameChanged = true;
        }
    }
}

void TimeLineSystem::ChangeHandlerTypeOfSelectedKeys(ShaderKeyPtr vKey, TimeLineHandlerType vTimeLineHandlerType) {
    ZoneScoped;

    if (!puSelectedKeys.empty()) {
        for (auto key : puSelectedKeys) {
            if (key) {
                key->timeLineHandlerType = vTimeLineHandlerType;
                ReComputeInterpolation(vKey, key->uniformName);
            }
        }

        puFrameChanged = true;
    }
}

bool TimeLineSystem::DoAnimation_WithoutUiTriggering(ShaderKeyPtr vKey) {
    ZoneScoped;

    bool change = false;

    if (vKey) {
        if (IsRendering()) {
            puCurrentFrame++;
            if (puCurrentFrame <= vKey->puTimeLine.rangeFrames.y) {
                puFrameChanged = true;
                change |= true;
            } else {
                puRendering = false;
            }
        } else if (IsPlaying()) {
            if (puAnimationTimer.IsTimeToAct(puFrameRateInMS, true)) {
                if (puPlayTimeLine) {
                    puCurrentFrame++;
                    if (puCurrentFrame > vKey->puTimeLine.rangeFrames.y) puCurrentFrame = vKey->puTimeLine.rangeFrames.x;
                    if (puCurrentFrame < vKey->puTimeLine.rangeFrames.x) puCurrentFrame = vKey->puTimeLine.rangeFrames.y;
                    puFrameChanged = true;
                    change |= true;
                }

                if (puPlayTimeLineReverse) {
                    puCurrentFrame--;
                    if (puCurrentFrame > vKey->puTimeLine.rangeFrames.y) puCurrentFrame = vKey->puTimeLine.rangeFrames.x;
                    if (puCurrentFrame < vKey->puTimeLine.rangeFrames.x) puCurrentFrame = vKey->puTimeLine.rangeFrames.y;
                    puFrameChanged = true;
                    change |= true;
                }
            }
        }
    }

    return change;
}

bool TimeLineSystem::ImGui_DrawTrashButton(bool /*vHovered*/, const ImVec2& pos) {
    ZoneScoped;

    using namespace ImGui;

    ImGuiContext& g     = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems) return false;

    const ImRect bb(pos, pos + ImVec2(g.FontSize, g.FontSize));

    // Render
    const ImVec2 center = bb.GetCenter();

    const int id = ImGui::IncPUSHID();

    bool hovered, held;
    const bool pressed = ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick);

    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    window->DrawList->AddRectFilled(bb.Min, bb.Max, col, 2);

    const float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;
    const bool pushed        = ImGui::PushStyleColorWithContrast(ImGuiCol_Button, ImGuiCol_Text, CustomStyle::puContrastedTextColor, CustomStyle::puContrastRatio);
    window->DrawList->AddLine(center + ImVec2(+cross_extent, +cross_extent), center + ImVec2(-cross_extent, -cross_extent), ImGui::GetColorU32(ImGuiCol_Text), 2.5f);
    window->DrawList->AddLine(center + ImVec2(+cross_extent, -cross_extent), center + ImVec2(-cross_extent, +cross_extent), ImGui::GetColorU32(ImGuiCol_Text), 2.5f);
    if (pushed) ImGui::PopStyleColor();
    return pressed;
}

bool TimeLineSystem::ImGui_DrawCheckButton(bool /*vHovered*/, const ImVec2& pos, bool vFlag) {
    ZoneScoped;

    using namespace ImGui;

    ImGuiContext& g     = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems) return false;

    const ImRect bb(pos, pos + ImVec2(g.FontSize, g.FontSize));

    // Render
    ImVec2 center = bb.GetCenter();

    const int id = ImGui::IncPUSHID();

    ImGui::SetItemAllowOverlap();

    bool hovered, held;
    const bool pressed = ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick);

    // render
    const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    ImGui::RenderFrame(bb.Min, bb.Max, col, true, g.Style.FrameRounding);
    if (vFlag) {
        const float sizey = g.FontSize;
        const float pad   = ImMax(1.0f, (float)(int)(sizey / 6.0f));
        ImGui::RenderCheckMark(window->DrawList, bb.Min + ImVec2(pad, pad - 0.1f * sizey), ImGui::GetColorU32(ImGuiCol_CheckMark), sizey - pad * 2.0f);
    }

    return pressed;
}

bool TimeLineSystem::ImGui_DragFloat(const float width, const char* label, const char* help, float* value) {
    ImGui::PushItemWidth(width);
    const bool di = ImGui::DragFloat(label, value);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(help);
    ImGui::PopItemWidth();
    return di;
}

bool TimeLineSystem::ImGui_DragInt(const float width, const char* label, const char* help, int* value) {
    ImGui::PushItemWidth(width);
    const bool di = ImGui::DragInt(label, value);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip(help);
    ImGui::PopItemWidth();
    return di;
}

void TimeLineSystem::ScaleViewForFrames(int vCountFrames, float vMouseWheel) {
    ZoneScoped;

    const float centralFrame = ((float)vCountFrames * 0.5f - puPaneOffsetX / puStepSize) * (float)puStepScale;
    puStepScale              = ct::maxi((int)(puStepScale - vMouseWheel), 1);  // clamp for avoid div by zero
    puPaneOffsetX            = ((float)vCountFrames * 0.5f - centralFrame / (float)puStepScale) * puStepSize;
}

void TimeLineSystem::ScaleViewForValues(ImRect vFrameBb, float vMouseWheel) {
    ZoneScoped;

    const float countValue   = ct::ceil((vFrameBb.Max.y - vFrameBb.Min.y) / pucurveStepSize);
    const float centralValue = (countValue * 0.5f - puPaneOffsetY / pucurveStepSize) * pucurveStepScale;
    if (vMouseWheel < 0.0f)
        pucurveStepScale *= 1.25f;
    else
        pucurveStepScale *= 0.8f;
    puPaneOffsetY = (countValue * 0.5f - centralValue / pucurveStepScale) * pucurveStepSize;
}

static const float DRAG_MOUSE_THRESHOLD_FACTOR = 0.50f;  // Multiplier for the default value of io.MouseDragThreshold to make DragFloat/DragInt react faster to mouse drags.

bool TimeLineSystem::DrawTimeLine(const char* label, ShaderKeyPtr vKey) {
    ZoneScoped;

    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems || !vKey) {
        return false;
    }

    ImGuiContext& g = *GImGui;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id        = window->GetID(label);
    ImVec2 label_size       = CalcTextSize(label, nullptr, true);

    bool showGraph    = IsShowingCurves(vKey);  // pas curve graph
    puMouseOverButton = false;

    ImVec2 max_size = GetContentRegionAvail();

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + max_size);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max);

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &frame_bb)) return false;

    int countFrames = (int)ct::ceil((frame_bb.Max.x - frame_bb.Min.x) / puStepSize);

    // mouse interaction
    const bool hovered = ItemHoverable(frame_bb, id, ImGuiItemFlags_None);
    /*bool temp_input_is_active = TempInputIsActive(id);
    if (!temp_input_is_active)
    {
        // Tabbing or CTRL-clicking on Drag turns it into an InputText
        const bool input_requested_by_tabbing = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_FocusedByTabbing) != 0;
        const bool clicked = hovered && IsMouseClicked(0, id);
        const bool double_clicked = (hovered && g.IO.MouseClickedCount[0] == 2 && TestKeyOwner(ImGuiKey_MouseLeft, id));
        const bool make_active = (input_requested_by_tabbing || clicked || double_clicked || g.NavActivateId == id || g.NavActivateInputId == id);
        if (make_active && (clicked || double_clicked))
            SetKeyOwner(ImGuiKey_MouseLeft, id);
        if (make_active)
            if (input_requested_by_tabbing || (clicked && g.IO.KeyCtrl) || double_clicked || g.NavActivateInputId == id)
                temp_input_is_active = true;

        // (Optional) simple click (without moving) turns Drag into an InputText
        if (g.IO.ConfigDragClickToInputText && !temp_input_is_active)
            if (g.ActiveId == id && hovered && g.IO.MouseReleased[0] && !IsMouseDragPastThreshold(0, g.IO.MouseDragThreshold * DRAG_MOUSE_THRESHOLD_FACTOR))
            {
                g.NavActivateId = g.NavActivateInputId = id;
                g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
                temp_input_is_active = true;
            }

        if (make_active && !temp_input_is_active)
        {
            SetActiveID(id, window);
            SetFocusID(id, window);
            FocusWindow(window);
            g.ActiveIdUsingNavDirMask = (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        }
    }*/

    bool value_changed = false;

    // calc rects
    ImRect valueBarRc;
    float curveBarW = 0.0f;
    if (showGraph) {
        curveBarW  = pucurveBarWidth;
        valueBarRc = ImRect(ImVec2(frame_bb.Min.x + puPaneWidth, frame_bb.Min.y + putimeBarHeight), ImVec2(frame_bb.Min.x + puPaneWidth + curveBarW, frame_bb.Max.y));
    }
    ImRect timeBarRc = ImRect(ImVec2(frame_bb.Min.x + puPaneWidth + curveBarW, frame_bb.Min.y), ImVec2(frame_bb.Max.x, frame_bb.Min.y + putimeBarHeight));
    ImRect viewRc    = ImRect(ImVec2(frame_bb.Min.x + puPaneWidth + curveBarW, frame_bb.Min.y + putimeBarHeight), ImVec2(frame_bb.Max.x, frame_bb.Max.y));

    if (hovered) {
        // mouse wheel
        if (IS_FLOAT_DIFFERENT(g.IO.MouseWheel, 0.0f)) {
            if (g.IO.MousePos.x < frame_bb.Min.x + puPaneWidth) {  // mouvement vertical sur le pan des noms d'uniforms
                puUniformsPaneOffsetY = ct::maxi(0.0f, puUniformsPaneOffsetY - g.IO.MouseWheel);
            } else {  // mouvement sur les barres
                if (timeBarRc.Contains(g.IO.MousePos)) {
                    ScaleViewForFrames(countFrames, g.IO.MouseWheel);
                } else if (valueBarRc.Contains(g.IO.MousePos) && showGraph) {
                    ScaleViewForValues(frame_bb, g.IO.MouseWheel);
                } else {  // les deux a la fois
                    ScaleViewForFrames(countFrames, g.IO.MouseWheel);
                    if (showGraph)
                        ScaleViewForValues(frame_bb, g.IO.MouseWheel);
                }
            }
        }

        if (viewRc.Contains(g.IO.MousePos)) {
            ShowMouseInfos(vKey, frame_bb, ct::fvec2(g.IO.MousePos.x, g.IO.MousePos.y));
        }

        // SetTooltip("ox : %.1f \noy : %.1f\nsx : %.i\nsy : %.1f", puPaneOffsetX, puPaneOffsetY, puStepScale, pucurveStepScale);
    }

    // Draw frame
    const ImU32 frame_col = GetColorU32(puFrameBg);
    RenderNavHighlight(frame_bb, id);
    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

    // draw items header
    // ImVec4 cat = ImVec4(0.0f, 0.5f, 0.1f, 0.5f);
    char buffer[100] = "\0";
    float offsetY    = label_size.y + puTextOffsetY * 2.0f + putimeBarHeight;

    ImVec2 a, b;

    ImGui::PushClipRect(frame_bb.Min, frame_bb.Max, false);
    {
        size_t paningIndex = 0;
        for (auto itSec = vKey->puTimeLine.timeLine.begin(); itSec != vKey->puTimeLine.timeLine.end(); ++itSec) {
            if (++paningIndex < puUniformsPaneOffsetY) continue;

            if (frame_bb.Min.y + offsetY < frame_bb.Max.y) {
                UniformTimeKey* vTimeKey = &itSec->second;

                UniformVariantPtr uni = nullptr;
                if (vKey) uni = vKey->GetUniformByName(vTimeKey->uniformName);

                if (uni) {
                    if (uni->loc >= 0 || puShowUnUsed) {
                        snprintf(buffer, 100, "%s", vTimeKey->uniformName.c_str());
                        label_size = CalcTextSize(buffer, nullptr, true);
                        if (!showGraph) {
                            ImU32 frameTextCol = GetColorU32(ImGui::GetUniformLocColor(uni->loc));
                            a                  = ImVec2(frame_bb.Min.x + g.Style.FramePadding.x, frame_bb.Min.y - g.Style.FramePadding.y * 0.25f + offsetY);
                            b                  = ImVec2(frame_bb.Max.x - g.Style.FramePadding.x, frame_bb.Min.y + g.Style.FramePadding.y * 0.25f + label_size.y + offsetY);
                            window->DrawList->AddRectFilled(a, b, frameTextCol);
                        } else {
                            ImU32 frameTextCol = GetColorU32(ImGui::GetUniformLocColor(uni->loc));
                            a                  = ImVec2(frame_bb.Min.x + g.Style.FramePadding.x, frame_bb.Min.y - g.Style.FramePadding.y * 0.25f + offsetY);
                            b                  = ImVec2(frame_bb.Min.x - g.Style.FramePadding.x + puPaneWidth, frame_bb.Min.y + g.Style.FramePadding.y * 0.25f + label_size.y + offsetY);
                            window->DrawList->AddRectFilled(a, b, frameTextCol);
                        }

                        if (ImGui_DrawTrashButton(hovered, frame_bb.Min + ImVec2(0, offsetY))) {
                            DelUniform(vKey, vTimeKey->uniformName);
                            return value_changed;
                        }

                        window->DrawList->AddText(frame_bb.Min + ImVec2(20.0f, offsetY), GetColorU32(ImVec4(1, 1, 1, 1)), buffer);

                        bool _showCategory = (puUniformsToEdit.find(vTimeKey->uniformName) != puUniformsToEdit.end());
                        if (ImGui_DrawCheckButton(hovered, frame_bb.Min + ImVec2(puPaneWidth - g.FontSize - g.Style.FramePadding.y, offsetY), _showCategory)) {
                            if (_showCategory)  // trouvé
                            {
                                HideCurveForUniform(vTimeKey->uniformName);
                            } else {
                                ShowCurveForUniform(vKey, vTimeKey->uniformName);
                            }
                            return value_changed;
                        }

                        offsetY += g.FontSize + g.Style.FramePadding.y;
                        float ox = g.Style.FramePadding.x * 2.0f;

                        int idx = 0;
                        for (auto it = vTimeKey->keys.begin(); it != vTimeKey->keys.end(); ++it) {
                            if (vTimeKey->glslType == uType::uTypeEnum::U_FLOAT || vTimeKey->glslType == uType::uTypeEnum::U_VEC2 || vTimeKey->glslType == uType::uTypeEnum::U_VEC3 || vTimeKey->glslType == uType::uTypeEnum::U_VEC4) {
                                if (it->first == 0)
                                    snprintf(buffer, 100, "x %.3f", uni->x);
                                else if (it->first == 1)
                                    snprintf(buffer, 100, "y %.3f", uni->y);
                                else if (it->first == 2)
                                    snprintf(buffer, 100, "z %.3f", uni->z);
                                else if (it->first == 3)
                                    snprintf(buffer, 100, "w %.3f", uni->w);
                            }

                            if (vTimeKey->glslType == uType::uTypeEnum::U_BOOL || vTimeKey->glslType == uType::uTypeEnum::U_BVEC2 || vTimeKey->glslType == uType::uTypeEnum::U_BVEC3 || vTimeKey->glslType == uType::uTypeEnum::U_BVEC4) {
                                if (it->first == 0)
                                    snprintf(buffer, 100, "x %s", uni->bx ? "true" : "false");
                                else if (it->first == 1)
                                    snprintf(buffer, 100, "y %s", uni->by ? "true" : "false");
                                else if (it->first == 2)
                                    snprintf(buffer, 100, "z %s", uni->bz ? "true" : "false");
                                else if (it->first == 3)
                                    snprintf(buffer, 100, "w %s", uni->bw ? "true" : "false");
                            }

                            if (vTimeKey->glslType == uType::uTypeEnum::U_INT || vTimeKey->glslType == uType::uTypeEnum::U_IVEC2 || vTimeKey->glslType == uType::uTypeEnum::U_IVEC3 || vTimeKey->glslType == uType::uTypeEnum::U_IVEC4) {
                                if (it->first == 0)
                                    snprintf(buffer, 100, "x %i", uni->ix);
                                else if (it->first == 1)
                                    snprintf(buffer, 100, "y %i", uni->iy);
                                else if (it->first == 2)
                                    snprintf(buffer, 100, "z %i", uni->iz);
                                else if (it->first == 3)
                                    snprintf(buffer, 100, "w %i", uni->iw);
                            }

                            /*if (vTimeKey->glslType == uType::uTypeEnum::U_MAT2 ||
                                vTimeKey->glslType == uType::uTypeEnum::U_MAT3 ||
                                vTimeKey->glslType == uType::uTypeEnum::U_MAT4)
                            {
                                if (it->first == 0) snprintf(buffer, 100, "x %.3f", uni->x);
                                else if (it->first == 1) snprintf(buffer, 100, "y %.3f", uni->y);
                                else if (it->first == 2) snprintf(buffer, 100, "z %.3f", uni->z);
                                else if (it->first == 3) snprintf(buffer, 100, "w %.3f", vTimeKey->uni->w);
                            }*/

                            if (!showGraph)  // pas curve graph
                            {
                                // draw line rect
                                a = ImVec2(frame_bb.Min.x + g.Style.FramePadding.x, frame_bb.Min.y - g.Style.FramePadding.y * 0.25f + offsetY);
                                b = ImVec2(frame_bb.Max.x - g.Style.FramePadding.x, frame_bb.Min.y + g.Style.FramePadding.y * 0.25f + label_size.y + offsetY);
                                window->DrawList->AddRectFilled(a, b, GetColorU32(idx % 2 ? puThickLinesLight : puThickLinesDark));
                            } else if (puUniformsToEdit.find(vTimeKey->uniformName) != puUniformsToEdit.end()) {
                                if (puUniformsToEdit[vTimeKey->uniformName] & puTimeLineItemAxisMasks[it->first]) {
                                    // draw line rect rgba
                                    a              = ImVec2(frame_bb.Min.x + g.Style.FramePadding.x, frame_bb.Min.y - g.Style.FramePadding.y * 0.25f + offsetY);
                                    b              = ImVec2(frame_bb.Min.x - g.Style.FramePadding.x + puPaneWidth, frame_bb.Min.y + g.Style.FramePadding.y * 0.25f + label_size.y + offsetY);
                                    ImVec4 colLine = ImVec4(0.9f, 0.1f, 0.1f, 0.6f);
                                    if (it->first == 1) colLine = ImVec4(0.1f, 0.9f, 0.1f, 0.6f);
                                    if (it->first == 2) colLine = ImVec4(0.1f, 0.1f, 0.9f, 0.6f);
                                    if (it->first == 3) colLine = ImVec4(0.6f, 0.6f, 0.6f, 0.6f);
                                    window->DrawList->AddRectFilled(a, b, GetColorU32(colLine));
                                }
                            }

                            // draw trash button
                            if (ImGui_DrawTrashButton(hovered, frame_bb.Min + ImVec2(0, offsetY))) {
                                DelUniformComponent(vKey, vTimeKey->uniformName, it->first);
                                return value_changed;
                            }

                            // draw text
                            label_size = CalcTextSize(buffer, nullptr, true);
                            window->DrawList->AddText(frame_bb.Min + ImVec2(25.0f + ox, offsetY), GetColorU32(ImVec4(1, 1, 1, 1)), buffer);

                            bool _showItem = false;
                            if (puUniformsToEdit.find(vTimeKey->uniformName) != puUniformsToEdit.end()) {
                                if (puUniformsToEdit[vTimeKey->uniformName] & puTimeLineItemAxisMasks[it->first]) {
                                    _showItem = true;
                                }
                            }

                            if (ImGui_DrawCheckButton(hovered, frame_bb.Min + ImVec2(puPaneWidth - g.FontSize - g.Style.FramePadding.y, offsetY), _showItem)) {
                                if (_showItem) {
                                    // remove axis
                                    HideCurveForUniformAxis(vTimeKey->uniformName, it->first);
                                } else {
                                    // add axis
                                    ShowCurveForUniformAxis(vTimeKey->uniformName, it->first);
                                }
                                return value_changed;
                            }

                            offsetY += g.FontSize + g.Style.FramePadding.y;
                            idx++;
                        }
                    }
                }
            } else {
                break;
            }
        }
    }
    ImGui::PopClipRect();

    // draw horizontal time steps
    int startFrame = (int)(-puPaneOffsetX / puStepSize);
    int endFrame   = startFrame + countFrames;
    ImGui_DrawHTimeBar(puPaneOffsetX, frame_bb, countFrames, startFrame);
    startFrame *= puStepScale;
    endFrame *= puStepScale;

    // draw vertical value steps
    if (showGraph) {
        ImGui_DrawVValueBar(puPaneOffsetY, frame_bb);

        // re arrange

        if (ImGui_DrawButton(ICON_NDP2_STRETCH_TO_PAGE_OUTLINE "##ReArrangeViewOnContent", "Arrange view on content", frame_bb.Min + ImVec2(puPaneWidth, 0.0f), frame_bb.Min + ImVec2(puPaneWidth + pucurveBarWidth, putimeBarHeight), hovered)) {
            ReArrangeForViewContent(vKey, viewRc, &puPaneOffsetX, &puPaneOffsetY);
        }
    }

    offsetY = label_size.y + puTextOffsetY * 2.0f + putimeBarHeight;

    if (!puUniformsToEdit.empty()) {
        ImGui::PushClipRect(ImVec2(frame_bb.Min.x + puPaneWidth + pucurveBarWidth, frame_bb.Min.y + putimeBarHeight), frame_bb.Max, false);
    } else {
        ImGui::PushClipRect(ImVec2(frame_bb.Min.x + puPaneWidth, frame_bb.Min.y + putimeBarHeight), frame_bb.Max, false);
    }

    // Draw Frame Range
    const ImU32 frame_range_col = GetColorU32(puRangeFrame);
    float startRangeOffset      = puPaneOffsetX + (int)(vKey->puTimeLine.rangeFrames.x * puStepSize) / puStepScale;
    float endRangeOffset        = puPaneOffsetX + (int)(vKey->puTimeLine.rangeFrames.y * puStepSize) / puStepScale;
    window->DrawList->AddRectFilled(frame_bb.Min + ImVec2(startRangeOffset, putimeBarHeight), frame_bb.Min + ImVec2(endRangeOffset, frame_bb.Max.y), frame_range_col, 0.0f);

    if (!showGraph)  // uniforms list
    {
        value_changed |= ImGui_DrawTimeLinePointList(vKey, offsetY, puPaneOffsetX, puUniformsPaneOffsetY, frame_bb, startFrame, endFrame, hovered);
    } else  // curve graph
    {
        value_changed |= ImGui_DrawTimeLineCurveGraph(vKey, offsetY, puPaneOffsetX, puPaneOffsetY, frame_bb, startFrame, endFrame, hovered);
    }

    // draw current frame grabber
    ImGuiID grabberId = 0;
    if (timeBarRc.Contains(g.IO.MousePos)) grabberId = id;
    ImGui_DrawGrabber(grabberId, puCurrentFrame, (int)puPaneOffsetX, frame_bb);

    ImGui_DrawMouseSelectionRect(vKey, ImVec4(1, 1, 0, 1), 1.0f);

    ImGui::PopClipRect();

    value_changed |= DoSelection(vKey, id, frame_bb, viewRc, timeBarRc, valueBarRc);

    return value_changed;
}

bool TimeLineSystem::ImGui_DrawHTimeBar(float vPaneOffsetX, ImRect vFrameBB, int vCountFrames, int vStartFrame) {
    ZoneScoped;

    const bool res = false;

    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    // ImGuiContext& g = *GImGui;

    char buffer[100] = "\0";

    if (!puUniformsToEdit.empty()) {
        ImGui::PushClipRect(ImVec2(vFrameBB.Min.x + puPaneWidth + pucurveBarWidth, vFrameBB.Min.y), vFrameBB.Max, false);
    } else {
        ImGui::PushClipRect(ImVec2(vFrameBB.Min.x + puPaneWidth, vFrameBB.Min.y), vFrameBB.Max, false);
    }

    const float startOffset = fmod(vPaneOffsetX, puStepSize);
    {
        for (int i = 0; i < vCountFrames; i++) {
            const int frame = vStartFrame + i;
            float lineThick = 1.0f;
            if (frame == 0) {
                lineThick = 4.0f;
            }
            snprintf(buffer, 100, "%i", frame * puStepScale);
            const ImVec2 label_size = CalcTextSize(buffer, nullptr, true);

            window->DrawList->AddText(ImVec2(vFrameBB.Min.x + startOffset + puStepSize * i - label_size.x * 0.5f, vFrameBB.Min.y + putimeBarHeight - puTextOffsetY - label_size.y), GetColorU32(ImVec4(1, 1, 1, 1)), buffer);

            window->DrawList->AddLine(ImVec2(vFrameBB.Min.x + startOffset + puStepSize * i, vFrameBB.Min.y + putimeBarHeight - label_size.y * 0.4f), ImVec2(vFrameBB.Min.x + startOffset + puStepSize * i, vFrameBB.Max.y),
                                      GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 0.5f)), lineThick);
        }

        // add separation
        window->DrawList->AddLine(ImVec2(vFrameBB.Min.x, vFrameBB.Min.y + putimeBarHeight), ImVec2(vFrameBB.Max.x, vFrameBB.Min.y + putimeBarHeight), GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 0.5f)));
    }

    ImGui::PopClipRect();

    return res;
}

bool TimeLineSystem::ImGui_DrawVValueBar(float vPaneOffsetY, ImRect vFrameBB) {
    ZoneScoped;

    const bool res = false;

    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    // ImGuiContext& g = *GImGui;

    char buffer[100] = "\0";

    ImGui::PushClipRect(ImVec2(vFrameBB.Min.x + puPaneWidth, vFrameBB.Min.y + putimeBarHeight), vFrameBB.Max, false);

    const int countValue = (int)ct::ceil((vFrameBB.Max.y - vFrameBB.Min.y) / pucurveStepSize);
    const int startValue = (int)(-vPaneOffsetY / pucurveStepSize);
    // int endValue = startValue + countValue;
    const float startOffset = fmod(vPaneOffsetY, pucurveStepSize);

    {
        const ImVec4 lineCol = ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
        for (int i = 0; i < countValue; i++) {
            float value     = (startValue + i) * -1.0f * pucurveStepScale;
            float lineThick = 1.0f;
            if (IS_FLOAT_EQUAL(value, 0.0f)) {
                value     = 0.0f;  // pour eviter l'affichage de -0.0
                lineThick = 4.0f;
            }
            snprintf(buffer, 100, "%.3f", value);
            const ImVec2 label_size = CalcTextSize(buffer, nullptr, true);

            window->DrawList->AddText(ImVec2(vFrameBB.Min.x + puPaneWidth, vFrameBB.Min.y + startOffset + pucurveStepSize * i - label_size.y * 0.5f), GetColorU32(ImVec4(1, 1, 1, 1)), buffer);

            window->DrawList->AddLine(ImVec2(vFrameBB.Min.x + puPaneWidth + pucurveBarWidth - label_size.x * 0.4f, vFrameBB.Min.y + startOffset + pucurveStepSize * i), ImVec2(vFrameBB.Max.x, vFrameBB.Min.y + startOffset + pucurveStepSize * i),
                                      GetColorU32(lineCol), lineThick);
        }

        // add separation
        window->DrawList->AddLine(ImVec2(vFrameBB.Min.x + puPaneWidth + pucurveBarWidth, vFrameBB.Min.y + startOffset), ImVec2(vFrameBB.Min.x + puPaneWidth + pucurveBarWidth, vFrameBB.Min.y + startOffset + pucurveStepSize * countValue),
                                  GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 0.5f)));
    }

    ImGui::PopClipRect();

    return res;
}

bool TimeLineSystem::ImGui_DrawButton(const char* vLabel, const char* vHelp, ImVec2 vStart, ImVec2 vEnd, bool
                                      /*vHovered*/) {
    ZoneScoped;

    using namespace ImGui;

    ImGuiContext& g     = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems) return false;

    const ImRect bb(vStart + g.Style.FramePadding, vEnd - g.Style.FramePadding);

    // Render
    ImVec2 center = bb.GetCenter();
    const int id  = ImGui::IncPUSHID();

    ImGui::SetItemAllowOverlap();

    bool hovered, held;
    const bool pressed = ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick);

    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    window->DrawList->AddRectFilled(bb.Min, bb.Max, col, 2);

    ImVec2 label_size = CalcTextSize(vLabel, nullptr, true);
    PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
    RenderTextClipped(bb.GetCenter() - label_size * 0.5f, bb.Max, vLabel, nullptr, &label_size);
    PopStyleColor();

    if (bb.Contains(g.IO.MousePos)) {
        ImGui::SetTooltip(vHelp);
    }

    return pressed;
}

bool TimeLineSystem::ImGui_DrawTimeLinePointList(ShaderKeyPtr vKey, float offsetY, float vPaneOffsetX, float vPaneOffsetY, ImRect frame_bb, int startFrame, int endFrame, bool hovered) {
    ZoneScoped;

    bool value_changed = false;

    if (vKey) {
        using namespace ImGui;

        // ImGuiWindow* window = GetCurrentWindow();
        ImGuiContext& g = *GImGui;

        size_t paningIndex = 0;

        // draw items points
        offsetY += g.FontSize + g.Style.FramePadding.y;
        for (auto sec : vKey->puTimeLine.timeLine) {
            if (++paningIndex < vPaneOffsetY) continue;

            UniformTimeKey* vTimeKey = &sec.second;

            UniformVariantPtr uni = vKey->GetUniformByName(vTimeKey->uniformName);

            if (uni) {
                if (uni->loc >= 0 || puShowUnUsed) {
                    if (frame_bb.Min.y + offsetY < frame_bb.Max.y) {
                        float oy = offsetY;
                        for (auto itChan = vTimeKey->keys.begin(); itChan != vTimeKey->keys.end(); ++itChan) {
                            if (!itChan->second.empty()) {
                                for (auto frame : itChan->second) {
                                    const int f = frame.first;  // frame
                                    if (frame.second.use_count()) {
                                        if (f >= startFrame && f <= endFrame) {
                                            const float ox      = vPaneOffsetX + (int)(f * puStepSize) / puStepScale;
                                            const ImVec2 center = ImVec2(frame_bb.Min.x + ox, frame_bb.Min.y + oy + g.FontSize * 0.5f);

                                            value_changed |= ImGui_DrawGraphPointButton(vTimeKey, frame.second, f, center, hovered);
                                        }
                                    }
                                }
                                oy += g.FontSize + g.Style.FramePadding.y;
                            }
                        }
                        offsetY += (g.FontSize + g.Style.FramePadding.y) * (float)(vTimeKey->keys.size() + 1);
                    } else {
                        break;
                    }
                }
            }
        }
    }

    return value_changed;
}

bool TimeLineSystem::ImGui_DrawTimeLineCurveGraph(ShaderKeyPtr vKey, float vOffsetY, float vPaneOffsetX, float vPaneOffsetY, ImRect vFrameBB, int vStartFrame, int vEndFrame, bool vHovered) {
    ZoneScoped;

    bool value_changed = false;

    if (vKey) {
        using namespace ImGui;

        ImGuiWindow* window = GetCurrentWindow();
        ImGuiContext& g     = *GImGui;

        window->DrawList->ChannelsSplit(3);

        // draw curve graph
        vOffsetY += g.FontSize + g.Style.FramePadding.y;
        if (!puUniformsToEdit.empty()) {
            for (auto it = puUniformsToEdit.begin(); it != puUniformsToEdit.end(); ++it) {
                if (vKey->puTimeLine.timeLine.find(it->first) != vKey->puTimeLine.timeLine.end()) {
                    UniformTimeKey* vTimeKey = &vKey->puTimeLine.timeLine[it->first];

                    UniformVariantPtr uni = vKey->GetUniformByName(vTimeKey->uniformName);

                    if (uni) {
                        if (uni->loc >= 0 || puShowUnUsed) {
                            if (vFrameBB.Min.y + vOffsetY < vFrameBB.Max.y) {
                                for (int axis = 0; axis < 4; axis++) {
                                    if (it->second & puTimeLineItemAxisMasks[axis]) {
                                        if (vTimeKey->keys.find(axis) != vTimeKey->keys.end())  // trouvé
                                        {
                                            auto lineStruct = &vTimeKey->keys[axis];

                                            if (lineStruct->size() > 0) {
                                                int _lastFrame   = lineStruct->begin()->first;
                                                auto _lastStruct = vTimeKey->keys[axis].begin()->second;
                                                ImVec2 _lastPos  = ImVec2(0.0f, 0.0f);
                                                for (auto frame : *lineStruct) {
                                                    int curFrame = frame.first;  // frame
                                                    auto _Struct = frame.second;
                                                    if (curFrame >= vStartFrame && curFrame <= vEndFrame) {
                                                        // float oy = 0.0f;
                                                        float v = 0.0f;
                                                        _Struct->GetValue(&v, axis);
                                                        ImVec2 pos;
                                                        pos.x = vFrameBB.Min.x + vPaneOffsetX + GetLocalPosFromFrame(curFrame);
                                                        pos.y = vFrameBB.Min.y + vPaneOffsetY - GetLocalPosFromValue(v);

                                                        if (curFrame != _lastFrame) {
                                                            ImVec4 colLine = ImVec4(0.9f, 0.1f, 0.1f, 0.6f);
                                                            if (axis == 1) colLine = ImVec4(0.1f, 0.9f, 0.1f, 0.6f);
                                                            if (axis == 2) colLine = ImVec4(0.1f, 0.1f, 0.9f, 0.6f);
                                                            if (axis == 3) colLine = ImVec4(0.6f, 0.6f, 0.6f, 0.6f);

                                                            value_changed |= ImGui_DrawGraphLineButton(vTimeKey, _lastFrame, curFrame, _lastPos, pos, _lastStruct, _Struct, colLine, vFrameBB, axis);
                                                        }

                                                        window->DrawList->ChannelsSetCurrent(2);
                                                        value_changed |= ImGui_DrawGraphPointButton(vTimeKey, _Struct, curFrame, pos, vHovered);

                                                        _lastFrame  = curFrame;
                                                        _lastStruct = _Struct;
                                                        _lastPos    = pos;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        window->DrawList->ChannelsMerge();
    }

    return value_changed;
}

bool TimeLineSystem::ImGui_DrawGraphLineButton(UniformTimeKey* vKeyStruct, int vLastFrame, int vFrame, ImVec2 vStartPoint, ImVec2 vEndPoint, std::shared_ptr<UploadableUniform> vStartStruct, std::shared_ptr<UploadableUniform> vEndStruct,
                                               ImVec4 vColor, ImRect vFrame_bb, int vAxis) {
    ZoneScoped;

    bool res = false;

    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    ImGuiContext& g     = *GImGui;

    const float thick = puTextOffsetY * 0.5f;

    if (vKeyStruct && vStartStruct && vEndStruct) {
        auto interpolation_mode = GetTimeLineSegmentInterpolationMode(vStartStruct, vEndStruct);

        if (interpolation_mode == TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_LINEAR) {
            if (puShowInterpolatedPoints) {
                // display pre calculated interpolated values
                window->DrawList->ChannelsSetCurrent(1);
                ImVec4 col           = vColor;
                col.w                = 1.0f;
                const int startFrame = vLastFrame / puStepScale;
                const int endFrame   = vFrame / puStepScale;
                for (int i = startFrame; i <= endFrame; i++) {
                    int frame      = i * puStepScale;
                    const int posX = (int)puPaneOffsetX + GetLocalPosFromFrame(frame);
                    float v        = 0.0f;
                    if (vKeyStruct->values[frame].use_count()) {
                        if (vKeyStruct->values[frame]->GetValue(&v, vAxis)) {
                            const int posY = (int)(puPaneOffsetY - GetLocalPosFromValue(v));
                            ImGui_DrawGraphPoint(vFrame_bb.Min + ImVec2((float)posX, (float)posY), col);
                        }
                    }
                }
            }

            window->DrawList->AddLine(vStartPoint, vEndPoint, GetColorU32(vColor), thick);
        } else if (interpolation_mode == TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_QUADRATIC) {
            if (puShowInterpolatedPoints) {
                // display pre calculated interpolated values
                window->DrawList->ChannelsSetCurrent(1);
                ImVec4 col           = vColor;
                col.w                = 1.0f;
                const int startFrame = vLastFrame / puStepScale;
                const int endFrame   = vFrame / puStepScale;
                for (int i = startFrame; i <= endFrame; i++) {
                    int frame      = i * puStepScale;
                    const int posX = (int)puPaneOffsetX + GetLocalPosFromFrame(frame);
                    float v        = 0.0f;
                    if (vKeyStruct->values[frame].use_count()) {
                        if (vKeyStruct->values[frame]->GetValue(&v, vAxis)) {
                            const int posY  = (int)(puPaneOffsetY - GetLocalPosFromValue(v));
                            const ImVec2 pt = vFrame_bb.Min + ImVec2((float)posX, (float)posY);
                            ImGui_DrawGraphPoint(pt, col);
                            {
                                const float r = puTextOffsetY * 0.75f;
                                if (ct::fvec2(g.IO.MousePos.x - pt.x, g.IO.MousePos.y - pt.y).lengthSquared() < r * r) {
                                    SetTooltip("f = %i\nv = %.3f", frame, v);
                                }
                            }
                        }
                    }
                }
            }

            // display bezier
            window->DrawList->ChannelsSetCurrent(0);
            const ct::fvec2 sv = GetLocalPosFromFrameValue(vStartStruct->bezierControlStartPoint);
            const ct::fvec2 ev = GetLocalPosFromFrameValue(vEndStruct->bezierContorlEndPoint);
            ImVec2 middleSplinePoint;
            if (vStartStruct->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_RIGHT || vStartStruct->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH)
                middleSplinePoint = vStartPoint + ImVec2(sv.x, -sv.y);
            else if (vEndStruct->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_LEFT || vEndStruct->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH)
                middleSplinePoint = vEndPoint - ImVec2(ev.x, -ev.y);
            else {
                CTOOL_DEBUG_BREAK;
            }
            window->DrawList->AddBezierQuadratic(vStartPoint, middleSplinePoint, vEndPoint, GetColorU32(vColor), thick);

            if (puShowControlPoints) {
                const int nFrames = vFrame - vLastFrame;

                if (vStartStruct->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_RIGHT || vStartStruct->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH) {
                    window->DrawList->AddLine(vStartPoint, middleSplinePoint, GetColorU32(vColor), thick);
                    res |= ImGui_DrawGraphSplinePointButton(vKeyStruct, vLastFrame, vStartPoint, middleSplinePoint, vStartStruct, nFrames, true, vAxis, vFrame_bb);
                }

                if (vEndStruct->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_LEFT || vEndStruct->timeLineHandlerType == TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH) {
                    window->DrawList->AddLine(vEndPoint, middleSplinePoint, GetColorU32(vColor), thick);
                    res |= ImGui_DrawGraphSplinePointButton(vKeyStruct, vFrame, vEndPoint, middleSplinePoint, vEndStruct, nFrames, false, vAxis, vFrame_bb);
                }
            }
        } else if (interpolation_mode == TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_CUBIC) {
            if (puShowInterpolatedPoints) {
                // display pre calculated interpolated values
                window->DrawList->ChannelsSetCurrent(1);
                ImVec4 col           = vColor;
                col.w                = 1.0f;
                const int startFrame = vLastFrame / puStepScale;
                const int endFrame   = vFrame / puStepScale;
                for (int i = startFrame; i <= endFrame; i++) {
                    int frame      = i * puStepScale;
                    const int posX = (int)puPaneOffsetX + GetLocalPosFromFrame(frame);
                    float v        = 0.0f;
                    if (vKeyStruct->values[frame].use_count()) {
                        if (vKeyStruct->values[frame]->GetValue(&v, vAxis)) {
                            const int posY  = (int)(puPaneOffsetY - GetLocalPosFromValue(v));
                            const ImVec2 pt = vFrame_bb.Min + ImVec2((float)posX, (float)posY);
                            ImGui_DrawGraphPoint(pt, col);
                            {
                                const float r = puTextOffsetY * 0.75f;
                                if (ct::fvec2(g.IO.MousePos.x - pt.x, g.IO.MousePos.y - pt.y).lengthSquared() < r * r) {
                                    SetTooltip("f = %i\nv = %.3f", frame, v);
                                }
                            }
                        }
                    }
                }
            }

            // display bezier
            window->DrawList->ChannelsSetCurrent(0);
            const ct::fvec2 sv            = GetLocalPosFromFrameValue(vStartStruct->bezierControlStartPoint);
            const ct::fvec2 ev            = GetLocalPosFromFrameValue(vEndStruct->bezierContorlEndPoint);
            const ImVec2 startSplinePoint = vStartPoint + ImVec2(sv.x, -sv.y);
            const ImVec2 endSplinePoint   = vEndPoint - ImVec2(ev.x, -ev.y);
            window->DrawList->AddBezierCubic(vStartPoint, startSplinePoint, endSplinePoint, vEndPoint, GetColorU32(vColor), thick);

            if (puShowControlPoints) {
                const int nFrames = vFrame - vLastFrame;

                window->DrawList->AddLine(vStartPoint, startSplinePoint, GetColorU32(vColor), thick);
                res |= ImGui_DrawGraphSplinePointButton(vKeyStruct, vLastFrame, vStartPoint, startSplinePoint, vStartStruct, nFrames, true, vAxis, vFrame_bb);

                window->DrawList->AddLine(vEndPoint, endSplinePoint, GetColorU32(vColor), thick);
                res |= ImGui_DrawGraphSplinePointButton(vKeyStruct, vFrame, vEndPoint, endSplinePoint, vEndStruct, nFrames, false, vAxis, vFrame_bb);
            }
        }
    } else {
        window->DrawList->ChannelsSetCurrent(0);
        window->DrawList->AddLine(vStartPoint, vEndPoint, GetColorU32(vColor), thick);
    }

    return res;
}

bool TimeLineSystem::ImGui_DrawGraphPointButton(UniformTimeKey* vKeyStruct, std::shared_ptr<UploadableUniform> vStruct, int vFrame, ImVec2 vPoint, bool vHovered) {
    ZoneScoped;

    const bool res = false;

    using namespace ImGui;

    // ImGuiWindow* window = GetCurrentWindow();
    ImGuiContext& g = *GImGui;

    SelectIfContainedInMouseSelectionRect(vStruct, vPoint);

    if (vKeyStruct && vStruct) {
        const float r = puTextOffsetY * 0.9f;
        ImVec4 c      = ImVec4(0.8f, 0.8f, 0.8f, 1);

        if (IsKeyInSelection(vStruct)) {
            c = ImVec4(1, 1, 0, 1);
        }

        if (vHovered && puUseMouseSelectionType != TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_GRABBER_MOVING) {
            if (ct::fvec2(g.IO.MousePos.x - vPoint.x, g.IO.MousePos.y - vPoint.y).lengthSquared() < r * r) {
                c                 = ImVec4(1, 0, 1, 1);
                puMouseOverButton = true;

                if (g.IO.MouseReleased[0]) {
                    if (IsKeyInSelection(vStruct))
                        RemoveKeyFromSelection(vStruct);
                    else
                        AddKeyToSelection(vStruct);
                }

                if (g.IO.MouseDoubleClicked[0]) {
                    GoToFrame(vFrame);
                }
            } else {
                if (puUseMouseSelectionType != TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_RECTANGLE && !puMouseStartOverButton) {
                    if (g.IO.MouseReleased[0] && !g.IO.KeyCtrl)  // left click and not control + click
                    {
                        RemoveKeyFromSelection(vStruct);
                    }
                }
            }
        }

        ImGui_DrawGraphPoint(vPoint, c);
    }

    return res;
}

void TimeLineSystem::ImGui_DrawGraphPoint(ImVec2 vPoint, ImVec4 vColor) {
    ZoneScoped;

    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    // ImGuiContext& g = *GImGui;

    const float r = puTextOffsetY * 0.9f;
    window->DrawList->AddCircleFilled(vPoint, r, GetColorU32(vColor), 4);
}

bool TimeLineSystem::ImGui_DrawGraphSplinePointButton(UniformTimeKey* vKeyStruct, int vFrame, ImVec2 vBasePoint, ImVec2 vControlPoint, std::shared_ptr<UploadableUniform> vStruct, int /*vCountFrames*/, bool vIsStartOrEnd, int vAxis,
                                                      ImRect /*vFrame_bb*/) {
    ZoneScoped;

    bool res = false;

    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();
    ImGuiContext& g     = *GImGui;

    if (vKeyStruct && vStruct.use_count()) {
        const float r = puTextOffsetY * 0.75f;
        ImVec4 c      = ImVec4(0, 1, 1, 1);

        if (puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_POINT || puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_CONTROL_POINT) {
            if (g.IO.MouseDown[0])  // left click
            {
                if (vKeyStruct->currentEditedPoint == vStruct && vKeyStruct->splineStartButtonDown && vKeyStruct->currentAxisButton == vAxis) {
                    if (vKeyStruct->currentStartButton == vFrame && vIsStartOrEnd) {
                        c = ImVec4(1, 0, 1, 1);

                        const ImVec2 cPos   = g.IO.MousePos;
                        const ImVec2 vec = cPos - vBasePoint;

                        // float nFrames = (float)vCountFrames;

                        vStruct->bezierControlStartPoint.x = GetFrameFromLocalPos(vec.x);
                        vStruct->bezierControlStartPoint.y = GetValueFromLocalPos(-vec.y);

                        if (puBezierHandler == TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_FREE) {
                        } else if (puBezierHandler == TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_ALIGNED) {
                            vStruct->bezierContorlEndPoint = vStruct->bezierControlStartPoint.GetNormalized() * vStruct->bezierContorlEndPoint.length();
                        } else if (puBezierHandler == TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_ALIGNED_SYMMETRIC) {
                            auto it = vKeyStruct->keys[vAxis].find(vFrame);
                            if (!vKeyStruct->keys[vAxis].empty()) {
                                const auto itBegin   = vKeyStruct->keys[vAxis].begin();
                                const int firstFrame = itBegin->first;
                                if (vFrame > firstFrame) {
                                    it--;
                                    vStruct->bezierContorlEndPoint = vStruct->bezierControlStartPoint;
                                }
                            }
                        }

                        // SetTooltip("Start Spline Button Clicked %i\n Angle %.2f\n Len %.2f", vKeyStruct->currentStartButton,
                        //	vStruct->bezierAngle, vStruct->bezierLen.x);
                    }
                } else if (vKeyStruct->currentEditedPoint == vStruct && vKeyStruct->splineEndButtonDown && vKeyStruct->currentAxisButton == vAxis) {
                    if (vKeyStruct->currentEndButton == vFrame && !vIsStartOrEnd) {
                        c = ImVec4(1, 0, 1, 1);

                        const ImVec2 cPos   = g.IO.MousePos;
                        const ImVec2 vec = vBasePoint - cPos;

                        // float nFrames = (float)vCountFrames;

                        vStruct->bezierContorlEndPoint.x = GetFrameFromLocalPos(vec.x);
                        vStruct->bezierContorlEndPoint.y = GetValueFromLocalPos(-vec.y);

                        if (puBezierHandler == TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_FREE) {
                        } else if (puBezierHandler == TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_ALIGNED) {
                            vStruct->bezierControlStartPoint = vStruct->bezierContorlEndPoint.GetNormalized() * vStruct->bezierControlStartPoint.length();
                        } else if (puBezierHandler == TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_ALIGNED_SYMMETRIC) {
                            vStruct->bezierControlStartPoint = vStruct->bezierContorlEndPoint;
                        }

                        // SetTooltip("End Spline Button Clicked %i\n Angle %.2f\n Len %.2f", vKeyStruct->currentEndButton,
                        //	vStruct->bezierAngle, vStruct->bezierLen.y);
                    }
                } else {
                    if (ct::fvec2(g.IO.MousePos.x - vControlPoint.x, g.IO.MousePos.y - vControlPoint.y).lengthSquared() < r * r) {
                        c = ImVec4(1, 0, 1, 1);

                        puUseMouseSelectionType = TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_CONTROL_POINT;

                        if (vIsStartOrEnd) {
                            vKeyStruct->splineStartButtonDown = true;
                            vKeyStruct->currentStartButton    = vFrame;
                            vKeyStruct->currentEditedPoint    = vStruct;
                            vKeyStruct->currentAxisButton     = vAxis;
                        } else {
                            vKeyStruct->splineEndButtonDown = true;
                            vKeyStruct->currentEndButton    = vFrame;
                            vKeyStruct->currentEditedPoint  = vStruct;
                            vKeyStruct->currentAxisButton   = vAxis;
                        }
                    }

                    // res = true;
                }
            } else {
                if (vKeyStruct->splineStartButtonDown || vKeyStruct->splineEndButtonDown) {
                    puUseMouseSelectionType = TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_NONE;

                    ReComputeInterpolation(vKeyStruct, vKeyStruct->currentAxisButton);

                    res = true;
                }

                vKeyStruct->splineStartButtonDown = false;
                vKeyStruct->currentStartButton    = -1;
                vKeyStruct->splineEndButtonDown   = false;
                vKeyStruct->currentEndButton      = -1;
                vKeyStruct->currentEditedPoint    = nullptr;
                vKeyStruct->currentAxisButton     = -1;
            }
        }

        if (ct::fvec2(g.IO.MousePos.x - vControlPoint.x, g.IO.MousePos.y - vControlPoint.y).lengthSquared() < r * r) {
            c = ImVec4(1, 0, 1, 1);

            SetTooltip("lens : last %0.3f, next %.3f", vStruct->bezierContorlEndPoint.length(), vStruct->bezierControlStartPoint.length());
        }

        window->DrawList->ChannelsSetCurrent(2);
        ImGui_DrawGraphPoint(vControlPoint, c);
    }

    return res;
}

int TimeLineSystem::GetLocalPosFromFrame(const int& vFrame) {
    ZoneScoped;

    return (int)(vFrame * puStepSize) / puStepScale;
}

float TimeLineSystem::GetLocalPosFromFrame(const float& vFrame) {
    ZoneScoped;

    return (vFrame * puStepSize) / (float)puStepScale;
}

int TimeLineSystem::GetFrameFromLocalPos(const int& vPos) {
    ZoneScoped;

    return (int)((float)(vPos * puStepScale) / puStepSize);
}

float TimeLineSystem::GetFrameFromLocalPos(const float& vPos) {
    return vPos * (float)puStepScale / puStepSize;
}

float TimeLineSystem::GetLocalPosFromValue(const float& vValue) {
    ZoneScoped;

    return vValue * pucurveStepSize / pucurveStepScale;
}

float TimeLineSystem::GetValueFromLocalPos(const float& vPos) {
    ZoneScoped;

    return vPos * pucurveStepScale / pucurveStepSize;
}

ct::fvec2 TimeLineSystem::GetFrameValueFromLocalPos(const ct::fvec2& vPos) {
    ZoneScoped;

    ct::fvec2 p = vPos;
    p.x         = GetFrameFromLocalPos(p.x);
    p.y         = GetValueFromLocalPos(p.y);
    return p;
}

ct::fvec2 TimeLineSystem::GetLocalPosFromFrameValue(const ct::fvec2& vFrameValue) {
    ZoneScoped;

    ct::fvec2 p = vFrameValue;
    p.x         = (float)GetLocalPosFromFrame(p.x);
    p.y         = GetLocalPosFromValue(p.y);
    return p;
}

ct::fvec2 TimeLineSystem::GetFrameValueFromWorldPos(const ct::fvec2& vPos, const ImRect& vZone) {
    ZoneScoped;

    ct::fvec2 p = vPos;
    p.x         = GetFrameFromLocalPos(p.x - vZone.Min.x - puPaneOffsetX);
    p.y         = GetValueFromLocalPos(vZone.Min.y + puPaneOffsetY - p.y);
    return p;
}

ct::fvec2 TimeLineSystem::GetWorldPosFromFrameValue(const ct::fvec2& vFrameValue, const ImRect& vZone) {
    ZoneScoped;

    ct::fvec2 p = vFrameValue;
    p.x         = vZone.Min.x + puPaneOffsetX + GetLocalPosFromFrame(p.x);
    p.y         = vZone.Min.y + puPaneOffsetY - GetLocalPosFromValue(p.y);
    return p;
}

void TimeLineSystem::ImGui_DrawGrabber(const ImGuiID& vId, const int& vFrame, const int& vOffset, const ImRect& frame_bb) {
    ZoneScoped;

    using namespace ImGui;

    ImGuiWindow* window = GetCurrentWindow();

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;

    char buffer[100] = "\0";

    if (!puUniformsToEdit.empty()) {
        ImGui::PushClipRect(ImVec2(frame_bb.Min.x + puPaneWidth + pucurveBarWidth, frame_bb.Min.y), frame_bb.Max, false);
    } else {
        ImGui::PushClipRect(ImVec2(frame_bb.Min.x + puPaneWidth, frame_bb.Min.y), frame_bb.Max, false);
    }

    const int sliderPos = GetLocalPosFromFrame(vFrame);
    const float offset  = (float)(vOffset + sliderPos);
    snprintf(buffer, 100, "%i", vFrame);
    const ImVec2 label_size = CalcTextSize(buffer, nullptr, true);
    window->DrawList->AddRectFilled(frame_bb.Min + ImVec2(offset - label_size.x, putimeBarHeight - label_size.y - puTextOffsetY * 2.0f), frame_bb.Min + ImVec2(offset + label_size.x, putimeBarHeight),
                                    GetColorU32(g.ActiveId == vId ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding * 0.5f);
    window->DrawList->AddLine(ImVec2(offset + frame_bb.Min.x, frame_bb.Min.y + putimeBarHeight + puTextOffsetY), ImVec2(offset + frame_bb.Min.x, frame_bb.Max.y), GetColorU32(g.ActiveId == vId ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), 3.0f);
    window->DrawList->AddText(ImVec2(offset + frame_bb.Min.x - label_size.x * 0.5f, frame_bb.Min.y + putimeBarHeight - puTextOffsetY - label_size.y), GetColorU32(ImVec4(1, 1, 1, 1)), buffer);

    ImGui::PopClipRect();
}

bool TimeLineSystem::ImGui_AutoKeyingButton(const char* vName, bool* vAutoKeying) {
    ZoneScoped;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g  = *GImGui;
    const ImGuiID id = window->GetID(vName);
    const float h    = ImGui::GetFrameHeight();
    const ImVec2 sz  = ImVec2(h, h);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + sz);
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered, held;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick);

    ImU32 col      = ImGui::GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImU32 pointCol = ImGui::GetColorU32(ImVec4(0, 0, 0, 1));
    if (vAutoKeying) {
        if (pressed) {
            *vAutoKeying = !*vAutoKeying;
        }

        if (*vAutoKeying) {
            col      = ImGui::GetColorU32((hovered && held) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
            pointCol = ImGui::GetColorU32(ImVec4(1, 1, 1, 1));
        }
    }

    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min, bb.Max, col, true, g.Style.FrameRounding);

    // Render Filled Circle
    const ImVec2 center = bb.GetCenter();
    window->DrawList->AddCircleFilled(center, sz.y * 0.25f, pointCol, 12);

    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Auto Keying with uniform tuning");

    return pressed;
}

bool TimeLineSystem::ImGui_AbortButton(const char* vName) {
    ZoneScoped;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g  = *GImGui;
    const ImGuiID id = window->GetID(vName);
    const float h    = ImGui::GetFrameHeight();
    const ImVec2 sz  = ImVec2(h, h);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + sz);
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered, held;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick);

    const ImU32 col = ImGui::GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    // ImU32 pointCol = ImGui::GetColorU32(ImVec4(0, 0, 0, 1));

    ImGui::RenderNavHighlight(bb, id);
    ImGui::RenderFrame(bb.Min, bb.Max, col, true, g.Style.FrameRounding);

    ImVec2 center     = bb.GetCenter();
    const bool pushed = ImGui::PushStyleColorWithContrast(ImGuiCol_Button, ImGuiCol_Text, ImGui::CustomStyle::puContrastedTextColor, ImGui::CustomStyle::puContrastRatio);
    window->DrawList->AddRectFilled(bb.Min + sz * 0.25f, bb.Max - sz * 0.25f, ImGui::GetColorU32(ImGuiCol_Text));
    if (pushed) ImGui::PopStyleColor();

    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Abort Rendering");

    return pressed;
}

void TimeLineSystem::ImGui_DrawMouseSelectionRect(ShaderKeyPtr /*vKey*/, ImVec4 vColor, float vThick) {
    ZoneScoped;

    if (puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_RECTANGLE) {
        using namespace ImGui;
        ImGuiWindow* window = GetCurrentWindow();
        // ImGuiContext& g = *GImGui;
        // const ImGuiStyle& style = g.Style;

        window->DrawList->AddRect(ImVec2(puStartMouseClick.x, puStartMouseClick.y), ImVec2(puEndMouseClick.x, puEndMouseClick.y), ImGui::GetColorU32(vColor), 0.0f, ImDrawFlags_RoundCornersAll, vThick);
    }
}

void TimeLineSystem::MoveSelectedKeysWithMouse(ShaderKeyPtr vKey, ImRect vZone, ct::fvec2 vStartPos, ct::fvec2 vEndPos) {
    ZoneScoped;

    if (vKey) {
        ct::fvec2 start  = GetFrameValueFromWorldPos(vStartPos, vZone);
        ct::fvec2 end    = GetFrameValueFromWorldPos(vEndPos, vZone);
        ct::fvec2 offset = end - start;

        if (!offset.emptyAND()) {
            bool showGraph = IsShowingCurves(vKey);  // pas curve graph

            // on cree un temporaire qu'on va utiliser pour explorer et faire les ops sans avoir de probleme d'iterateurs
            auto tmp = vKey->puTimeLine.timeLine;

            // on explore le tmp
            int lastFrame          = 0;
            bool needRecomputation = false;
            for (auto itStruct = tmp.begin(); itStruct != tmp.end(); ++itStruct) {
                std::string name = itStruct->first;
                for (auto itKey = itStruct->second.keys.begin(); itKey != itStruct->second.keys.end(); ++itKey) {
                    int chan = itKey->first;
                    for (auto itChan = itKey->second.begin(); itChan != itKey->second.end(); ++itChan) {
                        int frame = itChan->first;
                        auto st   = vKey->puTimeLine.timeLine[name].keys[chan][frame];
                        if (st.use_count()) {
                            if (IsKeyInSelection(st)) {
                                // on recup la valeur
                                float v = 0.0f;
                                st->GetValue(&v, chan);
                                // on offset la valeur
                                if (showGraph)  // on tune la valeur que si on montre les courbes
                                {
                                    v += offset.y;
                                    st->SetValue(v, chan);
                                }
                                // on va effacer la clef actuelle
                                auto st_copy = std::make_shared<UploadableUniform>();  // on fait une copie locale d'abord
                                st_copy->copy(st);
                                // on offset la frame
                                st_copy->movingFrame += offset.x;
                                // on verifie d'abord si la frame a changé
                                int newFrame = (int)st_copy->movingFrame;
                                if (frame != newFrame) {
                                    if (vKey->puTimeLine.timeLine[name].keys[chan].find(newFrame) == vKey->puTimeLine.timeLine[name].keys[chan].end())  // non trouvé
                                    {
                                        auto keysBefore = GetFramesToList(vKey, name, chan);

                                        // on efface la frame
                                        vKey->puTimeLine.timeLine[name].keys[chan].erase(frame);
                                        // on re met la nouvelle frame
                                        vKey->puTimeLine.timeLine[name].keys[chan][newFrame] = st_copy;
                                        // on remplace dans la selection
                                        st = vKey->puTimeLine.timeLine[name].keys[chan][newFrame];
                                        RemoveKeyFromSelection(st);
                                        AddKeyToSelection(st);

                                        auto keysAfter = GetFramesToList(vKey, name, chan);

                                        ReSizeControlPoints(vKey, name, chan, frame, keysBefore, newFrame, keysAfter);

                                        // newFrame devient frame
                                        frame = newFrame;
                                    }
                                } else {
                                    // on re met la nouvelle frame
                                    vKey->puTimeLine.timeLine[name].keys[chan][frame] = st_copy;
                                }

                                needRecomputation = true;
                            }
                        }

                        lastFrame = frame;
                    }

                    // si un remplacement a eu lieu, on re compute l'interpolation
                    if (needRecomputation) {
                        ReComputeInterpolation(vKey, name, chan);
                        needRecomputation = false;
                    }
                }
            }
        }
    }
}

std::vector<int> TimeLineSystem::GetFramesToList(ShaderKeyPtr vKey, std::string vUniformName, int vComponent) {
    ZoneScoped;

    std::vector<int> res;

    if (vKey) {
        auto st = &vKey->puTimeLine.timeLine[vUniformName].keys[vComponent];

        if (st) {
            for (auto it = st->begin(); it != st->end(); ++it) {
                res.emplace_back(it->first);
            }
        }
    }

    return res;
}

void TimeLineSystem::ReSizeControlPoints(ShaderKeyPtr vKey, std::string vUniformName, int vComponent, int vOldFrame, std::vector<int> vOldFrameKeys, int vNewFrame, std::vector<int> vNewFrameKeys) {
    ZoneScoped;

    // on va redefinir la taille des point de controles
    // qui depend du range de frames
    if (vKey) {
        auto st = &vKey->puTimeLine.timeLine[vUniformName].keys[vComponent];
        if (st) {
            const auto it = st->find(vNewFrame);
            if (it != st->end()) {
                // old
                size_t ofp       = 0;
                const size_t oos = vOldFrameKeys.size();
                for (size_t i = 0; i < oos; ++i) {
                    if (vOldFrameKeys[i] == vOldFrame) {
                        ofp = i;
                        break;
                    }
                }

                // new
                size_t nfp       = 0;
                const size_t nos = vNewFrameKeys.size();
                for (size_t i = 0; i < nos; ++i) {
                    if (vNewFrameKeys[i] == vNewFrame) {
                        nfp = i;
                        break;
                    }
                }

                if (ofp != nfp)  // des key on été insérées plutot que bougées
                {
                    if (nfp == 0) {
                        if (st->at(vNewFrame).use_count()) {
                            st->at(vNewFrame)->bezierControlStartPoint = st->at(vNewFrame)->bezierContorlEndPoint;
                        }
                    }
                    if (nfp == nos - 1) {
                        if (st->at(vNewFrame).use_count()) {
                            st->at(vNewFrame)->bezierContorlEndPoint = st->at(vNewFrame)->bezierControlStartPoint;
                        }
                    }
                }
            }
        }
    }
}

void TimeLineSystem::ShowMouseInfos(ShaderKeyPtr vKey, ImRect vZone, ct::fvec2 vMousePos) {
    ZoneScoped;

    const ct::fvec2 p = GetFrameValueFromWorldPos(vMousePos, vZone);

    if (IsShowingCurves(vKey)) {
        ImGui::SetTooltip("f : %i\nv : %.3f", (int)p.x, p.y);
    } else {
        ImGui::SetTooltip("f : %i\n", (int)p.x);
    }
}

void TimeLineSystem::AddKeyToSelection(std::shared_ptr<UploadableUniform> vStruct) {
    ZoneScoped;

    if (vStruct) {
        puSelectedKeys.emplace(vStruct);
    }
}

void TimeLineSystem::RemoveKeyFromSelection(std::shared_ptr<UploadableUniform> vStruct) {
    ZoneScoped;

    if (vStruct) {
        puSelectedKeys.erase(vStruct);
    }
}

bool TimeLineSystem::IsKeyInSelection(std::shared_ptr<UploadableUniform> vStruct) {
    ZoneScoped;

    if (vStruct) {
        if (puSelectedKeys.find(vStruct) != puSelectedKeys.end()) {
            return true;
        }
    }

    return false;
}

bool TimeLineSystem::IsSelectionExist() {
    ZoneScoped;

    return puSelectedKeys.size();
}

bool TimeLineSystem::DoSelection(ShaderKeyPtr vKey, ImGuiID /*vId*/, ImRect vWidgetZone, ImRect vViewZone, ImRect vTimebarZone, ImRect /*vValueBarZone*/) {
    ZoneScoped;

    bool value_changed = false;

    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    // const ImGuiStyle& style = g.Style;

    // ImGui::SetTooltip("Mouse Over Btn %s", puMouseOverButton ? "true" : "false");

    // mouse move
    if (ImGui::IsWindowHovered() && !IsRendering()) {
        if (!g.IO.MouseDown[0] &&  // pas de bouton gauche enfoncé
            !g.IO.MouseDown[2])    // pas de bouton droite enfoncé
        {
            // ClearActiveID();

            puUseMouseSelectionType = TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_NONE;
            puMouseStartOverButton  = false;
        } else {
            if (g.IO.MouseDown[0])  // bouton gauche enfoncé
            {
                if (vTimebarZone.Contains(g.IO.MousePos) && puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_NONE ||
                    puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_GRABBER_MOVING)  // selection de frame dans la timezone
                {
                    puUseMouseSelectionType   = TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_GRABBER_MOVING;
                    const float mouse_abs_pos = g.IO.MousePos.x - puPaneOffsetX - vWidgetZone.Min.x;
                    const int frameUnderMouse = (int)(mouse_abs_pos * (float)puStepScale / puStepSize);

                    // Apply result
                    if (puCurrentFrame != frameUnderMouse) {
                        GoToFrame(frameUnderMouse);
                        value_changed = true;
                    }
                } else if (vViewZone.Contains(g.IO.MousePos))  // view zone
                {
                    // first click
                    if (puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_NONE) {
                        puStartMouseClick.x     = g.IO.MousePos.x;
                        puStartMouseClick.y     = g.IO.MousePos.y;
                        puUseMouseSelectionType = TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_POINT;
                        if (puMouseOverButton) puMouseStartOverButton = true;
                    }

                    if (!puMouseStartOverButton)  // pas au dessus d'un bouton
                    {
                        // rectangle selection
                        if ((puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_POINT || puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_RECTANGLE)) {
                            if (IS_FLOAT_DIFFERENT(g.IO.MousePos.x, puStartMouseClick.x) || IS_FLOAT_DIFFERENT(g.IO.MousePos.y, puStartMouseClick.y)) {
                                puUseMouseSelectionType = TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_RECTANGLE;
                                puEndMouseClick.x       = g.IO.MousePos.x;
                                puEndMouseClick.y       = g.IO.MousePos.y;
                            }
                        }
                    }

                    if (IsSelectionExist())  // si une selection existe
                    {
                        if (puMouseOverButton && puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_POINT) {
                            puStartMouseClick.x     = g.IO.MousePos.x;
                            puStartMouseClick.y     = g.IO.MousePos.y;
                            puUseMouseSelectionType = TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_MOVE_POINT;
                        }

                        if (puUseMouseSelectionType == TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_MOVE_POINT) {
                            if (IS_FLOAT_DIFFERENT(g.IO.MousePos.x, puStartMouseClick.x) || IS_FLOAT_DIFFERENT(g.IO.MousePos.y, puStartMouseClick.y)) {
                                puEndMouseClick.x = g.IO.MousePos.x;
                                puEndMouseClick.y = g.IO.MousePos.y;

                                MoveSelectedKeysWithMouse(vKey, vWidgetZone, ct::fvec2(puStartMouseClick.x, puStartMouseClick.y), ct::fvec2(puEndMouseClick.x, puEndMouseClick.y));

                                puStartMouseClick.x = puEndMouseClick.x;
                                puStartMouseClick.y = puEndMouseClick.y;
                            }
                        }
                    }
                }
            } else if (g.IO.MouseDown[2])  // bouton milieu enfoncé
            {
                if (vViewZone.Contains(g.IO.MousePos)) {
                    puPaneOffsetX += g.IO.MouseDelta.x;
                    puPaneOffsetY += g.IO.MouseDelta.y;
                }

                puUseMouseSelectionType = TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_NONE;
            }
        }
    }

    return value_changed;
}

void TimeLineSystem::Clean_All(ShaderKeyPtr vKey) {
    ZoneScoped;

    if (vKey) {
        vKey->puTimeLine.timeLine.clear();
    }
}

void TimeLineSystem::Clean_KeepOnlyChangedUniforms(ShaderKeyPtr vKey) {
    ZoneScoped;

    if (vKey) {
        std::list<std::string> UniformsToRemove;
        for (auto itUni = vKey->puTimeLine.timeLine.begin(); itUni != vKey->puTimeLine.timeLine.end(); ++itUni) {
            std::string name          = itUni->first;
            uType::uTypeEnum glslType = itUni->second.glslType;
            bool change               = false;
            for (auto itChan = itUni->second.keys.begin(); itChan != itUni->second.keys.end(); ++itChan) {
                int chan                                = itChan->first;
                std::shared_ptr<UploadableUniform> last = nullptr;
                if (itChan->second.size() > 1) {
                    for (auto frame : itChan->second) {
                        auto curr = frame.second;
                        if (curr.use_count()) {
                            if (frame != *itChan->second.begin()) {
                                switch (glslType) {
                                    case uType::uTypeEnum::U_FLOAT:
                                    case uType::uTypeEnum::U_VEC2:
                                    case uType::uTypeEnum::U_VEC3:
                                    case uType::uTypeEnum::U_VEC4:
                                    case uType::uTypeEnum::U_BOOL:
                                    case uType::uTypeEnum::U_BVEC2:
                                    case uType::uTypeEnum::U_BVEC3:
                                    case uType::uTypeEnum::U_BVEC4: {
                                        change |= IS_FLOAT_DIFFERENT(last->xyzw[chan], curr->xyzw[chan]);
                                    } break;
                                    case uType::uTypeEnum::U_INT:
                                    case uType::uTypeEnum::U_IVEC2:
                                    case uType::uTypeEnum::U_IVEC3:
                                    case uType::uTypeEnum::U_IVEC4: {
                                        change |= (last->ixyzw[chan] != curr->ixyzw[chan]);
                                    } break;
                                    case uType::uTypeEnum::U_MAT2: {
                                        float* arrLast = glm::value_ptr(last->mat4[0]);
                                        float* arrCurr = glm::value_ptr(curr->mat4[0]);
                                        for (int i = 0; i < 4; i++) {
                                            change |= IS_FLOAT_DIFFERENT(arrLast[i], arrCurr[i]);
                                        }
                                    } break;
                                    case uType::uTypeEnum::U_MAT3: {
                                        float* arrLast = glm::value_ptr(last->mat4[0]);
                                        float* arrCurr = glm::value_ptr(curr->mat4[0]);
                                        for (int i = 0; i < 9; i++) {
                                            change |= IS_FLOAT_DIFFERENT(arrLast[i], arrCurr[i]);
                                        }
                                    } break;
                                    case uType::uTypeEnum::U_MAT4: {
                                        float* arrLast = glm::value_ptr(last->mat4[0]);
                                        float* arrCurr = glm::value_ptr(curr->mat4[0]);
                                        for (int i = 0; i < 16; i++) {
                                            change |= IS_FLOAT_DIFFERENT(arrLast[i], arrCurr[i]);
                                        }
                                    } break;
                                    default: break;
                                }
                            }
                            last = curr;
                        }
                    }
                }
            }

            if (!change) {
                UniformsToRemove.emplace_back(name);
            }
        }

        for (auto uni : UniformsToRemove) {
            vKey->puTimeLine.timeLine.erase(uni);
        }
    }
}

TimeLineInfos TimeLineSystem::LoadTimeLineConfig(std::string vConfigFile) {
    ZoneScoped;

    TimeLineInfos timeLineInfos;

    if (!vConfigFile.empty()) {
        std::string file;

        std::ifstream docFile(vConfigFile, std::ios::in);
        if (docFile.is_open()) {
            std::stringstream strStream;

            strStream << docFile.rdbuf();  // read the file

            file = strStream.str();
            ct::replaceString(file, "\r\n", "\n");

            docFile.close();
        }

        // LogVar("-------------------------------------------");
        // LogVar("Load Shader from Params File :" + configFile);

        const auto& lines = ct::splitStringToVector(file, "\n");
        for (const auto& line : lines) {
            // iMaterialColor:vec3:0(0,0.430333;21,0.37845;45,0.340716;68,0.334598;90,0.337801):1(0,0.346698;21,0.381457;45,0.384067;68,0.390679;90,0.330683):2(0,0.330683;21,0.339356;45,0.338691;68,0.405974;90,0.430333)

            const auto& arr = ct::splitStringToVector(line, ":", true);

            auto timekey = UniformTimeKey();
            if (arr.size() == 1) {
                std::string s = arr[0];
                size_t fc     = s.find('[');
                if (fc != std::string::npos) {
                    size_t ec = s.find(']', fc + 1);
                    if (ec != std::string::npos) {
                        s = s.substr(fc + 1, ec - fc - 1);

                        timeLineInfos.rangeFrames = ct::ivariant(s).GetV2();
                    }
                }
            } else {
                for (size_t idx = 0; idx < arr.size(); idx++) {
                    if (idx == 0) timekey.uniformName = arr[idx];
                    if (idx == 1) timekey.glslType = uType::GetGlslTypeFromString(arr[idx]);
                    if (idx == 2) timekey.widget = arr[idx];
                    if (idx > 2) {
                        // 0(0,0.430333;21,0.37845;45,0.340716;68,0.334598;90,0.337801)
                        auto str = arr[idx];
                        if (!str.empty()) {
                            std::string ch = str.substr(0, 1);
                            if (ch == "[") {
                                size_t endCrochet = str.find(']');
                                if (endCrochet != std::string::npos) {
                                    std::string res                                    = str.substr(1, endCrochet - 1);
                                    UniformHelper::FBOSizeForMouseUniformNormalization = ct::fvec2(res, ';');
                                }
                            } else {
                                int chan        = ct::ivariant(ch).GetI();
                                size_t firstPar = str.find('(', 0);
                                if (firstPar != std::string::npos) {
                                    size_t endPar = str.find(')', firstPar);
                                    if (endPar != std::string::npos) {
                                        firstPar++;
                                        // endPar--;
                                        std::string keysStr = str.substr(firstPar, endPar - firstPar);
                                        // keysStr => 0,0.430333;21,0.37845

                                        const auto& keys = ct::splitStringToVector(keysStr, ";");
                                        for (const auto& keys2 : keys) {
                                            const auto& key = ct::splitStringToVector(keys2, ",");
                                            if (key.size() > 1) {
                                                int frame = ct::ivariant(key[0]).GetI();

                                                std::shared_ptr<UploadableUniform> uni = std::make_shared<UploadableUniform>();
                                                uni->uniformName                       = timekey.uniformName;
                                                uni->glslType                          = timekey.glslType;

                                                switch (timekey.glslType) {
                                                    case uType::uTypeEnum::U_FLOAT:
                                                    case uType::uTypeEnum::U_VEC2:
                                                    case uType::uTypeEnum::U_VEC3:
                                                    case uType::uTypeEnum::U_VEC4: {
                                                        uni->useFloat[chan] = 1;
                                                        float v             = ct::fvariant(key[1]).GetF();
                                                        if (timekey.widget == "mouse_2pos_2click" || timekey.widget == "mouse_2press_2click") {
                                                            if (timekey.glslType == uType::uTypeEnum::U_VEC4) {
                                                                uni->xyzw[chan] = v * UniformHelper::FBOSizeForMouseUniformNormalization[chan % 2];
                                                            }
                                                        } else {
                                                            uni->xyzw[chan] = v;
                                                        }
                                                    } break;
                                                    case uType::uTypeEnum::U_BOOL:
                                                    case uType::uTypeEnum::U_BVEC2:
                                                    case uType::uTypeEnum::U_BVEC3:
                                                    case uType::uTypeEnum::U_BVEC4: {
                                                        uni->useBool[chan] = 1;
                                                        float v            = ct::fvariant(key[1]).GetF();
                                                        uni->xyzw[chan]    = v;
                                                    } break;
                                                    case uType::uTypeEnum::U_INT:
                                                    case uType::uTypeEnum::U_IVEC2:
                                                    case uType::uTypeEnum::U_IVEC3:
                                                    case uType::uTypeEnum::U_IVEC4: {
                                                        uni->useInt[chan] = 1;
                                                        int v             = ct::ivariant(key[1]).GetI();
                                                        uni->ixyzw[chan]  = v;
                                                    } break;
                                                    case uType::uTypeEnum::U_MAT2: {
                                                        uni->useMat     = 1;
                                                        const auto& vec = ct::fvariant(key[1]).GetVectorFloat(',');
                                                        if (vec.size() == 4) {
                                                            uni->mat4 = glm::mat2(vec[0], vec[1], vec[2], vec[3]);
                                                        }
                                                    } break;
                                                    case uType::uTypeEnum::U_MAT3: {
                                                        uni->useMat     = 1;
                                                        const auto& vec = ct::fvariant(key[1]).GetVectorFloat(',');
                                                        if (vec.size() == 9) {
                                                            uni->mat4 = glm::mat3(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5], vec[6], vec[7], vec[8]);
                                                        }
                                                    } break;
                                                    case uType::uTypeEnum::U_MAT4: {
                                                        uni->useMat     = 1;
                                                        const auto& vec = ct::fvariant(key[1]).GetVectorFloat(',');
                                                        if (vec.size() == 16) {
                                                            uni->mat4 = glm::mat4(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5], vec[6], vec[7], vec[8], vec[9], vec[10], vec[11], vec[12], vec[13], vec[14], vec[15]);
                                                        }
                                                    } break;
                                                    default: break;
                                                }

                                                // spline control points
                                                if (key.size() == 6) {
                                                    uni->bezierControlStartPoint = ct::fvec2(ct::fvariant(key[2]).GetF(), ct::fvariant(key[3]).GetF());
                                                    uni->bezierContorlEndPoint   = ct::fvec2(ct::fvariant(key[4]).GetF(), ct::fvariant(key[5]).GetF());
                                                }

                                                timekey.keys[chan][frame] = uni;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (!timekey.keys.empty()) {
                timeLineInfos.timeLine[timekey.uniformName] = timekey;

                ReComputeInterpolation(&timeLineInfos, timekey.uniformName);
            }
        }
    }

    return timeLineInfos;
}

bool TimeLineSystem::SaveTimeLineConfig(std::string vConfigFile, TimeLineInfos vTimeLineInfos) {
    ZoneScoped;

    bool res = false;

    if (!vConfigFile.empty()) {
        std::string timeLineStream;

        timeLineStream += "[" + vTimeLineInfos.rangeFrames.string() + "]\n";

        if (!vTimeLineInfos.timeLine.empty()) {
            for (auto it = vTimeLineInfos.timeLine.begin(); it != vTimeLineInfos.timeLine.end(); ++it) {
                uType::uTypeEnum glslType = it->second.glslType;
                std::string widget        = it->second.widget;
                ct::replaceString(widget, ":", "_");

                std::string lineUniform = it->second.uniformName + ":" + ConvertUniformsTypeEnumToString(glslType) + ":" + widget + ":";

                if (widget == "mouse_2pos_2click" || widget == "mouse_2press_2click") {
                    if (glslType == uType::uTypeEnum::U_VEC4) {
                        lineUniform += "[" + UniformHelper::FBOSizeForMouseUniformNormalization.string() + "]:";
                    }
                }

                for (auto itStr = it->second.keys.begin(); itStr != it->second.keys.end(); ++itStr) {
                    int chan = itStr->first;

                    if (itStr != it->second.keys.begin()) lineUniform += ":";

                    lineUniform += ct::toStr(chan) + "(";

                    for (auto itKey = itStr->second.begin(); itKey != itStr->second.end(); ++itKey) {
                        int frame = itKey->first;
                        auto uni  = itKey->second;
                        if (uni.use_count()) {
                            if (itKey != itStr->second.begin()) lineUniform += ";";

                            lineUniform += ct::toStr(frame) + ",";

                            switch (glslType) {
                                case uType::uTypeEnum::U_FLOAT:
                                case uType::uTypeEnum::U_VEC2:
                                case uType::uTypeEnum::U_VEC3:
                                case uType::uTypeEnum::U_VEC4: {
                                    if (widget == "mouse_2pos_2click" || widget == "mouse_2press_2click") {
                                        if (glslType == uType::uTypeEnum::U_VEC4) {
                                            lineUniform += ct::toStr(uni->xyzw[chan] / UniformHelper::FBOSizeForMouseUniformNormalization[chan % 2]);
                                        }
                                    } else {
                                        lineUniform += ct::toStr(uni->xyzw[chan]);
                                    }
                                } break;
                                case uType::uTypeEnum::U_BOOL:
                                case uType::uTypeEnum::U_BVEC2:
                                case uType::uTypeEnum::U_BVEC3:
                                case uType::uTypeEnum::U_BVEC4: {
                                    lineUniform += ct::toStr(uni->xyzw[chan]);
                                } break;
                                case uType::uTypeEnum::U_INT:
                                case uType::uTypeEnum::U_IVEC2:
                                case uType::uTypeEnum::U_IVEC3:
                                case uType::uTypeEnum::U_IVEC4: {
                                    lineUniform += ct::toStr(uni->ixyzw[chan]);
                                } break;
                                case uType::uTypeEnum::U_MAT2: {
                                    for (int i = 0; i < 4; i++) {
                                        if (i > 0) lineUniform += ',';
                                        lineUniform += ct::toStr(glm::value_ptr(uni->mat4)[i]);
                                    }
                                } break;
                                case uType::uTypeEnum::U_MAT3: {
                                    for (int i = 0; i < 9; i++) {
                                        if (i > 0) lineUniform += ',';
                                        lineUniform += ct::toStr(glm::value_ptr(uni->mat4)[i]);
                                    }
                                } break;
                                case uType::uTypeEnum::U_MAT4: {
                                    for (int i = 0; i < 16; i++) {
                                        if (i > 0) lineUniform += ',';
                                        lineUniform += ct::toStr(glm::value_ptr(uni->mat4)[i]);
                                    }
                                } break;
                                default: break;
                            }

                            // les control point du spline
                            lineUniform += "," + uni->bezierControlStartPoint.string(',') + "," + uni->bezierContorlEndPoint.string(',');
                        }
                    }

                    lineUniform += ")";
                }

                lineUniform += "\n";
                timeLineStream += lineUniform;
            }

            std::ofstream configFileWriter(vConfigFile, std::ios::out);

            if (configFileWriter.bad() == false) {
                configFileWriter << timeLineStream;
                configFileWriter.close();
            }

            res = true;
        } else {
            res = false;
        }
    }

    return res;
}

void TimeLineSystem::ShowCurveForUniform(ShaderKeyPtr vKey, std::string vUniformName) {
    ZoneScoped;

    // on va pas faire ca
    // mais plutot verifier les axes disponibles, sinon ca biaises les fittocontent et insertion en corus d'edition
    if (vKey) {
        auto it = vKey->puTimeLine.timeLine.find(vUniformName);
        if (it != vKey->puTimeLine.timeLine.end()) {
            //std::string name = it->first;
            for (auto itChan = it->second.keys.begin(); itChan != it->second.keys.end(); ++itChan) {
                ShowCurveForUniformAxis(vUniformName, itChan->first);
            }
        }
    }
    /*puUniformsToEdit[vUniformName] =
        TimeLineItemAxis::TIMELINE_AXIS_RED |
        TimeLineItemAxis::TIMELINE_AXIS_GREEN |
        TimeLineItemAxis::TIMELINE_AXIS_BLUE |
        TimeLineItemAxis::TIMELINE_AXIS_ALPHA;*/
}

void TimeLineSystem::HideCurveForUniform(std::string vUniformName) {
    ZoneScoped;

    puUniformsToEdit.erase(vUniformName);
}

void TimeLineSystem::ShowCurveForUniformAxis(std::string vUniformName, int vAxis) {
    ZoneScoped;

    puUniformsToEdit[vUniformName] |= puTimeLineItemAxisMasks[vAxis];
}

void TimeLineSystem::HideCurveForUniformAxis(std::string vUniformName, int vAxis) {
    ZoneScoped;

    if (puUniformsToEdit.find(vUniformName) != puUniformsToEdit.end())  // trouvé
    {
        puUniformsToEdit[vUniformName] &= ~puTimeLineItemAxisMasks[vAxis];
        if (puUniformsToEdit[vUniformName] == 0) HideCurveForUniform(vUniformName);
    }
}

bool TimeLineSystem::IsShowingCurves(ShaderKeyPtr vKey) {
    ZoneScoped;

    if (vKey) {
        if (vKey->puTimeLine.timeLine.empty()) puUniformsToEdit.clear();
    }
    return (!puUniformsToEdit.empty());
}