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

#include "GuiModel.h"

#include <ImGuiPack.h>
#include <Res/CustomFont.h>
#include <Res/CustomFont2.h>
#include <Renderer/RenderPack.h>
#include <Mesh/Model/BaseModel.h>
#include <Mesh/Model/PNTBTCModel.h>
#include <Mesh/Operations/MeshLoader.h>

GuiModel::GuiModel() {
}

GuiModel::~GuiModel() {
}

bool GuiModel::DrawGui(RenderPackWeak vParent, BaseModelWeak vBaseModelWeak, bool vCanOpenModel) {
    bool change = false;

    auto modelPtr = vBaseModelWeak.lock();
    if (modelPtr) {
        if (vCanOpenModel && modelPtr->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_MESH && modelPtr->GetMeshFormat() == BaseMeshFormatEnum::PRIMITIVE_FORMAT_PNTBTC) {
            if (ImGui::ContrastedButton(ICON_NDP_RESET)) {
                modelPtr->Reset();  // doit recharger le truc par default
                change |= true;
            }
            ImGui::SameLine();
            if (ImGui::ContrastedButton("Open Model 3D")) {
                MeshLoader::Instance()->OpenDialog(vParent);
            }
            MeshLoader::Instance()->DrawImGuiProgress(150.0f);

            ImGui::TextWrapped("Model : %s", modelPtr->GetFilePathName().c_str());

            int idx = 0;
            for (const auto& layout : modelPtr->GetLayouts()) {
                ImGui::TextWrapped("Attribute %i => %s", idx++, layout.c_str());
            }

            // dynamic not needed here, because we are a PRIMITIVE_TYPE_MESH
            auto meshPtr = std::static_pointer_cast<PNTBTCModel>(modelPtr);
            if (meshPtr) {
                ImGui::TextWrapped("Sub meshs count : %u", meshPtr->GetMeshCount());

                int32_t meshsCount = meshPtr->GetMeshCount();
                if (meshsCount) {
                    auto meshs = meshPtr->GetMeshs();
                    if (meshs) {
                        if (ImGui::BeginTable("##fileTable", 2, m_Flags)) {
                            ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch, -1, 0);
                            ImGui::TableSetupColumn("Show", ImGuiTableColumnFlags_WidthFixed, -1, 1);

                            ImGui::TableHeadersRow();

                            m_SceneClipper.Begin(meshsCount, ImGui::GetTextLineHeightWithSpacing());
                            while (m_SceneClipper.Step()) {
                                for (int i = m_SceneClipper.DisplayStart; i < m_SceneClipper.DisplayEnd; ++i) {
                                    if (i < 0 || i >= meshsCount)
                                        continue;

                                    auto ptr = meshs->at(i);
                                    if (ptr) {
                                        ImGui::TableNextRow();
                                        if (ImGui::TableSetColumnIndex(0))  // first column
                                        {
                                            ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
                                            selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                                            ImGui::Indent();
                                            bool _selectablePressed = ImGui::Selectable(ct::toStr("Sub Mesh %i", i).c_str(), m_Selection == i, selectableFlags);
                                            ImGui::Unindent();
                                            if (_selectablePressed) {
                                                m_Selection = i;
                                            }
                                        }
                                        if (ImGui::TableSetColumnIndex(1))  // second column
                                        {
                                            const char* lbl = "Hide";
                                            if (!ptr->m_CanWeRender)
                                                lbl = "Show";
                                            if (ImGui::SmallContrastedButton(lbl)) {
                                                ptr->m_CanWeRender = !ptr->m_CanWeRender;
                                                change = true;
                                            }
                                        }
                                    }
                                }
                            }
                            m_SceneClipper.End();

                            ImGui::EndTable();
                        }
                    }
                }
            }
        } else if (vCanOpenModel && modelPtr->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_MESH &&
                   modelPtr->GetMeshFormat() == BaseMeshFormatEnum::PRIMITIVE_FORMAT_PNC) {
            ImGui::TextWrapped("Model : %s", modelPtr->GetFilePathName().c_str());

            int idx = 0;
            for (const auto& layout : modelPtr->GetLayouts()) {
                ImGui::TextWrapped("Attribute %i => %s", idx++, layout.c_str());
            }

            // dynamic not needed here, because we are a PRIMITIVE_TYPE_MESH
            auto meshPtr = std::static_pointer_cast<PNTBTCModel>(modelPtr);
            if (meshPtr) {
                ImGui::TextWrapped("Sub meshs count : %u", meshPtr->GetMeshCount());

                int32_t meshsCount = meshPtr->GetMeshCount();
                if (meshsCount) {
                    auto meshs = meshPtr->GetMeshs();
                    if (meshs) {
                        if (ImGui::BeginTable("##fileTable", 2, m_Flags)) {
                            ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch, -1, 0);
                            ImGui::TableSetupColumn("Show", ImGuiTableColumnFlags_WidthFixed, -1, 1);

                            ImGui::TableHeadersRow();

                            m_SceneClipper.Begin(meshsCount, ImGui::GetTextLineHeightWithSpacing());
                            while (m_SceneClipper.Step()) {
                                for (int i = m_SceneClipper.DisplayStart; i < m_SceneClipper.DisplayEnd; ++i) {
                                    if (i < 0 || i >= meshsCount)
                                        continue;

                                    auto ptr = meshs->at(i);
                                    if (ptr) {
                                        ImGui::TableNextRow();
                                        if (ImGui::TableSetColumnIndex(0))  // first column
                                        {
                                            ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
                                            selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                                            ImGui::Indent();
                                            bool _selectablePressed = ImGui::Selectable(ct::toStr("Sub Mesh %i", i).c_str(), m_Selection == i, selectableFlags);
                                            ImGui::Unindent();
                                            if (_selectablePressed) {
                                                m_Selection = i;
                                            }
                                        }
                                        if (ImGui::TableSetColumnIndex(1))  // second column
                                        {
                                            const char* lbl = "Hide";
                                            if (!ptr->m_CanWeRender)
                                                lbl = "Show";
                                            if (ImGui::SmallContrastedButton(lbl)) {
                                                ptr->m_CanWeRender = !ptr->m_CanWeRender;
                                                change = true;
                                            }
                                        }
                                    }
                                }
                            }
                            m_SceneClipper.End();

                            ImGui::EndTable();
                        }
                    }
                }
            }
        }
    }

    return change;
}
