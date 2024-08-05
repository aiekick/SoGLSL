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

#include "InterfacePanes.h"
#include <CodeTree/CodeTree.h>
#include <Renderer/RenderPack.h>
#include <Gui/CustomGuiWidgets.h>
#include <Manager/HelpManager.h>
#include <CodeTree/Parsing/SectionCode.h>
#include <Uniforms/UniformWidgets.h>

InterfacePanes::InterfacePanes() {
    puForceRefreshCode = true;
    puShowCodeSection = 0;
    puShowCodeTreeStruct = false;
}

InterfacePanes::~InterfacePanes() {
}

void InterfacePanes::DisplayMessageOfRenderPack(const GuiBackend_Window& vWin,
                                                const bool& vHideWarnings,
                                                CodeTreePtr vCodeTree,
                                                RenderPackWeak vRenderPack,
                                                const bool& vShowCode,
                                                const bool& vUseTextEditor,
                                                const bool& vShowEditBtn) {
    bool show = false;

    auto rpPtr = vRenderPack.lock();
    if (!rpPtr)
        return;

    if (!rpPtr->GetSyntaxErrors())
        return;

    show = rpPtr->GetSyntaxErrors()->puIsThereSomeErrors;

    if (!vHideWarnings) {
        show |= rpPtr->GetSyntaxErrors()->puIsThereSomeWarnings;
    }

    if (show) {
        bool editCatched = false;

        if (rpPtr->GetSyntaxErrors()->ImGui_DisplayMessages(rpPtr->GetShaderKey(), rpPtr->puName.c_str(), true, vShowEditBtn, &editCatched)) {
            if (vShowCode) {
                ImGui::Indent();

                DisplayScriptCode(vWin, vCodeTree, vRenderPack, vUseTextEditor);

                ImGui::Unindent();
            }
        }
    }

    // buffers

    for (auto it : rpPtr->puBuffers) {
        DisplayMessageOfRenderPack(vWin, vHideWarnings, vCodeTree, it, vShowCode, vUseTextEditor, vShowEditBtn);
    }

    for (auto it : rpPtr->puSceneBuffers) {
        DisplayMessageOfRenderPack(vWin, vHideWarnings, vCodeTree, it, vShowCode, vUseTextEditor, vShowEditBtn);
    }
}

void InterfacePanes::DisplayScriptHelp(CodeTreePtr /*vCodeTree*/, RenderPackWeak /*vRenderPack*/) {
    HelpManager::Instance()->DrawImGui();
}

void InterfacePanes::DisplayOptiPane(const GuiBackend_Window& vWin, CodeTreePtr vCodeTree, RenderPackWeak vRenderPack) {
    UNUSED(vWin);
    UNUSED(vCodeTree);
    UNUSED(vRenderPack);

#ifdef USE_OPTIMIZER_SYSTEM
    OptimizerSystem::Instance()->DrawImGui(vWin, vCodeTree, vRenderPack);
#endif
}

void InterfacePanes::DisplayScriptCode(const GuiBackend_Window& vWin, CodeTreePtr /*vCodeTree*/, RenderPackWeak vRenderPack, const bool& vUseTextEditor) {
    auto rpPtr = vRenderPack.lock();
    if (rpPtr && rpPtr->GetShaderKey()) {
        // auto key = vRenderPack->GetShaderKey();

        ImGui::Text("RenderPack : %s", rpPtr->puName.c_str());

        if (rpPtr->GetShaderKey()->puIsVertexShaderPresent) {
            if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Vertex", "Show/Hide Vertex", puShowCodeSection == 0)) {
                puShowCodeSection = 0;
                puForceRefreshCode = true;
            }

            ImGui::SameLine();
        }

        if (rpPtr->GetShaderKey()->puIsGeometryShaderPresent) {
            if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Geometry", "Show/Hide Geometry", puShowCodeSection == 1)) {
                puShowCodeSection = 1;
                puForceRefreshCode = true;
            }

            ImGui::SameLine();
        }

        if (rpPtr->GetShaderKey()->puIsTesselationControlShaderPresent) {
            if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Tess Control", "Show/Hide Tesselation Control", puShowCodeSection == 2)) {
                puShowCodeSection = 2;
                puForceRefreshCode = true;
            }

            ImGui::SameLine();
        }

        if (rpPtr->GetShaderKey()->puIsTesselationEvalShaderPresent) {
            if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Tess Eval", "Show/Hide Tesselation Eval", puShowCodeSection == 3)) {
                puShowCodeSection = 3;
                puForceRefreshCode = true;
            }

            ImGui::SameLine();
        }

        if (rpPtr->GetShaderKey()->puIsFragmentShaderPresent) {
            if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Fragment", "Show/Hide Fragment", puShowCodeSection == 4)) {
                puShowCodeSection = 4;
                puForceRefreshCode = true;
            }

            ImGui::SameLine();
        }

        if (rpPtr->GetShaderKey()->puIsComputeShaderPresent) {
            if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "Compute", "Show/Hide Compute", puShowCodeSection == 5)) {
                puShowCodeSection = 5;
                puForceRefreshCode = true;
            }
        }

#ifdef _DEBUG
        ImGui::SameLine();

        ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "CodeTree", "Show/Hide CodeTree", &puShowCodeTreeStruct);
#endif

        if (puForceRefreshCode) {
            if (puShowCodeSection == 0) {
                puCodeContent = rpPtr->puLastShaderCode.GetSection("VERTEX").GetCode();
                if (vUseTextEditor)
                    puTextEditor.SetText(puCodeContent);
            } else if (puShowCodeSection == 1) {
                puCodeContent = rpPtr->puLastShaderCode.GetSection("GEOMETRY").GetCode();
                if (vUseTextEditor)
                    puTextEditor.SetText(puCodeContent);
            } else if (puShowCodeSection == 2) {
                puCodeContent = rpPtr->puLastShaderCode.GetSection("TESSCONTROL").GetCode();
                if (vUseTextEditor)
                    puTextEditor.SetText(puCodeContent);
            } else if (puShowCodeSection == 3) {
                puCodeContent = rpPtr->puLastShaderCode.GetSection("TESSEVAL").GetCode();
                if (vUseTextEditor)
                    puTextEditor.SetText(puCodeContent);
            } else if (puShowCodeSection == 4) {
                puCodeContent = rpPtr->puLastShaderCode.GetSection("FRAGMENT").GetCode();
                if (vUseTextEditor)
                    puTextEditor.SetText(puCodeContent);
            } else if (puShowCodeSection == 5) {
                puCodeContent = rpPtr->puLastShaderCode.GetSection("COMPUTE").GetCode();
                if (vUseTextEditor)
                    puTextEditor.SetText(puCodeContent);
            }

            puForceRefreshCode = false;
        }

#ifdef _DEBUG
        if (puShowCodeTreeStruct) {
            ImGui::Separator();

            if (puShowCodeSection == 0) {
                DisplayCodeTree("VERTEX", vRenderPack);
            } else if (puShowCodeSection == 1) {
                DisplayCodeTree("GEOMETRY", vRenderPack);
            } else if (puShowCodeSection == 2) {
                DisplayCodeTree("TESSCONTROL", vRenderPack);
            } else if (puShowCodeSection == 3) {
                DisplayCodeTree("TESSEVAL", vRenderPack);
            } else if (puShowCodeSection == 4) {
                DisplayCodeTree("FRAGMENT", vRenderPack);
            } else if (puShowCodeSection == 5) {
                DisplayCodeTree("COMPUTE", vRenderPack);
            }

            ImGui::Separator();
        }
#endif

        if (vUseTextEditor) {
            if (ImGui::ContrastedButton("Copy to Clipboard")) {
                FileHelper::Instance()->SaveInClipBoard(vWin.win, puTextEditor.GetText());
            }

            puTextEditor.Render("Code");
        } else {
            ImGui::Text(puCodeContent.c_str());
        }
    }
}

void InterfacePanes::DisplayCodeTree(std::string vSectionNameForErrorsLines, RenderPackWeak vRenderPack) {
    auto rpPtr = vRenderPack.lock();
    if (rpPtr && rpPtr->GetShaderKey()) {
        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable |
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoHostExtendY;
        if (ImGui::BeginTable("##DisplayCodeTree", 7, flags, ImVec2(0, 500))) {
            ImGui::TableSetupScrollFreeze(0, 1);  // Make header always visible
            ImGui::TableSetupColumn("type", ImGuiTableColumnFlags_WidthFixed, -1, 0);
            ImGui::TableSetupColumn("parent type", ImGuiTableColumnFlags_WidthFixed, -1, 1);
            ImGui::TableSetupColumn("section", ImGuiTableColumnFlags_WidthFixed, -1, 12);
            ImGui::TableSetupColumn("inFileBufferName", ImGuiTableColumnFlags_WidthFixed, -1, 3);
            ImGui::TableSetupColumn("start - end line", ImGuiTableColumnFlags_WidthFixed, -1, 4);
            ImGui::TableSetupColumn("include ?", ImGuiTableColumnFlags_WidthFixed, -1, 5);
            ImGui::TableSetupColumn("file path", ImGuiTableColumnFlags_WidthStretch, -1, 6);

            ImGui::TableHeadersRow();

            DisplayCodeTreeSectionRecurs(rpPtr->GetShaderKey()->puMainSection, vSectionNameForErrorsLines);

            ImGui::EndTable();
        }
    }
}

void InterfacePanes::DisplayCodeTreeSectionRecurs(std::shared_ptr<SectionCode> vSectionCode, std::string vSectionNameForErrorsLines) {
    if (vSectionCode) {
        if (!vSectionCode->subSections.empty()) {
            for (auto it = vSectionCode->subSections.begin(); it != vSectionCode->subSections.end(); ++it) {
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                    std::shared_ptr<SectionCode> sec = (*it2);
                    ImGui::PushID(ImGui::IncPUSHID());

                    bool opened = false;

                    if (ImGui::TableNextColumn())
                        opened =
                            ImGui::TreeNodeEx("##sectionTreeNode", ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_SpanAvailWidth, "%s", sec->currentType.c_str());
                    if (ImGui::TableNextColumn())
                        ImGui::Text("%s", sec->parentType.c_str());
                    if (ImGui::TableNextColumn())
                        ImGui::Text("%s", sec->name.c_str());
                    if (ImGui::TableNextColumn())
                        ImGui::Text("%s", sec->inFileBufferName.c_str());
                    if (ImGui::TableNextColumn())
                        ImGui::Text("%i->%i", sec->sourceCodeStartLine, sec->sourceCodeEndLine);
                    if (ImGui::TableNextColumn())
                        ImGui::Text("%s", (sec->isInclude ? "true" : "false"));
                    if (ImGui::TableNextColumn())
                        ImGui::Text("%s", sec->relativeFile.c_str());

                    ImGui::PopID();
                    if (opened) {
                        ImGui::Indent();
                        DisplayCodeTreeSectionRecurs(sec, vSectionNameForErrorsLines);
                        ImGui::Unindent();
                        ImGui::TreePop();
                    }
                }
            }
        } else {
            if (ImGui::TableNextColumn())
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "File %s", vSectionCode->relativeFile.c_str());
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");

            ImGui::TableNextRow();

            if (ImGui::TableNextColumn())
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                                   "Start Line Src(%u) Tgt(%u)",
                                   vSectionCode->sourceCodeStartLine,
                                   vSectionCode->finalCodeStartLine[vSectionNameForErrorsLines]);
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");

            ImGui::TableNextRow();

            if (ImGui::TableNextColumn())
                ImGui::Text("-------------------------\n%s-------------------------\n", vSectionCode->code.c_str());
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");

            ImGui::TableNextRow();

            if (ImGui::TableNextColumn())
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                                   "End Line Src(%u) Tgt(%u)",
                                   vSectionCode->sourceCodeStartLine,
                                   vSectionCode->finalCodeEndLine[vSectionNameForErrorsLines]);
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
            if (ImGui::TableNextColumn())
                ImGui::Text("");
        }
    }
}

void InterfacePanes::DisplayShaderNote(CodeTreePtr /*vCodeTree*/, RenderPackWeak vRenderPack) {
    auto rpPtr = vRenderPack.lock();
    if (rpPtr->puLastShaderNote.dico.find("name") != rpPtr->puLastShaderNote.dico.end())  // trouv� "name")
    {
        auto lst = &rpPtr->puLastShaderNote.dico["name"];
        for (auto it = lst->begin(); it != lst->end(); ++it) {
            ImGui::Text("Name : %s", (*it).c_str());
        }
    }
    if (rpPtr->puLastShaderNote.dico.find("date") != rpPtr->puLastShaderNote.dico.end())  // trouv� "date")
    {
        auto lst = &rpPtr->puLastShaderNote.dico["date"];
        for (auto it = lst->begin(); it != lst->end(); ++it) {
            ImGui::Text("Date : %s", (*it).c_str());
        }
    }
    if (rpPtr->puLastShaderNote.dico.find("url") != rpPtr->puLastShaderNote.dico.end())  // trouv�"url")
    {
        auto lst = &rpPtr->puLastShaderNote.dico["url"];
        for (auto it = lst->begin(); it != lst->end(); ++it) {
            ImGui::Text("Url :");
            ImGui::SameLine();
            ImGui::ClickableTextUrl((*it).c_str(), (*it).c_str());
        }
    }
    if (rpPtr->puLastShaderNote.dico.find("parent_url") != rpPtr->puLastShaderNote.dico.end())  // trouv� "parent_url")
    {
        auto lst = &rpPtr->puLastShaderNote.dico["parent_url"];
        for (auto it = lst->begin(); it != lst->end(); ++it) {
            ImGui::Text("Parent :");
            ImGui::SameLine();
            ImGui::ClickableTextUrl((*it).c_str(), (*it).c_str());
        }
    }
    if (rpPtr->puLastShaderNote.dico.find("user") != rpPtr->puLastShaderNote.dico.end())  // trouv� "user")
    {
        auto lst = &rpPtr->puLastShaderNote.dico["user"];
        for (auto it = lst->begin(); it != lst->end(); ++it) {
            if (rpPtr->puLastShaderNote.urls.find(*it) != rpPtr->puLastShaderNote.urls.end())  // trouv�
            {
                ImGui::Text("Author :");
                ImGui::SameLine();
                ImGui::ClickableTextUrl((*it).c_str(), rpPtr->puLastShaderNote.urls[*it].c_str());
            } else {
                ImGui::Text("Author : %s", (*it).c_str());
            }
        }
    }
    if (rpPtr->puLastShaderNote.dico.find("sound_url") != rpPtr->puLastShaderNote.dico.end())  // trouv� "sound_url")
    {
        auto lst = &rpPtr->puLastShaderNote.dico["sound_url"];
        for (auto it = lst->begin(); it != lst->end(); ++it) {
            ImGui::Text("Sound :");
            ImGui::SameLine();
            ImGui::ClickableTextUrl((*it).c_str(), (*it).c_str());
        }
    }
    auto lst = &rpPtr->puLastShaderNote.dico["infos"];
    for (auto it = lst->begin(); it != lst->end(); ++it) {
        ImGui::TextWrapped("%s", (*it).c_str());
    }
}

void InterfacePanes::DrawDebugPane(CodeTreePtr vCodeTree) {
    if (vCodeTree) {
        if (ImGui::CollapsingHeader("Debug")) {
            static char buffer[256] = "";
            static std::string debugSearchFilter = std::string();
            if (ImGui::InputText("Search", buffer, 256)) {
                debugSearchFilter = std::string(buffer);
            }
            static bool showControls = false;
            ImGui::Checkbox("Show Controls", &showControls);

            if (ImGui::CollapsingHeader("Uniforms")) {
                size_t idx = 0;
                for (auto itSha = vCodeTree->puShaderKeys.begin(); itSha != vCodeTree->puShaderKeys.end(); itSha++) {
                    ImGui::Text("Shader : %s", itSha->first.c_str());

                    ImGui::Indent();

                    for (auto itUni = itSha->second->puUniformsDataBase.begin(); itUni != itSha->second->puUniformsDataBase.end(); itUni++) {
                        if (itUni->first.find(debugSearchFilter) != std::string::npos || debugSearchFilter.empty()) {
                            ImVec4 col = ImVec4(0.8f, 0.2f, 0.2f, 1.00f);  // bad
                            if (itUni->second->loc >= 0)
                                col = ImVec4(0.2f, 0.8f, 0.2f, 1.00f);  // good

                            ImGui::PushStyleColor(ImGuiCol_Text, col);

                            ImGui::Text("%zu U %s : %p", idx, itUni->first.c_str(), (void*)itUni->second.get());

                            if (showControls) {
                                ImGui::Indent();

                                UniformWidgets::drawImGuiUniformWidgetForPanes(
                                    vCodeTree, itUni->second, ImGui::GetContentRegionMax().x, SHADER_UNIFORM_FIRST_COLUMN_WIDTH);

                                ImGui::Unindent();
                            }

                            ImGui::PopStyleColor();

                            idx++;
                        }
                    }

                    ImGui::Unindent();
                }

                idx = 0;
                for (auto itInc = vCodeTree->puIncludeKeys.begin(); itInc != vCodeTree->puIncludeKeys.end(); itInc++) {
                    ImGui::Text("include : %s", itInc->first.c_str());

                    ImGui::Indent();

                    for (auto itUni = itInc->second->puUniformsDataBase.begin(); itUni != itInc->second->puUniformsDataBase.end(); itUni++) {
                        if (itUni->second) {
                            if (itUni->first.find(debugSearchFilter) != std::string::npos || debugSearchFilter.empty()) {
                                ImVec4 col = ImVec4(0.8f, 0.2f, 0.2f, 1.00f);  // bad
                                if (itUni->second->loc >= 0)
                                    col = ImVec4(0.2f, 0.8f, 0.2f, 1.00f);  // good

                                ImGui::PushStyleColor(ImGuiCol_Text, col);

                                ImGui::Text("%zu U %s : %p", idx, itUni->first.c_str(), (void*)itUni->second.get());

                                if (showControls) {
                                    ImGui::Indent();

                                    UniformWidgets::drawImGuiUniformWidgetForPanes(
                                        vCodeTree, itUni->second, ImGui::GetContentRegionMax().x, SHADER_UNIFORM_FIRST_COLUMN_WIDTH);

                                    ImGui::Unindent();
                                }

                                ImGui::PopStyleColor();

                                idx++;
                            }
                        }
                    }

                    ImGui::Unindent();
                }
            }

            if (ImGui::CollapsingHeader("Uniforms MultiLoc")) {
                for (auto it = vCodeTree->puIncludeUniformSectionUniformsList.begin(); it != vCodeTree->puIncludeUniformSectionUniformsList.end(); ++it) {
                    // std::string includeFileName = it->first;

                    ImGui::Text("include : %s", it->first.c_str());

                    ImGui::Indent();

                    for (auto itSec = it->second.begin(); itSec != it->second.end(); ++itSec) {
                        // std::string section = itSec->first;
                        auto uniforms = &itSec->second;

                        for (auto itLoc = uniforms->begin(); itLoc != uniforms->end(); ++itLoc) {
                            auto multiLoc = *itLoc;

                            if (multiLoc->name.find(debugSearchFilter) != std::string::npos || debugSearchFilter.empty()) {
                                ImVec4 col = ImVec4(0.8f, 0.2f, 0.2f, 1.00f);  // bad
                                if (multiLoc->uniform->loc >= 0)
                                    col = ImVec4(0.2f, 0.8f, 0.2f, 1.00f);  // good

                                ImGui::PushStyleColor(ImGuiCol_Text, col);

                                ImGui::Text("MU %s : %p", multiLoc->name.c_str(), (void*)&multiLoc->uniform);

                                if (showControls) {
                                    ImGui::Indent();

                                    UniformWidgets::drawImGuiUniformWidgetForPanes(
                                        vCodeTree, multiLoc->uniform, ImGui::GetContentRegionMax().x, SHADER_UNIFORM_FIRST_COLUMN_WIDTH);

                                    ImGui::Unindent();
                                }

                                ImGui::PopStyleColor();

                                ImGui::Indent();

                                for (auto itMu = multiLoc->linkedUniforms.begin(); itMu != multiLoc->linkedUniforms.end(); ++itMu) {
                                    col = ImVec4(0.8f, 0.2f, 0.2f, 1.00f);  // bad
                                    if (itMu->first->loc >= 0)
                                        col = ImVec4(0.2f, 0.8f, 0.2f, 1.00f);  // good

                                    ImGui::PushStyleColor(ImGuiCol_Text, col);

                                    ImGui::Text("U %s : %p", itMu->first->name.c_str(), (void*)&(*itMu));

                                    if (showControls) {
                                        ImGui::Indent();

                                        UniformWidgets::drawImGuiUniformWidgetForPanes(
                                            vCodeTree, itMu->first, ImGui::GetContentRegionMax().x, SHADER_UNIFORM_FIRST_COLUMN_WIDTH);

                                        ImGui::Unindent();
                                    }

                                    ImGui::PopStyleColor();
                                }

                                ImGui::Unindent();
                            }
                        }
                    }

                    ImGui::Unindent();
                }
            }
        }
    }
}