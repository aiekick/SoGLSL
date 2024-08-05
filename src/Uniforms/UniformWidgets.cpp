#include "UniformWidgets.h"

#include <Res/CustomFont.h>
#include <Res/CustomFont2.h>
#include <Texture/Texture2D.h>
#include <Systems/MidiSystem.h>
#include <CodeTree/ShaderKey.h>
#include <Systems/GizmoSystem.h>
#include <Systems/SoundSystem.h>
#include <Renderer/RenderPack.h>
#include <Gui/CustomGuiWidgets.h>
#include <Systems/GamePadSystem.h>
#include <Uniforms/UniformVariant.h>
#include <CodeTree/ShaderKeyConfigSwitcherUnified.h>

using namespace std::placeholders;

static std::array<const char*, 4U> s_Labels{".x", ".y", ".z", ".w"};
static std::array<const char*, 4U> s_Tags{"##x", "##y", "##z", "##w"};

bool UniformWidgets::drawImGuiUniformWidgetForPanes(  //
    CodeTreePtr vCodeTreePtr,
    UniformVariantPtr vUniPtr,
    float vMaxWidth,
    float vFirstColumnWidth,
    RenderPackWeak vRenderPack,
    bool vShowUnUsed,
    bool vShowCustom) {
    bool change = false;
    ImVec2 widths(vFirstColumnWidth, vMaxWidth);
    if (vCodeTreePtr && vUniPtr) {
        const bool visible = checkUniformVisiblity(vUniPtr, vShowUnUsed);
        if (visible) {
            ShaderKeyPtr vShaderKeyPtr = nullptr;
            auto rpPtr = vRenderPack.lock();
            if (rpPtr)
                vShaderKeyPtr = rpPtr->GetShaderKey();
            if (GizmoSystem::Instance()->DrawWidget(vCodeTreePtr, vUniPtr, widths.y, widths.x, vRenderPack, vShowUnUsed, vShowCustom, false, &change)) {
                change = true;
            } else if (GamePadSystem::Instance()->DrawWidget(vCodeTreePtr, vUniPtr, widths.y, widths.x, vRenderPack, vShowUnUsed, vShowCustom, false, &change)) {
                change = true;
            } else if (MidiSystem::Instance()->DrawWidget(vCodeTreePtr, vUniPtr, widths.y, widths.x, vRenderPack, vShowUnUsed, vShowCustom, false, &change)) {
                change = true;
            } else if (SoundSystem::Instance()->DrawWidget(vCodeTreePtr, vUniPtr, widths.y, widths.x, vRenderPack, vShowUnUsed, vShowCustom, false, &change)) {
                change = true;
            }
#ifdef USE_VR
            else if (VRBackend::Instance()->DrawWidget(vCodeTreePtr, vUniPtr, vWidths.y, vWidths.x, vRenderPack, vShowUnUsed, vShowCustom, false, &change)) {
                change = true;
            }
#endif
            else if (vUniPtr->widgetType == "time") {
                change |= m_drawTimeWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->widgetType == "button") {
                change |= m_drawButtonWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->widgetType == "checkbox") {
                change |= m_drawCheckboxWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->widgetType == "radio") {
                change |= m_drawRadioWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->widgetType == "combobox") {
                change |= m_drawComboBoxWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->widgetType == "color") {
                change |= m_drawColorWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->widgetType == "deltatime") {
                change |= m_drawDeltaTimeWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->widgetType == "frame") {
                change |= m_drawFrameWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->widgetType == "date") {
                change |= m_drawDateWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->widgetType == "buffer") {
                change |= m_drawBufferWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->widgetType == "mouse") {
                change |= m_drawMouseWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->widgetType == "picture" && vUniPtr->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                change |= m_drawPictureWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                change |= m_drawSampler2DWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_SAMPLERCUBE) {
                change |= m_drawSamplerCubeWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_FLOAT) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetFloat(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 1);
                }
                else {
                    change |= m_drawSliderWidgetFloat(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 1);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_VEC2) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetFloat(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 2);
                } else {
                    change |= m_drawSliderWidgetFloat(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 2);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_VEC3) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetFloat(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 3);
                } else {
                    change |= m_drawSliderWidgetFloat(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 3);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_VEC4) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetFloat(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 4);
                } else {
                    change |= m_drawSliderWidgetFloat(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 4);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_INT) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 1);
                } else {
                    change |= m_drawSliderWidgetInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 1);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_IVEC2) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 2);
                } else {
                    change |= m_drawSliderWidgetInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 2);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_IVEC3) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 3);
                } else {
                    change |= m_drawSliderWidgetInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 3);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_IVEC4) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 4);
                } else {
                    change |= m_drawSliderWidgetInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 4);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_UINT) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetUInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 1);
                } else {
                    change |= m_drawSliderWidgetUInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 1);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_UVEC2) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetUInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 2);
                } else {
                    change |= m_drawSliderWidgetUInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 2);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_UVEC3) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetUInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 3);
                } else {
                    change |= m_drawSliderWidgetUInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 3);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_UVEC4) {
                if (vUniPtr->widgetType == "input") {
                    change |= m_drawInputWidgetUInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 4);
                } else {
                    change |= m_drawSliderWidgetUInt(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr, vShowCustom, 4);
                }
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_MAT4) {
                change |= m_drawMatrixWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            } else if (vUniPtr->glslType == uType::uTypeEnum::U_TEXT) {
                change |= m_drawTextWidget(widths, vCodeTreePtr, vShaderKeyPtr, vUniPtr);
            }
        }
    }
    return change;
}

bool UniformWidgets::checkUniformVisiblity(UniformVariantPtr vUniPtr, bool vShowUnUsed) {
    bool visible = false;

    if (vUniPtr != nullptr) {
        if (vUniPtr->useVisCheckCond) {
            if (vUniPtr->uniCheckCondPtr) {  // checkbox
                if (vUniPtr->uniCheckCond && *(vUniPtr->uniCheckCondPtr) > 0.5f) {
                    visible = true;
                } else if (!vUniPtr->uniCheckCond && *(vUniPtr->uniCheckCondPtr) < 0.5f) {
                    visible = true;
                }
            }
        } else if (vUniPtr->useVisComboCond) {  // combobox
            if (vUniPtr->uniComboCondPtr) {
                if (vUniPtr->uniComboCondDir && *(vUniPtr->uniComboCondPtr) == vUniPtr->uniComboCond) {
                    visible = true;
                } else if (!vUniPtr->uniComboCondDir && *(vUniPtr->uniComboCondPtr) != vUniPtr->uniComboCond) {
                    visible = true;
                }
            }
        } else if (vUniPtr->useVisOpCond && vUniPtr->uniCondPtr) {
            // 0 => no op // 1 > // 2 >= // 3 < // 4 <=
            if (vUniPtr->useVisOpCond == 1)
                visible = (*(vUniPtr->uniCondPtr) > vUniPtr->uniOpCondThreshold);
            if (vUniPtr->useVisOpCond == 2)
                visible = (*(vUniPtr->uniCondPtr) >= vUniPtr->uniOpCondThreshold);
            if (vUniPtr->useVisOpCond == 3)
                visible = (*(vUniPtr->uniCondPtr) < vUniPtr->uniOpCondThreshold);
            if (vUniPtr->useVisOpCond == 4)
                visible = (*(vUniPtr->uniCondPtr) <= vUniPtr->uniOpCondThreshold);
        } else {
            visible = true;
        }

        // si c'est des uniforms seulement pour l'ui on le met toujours visible
        if (vUniPtr->uiOnly) {
            vUniPtr->loc = 0;
        }

        if (visible && !vShowUnUsed) {
            if (vUniPtr->loc < 0) {
                visible = false;
            }
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
                        ICON_NDP2_CHECKBOX_BLANK_CIRCLE "##bulletforaddkeyintimeline", ImVec2(7, 7), ImVec4(0.5f, 0.5f, 0.5f, 1), "add vShaderKeyPtr in timeline");
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
                                             "add vShaderKeyPtr in timeline");
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

bool UniformWidgets::m_drawTimeWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    drawUniformName(vShaderKeyPtr, vUniPtr);
    ImGui::SameLine(vWidths.x);
    if (ImGui::ContrastedButton(ICON_NDP_RESET, "Reset")) {
        change |= true;
        vUniPtr->bx = (vUniPtr->def.x > 0.5f);
        vUniPtr->x = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::ToggleContrastedButton(ICON_NDP_PAUSE, ICON_NDP_PLAY, &vUniPtr->bx)) {
        change |= true;
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
    ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
    ImGui::PushID(ImGui::IncPUSHID());
    if (ImGui::InputFloat("##Time", &vUniPtr->x)) {
        change |= true;
        vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
    }
    ImGui::PopID();
    ImGui::PopItemWidth();
    ImGui::PopStyleColor();
    return change;
}

bool UniformWidgets::m_drawButtonWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    drawUniformName(vShaderKeyPtr, vUniPtr);
    ImGui::SameLine(vWidths.x);
    ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
    if (vUniPtr->count > 0) {
        vUniPtr->bx = false;
        if (ImGui::ContrastedButton(vUniPtr->buttonName0.c_str())) {
            vUniPtr->bx = true;
            change = true;
        }
    }
    if (vUniPtr->count > 1) {
        ImGui::SameLine();
        vUniPtr->by = false;
        if (ImGui::ContrastedButton(vUniPtr->buttonName1.c_str())) {
            vUniPtr->by = true;
            change = true;
        }
    }
    if (vUniPtr->count > 2) {
        ImGui::SameLine();
        vUniPtr->bz = false;
        if (ImGui::ContrastedButton(vUniPtr->buttonName2.c_str())) {
            vUniPtr->bz = true;
            change = true;
        }
    }
    if (vUniPtr->count > 3) {
        ImGui::SameLine();
        vUniPtr->bw = false;
        if (ImGui::ContrastedButton(vUniPtr->buttonName3.c_str())) {
            vUniPtr->bw = true;
            change = true;
        }
    }

    ImGui::PopItemWidth();
    return change;
}

bool UniformWidgets::m_drawCheckboxWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    char buffer[256];

    if (TimeLineSystem::Instance()->IsActive()) {
        if (vUniPtr->count > 0) {
            drawUniformName(vShaderKeyPtr, vUniPtr, 0);
            ImGui::SameLine(vWidths.x);
            ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
            snprintf(buffer, 256, "##checkbox%sx", vUniPtr->name.c_str());
            const bool btn = ImGui::ContrastedButton(ICON_NDP_RESET);
            if (btn) {
                change = true;
                vUniPtr->bx = vUniPtr->bdef.x;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(buffer, &vUniPtr->bx)) {
                change = true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
            }
            ImGui::PopStyleColor();
            ImGui::PopItemWidth();
        }
        if (vUniPtr->count > 1) {
            drawUniformName(vShaderKeyPtr, vUniPtr, 1);
            ImGui::SameLine(vWidths.x);
            ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
            snprintf(buffer, 256, "##checkbox%sy", vUniPtr->name.c_str());
            const bool btn = ImGui::ContrastedButton(ICON_NDP_RESET);
            if (btn) {
                change = true;
                vUniPtr->by = vUniPtr->bdef.y;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(buffer, &vUniPtr->by)) {
                change = true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
            }
            ImGui::PopStyleColor();
            ImGui::PopItemWidth();
        }
        if (vUniPtr->count > 2) {
            drawUniformName(vShaderKeyPtr, vUniPtr, 2);
            ImGui::SameLine(vWidths.x);
            ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
            snprintf(buffer, 256, "##checkbox%sz", vUniPtr->name.c_str());
            const bool btn = ImGui::ContrastedButton(ICON_NDP_RESET);
            if (btn) {
                change = true;
                vUniPtr->bz = vUniPtr->bdef.z;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(buffer, &vUniPtr->bz)) {
                change = true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
            }
            ImGui::PopStyleColor();
            ImGui::PopItemWidth();
        }
        if (vUniPtr->count > 3) {
            drawUniformName(vShaderKeyPtr, vUniPtr, 3);
            ImGui::SameLine(vWidths.x);
            ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
            snprintf(buffer, 256, "##checkbox%sw", vUniPtr->name.c_str());
            const bool btn = ImGui::ContrastedButton(ICON_NDP_RESET);
            if (btn) {
                change = true;
                vUniPtr->bw = vUniPtr->bdef.w;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(buffer, &vUniPtr->bw)) {
                change = true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 3);
            }
            ImGui::PopStyleColor();
            ImGui::PopItemWidth();
        }
    } else {
        drawUniformName(vShaderKeyPtr, vUniPtr);
        ImGui::SameLine(vWidths.x);
        ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));

        if (vUniPtr->count > 0) {
            snprintf(buffer, 256, "##checkbox%sx", vUniPtr->name.c_str());
            const bool btn = ImGui::ContrastedButton(ICON_NDP_RESET);
            if (btn) {
                change = true;
                vUniPtr->bx = vUniPtr->bdef.x;
                vUniPtr->by = vUniPtr->bdef.y;
                vUniPtr->bz = vUniPtr->bdef.z;
                vUniPtr->bw = vUniPtr->bdef.w;
            }
            ImGui::SameLine();
            if (ImGui::Checkbox(buffer, &vUniPtr->bx)) {
                change = true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
            }
        }
        if (vUniPtr->count > 1) {
            ImGui::SameLine();
            snprintf(buffer, 256, "##checkbox%sy", vUniPtr->name.c_str());
            if (ImGui::Checkbox(buffer, &vUniPtr->by)) {
                change = true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
            }
        }
        if (vUniPtr->count > 2) {
            ImGui::SameLine();
            snprintf(buffer, 256, "##checkbox%sz", vUniPtr->name.c_str());
            if (ImGui::Checkbox(buffer, &vUniPtr->bz)) {
                change = true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
            }
        }
        if (vUniPtr->count > 3) {
            ImGui::SameLine();
            snprintf(buffer, 256, "##checkbox%sw", vUniPtr->name.c_str());
            if (ImGui::Checkbox(buffer, &vUniPtr->bw)) {
                change = true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 3);
            }
        }

        ImGui::PopStyleColor();
        ImGui::PopItemWidth();
    }
    return change;
}

bool UniformWidgets::m_drawRadioWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    drawUniformName(vShaderKeyPtr, vUniPtr);
    ImGui::SameLine(vWidths.x);
    ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));

    char buffer[256];
    if (vUniPtr->count > 0) {
        if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
            change |= true;
            vUniPtr->bx = vUniPtr->bdef.x;
            vUniPtr->by = vUniPtr->bdef.y;
            vUniPtr->bz = vUniPtr->bdef.z;
            vUniPtr->bw = vUniPtr->bdef.w;
        }
        ImGui::SameLine();
        snprintf(buffer, 256, "##radio%sx", vUniPtr->name.c_str());
        if (ImGui::Checkbox(buffer, &vUniPtr->bx)) {
            change |= true;
            vUniPtr->by = false;
            vUniPtr->bz = false;
            vUniPtr->bw = false;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 3);
        }
    }
    if (vUniPtr->count > 1) {
        ImGui::SameLine();
        snprintf(buffer, 256, "##radio%sy", vUniPtr->name.c_str());
        if (ImGui::Checkbox(buffer, &vUniPtr->by)) {
            change |= true;
            vUniPtr->bx = false;
            vUniPtr->bz = false;
            vUniPtr->bw = false;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 3);
        }
    }
    if (vUniPtr->count > 2) {
        ImGui::SameLine();
        snprintf(buffer, 256, "##radio%sz", vUniPtr->name.c_str());
        if (ImGui::Checkbox(buffer, &vUniPtr->bz)) {
            change |= true;
            vUniPtr->bx = false;
            vUniPtr->by = false;
            vUniPtr->bw = false;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 3);
        }
    }
    if (vUniPtr->count > 3) {
        ImGui::SameLine();
        snprintf(buffer, 256, "##radio%sw", vUniPtr->name.c_str());
        if (ImGui::Checkbox(buffer, &vUniPtr->bw)) {
            change |= true;
            vUniPtr->bx = false;
            vUniPtr->by = false;
            vUniPtr->bz = false;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 3);
        }
    }

    ImGui::PopStyleColor();
    ImGui::PopItemWidth();
    return change;
}

bool UniformWidgets::m_drawComboBoxWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    drawUniformName(vShaderKeyPtr, vUniPtr);
    ImGui::SameLine(vWidths.x);
    ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX() - 24);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
    char buffer[256];
    snprintf(buffer, 256, "##combobox%s", vUniPtr->name.c_str());
    if (ImGui::ContrastedComboVectorDefault(
            vWidths.y - ImGui::GetCursorPosX(), buffer, &vUniPtr->ix, vUniPtr->choices, (int)vUniPtr->choices.size(), (int)vUniPtr->def.x)) {
        change |= true;
        vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
    }
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();
    return change;
}

bool UniformWidgets::m_drawColorWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    if (TimeLineSystem::Instance()->IsActive()) {
        const ImVec4 colLoc = ImGui::GetUniformLocColor(vUniPtr->loc);

        // red
        drawUniformName(vShaderKeyPtr, vUniPtr, 0);
        ImGui::SameLine(vWidths.x);
        if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
            change |= true;
            vUniPtr->x = vUniPtr->def.x;
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, colLoc);
        ImGui::PushID(ImGui::IncPUSHID());
        if (ImGui::DragFloat("##colorRed", &vUniPtr->x, 0.01f, 0.0f, 1.0f)) {
            change |= true;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
        }
        ImGui::PopID();
        ImGui::PopStyleColor();
        ImGui::PopItemWidth();

        const float cursorY = ImGui::GetCursorPosY();

        // green
        drawUniformName(vShaderKeyPtr, vUniPtr, 1);
        ImGui::SameLine(vWidths.x);
        if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
            change |= true;
            vUniPtr->y = vUniPtr->def.y;
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, colLoc);
        ImGui::PushID(ImGui::IncPUSHID());
        if (ImGui::DragFloat("##colorGreen", &vUniPtr->y, 0.01f, 0.0f, 1.0f)) {
            change |= true;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
        }
        ImGui::PopID();
        ImGui::PopStyleColor();
        ImGui::PopItemWidth();

        const float endY = ImGui::GetCursorPosY();

        // blue
        drawUniformName(vShaderKeyPtr, vUniPtr, 2);
        ImGui::SameLine(vWidths.x);
        if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
            change |= true;
            vUniPtr->z = vUniPtr->def.z;
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, colLoc);
        ImGui::PushID(ImGui::IncPUSHID());
        if (ImGui::DragFloat("##colorBlue", &vUniPtr->z, 0.01f, 0.0f, 1.0f)) {
            change |= true;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
        }
        ImGui::PopID();
        ImGui::PopStyleColor();
        ImGui::PopItemWidth();

        if (vUniPtr->count == 4) {
            // alpha
            drawUniformName(vShaderKeyPtr, vUniPtr, 3);
            ImGui::SameLine(vWidths.x);
            if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
                change |= true;
                vUniPtr->w = vUniPtr->def.w;
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(100.0f);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, colLoc);
            ImGui::PushID(ImGui::IncPUSHID());
            if (ImGui::DragFloat("##colorAlpha", &vUniPtr->w, 0.01f, 0.0f, 1.0f)) {
                change |= true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 3);
            }
            ImGui::PopID();
            ImGui::PopStyleColor();
            ImGui::PopItemWidth();
        }

        const float goodY = ImGui::GetCursorPosY();

        ImGui::SameLine();

        if (vUniPtr->count == 4) {
            ImGui::SetCursorPosY((cursorY + endY) * 0.5f);

            ImGui::PushID(ImGui::IncPUSHID());
            if (ImGui::ColorEdit4("##color", &vUniPtr->x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB)) {
                change |= true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 3);
            }
            ImGui::PopID();
        } else {
            ImGui::SetCursorPosY(cursorY);

            ImGui::PushID(ImGui::IncPUSHID());
            if (ImGui::ColorEdit3("##color", &vUniPtr->x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB)) {
                change |= true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
            }
            ImGui::PopID();
        }

        ImGui::SetCursorPosY(goodY);
    } else {
        drawUniformName(vShaderKeyPtr, vUniPtr);
        ImGui::SameLine(vWidths.x);
        if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
            change |= true;
            vUniPtr->x = vUniPtr->def.x;
            vUniPtr->y = vUniPtr->def.y;
            vUniPtr->z = vUniPtr->def.z;
            if (vUniPtr->count == 4)
                vUniPtr->w = vUniPtr->def.w;
        }
        float w = vWidths.y - ImGui::GetItemRectSize().x - vWidths.x;
        ImGui::SameLine();
        ImGui::PushItemWidth(w);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
        ImGui::PushID(ImGui::IncPUSHID());
        if (vUniPtr->count == 4) {
            if (ImGui::ColorEdit4("##colorValue", &vUniPtr->x, ImGuiColorEditFlags_Float)) {
                change |= true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 3);
            }
        } else {
            if (ImGui::ColorEdit3("##colorValue", &vUniPtr->x, ImGuiColorEditFlags_Float)) {
                change |= true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 3);
            }
        }
        ImGui::PopID();
        ImGui::PopStyleColor();
        ImGui::PopItemWidth();
    }
    return change;
}

bool UniformWidgets::m_drawDeltaTimeWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    drawUniformName(vShaderKeyPtr, vUniPtr);
    ImGui::SameLine(vWidths.x);
    ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
    ImGui::Text("(ms) %.6f\n FPS Max (f/s) %.0f)", vUniPtr->x, vUniPtr->x * 1000000);
    ImGui::PopItemWidth();
    return change;
}

bool UniformWidgets::m_drawFrameWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    drawUniformName(vShaderKeyPtr, vUniPtr);
    ImGui::SameLine(vWidths.x);
    ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
    ImGui::Text("%i", vUniPtr->ix);
    ImGui::PopItemWidth();
    return change;
}

bool UniformWidgets::m_drawDateWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    drawUniformName(vShaderKeyPtr, vUniPtr);
    ImGui::SameLine(vWidths.x);
    ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
    ImGui::Text("%.0f / %.0f / %.0f / %.3f", vUniPtr->x, vUniPtr->y, vUniPtr->z, vUniPtr->w);
    ImGui::PopItemWidth();
    return change;
}

bool UniformWidgets::m_drawBufferWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    if (vUniPtr->glslType == uType::uTypeEnum::U_VEC2) {
        ImGui::Separator();

        drawUniformName(vShaderKeyPtr, vUniPtr);
        ImGui::SameLine(vWidths.x);
        ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
        ImGui::Text("x:%.0f y:%.0f", vUniPtr->x, vUniPtr->y);
        ImGui::PopItemWidth();
    } else if (vUniPtr->glslType == uType::uTypeEnum::U_VEC3) {
        ImGui::Separator();

        drawUniformName(vShaderKeyPtr, vUniPtr);
        ImGui::SameLine(vWidths.x);
        ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
        ImGui::Text("x:%.0f y:%.0f z:%.0f", vUniPtr->x, vUniPtr->y, vUniPtr->z);
        ImGui::PopItemWidth();
    }
    if (vUniPtr->glslType == uType::uTypeEnum::U_IVEC2) {
        ImGui::Separator();

        drawUniformName(vShaderKeyPtr, vUniPtr);
        ImGui::SameLine(vWidths.x);
        ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
        ImGui::Text("x:%i y:%i", vUniPtr->ix, vUniPtr->iy);
        ImGui::PopItemWidth();
    } else if (vUniPtr->glslType == uType::uTypeEnum::U_IVEC3) {
        ImGui::Separator();

        drawUniformName(vShaderKeyPtr, vUniPtr);
        ImGui::SameLine(vWidths.x);
        ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
        ImGui::Text("x:%i y:%i z:%i", vUniPtr->ix, vUniPtr->iy, vUniPtr->iz);
        ImGui::PopItemWidth();
    } else if (vUniPtr->glslType == uType::uTypeEnum::U_UVEC2) {
        ImGui::Separator();

        drawUniformName(vShaderKeyPtr, vUniPtr);
        ImGui::SameLine(vWidths.x);
        ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
        ImGui::Text("x:%u y:%u", vUniPtr->ux, vUniPtr->uy);
        ImGui::PopItemWidth();
    } else if (vUniPtr->glslType == uType::uTypeEnum::U_UVEC3) {
        ImGui::Separator();

        drawUniformName(vShaderKeyPtr, vUniPtr);
        ImGui::SameLine(vWidths.x);
        ImGui::PushItemWidth(vWidths.y - ImGui::GetCursorPosX());
        ImGui::Text("x:%u y:%u z:%u", vUniPtr->ux, vUniPtr->uy, vUniPtr->uz);
        ImGui::PopItemWidth();
    } else if (vUniPtr->glslType == uType::uTypeEnum::U_SAMPLER2D) {
        ImGui::Separator();

        drawUniformName(vShaderKeyPtr, vUniPtr);
        ctTexturePtr tex = nullptr;
        if (vUniPtr->pipe)
            tex = vUniPtr->pipe->getBackTexture(vUniPtr->attachment);
        else if (vUniPtr->texture_ptr)
            tex = vUniPtr->texture_ptr->getBack();
        if (vUniPtr->bufferFileChoosebox || vUniPtr->bufferChoiceActivated) {
            ImGui::SameLine(vWidths.x);
            if (ImGui::TextureButton(tex, (vWidths.y - vWidths.x) * 0.5f, ImGui::GetUniformLocColor(vUniPtr->loc), 10)) {
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
                vCodeTreePtr->InitBufferChooseDialogWithUniform(vUniPtr);
            }
            change |= vCodeTreePtr->BufferPopupCheck(vUniPtr);
        } else if (tex != nullptr) {
            ImGui::SameLine(vWidths.x);
            ImGui::Texture(tex, (vWidths.y - vWidths.x) * 0.5f, ImGui::GetUniformLocColor(vUniPtr->loc), 10);
            change |= vCodeTreePtr->BufferPopupCheck(vUniPtr);
        }
    }
    return change;
}

bool UniformWidgets::m_drawMouseWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    if (vUniPtr->glslType == uType::uTypeEnum::U_VEC4) {
        ImGui::Separator();

        drawUniformName(vShaderKeyPtr, vUniPtr, 0, ".x");
        ImGui::SameLine(vWidths.x);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
        if (ImGui::SliderFloatDefault(vWidths.y - ImGui::GetCursorPosX(),
                                      ("##x" + vUniPtr->name).c_str(),
                                      &vUniPtr->x,
                                      vUniPtr->inf.x,
                                      vUniPtr->sup.x,
                                      vUniPtr->def.x,
                                      vUniPtr->step.x,
                                      "%.5f")) {
            change |= true;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
        }
        ImGui::PopStyleColor();

        drawUniformName(vShaderKeyPtr, vUniPtr, 1, ".y");
        ImGui::SameLine(vWidths.x);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
        if (ImGui::SliderFloatDefault(vWidths.y - ImGui::GetCursorPosX(),
                                      ("##y" + vUniPtr->name).c_str(),
                                      &vUniPtr->y,
                                      vUniPtr->inf.y,
                                      vUniPtr->sup.y,
                                      vUniPtr->def.y,
                                      vUniPtr->step.y,
                                      "%.5f")) {
            change |= true;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 1);
        }
        ImGui::PopStyleColor();

        drawUniformName(vShaderKeyPtr, vUniPtr, 2, ".z");
        ImGui::SameLine(vWidths.x);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
        if (ImGui::SliderFloatDefault(vWidths.y - ImGui::GetCursorPosX(),
                                      ("##z" + vUniPtr->name).c_str(),
                                      &vUniPtr->z,
                                      vUniPtr->inf.z,
                                      vUniPtr->sup.z,
                                      vUniPtr->def.z,
                                      vUniPtr->step.z,
                                      "%.5f")) {
            change |= true;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 2);
        }
        ImGui::PopStyleColor();

        drawUniformName(vShaderKeyPtr, vUniPtr, 3, ".w");
        ImGui::SameLine(vWidths.x);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
        if (ImGui::SliderFloatDefault(vWidths.y - ImGui::GetCursorPosX(),
                                      ("##w" + vUniPtr->name).c_str(),
                                      &vUniPtr->w,
                                      vUniPtr->inf.w,
                                      vUniPtr->sup.w,
                                      vUniPtr->def.w,
                                      vUniPtr->step.w,
                                      "%.5f")) {
            change |= true;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 3);
        }
        ImGui::PopStyleColor();
    }
    return change;
}

bool UniformWidgets::m_drawPictureWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    drawUniformName(vShaderKeyPtr, vUniPtr);
    ctTexturePtr tex = nullptr;
    if (vUniPtr->texture_ptr)
        tex = vUniPtr->texture_ptr->getBack();
    if (vUniPtr->textureFileChoosebox || vUniPtr->textureChoiceActivated) {
        ImGui::SameLine(vWidths.x);
        if (ImGui::TextureButton(tex, (vWidths.y - vWidths.x) * 0.5f, ImGui::GetUniformLocColor(vUniPtr->loc), 10)) {
            if (!vUniPtr->filePathNames.empty()) {
                vCodeTreePtr->puPictureFilePathName = vUniPtr->filePathNames[0];
                if (!FileHelper::Instance()->IsFileExist(vCodeTreePtr->puPictureFilePathName, true))
                    vCodeTreePtr->puPictureFilePathName = FileHelper::Instance()->GetAbsolutePathForFileLocation(vCodeTreePtr->puPictureFilePathName,
                                                                                                                 (int)FILE_LOCATION_Enum::FILE_LOCATION_ASSET_TEXTURE_2D);
            }
            IGFD::FileDialogConfig config;
            config.filePathName = vCodeTreePtr->puPictureFilePathName;
            config.countSelectionMax = 1;
            config.flags = ImGuiFileDialogFlags_Modal;
            ImGuiFileDialog::Instance()->OpenDialog(
                "PictureDialog", "Open Picture File", "Image files (*.png *.jpg *.jpeg *.tga *.hdr){.png,.jpg,.jpeg,.tga,.hdr},.png,.jpg,.jpeg,.tga,.hdr", config);
            vCodeTreePtr->InitTextureChooseDialogWithUniform(vUniPtr);
        }
        change |= vCodeTreePtr->TexturePopupCheck(vUniPtr);
    } else if (tex != nullptr) {
        ImGui::SameLine(vWidths.x);
        ImGui::Texture(tex, (vWidths.y - vWidths.x) * 0.5f, ImGui::GetUniformLocColor(vUniPtr->loc), 10);
        change |= vCodeTreePtr->TexturePopupCheck(vUniPtr);
    }
    return change;
}

bool UniformWidgets::m_drawSampler2DWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    drawUniformName(vShaderKeyPtr, vUniPtr);
    ctTexturePtr tex = nullptr;
    if (vUniPtr->pipe)
        tex = vUniPtr->pipe->getBackTexture(vUniPtr->attachment);
    else if (vUniPtr->texture_ptr)
        tex = vUniPtr->texture_ptr->getBack();
    if (tex != nullptr) {
        ImGui::SameLine(vWidths.x);
        ImGui::Texture(tex, (vWidths.y - vWidths.x) * 0.5f, ImGui::GetUniformLocColor(vUniPtr->loc), 10);
    } else if (vUniPtr->uSampler2D && vUniPtr->ratioXY > 0.0f) {
        ImGui::SameLine(vWidths.x);
        ImGui::ImageRatio((ImTextureID)(size_t)vUniPtr->uSampler2D, vUniPtr->ratioXY, (vWidths.y - vWidths.x) * 0.5f, ImGui::GetUniformLocColor(vUniPtr->loc), 10);
        ;
    }
    return change;
}

bool UniformWidgets::m_drawSamplerCubeWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();

    drawUniformName(vShaderKeyPtr, vUniPtr);
    ImGui::SameLine(vWidths.x);

    if (vUniPtr->cubemap_ptr != nullptr) {
        ImGui::Texture((vWidths.y - vWidths.x) * 0.8f, vUniPtr->name.c_str(), vUniPtr->cubemap_ptr, vUniPtr->loc);
    } else {
        ImGui::Text("Pictures not found");
    }
    return change;
}

bool UniformWidgets::m_drawMatrixWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();
    drawUniformName(vShaderKeyPtr, vUniPtr);
    ImGui::SameLine(vWidths.x);

    if (vUniPtr->loc > -1)
        ImGui::Text("Matrix 4x4");
    else
        ImGui::Text("Not Used");

    if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
        ImGui::Indent();

        if (vUniPtr->uFloatArr) {
            ImGui::Text("0 : %.2f %.2f %.2f %.2f\n1 : %.2f %.2f %.2f %.2f\n2 : %.2f %.2f %.2f %.2f\n3 : %.2f %.2f %.2f %.2f",
                        vUniPtr->uFloatArr[0],
                        vUniPtr->uFloatArr[1],
                        vUniPtr->uFloatArr[2],
                        vUniPtr->uFloatArr[3],
                        vUniPtr->uFloatArr[4],
                        vUniPtr->uFloatArr[5],
                        vUniPtr->uFloatArr[6],
                        vUniPtr->uFloatArr[7],
                        vUniPtr->uFloatArr[8],
                        vUniPtr->uFloatArr[9],
                        vUniPtr->uFloatArr[10],
                        vUniPtr->uFloatArr[11],
                        vUniPtr->uFloatArr[12],
                        vUniPtr->uFloatArr[13],
                        vUniPtr->uFloatArr[14],
                        vUniPtr->uFloatArr[15]);
        } else {
            ImGui::Text("0 : %.2f %.2f %.2f %.2f\n1 : %.2f %.2f %.2f %.2f\n2 : %.2f %.2f %.2f %.2f\n3 : %.2f %.2f %.2f %.2f",
                        vUniPtr->mat4[0][0],
                        vUniPtr->mat4[1][0],
                        vUniPtr->mat4[2][0],
                        vUniPtr->mat4[3][0],
                        vUniPtr->mat4[0][1],
                        vUniPtr->mat4[1][1],
                        vUniPtr->mat4[2][1],
                        vUniPtr->mat4[3][1],
                        vUniPtr->mat4[0][2],
                        vUniPtr->mat4[1][2],
                        vUniPtr->mat4[2][2],
                        vUniPtr->mat4[3][2],
                        vUniPtr->mat4[0][3],
                        vUniPtr->mat4[1][3],
                        vUniPtr->mat4[2][3],
                        vUniPtr->mat4[3][3]);
        }

        ImGui::Unindent();
    }
    return change;
}

bool UniformWidgets::m_drawTextWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr) {
    bool change = false;
    ImGui::Separator();
    drawUniformName(vShaderKeyPtr, vUniPtr);
    ImGui::SameLine(vWidths.x);
    ImGui::Text(vUniPtr->text.c_str());
    return change;
}

bool UniformWidgets::m_drawSliderWidgetFloat(const ImVec2& vWidths,
                                             CodeTreePtr vCodeTreePtr,
                                             ShaderKeyPtr vShaderKeyPtr,
                                             UniformVariantPtr vUniPtr,
                                             const bool vShowCustom,
                                             const size_t vChannels) {
    bool change = false;
    if (vUniPtr->constant || !vUniPtr->widget.empty()) {
        if (vShowCustom) {
            ImGui::Separator();
            for (size_t idx = 0; idx < vChannels; ++idx) {
                drawUniformName(vShaderKeyPtr, vUniPtr, idx-1, idx > 1 ? s_Labels[idx] : nullptr);
                ImGui::SameLine(vWidths.x);
                ImGui::Text("%.2f", vUniPtr->x);
            }
        }
    } else {
        ImGui::Separator();
        for (size_t idx = 0; idx < vChannels; ++idx) {
            drawUniformName(vShaderKeyPtr, vUniPtr, idx, vChannels > 1 ? s_Labels[idx] : nullptr);
            ImGui::SameLine(vWidths.x);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
            float* v = &vUniPtr->x + idx;
            float* vi = &vUniPtr->inf.x + idx;
            float* vs = &vUniPtr->sup.x + idx;
            float* vd = &vUniPtr->def.x + idx;
            float* vst = &vUniPtr->step.x + idx;
            if (ImGui::SliderFloatDefault(vWidths.y - ImGui::GetCursorPosX(), (s_Tags[idx] + vUniPtr->name).c_str(), v, *vi, *vs, *vd, *vst, "%f")) {
                change |= true;
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
            }
            ImGui::PopStyleColor();
        }
    }
    return change;
}

bool UniformWidgets::m_drawSliderWidgetInt(const ImVec2& vWidths,
                                           CodeTreePtr vCodeTreePtr,
                                           ShaderKeyPtr vShaderKeyPtr,
                                           UniformVariantPtr vUniPtr,
                                           const bool vShowCustom,
                                           const size_t vChannels) {
    bool change = false;
    if (vUniPtr->constant || !vUniPtr->widget.empty()) {
        if (vShowCustom) {
            ImGui::Separator();

            drawUniformName(vShaderKeyPtr, vUniPtr);
            ImGui::SameLine(vWidths.x);
            ImGui::Text("%i", vUniPtr->ix);
        }
    } else {
        ImGui::Separator();

        drawUniformName(vShaderKeyPtr, vUniPtr);
        ImGui::SameLine(vWidths.x);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
        if (ImGui::SliderIntDefault(vWidths.y - ImGui::GetCursorPosX(),
                                    ("##x" + vUniPtr->name).c_str(),
                                    &vUniPtr->ix,
                                    (int)vUniPtr->inf.x,
                                    (int)vUniPtr->sup.x,
                                    (int)vUniPtr->def.x,
                                    (int)vUniPtr->step.x)) {
            change |= true;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
        }
        ImGui::PopStyleColor();
    }
    return change;
}

bool UniformWidgets::m_drawSliderWidgetUInt(const ImVec2& vWidths,
                                            CodeTreePtr vCodeTreePtr,
                                            ShaderKeyPtr vShaderKeyPtr,
                                            UniformVariantPtr vUniPtr,
                                            const bool vShowCustom,
                                            const size_t vChannels) {
    bool change = false;
    if (vUniPtr->constant || !vUniPtr->widget.empty()) {
        if (vShowCustom) {
            ImGui::Separator();

            drawUniformName(vShaderKeyPtr, vUniPtr);
            ImGui::SameLine(vWidths.x);
            ImGui::Text("%i", vUniPtr->ix);
        }
    } else {
        ImGui::Separator();

        drawUniformName(vShaderKeyPtr, vUniPtr);
        ImGui::SameLine(vWidths.x);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
        if (ImGui::SliderIntDefault(vWidths.y - ImGui::GetCursorPosX(),
                                    ("##x" + vUniPtr->name).c_str(),
                                    &vUniPtr->ix,
                                    (int)vUniPtr->inf.x,
                                    (int)vUniPtr->sup.x,
                                    (int)vUniPtr->def.x,
                                    (int)vUniPtr->step.x)) {
            change |= true;
            vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
        }
        ImGui::PopStyleColor();
    }
    return change;
}

bool UniformWidgets::m_drawInputWidgetFloat(const ImVec2& vWidths,
                                            CodeTreePtr vCodeTreePtr,
                                            ShaderKeyPtr vShaderKeyPtr,
                                            UniformVariantPtr vUniPtr,
                                            const bool vShowCustom,
                                            const size_t vChannels) {
    bool change = false;
    if (vUniPtr->constant || !vUniPtr->widget.empty()) {
        if (vShowCustom) {
            ImGui::Separator();
            for (size_t idx = 0; idx < vChannels; ++idx) {
                drawUniformName(vShaderKeyPtr, vUniPtr, idx, idx > 1 ? s_Labels[idx] : nullptr);
                ImGui::SameLine(vWidths.x);
                ImGui::Text("%.2f", vUniPtr->x);
            }
        }
    } else {
        ImGui::Separator();
        for (size_t idx = 0; idx < vChannels; ++idx) {
            drawUniformName(vShaderKeyPtr, vUniPtr, idx, vChannels > 1 ? s_Labels[idx] : nullptr);
            ImGui::SameLine(vWidths.x);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
            auto* v = &vUniPtr->x + idx;
            auto* vi = &vUniPtr->inf.x + idx;
            auto* vs = &vUniPtr->sup.x + idx;
            auto* vd = &vUniPtr->def.x + idx;
            auto* vst = &vUniPtr->step.x + idx;
            if (ImGui::InputFloatDefault(vWidths.y - ImGui::GetCursorPosX(),
                                         (s_Tags[idx] + vUniPtr->name).c_str(),  //
                                         v,
                                         *vd,
                                         "%f",
                                         "%f",
                                         true,
                                         *vst,
                                         (*vst) * 2)) {
                change |= true;
                *v = ImClamp(*v, *vi, *vs);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
            }
            ImGui::PopStyleColor();
        }
    }
    return change;
}

bool UniformWidgets::m_drawInputWidgetInt(const ImVec2& vWidths,
                                          CodeTreePtr vCodeTreePtr,
                                          ShaderKeyPtr vShaderKeyPtr,
                                          UniformVariantPtr vUniPtr,
                                          const bool vShowCustom,
                                          const size_t vChannels) {
    bool change = false;
    if (vUniPtr->constant || !vUniPtr->widget.empty()) {
        if (vShowCustom) {
            ImGui::Separator();
            for (size_t idx = 0; idx < vChannels; ++idx) {
                drawUniformName(vShaderKeyPtr, vUniPtr, idx, idx > 1 ? s_Labels[idx] : nullptr);
                ImGui::SameLine(vWidths.x);
                ImGui::Text("%.2f", vUniPtr->x);
            }
        }
    } else {
        ImGui::Separator();
        for (size_t idx = 0; idx < vChannels; ++idx) {
            drawUniformName(vShaderKeyPtr, vUniPtr, idx, vChannels > 1 ? s_Labels[idx] : nullptr);
            ImGui::SameLine(vWidths.x);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
            auto* v = &vUniPtr->ix + idx;
            auto* vi = &vUniPtr->inf.x + idx;
            auto* vs = &vUniPtr->sup.x + idx;
            auto* vd = &vUniPtr->def.x + idx;
            auto* vst = &vUniPtr->step.x + idx;
            if (ImGui::InputIntDefault(vWidths.y - ImGui::GetCursorPosX(),
                                       (s_Tags[idx] + vUniPtr->name).c_str(),  //
                                       v,
                                       (int32_t)*vd,
                                       (int32_t)*vst,
                                       ((int32_t)*vst) * 2)) {
                change |= true;
                *v = ImClamp(*v, (int32_t)*vi, (int32_t)*vs);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
            }
            ImGui::PopStyleColor();
        }
    }
    return change;
}

bool UniformWidgets::m_drawInputWidgetUInt(const ImVec2& vWidths,
                                           CodeTreePtr vCodeTreePtr,
                                           ShaderKeyPtr vShaderKeyPtr,
                                           UniformVariantPtr vUniPtr,
                                           const bool vShowCustom,
                                           const size_t vChannels) {
    bool change = false;
    if (vUniPtr->constant || !vUniPtr->widget.empty()) {
        if (vShowCustom) {
            ImGui::Separator();
            for (size_t idx = 0; idx < vChannels; ++idx) {
                drawUniformName(vShaderKeyPtr, vUniPtr, idx, idx > 1 ? s_Labels[idx] : nullptr);
                ImGui::SameLine(vWidths.x);
                ImGui::Text("%.2f", vUniPtr->x);
            }
        }
    } else {
        ImGui::Separator();
        for (size_t idx = 0; idx < vChannels; ++idx) {
            drawUniformName(vShaderKeyPtr, vUniPtr, idx, vChannels > 1 ? s_Labels[idx] : nullptr);
            ImGui::SameLine(vWidths.x);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(vUniPtr->loc));
            auto* v = &vUniPtr->ux + idx;
            auto* vi = &vUniPtr->inf.x + idx;
            auto* vs = &vUniPtr->sup.x + idx;
            auto* vd = &vUniPtr->def.x + idx;
            auto* vst = &vUniPtr->step.x + idx;
            if (ImGui::InputUIntDefault(vWidths.y - ImGui::GetCursorPosX(),
                                        (s_Tags[idx] + vUniPtr->name).c_str(),  //
                                        v,
                                        (uint32_t)*vd,
                                        (uint32_t)*vst,
                                        ((uint32_t)*vst)*2)) {
                change |= true;
                *v = ImClamp(*v, (uint32_t)*vi, (uint32_t)*vs);
                vCodeTreePtr->RecordToTimeLine(vShaderKeyPtr, vUniPtr, 0);
            }
            ImGui::PopStyleColor();
        }
    }
    return change;
}