#include "UniformWidgets.h"

#include <Res/CustomFont.h>
#include <Res/CustomFont2.h>
#include <Texture/Texture2D.h>
#include <Systems/MidiSystem.h>
#include <CodeTree/ShaderKey.h>
#include <Systems/GizmoSystem.h>
#include <ImGuiPack/ImGuiPack.h>
#include <Systems/SoundSystem.h>
#include <Renderer/RenderPack.h>
#include <Gui/CustomGuiWidgets.h>
#include <Systems/GamePadSystem.h>
#include <Uniforms/UniformVariant.h>
#include <CodeTree/ShaderKeyConfigSwitcherUnified.h>

using namespace std::placeholders;

bool UniformWidgets::drawImGuiUniformWidgetForPanes(  //
    CodeTreePtr vCodeTreePtr,
    UniformVariantPtr vUniPtr,
    float vMaxWidth,
    float vFirstColumnWidth,
    RenderPackWeak vRenderPack,
    bool vShowUnUsed,
    bool vShowCustom) {
    bool change = false;

    if (vCodeTreePtr && vUniPtr) {
        UniformVariantPtr v = vUniPtr;

        const bool visible = checkUniformVisiblity(vUniPtr, vShowUnUsed);
        if (visible) {
            ShaderKeyPtr key = nullptr;
            auto rpPtr = vRenderPack.lock();
            if (rpPtr)
                key = rpPtr->GetShaderKey();
            if (GizmoSystem::Instance()->DrawWidget(vCodeTreePtr, v, vMaxWidth, vFirstColumnWidth, vRenderPack, vShowUnUsed, vShowCustom, false, &change)) {
                return change;
            } else if (GamePadSystem::Instance()->DrawWidget(vCodeTreePtr, v, vMaxWidth, vFirstColumnWidth, vRenderPack, vShowUnUsed, vShowCustom, false, &change)) {
                return change;
            } else if (MidiSystem::Instance()->DrawWidget(vCodeTreePtr, v, vMaxWidth, vFirstColumnWidth, vRenderPack, vShowUnUsed, vShowCustom, false, &change)) {
                return change;
            } else if (SoundSystem::Instance()->DrawWidget(vCodeTreePtr, v, vMaxWidth, vFirstColumnWidth, vRenderPack, vShowUnUsed, vShowCustom, false, &change)) {
                return change;
            }
#ifdef USE_VR
            else if (VRBackend::Instance()->DrawWidget(vCodeTreePtr, v, vMaxWidth, vFirstColumnWidth, vRenderPack, vShowUnUsed, vShowCustom, false, &change)) {
                return change;
            }
#endif
            else if (v->widgetType == "time") {
                ImGui::Separator();

                drawUniformName(key, v);
                ImGui::SameLine(vFirstColumnWidth);
                if (ImGui::ContrastedButton(ICON_NDP_RESET, "Reset")) {
                    change |= true;
                    v->bx = (v->def.x > 0.5f);
                    v->x = 0.0f;
                }
                ImGui::SameLine();
                if (ImGui::ToggleContrastedButton(ICON_NDP_PAUSE, ICON_NDP_PLAY, &v->bx)) {
                    change |= true;
                }
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                ImGui::PushID(ImGui::IncPUSHID());
                if (ImGui::InputFloat("##Time", &v->x)) {
                    change |= true;
                    vCodeTreePtr->RecordToTimeLine(key, v, 0);
                }
                ImGui::PopID();
                ImGui::PopItemWidth();
                ImGui::PopStyleColor();
            } else if (v->widgetType == "button") {
                ImGui::Separator();

                drawUniformName(key, v);
                ImGui::SameLine(vFirstColumnWidth);
                ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                if (v->count > 0) {
                    v->bx = false;
                    if (ImGui::ContrastedButton(v->buttonName0.c_str())) {
                        v->bx = true;
                        change = true;
                    }
                }
                if (v->count > 1) {
                    ImGui::SameLine();
                    v->by = false;
                    if (ImGui::ContrastedButton(v->buttonName1.c_str())) {
                        v->by = true;
                        change = true;
                    }
                }
                if (v->count > 2) {
                    ImGui::SameLine();
                    v->bz = false;
                    if (ImGui::ContrastedButton(v->buttonName2.c_str())) {
                        v->bz = true;
                        change = true;
                    }
                }
                if (v->count > 3) {
                    ImGui::SameLine();
                    v->bw = false;
                    if (ImGui::ContrastedButton(v->buttonName3.c_str())) {
                        v->bw = true;
                        change = true;
                    }
                }

                ImGui::PopItemWidth();
            } else if (v->widgetType == "checkbox") {
                ImGui::Separator();

                char buffer[256];

                if (TimeLineSystem::Instance()->IsActive()) {
                    if (v->count > 0) {
                        drawUniformName(key, v, 0);
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                        snprintf(buffer, 256, "##checkbox%sx", v->name.c_str());
                        const bool btn = ImGui::ContrastedButton(ICON_NDP_RESET);
                        if (btn) {
                            change = true;
                            v->bx = v->bdef.x;
                        }
                        ImGui::SameLine();
                        if (ImGui::Checkbox(buffer, &v->bx)) {
                            change = true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 0);
                        }
                        ImGui::PopStyleColor();
                        ImGui::PopItemWidth();
                    }
                    if (v->count > 1) {
                        drawUniformName(key, v, 1);
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                        snprintf(buffer, 256, "##checkbox%sy", v->name.c_str());
                        const bool btn = ImGui::ContrastedButton(ICON_NDP_RESET);
                        if (btn) {
                            change = true;
                            v->by = v->bdef.y;
                        }
                        ImGui::SameLine();
                        if (ImGui::Checkbox(buffer, &v->by)) {
                            change = true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 1);
                        }
                        ImGui::PopStyleColor();
                        ImGui::PopItemWidth();
                    }
                    if (v->count > 2) {
                        drawUniformName(key, v, 2);
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                        snprintf(buffer, 256, "##checkbox%sz", v->name.c_str());
                        const bool btn = ImGui::ContrastedButton(ICON_NDP_RESET);
                        if (btn) {
                            change = true;
                            v->bz = v->bdef.z;
                        }
                        ImGui::SameLine();
                        if (ImGui::Checkbox(buffer, &v->bz)) {
                            change = true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 2);
                        }
                        ImGui::PopStyleColor();
                        ImGui::PopItemWidth();
                    }
                    if (v->count > 3) {
                        drawUniformName(key, v, 3);
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                        snprintf(buffer, 256, "##checkbox%sw", v->name.c_str());
                        const bool btn = ImGui::ContrastedButton(ICON_NDP_RESET);
                        if (btn) {
                            change = true;
                            v->bw = v->bdef.w;
                        }
                        ImGui::SameLine();
                        if (ImGui::Checkbox(buffer, &v->bw)) {
                            change = true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 3);
                        }
                        ImGui::PopStyleColor();
                        ImGui::PopItemWidth();
                    }
                } else {
                    drawUniformName(key, v);
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));

                    if (v->count > 0) {
                        snprintf(buffer, 256, "##checkbox%sx", v->name.c_str());
                        const bool btn = ImGui::ContrastedButton(ICON_NDP_RESET);
                        if (btn) {
                            change = true;
                            v->bx = v->bdef.x;
                            v->by = v->bdef.y;
                            v->bz = v->bdef.z;
                            v->bw = v->bdef.w;
                        }
                        ImGui::SameLine();
                        if (ImGui::Checkbox(buffer, &v->bx)) {
                            change = true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 0);
                        }
                    }
                    if (v->count > 1) {
                        ImGui::SameLine();
                        snprintf(buffer, 256, "##checkbox%sy", v->name.c_str());
                        if (ImGui::Checkbox(buffer, &v->by)) {
                            change = true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 1);
                        }
                    }
                    if (v->count > 2) {
                        ImGui::SameLine();
                        snprintf(buffer, 256, "##checkbox%sz", v->name.c_str());
                        if (ImGui::Checkbox(buffer, &v->bz)) {
                            change = true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 2);
                        }
                    }
                    if (v->count > 3) {
                        ImGui::SameLine();
                        snprintf(buffer, 256, "##checkbox%sw", v->name.c_str());
                        if (ImGui::Checkbox(buffer, &v->bw)) {
                            change = true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 3);
                        }
                    }

                    ImGui::PopStyleColor();
                    ImGui::PopItemWidth();
                }
            } else if (v->widgetType == "radio") {
                ImGui::Separator();

                drawUniformName(key, v);
                ImGui::SameLine(vFirstColumnWidth);
                ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));

                char buffer[256];
                if (v->count > 0) {
                    snprintf(buffer, 256, "R##checkbox%sreset", v->name.c_str());
                    if (ImGui::ContrastedButton(buffer)) {
                        change |= true;
                        v->bx = v->bdef.x;
                        v->by = v->bdef.y;
                        v->bz = v->bdef.z;
                        v->bw = v->bdef.w;
                    }
                    ImGui::SameLine();
                    snprintf(buffer, 256, "##radio%sx", v->name.c_str());
                    if (ImGui::Checkbox(buffer, &v->bx)) {
                        change |= true;
                        v->by = false;
                        v->bz = false;
                        v->bw = false;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                        vCodeTreePtr->RecordToTimeLine(key, v, 3);
                    }
                }
                if (v->count > 1) {
                    ImGui::SameLine();
                    snprintf(buffer, 256, "##radio%sy", v->name.c_str());
                    if (ImGui::Checkbox(buffer, &v->by)) {
                        change |= true;
                        v->bx = false;
                        v->bz = false;
                        v->bw = false;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                        vCodeTreePtr->RecordToTimeLine(key, v, 3);
                    }
                }
                if (v->count > 2) {
                    ImGui::SameLine();
                    snprintf(buffer, 256, "##radio%sz", v->name.c_str());
                    if (ImGui::Checkbox(buffer, &v->bz)) {
                        change |= true;
                        v->bx = false;
                        v->by = false;
                        v->bw = false;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                        vCodeTreePtr->RecordToTimeLine(key, v, 3);
                    }
                }
                if (v->count > 3) {
                    ImGui::SameLine();
                    snprintf(buffer, 256, "##radio%sw", v->name.c_str());
                    if (ImGui::Checkbox(buffer, &v->bw)) {
                        change |= true;
                        v->bx = false;
                        v->by = false;
                        v->bz = false;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                        vCodeTreePtr->RecordToTimeLine(key, v, 3);
                    }
                }

                ImGui::PopStyleColor();
                ImGui::PopItemWidth();
            } else if (v->widgetType == "combobox") {
                ImGui::Separator();

                drawUniformName(key, v);
                ImGui::SameLine(vFirstColumnWidth);
                ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX() - 24);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                char buffer[256];
                snprintf(buffer, 256, "##combobox%s", v->name.c_str());
                if (ImGui::ContrastedComboVectorDefault(vMaxWidth - ImGui::GetCursorPosX(), buffer, &v->ix, v->choices, (int)v->choices.size(), (int)v->def.x)) {
                    change |= true;
                    vCodeTreePtr->RecordToTimeLine(key, v, 0);
                }
                ImGui::PopStyleColor();
                ImGui::PopItemWidth();
            } else if (v->widgetType == "color") {
                ImGui::Separator();

                if (TimeLineSystem::Instance()->IsActive()) {
                    const ImVec4 colLoc = ImGui::GetUniformLocColor(v->loc);

                    // red
                    drawUniformName(key, v, 0);
                    ImGui::SameLine(vFirstColumnWidth);
                    if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
                        change |= true;
                        v->x = v->def.x;
                    }
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, colLoc);
                    ImGui::PushID(ImGui::IncPUSHID());
                    if (ImGui::DragFloat("##colorRed", &v->x, 0.01f, 0.0f, 1.0f)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopID();
                    ImGui::PopStyleColor();
                    ImGui::PopItemWidth();

                    const float cursorY = ImGui::GetCursorPosY();

                    // green
                    drawUniformName(key, v, 1);
                    ImGui::SameLine(vFirstColumnWidth);
                    if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
                        change |= true;
                        v->y = v->def.y;
                    }
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, colLoc);
                    ImGui::PushID(ImGui::IncPUSHID());
                    if (ImGui::DragFloat("##colorGreen", &v->y, 0.01f, 0.0f, 1.0f)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                    }
                    ImGui::PopID();
                    ImGui::PopStyleColor();
                    ImGui::PopItemWidth();

                    const float endY = ImGui::GetCursorPosY();

                    // blue
                    drawUniformName(key, v, 2);
                    ImGui::SameLine(vFirstColumnWidth);
                    if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
                        change |= true;
                        v->z = v->def.z;
                    }
                    ImGui::SameLine();
                    ImGui::PushItemWidth(100.0f);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, colLoc);
                    ImGui::PushID(ImGui::IncPUSHID());
                    if (ImGui::DragFloat("##colorBlue", &v->z, 0.01f, 0.0f, 1.0f)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                    }
                    ImGui::PopID();
                    ImGui::PopStyleColor();
                    ImGui::PopItemWidth();

                    if (v->count == 4) {
                        // alpha
                        drawUniformName(key, v, 3);
                        ImGui::SameLine(vFirstColumnWidth);
                        if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
                            change |= true;
                            v->w = v->def.w;
                        }
                        ImGui::SameLine();
                        ImGui::PushItemWidth(100.0f);
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, colLoc);
                        ImGui::PushID(ImGui::IncPUSHID());
                        if (ImGui::DragFloat("##colorAlpha", &v->w, 0.01f, 0.0f, 1.0f)) {
                            change |= true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 3);
                        }
                        ImGui::PopID();
                        ImGui::PopStyleColor();
                        ImGui::PopItemWidth();
                    }

                    const float goodY = ImGui::GetCursorPosY();

                    ImGui::SameLine();

                    if (v->count == 4) {
                        ImGui::SetCursorPosY((cursorY + endY) * 0.5f);

                        ImGui::PushID(ImGui::IncPUSHID());
                        if (ImGui::ColorEdit4("##color", &v->x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB)) {
                            change |= true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 0);
                            vCodeTreePtr->RecordToTimeLine(key, v, 1);
                            vCodeTreePtr->RecordToTimeLine(key, v, 2);
                            vCodeTreePtr->RecordToTimeLine(key, v, 3);
                        }
                        ImGui::PopID();
                    } else {
                        ImGui::SetCursorPosY(cursorY);

                        ImGui::PushID(ImGui::IncPUSHID());
                        if (ImGui::ColorEdit3("##color", &v->x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB)) {
                            change |= true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 0);
                            vCodeTreePtr->RecordToTimeLine(key, v, 1);
                            vCodeTreePtr->RecordToTimeLine(key, v, 2);
                        }
                        ImGui::PopID();
                    }

                    ImGui::SetCursorPosY(goodY);
                } else {
                    drawUniformName(key, v);
                    ImGui::SameLine(vFirstColumnWidth);
                    if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
                        change |= true;
                        v->x = v->def.x;
                        v->y = v->def.y;
                        v->z = v->def.z;
                        if (v->count == 4)
                            v->w = v->def.w;
                    }
                    ImGui::SameLine();
                    ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX() - 9.0f);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    ImGui::PushID(ImGui::IncPUSHID());
                    if (v->count == 4) {
                        if (ImGui::ColorEdit4("##colorValue", &v->x, ImGuiColorEditFlags_Float)) {
                            change |= true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 0);
                            vCodeTreePtr->RecordToTimeLine(key, v, 1);
                            vCodeTreePtr->RecordToTimeLine(key, v, 2);
                            vCodeTreePtr->RecordToTimeLine(key, v, 3);
                        }
                    } else {
                        if (ImGui::ColorEdit3("##colorValue", &v->x, ImGuiColorEditFlags_Float)) {
                            change |= true;
                            vCodeTreePtr->RecordToTimeLine(key, v, 0);
                            vCodeTreePtr->RecordToTimeLine(key, v, 1);
                            vCodeTreePtr->RecordToTimeLine(key, v, 2);
                            vCodeTreePtr->RecordToTimeLine(key, v, 3);
                        }
                    }
                    ImGui::PopID();
                    ImGui::PopStyleColor();
                    ImGui::PopItemWidth();
                }
            } else if (v->widgetType == "deltatime") {
                ImGui::Separator();

                drawUniformName(key, v);
                ImGui::SameLine(vFirstColumnWidth);
                ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                ImGui::Text("(ms) %.6f\n FPS Max (f/s) %.0f)", v->x, v->x * 1000000);
                ImGui::PopItemWidth();
            } else if (v->widgetType == "frame") {
                ImGui::Separator();

                drawUniformName(key, v);
                ImGui::SameLine(vFirstColumnWidth);
                ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                ImGui::Text("%i", v->ix);
                ImGui::PopItemWidth();
            } else if (v->widgetType == "date") {
                ImGui::Separator();

                drawUniformName(key, v);
                ImGui::SameLine(vFirstColumnWidth);
                ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                ImGui::Text("%.0f / %.0f / %.0f / %.3f", v->x, v->y, v->z, v->w);
                ImGui::PopItemWidth();
            } else if (v->widgetType == "buffer") {
                if (v->glslType == uType::uTypeEnum::U_VEC2) {
                    ImGui::Separator();

                    drawUniformName(key, v);
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                    ImGui::Text("x:%.0f y:%.0f", v->x, v->y);
                    ImGui::PopItemWidth();
                } else if (v->glslType == uType::uTypeEnum::U_VEC3) {
                    ImGui::Separator();

                    drawUniformName(key, v);
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                    ImGui::Text("x:%.0f y:%.0f z:%.0f", v->x, v->y, v->z);
                    ImGui::PopItemWidth();
                }
                if (v->glslType == uType::uTypeEnum::U_IVEC2) {
                    ImGui::Separator();

                    drawUniformName(key, v);
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                    ImGui::Text("x:%i y:%i", v->ix, v->iy);
                    ImGui::PopItemWidth();
                } else if (v->glslType == uType::uTypeEnum::U_IVEC3) {
                    ImGui::Separator();

                    drawUniformName(key, v);
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                    ImGui::Text("x:%i y:%i z:%i", v->ix, v->iy, v->iz);
                    ImGui::PopItemWidth();
                } else if (v->glslType == uType::uTypeEnum::U_UVEC2) {
                    ImGui::Separator();

                    drawUniformName(key, v);
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                    ImGui::Text("x:%u y:%u", v->ux, v->uy);
                    ImGui::PopItemWidth();
                } else if (v->glslType == uType::uTypeEnum::U_UVEC3) {
                    ImGui::Separator();

                    drawUniformName(key, v);
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushItemWidth(vMaxWidth - ImGui::GetCursorPosX());
                    ImGui::Text("x:%u y:%u z:%u", v->ux, v->uy, v->uz);
                    ImGui::PopItemWidth();
                } else if (v->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                    ImGui::Separator();

                    drawUniformName(key, v);
                    ctTexturePtr tex = nullptr;
                    if (v->pipe)
                        tex = v->pipe->getBackTexture(v->attachment);
                    else if (v->texture_ptr)
                        tex = v->texture_ptr->getBack();
                    if (v->bufferFileChoosebox || v->bufferChoiceActivated) {
                        ImGui::SameLine(vFirstColumnWidth);
                        if (ImGui::TextureButton(tex, (vMaxWidth - vFirstColumnWidth) * 0.5f, ImGui::GetUniformLocColor(v->loc), 10)) {
                            vCodeTreePtr->puBufferFilePath = FileHelper::Instance()->GetAbsolutePathForFileLocation("", (int)FILE_LOCATION_Enum::FILE_LOCATION_IMPORT);
                            if (vCodeTreePtr->puBufferFilePathName.find(".glsl") == std::string::npos)
                                vCodeTreePtr->puBufferFilePathName.clear();
                            IGFD::FileDialogConfig config;
                            config.path = vCodeTreePtr->puBufferFilePath;
                            config.filePathName = vCodeTreePtr->puBufferFilePathName;
                            config.sidePane = std::bind(&CodeTree::DrawBufferOptions, vCodeTreePtr, _1, _2, _3);
                            config.sidePaneWidth = 250.0f;
                            config.countSelectionMax = 1;
                            config.flags = ImGuiFileDialogFlags_DisableThumbnailMode | ImGuiFileDialogFlags_Modal;
                            ImGuiFileDialog::Instance()->OpenDialog("BufferDialog", "Open Buffer File", ".glsl", config);
                            vCodeTreePtr->InitBufferChooseDialogWithUniform(v);
                        }
                        change |= vCodeTreePtr->BufferPopupCheck(v);
                    } else if (tex != nullptr) {
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Texture(tex, (vMaxWidth - vFirstColumnWidth) * 0.5f, ImGui::GetUniformLocColor(v->loc), 10);
                        change |= vCodeTreePtr->BufferPopupCheck(v);
                    }
                }
            } else if (v->widgetType == "mouse") {
                if (v->glslType == uType::uTypeEnum::U_VEC4) {
                    ImGui::Separator();

                    drawUniformName(key, v, 0, ".x");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##x" + v->name).c_str(), &v->x, v->inf.x, v->sup.x, v->def.x, v->step.x, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 1, ".y");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##y" + v->name).c_str(), &v->y, v->inf.y, v->sup.y, v->def.y, v->step.y, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 2, ".z");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##z" + v->name).c_str(), &v->z, v->inf.z, v->sup.z, v->def.z, v->step.z, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 3, ".w");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##w" + v->name).c_str(), &v->w, v->inf.w, v->sup.w, v->def.w, v->step.w, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 3);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->widgetType == "picture" && v->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                ImGui::Separator();

                drawUniformName(key, v);
                ctTexturePtr tex = nullptr;
                if (v->texture_ptr)
                    tex = v->texture_ptr->getBack();
                if (v->textureFileChoosebox || v->textureChoiceActivated) {
                    ImGui::SameLine(vFirstColumnWidth);
                    if (ImGui::TextureButton(tex, (vMaxWidth - vFirstColumnWidth) * 0.5f, ImGui::GetUniformLocColor(v->loc), 10)) {
                        if (!v->filePathNames.empty()) {
                            vCodeTreePtr->puPictureFilePathName = v->filePathNames[0];
                            if (!FileHelper::Instance()->IsFileExist(vCodeTreePtr->puPictureFilePathName, true))
                                vCodeTreePtr->puPictureFilePathName = FileHelper::Instance()->GetAbsolutePathForFileLocation(
                                    vCodeTreePtr->puPictureFilePathName,
                                                                                                               (int)FILE_LOCATION_Enum::FILE_LOCATION_ASSET_TEXTURE_2D);
                        }
                        IGFD::FileDialogConfig config;
                        config.filePathName = vCodeTreePtr->puPictureFilePathName;
                        config.countSelectionMax = 1;
                        config.flags = ImGuiFileDialogFlags_Modal;
                        ImGuiFileDialog::Instance()->OpenDialog("PictureDialog",
                                                                "Open Picture File",
                                                                "Image files (*.png *.jpg *.jpeg *.tga *.hdr){.png,.jpg,.jpeg,.tga,.hdr},.png,.jpg,.jpeg,.tga,.hdr",
                                                                config);
                        vCodeTreePtr->InitTextureChooseDialogWithUniform(v);
                    }
                    change |= vCodeTreePtr->TexturePopupCheck(v);
                } else if (tex != nullptr) {
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::Texture(tex, (vMaxWidth - vFirstColumnWidth) * 0.5f, ImGui::GetUniformLocColor(v->loc), 10);
                    change |= vCodeTreePtr->TexturePopupCheck(v);
                }
            } else if (v->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                ImGui::Separator();

                drawUniformName(key, v);
                ctTexturePtr tex = nullptr;
                if (v->pipe)
                    tex = v->pipe->getBackTexture(v->attachment);
                else if (v->texture_ptr)
                    tex = v->texture_ptr->getBack();
                if (tex != nullptr) {
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::Texture(tex, (vMaxWidth - vFirstColumnWidth) * 0.5f, ImGui::GetUniformLocColor(v->loc), 10);
                } else if (v->uSampler2D && v->ratioXY > 0.0f) {
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::ImageRatio((ImTextureID)(size_t)v->uSampler2D, v->ratioXY, (vMaxWidth - vFirstColumnWidth) * 0.5f, ImGui::GetUniformLocColor(v->loc), 10);
                    ;
                }
            } else if (v->glslType == uType::uTypeEnum::U_SAMPLERCUBE) {
                ImGui::Separator();

                drawUniformName(key, v);
                ImGui::SameLine(vFirstColumnWidth);

                if (v->cubemap_ptr != nullptr) {
                    ImGui::Texture((vMaxWidth - vFirstColumnWidth) * 0.8f, v->name.c_str(), v->cubemap_ptr, v->loc);
                } else {
                    ImGui::Text("Pictures not found");
                }
            } else if (v->glslType == uType::uTypeEnum::U_FLOAT) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v);
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%.2f", v->x);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v);
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##x" + v->name).c_str(), &v->x, v->inf.x, v->sup.x, v->def.x, v->step.x, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_INT) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v);
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%i", v->ix);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v);
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderIntDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##x" + v->name).c_str(), &v->ix, (int)v->inf.x, (int)v->sup.x, (int)v->def.x, (int)v->step.x)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_UINT) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v);
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%u", v->ux);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v);
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderUIntDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##x" + v->name).c_str(), &v->ux, (int)v->inf.x, (int)v->sup.x, (int)v->def.x, (int)v->step.x)) {
                        change |= true;
                        v->ux = ct::maxi<uint32_t>(v->ux, 0U);
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_VEC2) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v, 0, ".x");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%.2f", v->x);

                        drawUniformName(key, v, 1, ".y");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%.2f", v->y);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v, 0, ".x");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##x" + v->name).c_str(), &v->x, v->inf.x, v->sup.x, v->def.x, v->step.x, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 1, ".y");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##y" + v->name).c_str(), &v->y, v->inf.y, v->sup.y, v->def.y, v->step.y, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_VEC3) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v, 0, ".x");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%.2f", v->x);

                        drawUniformName(key, v, 1, ".y");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%.2f", v->y);

                        drawUniformName(key, v, 2, ".z");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%.2f", v->z);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v, 0, ".x");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##x" + v->name).c_str(), &v->x, v->inf.x, v->sup.x, v->def.x, v->step.x, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 1, ".y");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##y" + v->name).c_str(), &v->y, v->inf.y, v->sup.y, v->def.y, v->step.y, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 2, ".z");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##z" + v->name).c_str(), &v->z, v->inf.z, v->sup.z, v->def.z, v->step.z, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_VEC4) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v, 0, ".x");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%.2f", v->x);

                        drawUniformName(key, v, 1, ".y");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%.2f", v->y);

                        drawUniformName(key, v, 2, ".z");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%.2f", v->z);

                        drawUniformName(key, v, 3, ".w");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%.2f", v->w);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v, 0, ".x");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##x" + v->name).c_str(), &v->x, v->inf.x, v->sup.x, v->def.x, v->step.x, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 1, ".y");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##y" + v->name).c_str(), &v->y, v->inf.y, v->sup.y, v->def.y, v->step.y, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 2, ".z");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##z" + v->name).c_str(), &v->z, v->inf.z, v->sup.z, v->def.z, v->step.z, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 3, ".w");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderFloatDefault(
                            vMaxWidth - ImGui::GetCursorPosX(), ("##w" + v->name).c_str(), &v->w, v->inf.w, v->sup.w, v->def.w, v->step.w, "%.5f")) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 3);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_IVEC2) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v, 0, ".x");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%i", v->ix);

                        drawUniformName(key, v, 1, ".y");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%i", v->iy);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v, 0, ".x");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                ("##x" + v->name).c_str(),
                                                &v->ix,
                                                (int32_t)v->inf.x,
                                                (int32_t)v->sup.x,
                                                (int32_t)v->def.x,
                                                (int32_t)v->step.x)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 1, ".y");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                ("##y" + v->name).c_str(),
                                                &v->iy,
                                                (int32_t)v->inf.y,
                                                (int32_t)v->sup.y,
                                                (int32_t)v->def.y,
                                                (int32_t)v->step.y)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_IVEC3) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v, 0, ".x");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%i", v->ix);

                        drawUniformName(key, v, 1, ".y");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%i", v->iy);

                        drawUniformName(key, v, 2, ".z");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%i", v->iz);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v, 0, ".x");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                ("##x" + v->name).c_str(),
                                                &v->ix,
                                                (int32_t)v->inf.x,
                                                (int32_t)v->sup.x,
                                                (int32_t)v->def.x,
                                                (int32_t)v->step.x)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 1, ".y");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                ("##y" + v->name).c_str(),
                                                &v->iy,
                                                (int32_t)v->inf.y,
                                                (int32_t)v->sup.y,
                                                (int32_t)v->def.y,
                                                (int32_t)v->step.y)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 2, ".z");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                ("##z" + v->name).c_str(),
                                                &v->iz,
                                                (int32_t)v->inf.z,
                                                (int32_t)v->sup.z,
                                                (int32_t)v->def.z,
                                                (int32_t)v->step.z)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_IVEC4) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v, 0, ".x");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%i", v->ix);

                        drawUniformName(key, v, 1, ".y");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%i", v->iy);

                        drawUniformName(key, v, 2, ".z");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%i", v->iz);

                        drawUniformName(key, v, 3, ".w");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%i", v->iw);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v, 0, ".x");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                ("##x" + v->name).c_str(),
                                                &v->ix,
                                                (int32_t)v->inf.x,
                                                (int32_t)v->sup.x,
                                                (int32_t)v->def.x,
                                                (int32_t)v->step.x)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 1, ".y");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                ("##y" + v->name).c_str(),
                                                &v->iy,
                                                (int32_t)v->inf.y,
                                                (int32_t)v->sup.y,
                                                (int32_t)v->def.y,
                                                (int32_t)v->step.y)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 2, ".z");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                ("##z" + v->name).c_str(),
                                                &v->iz,
                                                (int32_t)v->inf.z,
                                                (int32_t)v->sup.z,
                                                (int32_t)v->def.z,
                                                (int32_t)v->step.z)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 3, ".w");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                ("##w" + v->name).c_str(),
                                                &v->iw,
                                                (int32_t)v->inf.w,
                                                (int32_t)v->sup.w,
                                                (int32_t)v->def.w,
                                                (int32_t)v->step.w)) {
                        change |= true;
                        vCodeTreePtr->RecordToTimeLine(key, v, 3);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_UVEC2) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v, 0, ".x");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%u", v->ux);

                        drawUniformName(key, v, 1, ".y");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%u", v->uy);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v, 0, ".x");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderUIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                 ("##x" + v->name).c_str(),
                                                 &v->ux,
                                                 (uint32_t)v->inf.x,
                                                 (uint32_t)v->sup.x,
                                                 (uint32_t)v->def.x,
                                                 (uint32_t)v->step.x)) {
                        change |= true;
                        v->ux = ct::maxi<uint32_t>(v->ux, 0U);
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 1, ".y");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderUIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                 ("##y" + v->name).c_str(),
                                                 &v->uy,
                                                 (uint32_t)v->inf.y,
                                                 (uint32_t)v->sup.y,
                                                 (uint32_t)v->def.y,
                                                 (uint32_t)v->step.y)) {
                        change |= true;
                        v->uy = ct::maxi<uint32_t>(v->uy, 0U);
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_UVEC3) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v, 0, ".x");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%u", v->ux);

                        drawUniformName(key, v, 1, ".y");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%u", v->uy);

                        drawUniformName(key, v, 2, ".z");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%u", v->uz);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v, 0, ".x");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderUIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                 ("##x" + v->name).c_str(),
                                                 &v->ux,
                                                 (uint32_t)v->inf.x,
                                                 (uint32_t)v->sup.x,
                                                 (uint32_t)v->def.x,
                                                 (uint32_t)v->step.x)) {
                        change |= true;
                        v->ux = ct::maxi<uint32_t>(v->ux, 0U);
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 1, ".y");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderUIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                 ("##y" + v->name).c_str(),
                                                 &v->uy,
                                                 (uint32_t)v->inf.y,
                                                 (uint32_t)v->sup.y,
                                                 (uint32_t)v->def.y,
                                                 (uint32_t)v->step.y)) {
                        change |= true;
                        v->uy = ct::maxi<uint32_t>(v->uy, 0U);
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 2, ".z");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderUIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                 ("##z" + v->name).c_str(),
                                                 &v->uz,
                                                 (uint32_t)v->inf.z,
                                                 (uint32_t)v->sup.z,
                                                 (uint32_t)v->def.z,
                                                 (uint32_t)v->step.z)) {
                        change |= true;
                        v->uz = ct::maxi<uint32_t>(v->uz, 0U);
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_UVEC4) {
                if (v->constant || !v->widget.empty()) {
                    if (vShowCustom) {
                        ImGui::Separator();

                        drawUniformName(key, v, 0, ".x");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%u", v->ux);

                        drawUniformName(key, v, 1, ".y");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%u", v->uy);

                        drawUniformName(key, v, 2, ".z");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%u", v->uz);

                        drawUniformName(key, v, 3, ".w");
                        ImGui::SameLine(vFirstColumnWidth);
                        ImGui::Text("%u", v->uw);
                    }
                } else {
                    ImGui::Separator();

                    drawUniformName(key, v, 0, ".x");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderUIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                 ("##x" + v->name).c_str(),
                                                 &v->ux,
                                                 (uint32_t)v->inf.x,
                                                 (uint32_t)v->sup.x,
                                                 (uint32_t)v->def.x,
                                                 (uint32_t)v->step.x)) {
                        change |= true;
                        v->ux = ct::maxi<uint32_t>(v->ux, 0U);
                        vCodeTreePtr->RecordToTimeLine(key, v, 0);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 1, ".y");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderUIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                 ("##y" + v->name).c_str(),
                                                 &v->uy,
                                                 (uint32_t)v->inf.y,
                                                 (uint32_t)v->sup.y,
                                                 (uint32_t)v->def.y,
                                                 (uint32_t)v->step.y)) {
                        change |= true;
                        v->uy = ct::maxi<uint32_t>(v->uy, 0U);
                        vCodeTreePtr->RecordToTimeLine(key, v, 1);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 2, ".z");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderUIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                 ("##z" + v->name).c_str(),
                                                 &v->uz,
                                                 (uint32_t)v->inf.z,
                                                 (uint32_t)v->sup.z,
                                                 (uint32_t)v->def.z,
                                                 (uint32_t)v->step.z)) {
                        change |= true;
                        v->uz = ct::maxi<uint32_t>(v->uz, 0U);
                        vCodeTreePtr->RecordToTimeLine(key, v, 2);
                    }
                    ImGui::PopStyleColor();

                    drawUniformName(key, v, 3, ".w");
                    ImGui::SameLine(vFirstColumnWidth);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
                    if (ImGui::SliderUIntDefault(vMaxWidth - ImGui::GetCursorPosX(),
                                                 ("##w" + v->name).c_str(),
                                                 &v->uw,
                                                 (uint32_t)v->inf.w,
                                                 (uint32_t)v->sup.w,
                                                 (uint32_t)v->def.w,
                                                 (uint32_t)v->step.w)) {
                        change |= true;
                        v->uw = ct::maxi<uint32_t>(v->uw, 0U);
                        vCodeTreePtr->RecordToTimeLine(key, v, 3);
                    }
                    ImGui::PopStyleColor();
                }
            } else if (v->glslType == uType::uTypeEnum::U_MAT4) {
                ImGui::Separator();
                drawUniformName(key, v);
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
            } else if (v->glslType == uType::uTypeEnum::U_TEXT) {
                ImGui::Separator();
                drawUniformName(key, v);
                ImGui::SameLine(vFirstColumnWidth);
                ImGui::Text(v->text.c_str());
            }
        }
    }

    return change;
}

bool UniformWidgets::checkUniformVisiblity(UniformVariantPtr v, bool vShowUnUsed) {
    bool visible = false;

    if (v->useVisCheckCond) {
        if (v->uniCheckCondPtr)  // checkbox
        {
            if (v->uniCheckCond && *(v->uniCheckCondPtr) > 0.5f) {
                visible = true;
            } else if (!v->uniCheckCond && *(v->uniCheckCondPtr) < 0.5f) {
                visible = true;
            }
        }
    } else if (v->useVisComboCond)  // combobox
    {
        if (v->uniComboCondPtr) {
            if (v->uniComboCondDir && *(v->uniComboCondPtr) == v->uniComboCond) {
                visible = true;
            } else if (!v->uniComboCondDir && *(v->uniComboCondPtr) != v->uniComboCond) {
                visible = true;
            }
        }
    } else if (v->useVisOpCond && v->uniCondPtr) {
        // 0 => no op // 1 > // 2 >= // 3 < // 4 <=
        if (v->useVisOpCond == 1)
            visible = (*(v->uniCondPtr) > v->uniOpCondThreshold);
        if (v->useVisOpCond == 2)
            visible = (*(v->uniCondPtr) >= v->uniOpCondThreshold);
        if (v->useVisOpCond == 3)
            visible = (*(v->uniCondPtr) < v->uniOpCondThreshold);
        if (v->useVisOpCond == 4)
            visible = (*(v->uniCondPtr) <= v->uniOpCondThreshold);
    } else {
        visible = true;
    }

    // si c'est des uniforms seulement pour l'ui on le met toujours visible
    if (v->uiOnly)
        v->loc = 0;

    if (visible && !vShowUnUsed) {
        if (v->loc < 0) {
            visible = false;
        }
    }

    return visible;
}

bool UniformWidgets::drawUniformName(ShaderKeyPtr vKey, UniformVariantPtr vUniPtr, int vComponent, const char* vTxt) {
    bool keyChanged = false;

    if (vUniPtr) {
        if (ShaderKeyConfigSwitcherUnified::Instance()->IsActivated()) {
            // on veut afficher un bouton cadenas pour pouvoir bloquer la maj de l'uniform en question quand on charge une nouvelle config
            ImGui::PushID(ImGui::IncPUSHID());
            if (vUniPtr->lockedAgainstConfigLoading) {
                if (ImGui::ButtonNoFrame(ICON_NDP2_SHIELD "##lockedagainstconfigloading", ImVec2(7, 7), ImVec4(0.9f, 0.1f, 0.5f, 1), "locked against config loading"))
                    vUniPtr->lockedAgainstConfigLoading = !vUniPtr->lockedAgainstConfigLoading;
            } else {
                if (ImGui::ButtonNoFrame(
                        ICON_NDP2_SHIELD_OUTLINE "##notlockedagainstconfigloading", ImVec2(7, 7), ImVec4(0.5f, 0.5f, 0.5f, 1), "not locked against config loading"))
                    vUniPtr->lockedAgainstConfigLoading = !vUniPtr->lockedAgainstConfigLoading;
            }

            ImGui::PopID();

            ImGui::SameLine();
        }

        if (vUniPtr->timeLineSupported && TimeLineSystem::Instance()->IsActive()) {
            if (vUniPtr->glslType == uType::uTypeEnum::U_BOOL || vUniPtr->glslType == uType::uTypeEnum::U_BVEC2 || vUniPtr->glslType == uType::uTypeEnum::U_BVEC3 ||
                vUniPtr->glslType == uType::uTypeEnum::U_BVEC4 || vUniPtr->glslType == uType::uTypeEnum::U_FLOAT || vUniPtr->glslType == uType::uTypeEnum::U_VEC2 ||
                vUniPtr->glslType == uType::uTypeEnum::U_VEC3 || vUniPtr->glslType == uType::uTypeEnum::U_VEC4 || vUniPtr->glslType == uType::uTypeEnum::U_UINT ||
                vUniPtr->glslType == uType::uTypeEnum::U_UVEC2 || vUniPtr->glslType == uType::uTypeEnum::U_UVEC3 || vUniPtr->glslType == uType::uTypeEnum::U_UVEC4 ||
                vUniPtr->glslType == uType::uTypeEnum::U_INT || vUniPtr->glslType == uType::uTypeEnum::U_IVEC2 || vUniPtr->glslType == uType::uTypeEnum::U_IVEC3 ||
                vUniPtr->glslType == uType::uTypeEnum::U_IVEC4 || vUniPtr->glslType == uType::uTypeEnum::U_MAT2 || vUniPtr->glslType == uType::uTypeEnum::U_MAT3 ||
                vUniPtr->glslType == uType::uTypeEnum::U_MAT4) {
                bool status = TimeLineSystem::Instance()->IsKeyExist(vKey, vUniPtr, vComponent);
                if (!status) {
                    ImGui::PushID(ImGui::IncPUSHID());
                    keyChanged = ImGui::ButtonNoFrame(
                        ICON_NDP2_CHECKBOX_BLANK_CIRCLE "##bulletforaddkeyintimeline", ImVec2(7, 7), ImVec4(0.5f, 0.5f, 0.5f, 1), "add key in timeline");
                    ImGui::PopID();
                    if (keyChanged) {
                        status = !status;
                        if (status) {
                            TimeLineSystem::Instance()->AddKeyForCurrentFrame(vKey, vUniPtr, vComponent);
                        } else {
                            TimeLineSystem::Instance()->DelKeyForCurrentFrame(vKey, vUniPtr, vComponent);
                        }
                    }
                } else {
                    status = TimeLineSystem::Instance()->IsKeyExistForCurrentFrame(vKey, vUniPtr, vComponent);
                    ImGui::PushID(ImGui::IncPUSHID());
                    keyChanged =
                        ImGui::ButtonNoFrame(status ? ICON_NDP2_HEXAGON "##losangeforaddkeyintimeline" : ICON_NDP2_HEXAGON_OUTLINE "##losangeforaddkeyintimeline",
                                             ImVec2(7, 7),
                                             ImVec4(0.9f, 0.9f, 0.1f, 1),
                                             "add key in timeline");
                    ImGui::PopID();
                    if (keyChanged) {
                        status = !status;
                        if (status) {
                            TimeLineSystem::Instance()->AddKeyForCurrentFrame(vKey, vUniPtr, vComponent);
                        } else {
                            TimeLineSystem::Instance()->DelKeyForCurrentFrame(vKey, vUniPtr, vComponent);
                        }
                    }
                }

                ImGui::SameLine();
            }
        }

        if (vTxt) {
            ImGui::Text("%s %s", vUniPtr->name.c_str(), vTxt);
        } else {
            ImGui::Text(vUniPtr->name.c_str());
        }

        drawUniformComment(vUniPtr);
    }

    return keyChanged;
}

void UniformWidgets::drawUniformComment(UniformVariantPtr vUniPtr) {
    if (vUniPtr) {
        if (ImGui::IsItemActive() || ImGui::IsItemHovered())
            if (!vUniPtr->comment.empty())
                ImGui::SetTooltip(vUniPtr->comment.c_str());
    }
}