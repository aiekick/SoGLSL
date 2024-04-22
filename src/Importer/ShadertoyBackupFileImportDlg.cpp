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

#include "ShadertoyBackupFileImportDlg.h"

#include <imgui.h>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <Gui/CustomGuiWidgets.h>
#include <ImGuiPack.h>

#include <Importer/Format_ShaderToy.h>

ShadertoyBackupFileImportDlg::ShadertoyBackupFileImportDlg() {
    puShowDialog = false;
}

ShadertoyBackupFileImportDlg::~ShadertoyBackupFileImportDlg() {
}

void ShadertoyBackupFileImportDlg::OpenDialog(const std::string& vFilePathName) {
    if (puShowDialog)
        return;

    Init(vFilePathName);

    puShowDialog = true;
}

void ShadertoyBackupFileImportDlg::CloseDialog() {
    puShowDialog = false;
}

bool ShadertoyBackupFileImportDlg::DrawDialog() {
    if (puShowDialog) {
        const auto res = false;

        ImGui::Begin("Settings");

        DrawButtonsPane();

        ImGui::Separator();

        DrawContentPane();

        ImGui::End();

        if (ImGuiFileDialog::Instance()->Display("ImportSelectedShaders")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                prImportSelectedShaders(ImGuiFileDialog::Instance()->GetCurrentPath());
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("ImportAllShaders")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                prImportAllShaders(ImGuiFileDialog::Instance()->GetCurrentPath());
            }
            ImGuiFileDialog::Instance()->Close();
        }

        return res;
    }

    return false;
}

void ShadertoyBackupFileImportDlg::SetFunction_For_CreateManyFilesShader(std::function<std::string(std::string, std::list<ShaderInfos>)> vCreateManyFilesShaderFunc) {
    m_CreateManyFilesShaderFunc = vCreateManyFilesShaderFunc;
}

void ShadertoyBackupFileImportDlg::SetFunction_For_CreateOneFileShader(std::function<std::string(std::string, std::list<ShaderInfos>)> vCreateOneFileShaderFunc) {
    m_CreateOneFileShaderFunc = vCreateOneFileShaderFunc;
}

/////////////////////////////////////////////////////////////
///// PRIVATE ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void ShadertoyBackupFileImportDlg::ParseJsonArray(const picojson::array& vJsonArray) {
    for (const auto& shader : vJsonArray) {
        ShadertoyBackupFileInfo sha_info;

        sha_info.shader_code = shader.serialize(false);

        if (shader.contains("info")) {
            picojson::value info = shader.get("info");

            sha_info.id = info.get("id").to_str();
            sha_info.id_for_search = ct::toLower(sha_info.id);

            // 1668687822.067365000 => 17/11/2022 13:23:42.067365000
            auto date = info.get("date").to_str();
            if (date != "0") {
                auto time = ct::ivariant(date).GetD();
                std::time_t _epoch_time = (std::time_t)time;
                auto tm = std::localtime(&_epoch_time);
                sha_info.date = ct::toStr("%i/%i/%i %i:%i", tm->tm_year + 1900, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min);
            }

            sha_info.viewed = info.get("viewed").to_str();
            sha_info.name = info.get("name").to_str();
            sha_info.username = info.get("username").to_str();
            sha_info.description = info.get("description").to_str();
            sha_info.description_for_search = ct::toLower(sha_info.description);
            sha_info.likes = info.get("likes").to_str();
            sha_info.published = info.get("published").to_str();
            sha_info.flags = info.get("flags").to_str();
            sha_info.hasliked = info.get("hasliked").to_str();

            if (info.get("tags").is<picojson::array>()) {
                picojson::array tags = info.get("tags").get<picojson::array>();
                for (const auto& tag : tags) {
                    if (!sha_info.tags.empty())
                        sha_info.tags += ", ";
                    sha_info.tags += tag.to_str();
                }
            }
            sha_info.tags_for_search = ct::toLower(sha_info.tags);
        }

        if (shader.get("renderpass").is<picojson::array>()) {
            sha_info.count_renderpasses = shader.get("renderpass").get<picojson::array>().size();
        }

        m_Shaders.push_back(sha_info);
    }

    prApplyFiltering(m_SearchBuffer);
}

void ShadertoyBackupFileImportDlg::Init(const std::string& vFilePathName) {
    m_FilePathName = vFilePathName;

    if (!m_FilePathName.empty()) {
        m_Shaders.clear();

        auto file_string = FileHelper::Instance()->LoadFileToString(m_FilePathName);

        picojson::value _jsonParser;
        picojson::parse(_jsonParser, file_string);

        const std::string& err = picojson::get_last_error();
        if (!err.empty()) {
            LogVarError("%s", err.c_str());
        } else if (_jsonParser.is<picojson::object>()) {
            if (_jsonParser.contains("shaders")) {
                if (_jsonParser.get("shaders").is<picojson::array>()) {
                    ParseJsonArray(_jsonParser.get("shaders").get<picojson::array>());
                }
            }
        } else if (_jsonParser.is<picojson::array>()) {
            ParseJsonArray(_jsonParser.get<picojson::array>());
        }
    }
}

void ShadertoyBackupFileImportDlg::DrawContentPane() {
    auto size = ImGui::GetContentRegionMax() - ImVec2(100, 68);

    if (!ImGui::GetCurrentWindow()->ScrollbarY) {
        size.x -= ImGui::GetStyle().ScrollbarSize;
    }

    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoHostExtendY;

    bool selected = false;

    auto listViewID = ImGui::GetID("##ShadertoyBackupFileImportDlg_DrawContentPane");
    if (ImGui::BeginTableEx("Shaders##ShadertoyBackupFileImportDlg_DrawContentPane", listViewID, 10, flags))  //-V112
    {
        ImGui::TableSetupScrollFreeze(0, 1);  // Make header always visible
        ImGui::TableSetupColumn("Sel", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide);
        ImGui::TableSetupColumn("Tags", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Likes", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Views", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Child Passes", ImGuiTableColumnFlags_WidthFixed);

        ImGui::TableHeadersRow();

        m_VirtualClipper.Begin((int)m_FilteredShaders.size(), ImGui::GetFrameHeightWithSpacing());
        while (m_VirtualClipper.Step()) {
            for (int i = m_VirtualClipper.DisplayStart; i < m_VirtualClipper.DisplayEnd; ++i) {
                if (i < 0)
                    continue;

                auto& infos = m_FilteredShaders.at((size_t)i);

                ImGui::TableNextRow();

                ImGui::PushID(i);

                ImGui::TableSetColumnIndex(0);  // Selected
                ImGui::Checkbox("##selected", &infos.selected);

                ImGui::TableSetColumnIndex(1);  // Id
                ImGui::Selectable(infos.id.c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns);

                ImGui::TableSetColumnIndex(2);  // Name
                ImGui::Selectable(infos.name.c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns);

                ImGui::TableSetColumnIndex(3);  // Description
                ImGui::Selectable(infos.description.c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns);

                ImGui::TableSetColumnIndex(4);  // Tags
                ImGui::Selectable(infos.tags.c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns);

                ImGui::TableSetColumnIndex(5);  // Status
                ImGui::Selectable(infos.published.c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns);

                ImGui::TableSetColumnIndex(6);  // Date
                ImGui::Selectable(infos.date.c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns);

                ImGui::TableSetColumnIndex(7);  // Likes
                ImGui::Selectable(infos.likes.c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns);

                ImGui::TableSetColumnIndex(8);  // Views
                ImGui::Selectable(infos.viewed.c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns);

                ImGui::TableSetColumnIndex(9);  // Renderpasses

                if (infos.count_renderpasses > 1U) {
                    ImGui::Text("%u", (uint32_t)(infos.count_renderpasses - 1U));
                } else {
                    ImGui::Text("");
                }

                ImGui::PopID();
            }
        }
        m_VirtualClipper.End();

        ImGui::EndTable();
    }
}

void ShadertoyBackupFileImportDlg::DrawButtonsPane() {
    if (ImGui::ContrastedButton("Close##ShadertoyBackupFileImportDlg")) {
        CloseDialog();
    }

    ImGui::SameLine();

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

    /*ImGui::SameLine();

    if (ImGui::ContrastedButton("Multipass"))
    {
        CTOOL_DEBUG_BREAK;
    }

    ImGui::SameLine();

    if (ImGui::ContrastedButton("GPU Sound"))
    {
        CTOOL_DEBUG_BREAK;
    }

    ImGui::SameLine();

    if (ImGui::ContrastedButton("VR"))
    {
        CTOOL_DEBUG_BREAK;
    }

    ImGui::SameLine();

    if (ImGui::ContrastedButton("Mic"))
    {
        CTOOL_DEBUG_BREAK;
    }

    ImGui::SameLine();

    if (ImGui::ContrastedButton("Soundcloud"))
    {
        CTOOL_DEBUG_BREAK;
    }

    ImGui::SameLine();

    if (ImGui::ContrastedButton("Webcam"))
    {
        CTOOL_DEBUG_BREAK;
    }*/

    ImGui::SameLine();

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

    ImGui::SameLine();

    if (ImGui::ContrastedButton("Import Selected Shaders")) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("ImportSelectedShaders", "Where create shader files (if exsiting files will be overwritten)", nullptr, config);
    }

    ImGui::SameLine();

    if (ImGui::ContrastedButton("Import All Shaders")) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.countSelectionMax = 1;
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("ImportAllShaders", "Where create shader files (if exsiting files will be overwritten)", nullptr, config);
    }

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

    ImGui::Text("Search : ");

    ImGui::SameLine();

    if (ImGui::ContrastedButton("R")) {
        ct::ResetBuffer(m_SearchBuffer);
        prApplyFiltering(m_SearchBuffer);
    }

    ImGui::SameLine();

    if (ImGui::InputText("##searchShadertoyBackupFileImportDlg", m_SearchBuffer, 1024)) {
        prApplyFiltering(m_SearchBuffer);
    }
}

std::string ShadertoyBackupFileImportDlg::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    std::string str;

    str += vOffset + "<shadertoy_backup_file_import>\n";

    str += vOffset + "</shadertoy_backup_file_import>\n";

    return str;
}

bool ShadertoyBackupFileImportDlg::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    // The value of this child identifies the name of this element
    std::string strName = "";
    std::string strValue = "";
    std::string strParentName = "";

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "shadertoy_backup_file_import") {
    }

    return false;
}

void ShadertoyBackupFileImportDlg::prImportSelectedShaders(const std::string& vPath) {
    if (m_CreateOneFileShader) {
        LogAssert(m_CreateOneFileShaderFunc != nullptr, "m_CreateOneFileShaderFunc is null");
    } else {
        LogAssert(m_CreateManyFilesShaderFunc != nullptr, "m_CreateManyFilesShaderFunc is null");
    }

    std::unordered_map<std::string, std::list<ShaderInfos>> _shaders;

    for (const auto& shader : m_FilteredShaders) {
        if (shader.selected) {
            ImporterFromShadertoy _importer;
            auto shader_list = _importer.ParseBuffer(shader.shader_code, shader.id);
            if (!shader_list.empty()) {
                _shaders[shader.id] = shader_list;
            }
        }
    }

    for (const auto& shader : _shaders) {
        if (m_CreateOneFileShader && m_CreateOneFileShaderFunc) {
            m_CreateOneFileShaderFunc(vPath, shader.second);
        } else if (m_CreateManyFilesShaderFunc) {
            m_CreateManyFilesShaderFunc(vPath, shader.second);
        }
    }
}

void ShadertoyBackupFileImportDlg::prImportAllShaders(const std::string& vPath) {
    if (m_CreateOneFileShader) {
        LogAssert(m_CreateOneFileShaderFunc != nullptr, "m_CreateOneFileShaderFunc is null");
    } else {
        LogAssert(m_CreateManyFilesShaderFunc != nullptr, "m_CreateManyFilesShaderFunc is null");
    }

    std::unordered_map<std::string, std::list<ShaderInfos>> _shaders;

    for (const auto& shader : m_FilteredShaders) {
        ImporterFromShadertoy _importer;
        auto shader_list = _importer.ParseBuffer(shader.shader_code, shader.id);
        if (!shader_list.empty()) {
            _shaders[shader.id] = shader_list;
        }
    }

    for (const auto& shader : _shaders) {
        if (m_CreateOneFileShader && m_CreateOneFileShaderFunc) {
            m_CreateOneFileShaderFunc(vPath, shader.second);
        } else if (m_CreateManyFilesShaderFunc) {
            m_CreateManyFilesShaderFunc(vPath, shader.second);
        }
    }
}

void ShadertoyBackupFileImportDlg::prApplyFiltering(const std::string& vSearchPattern) {
    m_FilteredShaders.clear();

    if (vSearchPattern.empty()) {
        m_FilteredShaders = m_Shaders;
        return;
    }

    const auto pattern = ct::toLower(vSearchPattern);

    for (const auto& shader : m_Shaders) {
        if (shader.description_for_search.find(pattern) != std::string::npos || shader.tags_for_search.find(pattern) != std::string::npos ||
            shader.id_for_search.find(pattern) != std::string::npos) {
            m_FilteredShaders.push_back(shader);
        }
    }
}