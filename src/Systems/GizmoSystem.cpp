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

#include <Systems/GizmoSystem.h>
#include <Renderer/RenderPack.h>
#include <CodeTree/CodeTree.h>
#include <imgui.h>
#include <Gui/CustomGuiWidgets.h>
#include <Systems/CameraSystem.h>
#include <CodeTree/Parsing/SectionCode.h>
#include <CodeTree/ShaderKey.h>
#include <ctools/Logger.h>
#include <Res/CustomFont2.h>
#include <Uniforms/UniformWidgets.h>

static bool useGizmoCulling = false;

GizmoSystem::GizmoSystem() {
    puGizmoCulling_RenderPack = nullptr;
    puGizmoCulling_Key = nullptr;
    puGizmoCullingUniform = nullptr;

    Init(nullptr);
}

GizmoSystem::~GizmoSystem() {
}

bool GizmoSystem::Init(CodeTreePtr vCodeTree) {
    puActivated = false;
    puUseGizmoCulling = false;
    puCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
    puCurrentGizmoMode = ImGuizmo::MODE::WORLD;
    puGizmoCullingPrimitiveEnum = GizmoCullingPimitiveEnum::GIZMO_CULLING_PRIMITIVE_NONE;
    puNeedOneUniformUpdate = false;

    return true;
}

void GizmoSystem::Unit() {
}

bool GizmoSystem::Use() {
    return puActivated || puNeedOneUniformUpdate;
}

bool GizmoSystem::UseCulling() {
    return puActivated && puUseGizmoCulling || puNeedOneUniformUpdate;
}

bool GizmoSystem::IsActivated() {
    return puActivated;
}

void GizmoSystem::SetActivation(bool vActivation) {
    puActivated = vActivation;

    if (!puActivated) {
        // on doit propoager une derniere fois avant de desactiver les gizmo, pour desactiver le useCulling
        puNeedOneUniformUpdate = true;

        useGizmoCulling = false;
    } else {
        useGizmoCulling = puUseGizmoCulling;
    }
}

bool GizmoSystem::DrawMenu() {
    bool change = false;

    if (puActivated) {
        if (ImGui::BeginMenu(ICON_NDP2_AXIS_ARROW " Gizmo")) {
            if (ImGui::Checkbox("Use Culling", &puUseGizmoCulling)) {
                useGizmoCulling = puUseGizmoCulling;
                if (puGizmoCullingPrimitiveEnum == GizmoCullingPimitiveEnum::GIZMO_CULLING_PRIMITIVE_NONE)
                    puGizmoCullingPrimitiveEnum = GizmoCullingPimitiveEnum::GIZMO_CULLING_PRIMITIVE_CUBE;  // default
                change |= true;
            }

            if (puUseGizmoCulling) {
                ImGui::Indent();

                bool b = (puGizmoCullingPrimitiveEnum == GizmoCullingPimitiveEnum::GIZMO_CULLING_PRIMITIVE_CUBE);
                if (ImGui::Checkbox("Cube", &b)) {
                    puGizmoCullingPrimitiveEnum = GizmoCullingPimitiveEnum::GIZMO_CULLING_PRIMITIVE_CUBE;
                    change |= true;
                }
                b = (puGizmoCullingPrimitiveEnum == GizmoCullingPimitiveEnum::GIZMO_CULLING_PRIMITIVE_SPHERE);
                if (ImGui::Checkbox("Sphere", &b)) {
                    puGizmoCullingPrimitiveEnum = GizmoCullingPimitiveEnum::GIZMO_CULLING_PRIMITIVE_SPHERE;
                    change |= true;
                }

                ImGui::Separator();

                if (ImGui::ContrastedButton("Select Gizmo")) {
                    CodeTree::puCurrentGizmo = puGizmoCullingUniform;
                }

                ImGui::Unindent();
            }

            ImGui::EndMenu();
        }
    }

    return change;
}

bool GizmoSystem::DrawTooltips(RenderPackWeak vRenderPack, float /*vDisplayQuality*/, CameraSystem* vCamera, ct::ivec2 vScreenSize) {
    bool change = false;

    if ((puActivated || puNeedOneUniformUpdate)) {
        auto rpPtr = vRenderPack.lock();
        if (rpPtr && rpPtr->GetShaderKey())  // archivage de tout les gizmo
        {
            auto lst = rpPtr->GetShaderKey()->GetUniformsByWidget("gizmo");
            if (lst) {
                for (auto it = lst->begin(); it != lst->end(); ++it) {
                    change |= UpdateGizmos(vRenderPack, *it, vCamera, vScreenSize);
                }
            }
        }

        // affichage du gizmo culling
        if (puUseGizmoCulling && puGizmoCulling_Key) {
            glm::mat4 m = vCamera->uView * vCamera->uModel;

            if (puGizmoCullingUniform) {
                if (ImGuizmo::DrawPoint(glm::value_ptr(m),
                                        glm::value_ptr(vCamera->uProj),
                                        glm::value_ptr(puGizmoCullingUniform->mat4),
                                        puGizmoCullingUniform->name.c_str(),
                                        10.0f,
                                        puGizmoCullingUniform->bx,
                                        puGizmoCullingUniform->by,
                                        ImVec4(1.0f, 1.f, 0.2f, 1.0f),
                                        ImVec4(0.2f, 1.0f, 1.0f, 1.0f),
                                        ImVec4(0.2f, 0.2f, 1.0f, 1.0f))) {
                    CodeTree::puCurrentGizmo = puGizmoCullingUniform;
                }

                if (puGizmoCullingUniform == CodeTree::puCurrentGizmo) {
                    change |= DrawGizmoTransformDialog(puGizmoCullingUniform, vCamera);

                    // glm::mat4 m = vCamera->uView * vCamera->uModel;
                    ImGuizmo::Manipulate(
                        glm::value_ptr(m), glm::value_ptr(vCamera->uProj), puCurrentGizmoOperation, puCurrentGizmoMode, glm::value_ptr(puGizmoCullingUniform->mat4));

                    change |= ImGuizmo::IsUsing();
                }
            }

            if (useGizmoCulling) {
                if (puGizmoCullingPrimitiveEnum == GizmoCullingPimitiveEnum::GIZMO_CULLING_PRIMITIVE_CUBE) {
                    /*ImGuizmo::DrawCube(
                        glm::value_ptr(m),
                        glm::value_ptr(vCamera->uProj),
                        glm::value_ptr(v->mat4), 5)*/
                } else if (puGizmoCullingPrimitiveEnum == GizmoCullingPimitiveEnum::GIZMO_CULLING_PRIMITIVE_SPHERE) {
                }
            }
        }

        puNeedOneUniformUpdate = false;
    }

    return change;
}

bool GizmoSystem::DrawPane(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack, float vDisplayQuality, CameraSystem* vCamera) {
    UNUSED(vDisplayQuality);

    bool change = false;

    if (!vRenderPack.expired()) {
        if (Use()) {
            if (ImGui::CollapsingHeader("Gizmo System")) {
                ImGui::Indent();

                if (puUseGizmoCulling) {
                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Gizmo Culling");

                    if (vCodeTree) {
                        DrawWidget(vCodeTree, puGizmoCullingUniform, 150.0f, 0.0, vRenderPack, true, true, false, &change);
                    }
                }

                ImGui::Separator();

                change |= DrawGizmoTransformDialog(CodeTree::puCurrentGizmo, vCamera);

                ImGui::Unindent();
            }
        }
    }

    return change;
}

void GizmoSystem::Capture(RenderPackWeak /*vRenderPack*/, float /*vDisplayQuality*/, CameraSystem* /*vCamera*/) {
    /*auto rpPtr = vRenderPack.lock();
    if (rpPtr && rpPtr->GetPipe())
    {

    }*/
}

void GizmoSystem::Resize(RenderPackWeak /*vRenderPack*/, float /*vDisplayQuality*/, CameraSystem* /*vCamera*/) {
    /*auto rpPtr = vRenderPack.lock();
    if (rpPtr && rpPtr->GetPipe())
    {

    }*/
}

bool GizmoSystem::NeedRefresh() {
    return false;
}

void GizmoSystem::ResetToDefault(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack) {
}

std::string GizmoSystem::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    std::string str;

    str += vOffset + "<gizmosystem>\n";

    str += vOffset + "\t<active>" + ct::toStr(puActivated ? "true" : "false") + "</active>\n";
    str += vOffset + "\t<culling>" + ct::toStr(puUseGizmoCulling ? "true" : "false") + "</culling>\n";
    str += vOffset + "\t<cullingprimitive>" + ct::toStr(puGizmoCullingPrimitiveEnum) + "</cullingprimitive>\n";

    if (puGizmoCullingUniform) {
        str += vOffset + "\t<cullingmatrix>";
        for (int i = 0; i < 16; i++) {
            if (i > 0)
                str += ',';
            str += ct::toStr(glm::value_ptr(puGizmoCullingUniform->mat4)[i]);
        }
        str += ":" + ct::toStr(puGizmoCullingUniform->bx ? "true" : "false");
        str += ":" + ct::toStr(puGizmoCullingUniform->by ? "true" : "false");
        str += "\t</cullingmatrix>\n";
    }

    str += vOffset + "</gizmosystem>\n";

    return str;
}

bool GizmoSystem::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "gizmosystem") {
        if (strName == "active")
            puActivated = ct::ivariant(strValue).GetB();
        if (strName == "culling") {
            puUseGizmoCulling = ct::ivariant(strValue).GetB();
            useGizmoCulling = puUseGizmoCulling;
        }
        if (strName == "cullingprimitive")
            puGizmoCullingPrimitiveEnum = (GizmoCullingPimitiveEnum)ct::ivariant(strValue).GetI();
        if (strName == "cullingmatrix") {
            auto vec = ct::splitStringToVector(strValue, ":", true);
            if (vec.size() == 3) {
                if (puGizmoCullingUniform) {
                    auto vecMat = ct::fvariant(vec[0]).GetVectorFloat(',');
                    if (vecMat.size() == 16) {
                        puGizmoCullingUniform->mat4 = glm::mat4(vecMat[0],
                                                                vecMat[1],
                                                                vecMat[2],
                                                                vecMat[3],
                                                                vecMat[4],
                                                                vecMat[5],
                                                                vecMat[6],
                                                                vecMat[7],
                                                                vecMat[8],
                                                                vecMat[9],
                                                                vecMat[10],
                                                                vecMat[11],
                                                                vecMat[12],
                                                                vecMat[13],
                                                                vecMat[14],
                                                                vecMat[15]);
                    }
                    puGizmoCullingUniform->bx = ct::ivariant(vec[1]).GetB();
                    puGizmoCullingUniform->by = ct::ivariant(vec[2]).GetB();
                }
            }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////

bool GizmoSystem::UpdateGizmos(RenderPackWeak vRenderPack, UniformVariantPtr vUniPtr, CameraSystem* vCamera, ct::ivec2 vScreenSize) {
    bool change = false;

    if (vCamera) {
        if (vUniPtr) {
            if (!vRenderPack.expired()) {
                if (vCamera->UpdateIfNeeded(vScreenSize)) {
                    glm::mat4 m = vCamera->uView * vCamera->uModel;

                    ImGuizmo::InitMVP(glm::value_ptr(m), glm::value_ptr(vCamera->uProj), glm::value_ptr(vUniPtr->mat4), puCurrentGizmoMode);
                }
            }

            if (vUniPtr->widget == "gizmo") {
                glm::mat4 m = vCamera->uView * vCamera->uModel;

                if (ImGuizmo::DrawPoint(glm::value_ptr(m),
                                        glm::value_ptr(vCamera->uProj),
                                        glm::value_ptr(vUniPtr->mat4),
                                        vUniPtr->name.c_str(),
                                        5.0f,
                                        vUniPtr->bx,
                                        vUniPtr->by,
                                        ImVec4(1.0f, 1.f, 0.2f, 1.0f),
                                        ImVec4(0.2f, 1.0f, 1.0f, 1.0f),
                                        ImVec4(0.2f, 0.2f, 1.0f, 1.0f))) {
                    CodeTree::puCurrentGizmo = vUniPtr;
                }

                if (vUniPtr == CodeTree::puCurrentGizmo) {
                    // glm::mat4 m = vCamera->uView * vCamera->uModel;
                    ImGuizmo::Manipulate(glm::value_ptr(m), glm::value_ptr(vCamera->uProj), puCurrentGizmoOperation, puCurrentGizmoMode, glm::value_ptr(vUniPtr->mat4));

                    change |= ImGuizmo::IsUsing();
                }
            }
        }
    }

    return change;
}

bool GizmoSystem::UpdateUniforms(UniformVariantPtr vUniPtr) {
    const bool change = false;

    if (vUniPtr) {
        if (vUniPtr->widget == "gizmo") {
            vUniPtr->uFloatArr = glm::value_ptr(vUniPtr->mat4);
            vUniPtr->ownFloatArr = false;
        }
        if (vUniPtr->widget == "cullingtype") {
            vUniPtr->ix = (int)puGizmoCullingPrimitiveEnum;
        } else if (vUniPtr->widget == "useculling") {
            vUniPtr->bx = useGizmoCulling && puActivated;
            vUniPtr->x = (vUniPtr->bx ? 1.0f : 0.0f);
        } else if (vUniPtr->widget == "culling" && puGizmoCullingUniform) {
            vUniPtr->uFloatArr = glm::value_ptr(puGizmoCullingUniform->mat4[0]);
            vUniPtr->ownFloatArr = false;
        }
    }

    return change;
}

bool GizmoSystem::DrawGizmoTransformDialog(UniformVariantPtr vUniPtr, CameraSystem* vCamera) {
    bool change = false;

    if (vUniPtr) {
        float matrixTranslation[3], matrixRotation[3], matrixScale[3], matrixRotationEmpty[3] = {0.0f, 0.0f, 0.0f};
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(vUniPtr->mat4), matrixTranslation, matrixRotation, matrixScale);

        ImGui::BeginGroup();

        // bool useRot = (useGizmoCulling && puGizmoCullingPrimitiveEnum != GizmoCullingPimitiveEnum::GIZMO_CULLING_PRIMITIVE_SPHERE) || !useGizmoCulling;

        if (ImGui::ContrastedButton("UnSelect")) {
            CodeTree::puCurrentGizmo = nullptr;
        }

        ImGui::Text("Reset :");

        ////////////////////////////////

        if (ImGui::ContrastedButton("Matrix")) {
            matrixTranslation[0] = 0.0f;
            matrixTranslation[1] = 0.0f;
            matrixTranslation[2] = 0.0f;
            matrixRotation[0] = 0.0f;
            matrixRotation[1] = 0.0f;
            matrixRotation[2] = 0.0f;
            matrixScale[0] = 1.0f;
            matrixScale[1] = 1.0f;
            matrixScale[2] = 1.0f;
            change = true;
        }

        ImGui::SameLine();

        if (ImGui::ContrastedButton("Pos")) {
            matrixTranslation[0] = 0.0f;
            matrixTranslation[1] = 0.0f;
            matrixTranslation[2] = 0.0f;
            change = true;
        }

        ImGui::SameLine();

        // if (useRot)
        {
            if (ImGui::ContrastedButton("Rot")) {
                matrixRotation[0] = 0.0f;
                matrixRotation[1] = 0.0f;
                matrixRotation[2] = 0.0f;
                change = true;
            }

            ImGui::SameLine();
        }

        if (ImGui::ContrastedButton("Scale")) {
            matrixScale[0] = 1.0f;
            matrixScale[1] = 1.0f;
            matrixScale[2] = 1.0f;
            change = true;
        }

        if (vCamera) {
            ImGui::SameLine();

            if (ImGui::ContrastedButton("Center CameraSystem")) {
                vCamera->SetTargetXYZ(-ct::fvec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2])),
                    // vCamera->SetTranslateXY(-ct::fvec2(matrixTranslation[0], matrixTranslation[1]));
                    // vCamera->SetZoom(-matrixTranslation[2]);
                    change = true;
            }
        }

        ////////////////////////////////

        if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Local", "", (puCurrentGizmoMode == ImGuizmo::MODE::LOCAL))) {
            puCurrentGizmoMode = ImGuizmo::MODE::LOCAL;
            change = true;
        }

        ImGui::SameLine();

        if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "World", "", (puCurrentGizmoMode == ImGuizmo::MODE::WORLD))) {
            puCurrentGizmoMode = ImGuizmo::MODE::WORLD;
            change = true;
        }

        ////////////////////////////////

        if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Trans", "", (puCurrentGizmoOperation == ImGuizmo::OPERATION::TRANSLATE))) {
            puCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
            change = true;
        }

        ImGui::SameLine();

        // if (useRot)
        {
            if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Rot", "", (puCurrentGizmoOperation == ImGuizmo::OPERATION::ROTATE))) {
                puCurrentGizmoOperation = ImGuizmo::OPERATION::ROTATE;
                change = true;
            }

            ImGui::SameLine();
        }

        if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Scale", "", (puCurrentGizmoOperation == ImGuizmo::OPERATION::SCALE))) {
            puCurrentGizmoOperation = ImGuizmo::OPERATION::SCALE;
            change = true;
        }

        ////////////////////////////////

        if (puCurrentGizmoOperation == ImGuizmo::OPERATION::TRANSLATE) {
            change |= ImGui::InputFloatDefault(150.0f, "tx", &matrixTranslation[0], 0.0f);
            change |= ImGui::InputFloatDefault(150.0f, "ty", &matrixTranslation[1], 0.0f);
            change |= ImGui::InputFloatDefault(150.0f, "tz", &matrixTranslation[2], 0.0f);
        }

        // if (useRot)
        {
            if (puCurrentGizmoOperation == ImGuizmo::OPERATION::ROTATE) {
                change |= ImGui::InputFloatDefault(150.0f, "rx", &matrixRotation[0], 0.0f);
                change |= ImGui::InputFloatDefault(150.0f, "ry", &matrixRotation[1], 0.0f);
                change |= ImGui::InputFloatDefault(150.0f, "rz", &matrixRotation[2], 0.0f);
            }
        }

        if (puCurrentGizmoOperation == ImGuizmo::OPERATION::SCALE) {
            change |= ImGui::InputFloatDefault(150.0f, "sx", &matrixScale[0], 1.0f);
            change |= ImGui::InputFloatDefault(150.0f, "sy", &matrixScale[1], 1.0f);
            change |= ImGui::InputFloatDefault(150.0f, "sz", &matrixScale[2], 1.0f);
        }

        ////////////////////////////////

        /*float *m = glm::value_ptr(vUniPtr->mat4);
        ImGui::Text("Matrix 4x4\n 0 : %.2f %.2f %.2f %.2f\n1 : %.2f %.2f %.2f %.2f\n2 : %.2f %.2f %.2f %.2f\n3 : %.2f %.2f %.2f %.2f",
            m[0], m[1], m[2], m[3],
            m[4], m[5], m[6], m[7],
            m[8], m[9], m[10], m[11],
            m[12], m[13], m[14], m[15]);*/

        ImGui::EndGroup();

        if (change) {
            // if (useRot)
            { ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, glm::value_ptr(vUniPtr->mat4)); }
            /*else
            {
                // comme cela on ne nique pas la rotation pour le cube on n'en tient jute aps ;compte pour la spahere le temps de trouve run focntion glsl
                // qui pemette de faire troune l'ellipsoide
                ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotationEmpty, matrixScale, glm::value_ptr(vUniPtr->mat4));
            }*/
        }
    }

    return change;
}

bool GizmoSystem::DrawWidget(CodeTreePtr vCodeTree,
                             UniformVariantPtr vUniPtr,
                             const float& vMaxWidth,
                             const float& vFirstColumnWidth,
                             RenderPackWeak vRenderPack,
                             const bool& /*vShowUnUsed*/,
                             const bool& /*vShowCustom*/,
                             const bool& vForNodes,
                             bool* vChange) {
    bool catched = false;

    if (vUniPtr && vCodeTree && vChange) {
        UniformVariantPtr v = vUniPtr;

        ShaderKeyPtr key = nullptr;
        auto rpPtr = vRenderPack.lock();
        if (rpPtr)
            key = rpPtr->GetShaderKey();

        if (vUniPtr->widget == "cullingtype") {
            UniformWidgets::drawUniformName(key, v);
            ImGui::SameLine(vFirstColumnWidth);
            ImGui::Text("%i", vUniPtr->ix);

            catched = true;
        } else if (vUniPtr->widget == "useculling") {
            UniformWidgets::drawUniformName(key, v);
            ImGui::SameLine(vFirstColumnWidth);
            ImGui::Text("%1.f", vUniPtr->x);

            catched = true;
        } else if (vUniPtr->widget == "culling") {
            UniformWidgets::drawUniformName(key, v);
            ImGui::SameLine(vFirstColumnWidth);

            if (v->loc > -1)
                ImGui::Text("Matrix 4x4");
            else
                ImGui::Text("Not Used");

            if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
                ImGui::Indent();

                if (v->uFloatArr) {
                    ImGui::Text("0 : %.2f %.2f %.2f %.2f\n1 : %.2f %.2f %.2f %.2f\n2 : %.2f %.2f %.2f %.2f\n3 : %.2f %.2f %.2f %.2f",
                                v->uFloatArr[0],
                                v->uFloatArr[1],
                                v->uFloatArr[2],
                                v->uFloatArr[3],
                                v->uFloatArr[4],
                                v->uFloatArr[5],
                                v->uFloatArr[6],
                                v->uFloatArr[7],
                                v->uFloatArr[8],
                                v->uFloatArr[9],
                                v->uFloatArr[10],
                                v->uFloatArr[11],
                                v->uFloatArr[12],
                                v->uFloatArr[13],
                                v->uFloatArr[14],
                                v->uFloatArr[15]);
                } else {
                    ImGui::Text("0 : %.2f %.2f %.2f %.2f\n1 : %.2f %.2f %.2f %.2f\n2 : %.2f %.2f %.2f %.2f\n3 : %.2f %.2f %.2f %.2f",
                                v->mat4[0][0],
                                v->mat4[1][0],
                                v->mat4[2][0],
                                v->mat4[3][0],
                                v->mat4[0][1],
                                v->mat4[1][1],
                                v->mat4[2][1],
                                v->mat4[3][1],
                                v->mat4[0][2],
                                v->mat4[1][2],
                                v->mat4[2][2],
                                v->mat4[3][2],
                                v->mat4[0][3],
                                v->mat4[1][3],
                                v->mat4[2][3],
                                v->mat4[3][3]);
                }

                ImGui::Unindent();
            }

            catched = true;
        } else if (vUniPtr->widget == "gizmo") {
            if (vForNodes)  // pour les nodes
            {
                ImGui::PushItemWidth(vMaxWidth);
                ImGui::BeginGroup();
                if (ImGui::ContrastedButton("Reset", "Reset Matrix to Default")) {
                    v->mat4 = v->mat4Def;
                }
                ImGui::SameLine();
                if (ImGui::ContrastedButton("Select", "Select Gizmo")) {
                    if (!vRenderPack.expired()) {
                        CodeTree::puCurrentGizmo = v;
                    }
                }
                ImGui::EndGroup();
                ImGui::PopItemWidth();
            } else {
                ImGui::Separator();

                UniformWidgets::drawUniformName(key, v);
                ImGui::SameLine(vFirstColumnWidth);
                ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                ImGui::BeginGroup();
                if (ImGui::ContrastedButton("R", "Reset Matrix to Default")) {
                    *vChange |= true;
                    v->mat4 = v->mat4Def;
                }
                ImGui::SameLine();
                if (ImGui::ContrastedButton("Sel", "Select Gizmo")) {
                    if (!vRenderPack.expired()) {
                        *vChange |= true;
                        CodeTree::puCurrentGizmo = v;
                    }
                }
                ImGui::SameLine();
                ImGui::PushID(ImGui::IncPUSHID());
                *vChange |= ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Icon", "Show/Hide Gizmo Icon", &v->bx);
                ImGui::PopID();
                ImGui::SameLine();
                ImGui::PushID(ImGui::IncPUSHID());
                *vChange |= ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Txt", "Show/Hide Gizmo Text", &v->by);
                ImGui::PopID();
                ImGui::EndGroup();
                ImGui::PopItemWidth();
            }

            catched = true;
        }
    }

    return catched;
}

bool GizmoSystem::SerializeUniform(UniformVariantPtr vUniform, std::string* vStr) {
    bool catched = false;

    if (vStr) {
        if (vUniform->widget == "gizmo") {
            *vStr = vUniform->name + ":";

            for (int i = 0; i < 16; i++) {
                if (i > 0)
                    *vStr += ',';
                *vStr += ct::toStr(glm::value_ptr(vUniform->mat4)[i]);
            }

            *vStr += ":" + ct::toStr(vUniform->bx ? "true" : "false");
            *vStr += ":" + ct::toStr(vUniform->by ? "true" : "false");
            *vStr += '\n';

            catched = true;
        } else if (vUniform->widget == "cullingtype") {
            *vStr = vUniform->name + ":" + ct::toStr(vUniform->ix) + "\n";

            catched = true;
        } else if (vUniform->widget == "useculling") {
            *vStr = vUniform->name + ":" + ct::toStr(vUniform->x) + "\n";

            catched = true;
        }
    }

    return catched;
}

bool GizmoSystem::DeSerializeUniform(ShaderKeyPtr /*vShaderKey*/, UniformVariantPtr vUniform, const std::vector<std::string>& vParams) {
    bool catched = false;

    if (vUniform->widget == "gizmo") {
        if (vParams.size() == 4) {
            auto vec = ct::fvariant(vParams[1]).GetVectorFloat(',');
            if (vec.size() == 16) {
                vUniform->mat4 =
                    glm::mat4(vec[0], vec[1], vec[2], vec[3], vec[4], vec[5], vec[6], vec[7], vec[8], vec[9], vec[10], vec[11], vec[12], vec[13], vec[14], vec[15]);
            }
            vUniform->bx = ct::ivariant(vParams[2]).GetB();
            vUniform->by = ct::ivariant(vParams[3]).GetB();
        }

        catched = true;
    } else if (vUniform->widget == "cullingtype") {
        const int x = ct::ivariant(vParams[1]).GetI();
        vUniform->ix = x;

        catched = true;
    } else if (vUniform->widget == "useculling") {
        const float x = ct::fvariant(vParams[1]).GetF();
        vUniform->x = x;

        catched = true;
    }

    return catched;
}
void GizmoSystem::Complete_Uniform(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->widget = vUniform->widgetType;

    if (vUniform->widgetType == "useculling" || vUniform->widgetType == "culling" || vUniform->widgetType == "cullingtype") {
        Complete_Uniform_Culling(vParentKey, vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "gizmo") {
        Complete_Uniform_Gizmo(vParentKey, vRenderPack, vUniformParsed, vUniform);
    }
}

void GizmoSystem::Complete_Uniform_Gizmo(ShaderKeyPtr /*vParentKey*/, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vRenderPack.expired())
        return;

    vUniform->widget = "gizmo";
    vUniform->timeLineSupported = true;

    if (vUniform->glslType == uType::uTypeEnum::U_MAT4) {
        if (!vUniformParsed.paramsDico.empty()) {
            float matrixTranslation[3], matrixRotation[3], matrixScale[3];
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(vUniform->mat4), matrixTranslation, matrixRotation, matrixScale);

            for (auto it = vUniformParsed.paramsDico.begin(); it != vUniformParsed.paramsDico.end(); ++it) {
                std::string key = it->first;

                if (key == "trans") {
                    if (it->second.size() == 3) {
                        auto itv = it->second.begin();
                        matrixTranslation[0] = ct::fvariant(*itv++).GetF();
                        matrixTranslation[1] = ct::fvariant(*itv++).GetF();
                        matrixTranslation[2] = ct::fvariant(*itv++).GetF();
                    }
                }
                if (key == "rot") {
                    if (it->second.size() == 3) {
                        auto itv = it->second.begin();
                        matrixRotation[0] = ct::fvariant(*itv++).GetF();
                        matrixRotation[1] = ct::fvariant(*itv++).GetF();
                        matrixRotation[2] = ct::fvariant(*itv++).GetF();
                    }
                }
                if (key == "scale") {
                    if (it->second.size() == 3) {
                        auto itv = it->second.begin();
                        matrixScale[0] = ct::fvariant(*itv++).GetF();
                        matrixScale[1] = ct::fvariant(*itv++).GetF();
                        matrixScale[2] = ct::fvariant(*itv++).GetF();
                    }
                }
            }

            vUniform->bx = true;  // show icon
            vUniform->by = true;  // show text

            ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, glm::value_ptr(vUniform->mat4));
        } else {
            vUniform->mat4 = glm::mat4(1.0f);
            vUniform->mat4Def = vUniform->mat4;
        }
    }
}

void GizmoSystem::Complete_Uniform_Culling(ShaderKeyPtr /*vParentKey*/,
                                           RenderPackWeak vRenderPack,
                                           const UniformParsedStruct&
                                           /*vUniformParsed*/,
                                           UniformVariantPtr vUniform) {
    if (!vRenderPack.expired())
        return;

    if (vUniform->widget == "useculling")  // float, true, false
    {
    } else if (vUniform->widget == "cullingtype")  // int, cube=0, sphere=1
    {
    } else if (vUniform->widget == "culling")  // mat4
    {
        vUniform->mat4 = glm::mat4(1.0f);
        vUniform->mat4Def = vUniform->mat4;
    }
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

std::string GizmoSystem::InitRenderPack(const GuiBackend_Window& vWin, CodeTreePtr vCodeTree) {
    if (vCodeTree) {
        puGizmoCulling_RenderPack = nullptr;

        const std::string path = FileHelper::Instance()->GetExistingFilePathForFile("culling.glsl", true);
        if (path.empty())
            CreateFilePathName("predefined/helpers/culling.glsl", "culling", false);

        const std::string scriptName = "GizmoCulling.glsl";
        std::string scriptPath = FileHelper::Instance()->GetExistingFilePathForFile(scriptName, true);
        if (scriptPath.empty()) {
            CreateFilePathName("predefined/shaders/GizmoCulling.glsl", "GizmoCulling", false);
            // scriptPath = "predefined\\shaders\\3dGrid.glsl";
        }
        puGizmoCulling_Key = vCodeTree->LoadFromFile(scriptPath, KEY_TYPE_Enum::KEY_TYPE_SHADER);
        if (puGizmoCulling_Key) {
            puGizmoCullingUniform = UniformVariant::create(puGizmoCulling_Key, "GizmoCulling");
            puGizmoCullingUniform->widget = "gizmo";
            puGizmoCullingUniform->glslType = uType::uTypeEnum::U_MAT4;
            puGizmoCullingUniform->mat4 = glm::mat4(1.0f);
            puGizmoCullingUniform->mat4Def = glm::mat4(1.0f);
            puGizmoCullingUniform->bx = true;
            puGizmoCullingUniform->by = true;
        }
        puGizmoCulling_RenderPack = RenderPack::createBufferWithFileWithoutLoading(vWin, "GizmoCulling", ct::ivec3(0), puGizmoCulling_Key, true, false);
        if (puGizmoCulling_RenderPack) {
            puGizmoCulling_RenderPack->puShowZBufferButton = true;
            // puGizmoCulling_RenderPack->puShowBlendingButton = true;
        }

        return scriptPath;
    }

    return "";
}

bool GizmoSystem::LoadRenderPack() {
    if (puGizmoCulling_RenderPack) {
        return puGizmoCulling_RenderPack->Load();
    }
    return false;
}

void GizmoSystem::SetRenderPackFXAA(const bool& vUseFXAA, const int32_t& vCountSamples) {
    if (puGizmoCulling_RenderPack) {
        puGizmoCulling_RenderPack->puUseFXAA = vUseFXAA;
        puGizmoCulling_RenderPack->puCountFXAASamples = vCountSamples;
    }
}

void GizmoSystem::SaveRenderPack() {
    if (puGizmoCulling_RenderPack->GetShaderKey())
        puGizmoCulling_RenderPack->GetShaderKey()->SaveRenderPackConfig(CONFIG_TYPE_Enum::CONFIG_TYPE_ALL);
}

void GizmoSystem::FinishRenderPack() {
    puGizmoCulling_RenderPack->Finish(false);
}

void GizmoSystem::DestroyRenderPack() {
    UniformVariant::destroy(puGizmoCullingUniform, puGizmoCulling_Key);
    puGizmoCulling_RenderPack.reset();
}

void GizmoSystem::CreateGizmoCullingScript() {
}

bool GizmoSystem::CreateFilePathName(const std::string& vFilePathName, std::string vType, bool useStandardPaths) {
    ShaderInfos infos;

    if (vType == "culling") {
        infos.framebuffer +=
            u8R"(
@UNIFORMS

uniform float(useculling) _useGizmoCull; // hidden
uniform(_useGizmoCull==true) int(cullingtype) _cullingType; // hidden // 0=>cube, 1=>sphere
uniform(_useGizmoCull==true) mat4(culling) _GizmoCull; // _GizmoCullingVolume

@FRAGMENT

// Volume Culling

// based on http://iquilezles.org/www/articles/intersectors/intersectors.htm
void getSpherePoint( in vec3 ro, in vec3 rd, in mat4 txx, out vec3 facePoint, out vec2 d, out vec3 nor)
{
	mat4 txi = inverse(txx);
	
	float ra = 0.57;
	
	vec3 ocn = (txi*vec4(ro,1.0)).xyz/ra;
    vec3 rdn = (txi*vec4(rd,0.0)).xyz/ra;
    
    float a = dot( rdn, rdn );
    float b = dot( ocn, rdn );
    float c = dot( ocn, ocn );
    float h = b*b - a*(c-1.0);
	
    if( h < 0.0 )
	{
		facePoint = ro; 
		d = vec2(1.0,-1.0);
	}
	
    h = sqrt(h);
	
    float d1 = (-b-h)/a;
    float d2 = (-b+h)/a;
	
	facePoint = ro + rd * d1;
	d = vec2(d1, d2);
	nor = normalize(facePoint -  txx[3].xyz);
}

void getBoxPoint(in vec3 ro, in vec3 rd, in mat4 txx, out vec3 facePoint, out vec2 d, out vec3 nor) 
{
	mat4 txi = inverse(txx);
	
	// convert from ray to box space
	vec3 rdd = (txi*vec4(rd,0.0)).xyz;
	vec3 roo = (txi*vec4(ro,1.0)).xyz;

	// ray-box intersection in box space
    vec3 m = 1.0/rdd;
    vec3 n = m*roo;
    vec3 k = abs(m) * 0.5;
	
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;

	float d1 = max( max( t1.x, t1.y ), t1.z ); // near
	float d2 = min( min( t2.x, t2.y ), t2.z ); // far
	
	if( d1 >= d2 || d2 <= 0.0)
	{
		facePoint = ro;
		d = vec2(1.0,-1.0);
	}

	facePoint = (txx * vec4(roo + rdd * d1, 1.0)).xyz;
	d = vec2(d1, d2);
	nor = -sign(rdd)*step(t1.yzx,t1.xyz)*step(t1.zxy,t1.xyz);
}

bool getCullingPoint(in vec3 ro, in vec3 rd, out vec3 facePoint, out vec2 d, inout float maxDistance, out vec3 nor)
{
	facePoint = ro;
	d = vec2(1.0,-1.0);
	nor = vec3(0);
	
	bool res = false;
	
	if (_useGizmoCull > 0.5)
	{
		if (_cullingType == 0) // box
		{
			getBoxPoint(ro, rd, _GizmoCull, facePoint, d, nor);
		}
		else if (_cullingType == 1) // sphere
		{
			getSpherePoint(ro, rd, _GizmoCull, facePoint, d, nor);
		}
		
		if (d.y > d.x)
		{
			maxDistance = min(maxDistance, d.y - d.x);
			res = true;
		}
	}
	else
	{
		res = true;
	}
	
	return res;
}
)";
    } else if (vType == "GizmoCulling") {
        infos.framebuffer +=
            u8R"(
@UNIFORMS

//uniform float(maxpoints) uCountVertex; // Count vertex for range operations on vertexId
uniform mat4(camera:mvp) _cam;

uniform int(cullingtype) cullingType; // hidden // 0=>cube, 1=>sphere
uniform float(0:20:2) cullingSize;
uniform float(0:20:5:1) subdivisions;
uniform float(0:100:50:1) circleSegments;
uniform mat4(culling) _CullingMatrix;

@VERTEX POINTS(15000) DISPLAY(LINES)

layout(location = 0) in float vertexId;
out vec4 v_color;

vec3 getRect(float id)
{
	float index = mod(id, 8.);
	float idx = floor(id/8.);
	
	if (index < 0.5) return vec3(0,0,idx);
	if (index < 1.5) return vec3(1,0,idx);
	if (index < 2.5) return vec3(1,0,idx);
	if (index < 3.5) return vec3(1,1,idx);
	if (index < 4.5) return vec3(1,1,idx);
	if (index < 5.5) return vec3(0,1,idx);
	if (index < 6.5) return vec3(0,1,idx);
	if (index < 7.5) return vec3(0,0,idx);
	
	return vec3(0);
}

bool getWireCube(float id, out vec3 p)
{
	vec3 pid = getRect(id);
	float idx = floor(pid.z/3.);
	
	if (idx > subdivisions + 1.)
	{
		gl_Position = vec4(0.0);
		v_color = vec4(0.0);
		return false;
	}
	
	idx /= subdivisions + 1.;
	
	float index = mod(pid.z, 3.);
	if (index < 0.5) p = vec3(idx, pid.y, pid.x);
	else if (index < 1.5) p = vec3(pid.x, idx, pid.y);
	else if (index < 2.5) p = vec3(pid.x, pid.y, idx);
	
	p -= 0.5;
	
	return true;
}


vec3 getCircle(float id)
{
	float pi2 = radians(360.);
	float n = circleSegments;
	
	float index = mod(id, 2.);
	float idxSegment = floor(id / 2.);
	
	float asp = pi2 / n; // angle step line
	float ap0 = asp * idxSegment; // angle line 0
	float ap1 = asp * (idxSegment + 1.); // angle line 0
	
  	float a = 0.0;
	
	if (index == 0.) a = ap0;
	if (index == 1.) a = ap1;
	
	float idxCircle = floor(idxSegment / n);
	
	return vec3(cos(a), sin(a), idxCircle);
}

bool getWireSphere(float id, out vec3 p)
{
	float coundSubdivisions = subdivisions + 1.;
	
	vec3 pid = getCircle(id);
	pid.z = pid.z;
	
	float idx = floor(pid.z/3.);
	
	if (idx > coundSubdivisions)
	{
		gl_Position = vec4(0.0);
		v_color = vec4(0.0);
		return false;
	}
	
	idx /= coundSubdivisions;
	idx -= 0.5;
	
	float r = 1.0;
	float a = cos(idx*1.5/r);
	float h = sin(a) * r;
	pid.xy *= h;
	
	float index = mod(pid.z, 3.);
	if (index < 0.5) p = vec3(idx, pid.y, pid.x);
	else if (index < 1.5) p = vec3(pid.x, idx, pid.y);
	else if (index < 2.5) p = vec3(pid.x, pid.y, idx);
	
	p *= 0.68;
	return true;
}

void main()
{
	vec3 p = vec3(0);
	bool res  = false;
	
	if (cullingType == 0) res = getWireCube(vertexId, p);
	else if (cullingType == 1) res = getWireSphere(vertexId, p);
	
	if (res == false) return;
	
	gl_Position = _cam * _CullingMatrix * vec4(p.x, p.y, p.z, 1);
	v_color = vec4(1.0);
}

@FRAGMENT

layout(location = 0) out vec4 fragColor;
in vec4 v_color;

void main(void)
{
	fragColor = v_color;
}

)";
    }

    std::string file;

    file += infos.framebuffer;
    file += infos.common_uniforms;
    file += infos.specific_uniforms;
    file += infos.common;
    file += infos.note;
    file += infos.vertex;
    file += infos.fragment;

    if (!file.empty()) {
        FileHelper::Instance()->SaveToFile(
            file, vFilePathName, useStandardPaths ? (int)FILE_LOCATION_Enum::FILE_LOCATION_SCRIPT : (int)FILE_LOCATION_Enum::FILE_LOCATION_NONE);

        return true;
    }

    return false;
}
