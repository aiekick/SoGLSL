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

#include "ShaderKeyConfigSwitcherUnified.h"

#include <algorithm>
#include <filesystem>
#include <ctools/Logger.h>
#include <CodeTree/CodeTree.h>
#include <Helper/NaturalSort.h>
#include <CodeTree/ShaderKey.h>
#include <Renderer/RenderPack.h>
#include <imgui_internal.h>
#include <Gui/CustomGuiWidgets.h>
#include <Res/CustomFontToolBar.h>
#include <Uniforms/UniformHelper.h>
#include <alphanum/alphanum.hpp>  // natural sorting

inline std::set<std::string> inConfigSwitchertScanDirectory(const std::string& vPath) {
    std::set<std::string> res;

    if (!vPath.empty()) {
        const std::filesystem::path fspath(vPath);
        const auto dir_iter = std::filesystem::directory_iterator(fspath);
        for (const auto& file : dir_iter) {
            if (file.is_regular_file()) {
                const auto file_name = file.path().filename().string();
                const auto ps        = FileHelper::Instance()->ParsePathFileName(file_name);
                if (ps.isOk) {
                    if (ps.ext == "conf") {
                        res.emplace(file_name);
                    }
                }
            }
        }
    }

    return res;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool ConfigSwitcherUnit::IsEmpty() {
    return puUniformSettings.empty();
}

void ConfigSwitcherUnit::Clear(const std::string& vKey) {
    puUniformSettings.clear();
    puOrderedStrings.clear();
    LoadUniformConfigSummaryFile(vKey);
}

void ConfigSwitcherUnit::Sort() {
    std::sort(puOrderedStrings.begin(), puOrderedStrings.end(),                  //
              [](const ConfInfos& left, const ConfInfos& right) {                //
                  return doj::alphanum_comp(left.confName, right.confName) < 0;  //
              }                                                                  //
    );                                                                           //
}

void ConfigSwitcherUnit::AddConfigName(const std::string& vName, const uint32_t& vCountUniforms) {
    ConfInfos ci;
    ci.confName              = vName;
    ci.countUniforms         = vCountUniforms;
    puUniformSettings[vName] = ci;
    puOrderedStrings.emplace_back(ci);
}

static std::vector<ConfInfos>::iterator searchForConfNameInContainer(const std::string& vName, std::vector<ConfInfos>& vContainer) {
    std::vector<ConfInfos>::iterator it;
    for (it = vContainer.begin(); it != vContainer.end(); ++it) {
        if (it->confName == vName) {
            break;
        }
    }
    return it;
}

void ConfigSwitcherUnit::RemoveConfigName(const std::string& vName) {
    puUniformSettings.erase(vName);

    // erase in puOrderedStrings
    auto it = searchForConfNameInContainer(vName, puOrderedStrings);
    if (it != puOrderedStrings.end()) {
        puOrderedStrings.erase(it);
    }
}

void ConfigSwitcherUnit::SaveUniformConfigSummaryFile(const std::string& vKey) {
    SaveUniformConfigSummaryFile(vKey, puUniformSettings);
}

void ConfigSwitcherUnit::SaveUniformConfigSummaryFile(const std::string& vKey, const std::unordered_map<std::string, ConfInfos>& vFilesToSave) {
    if (!vKey.empty() && !vFilesToSave.empty()) {
        const std::string configFile = CodeTree::GetConfigFileName(vKey, "") + "s";

        if (!configFile.empty()) {
            std::string fileStream;

            for (auto file : vFilesToSave) {
                fileStream += file.second.confName + ct::toStr(":%u", file.second.countUniforms) + "\n";
            }

            std::ofstream configFileWriter(configFile, std::ios::out);

            if (configFileWriter.bad() == false) {
                configFileWriter << fileStream;  // write the file
                configFileWriter.close();
            }
        }
    }
}

bool ConfigSwitcherUnit::LoadUniformConfigSummaryFile(const std::string& vKey) {
    bool res = false;

    if (!vKey.empty()) {
        std::string configFile = CodeTree::GetConfigFileName(vKey, "") + "s";
        if (!configFile.empty()) {
            std::string file;

            std::ifstream docFile(configFile, std::ios::in);
            if (docFile.is_open()) {
                std::stringstream strStream;

                strStream << docFile.rdbuf();  // read the file

                file = strStream.str();
                ct::replaceString(file, "\r\n", "\n");

                docFile.close();

                std::vector<std::string> lines = ct::splitStringToVector(file, "\n");

                if (!lines.empty()) {
                    puUniformSettings.clear();
                    puOrderedStrings.clear();

                    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
                        std::string configRow = *it;

                        std::string name       = configRow;
                        uint32_t countUniforms = 0U;
                        size_t pos             = configRow.find(':');
                        if (pos != std::string::npos) {
                            name          = configRow.substr(0, pos);
                            countUniforms = ct::uvariant(configRow.substr(pos + 1)).GetU();
                        }
                        if (!name.empty()) {
                            AddConfigName(name, countUniforms);
                        }
                    }

                    Sort();

                    res = true;
                }
            }
        }
    }

    return res;
}

bool ConfigSwitcherUnit::ReGenerateConfigSummaryFileByDirectoryScanning() {
    bool res = false;

    if (prShaderKey) {
        std::unordered_map<std::string, ConfInfos> finalFilesToSave;

        auto filePathName = prShaderKey->puKey;
        auto ps           = FileHelper::Instance()->ParsePathFileName(filePathName);
        if (ps.isOk) {
            auto fileName = ps.name;
            auto path     = prShaderKey->puPath;
            auto files    = inConfigSwitchertScanDirectory(path);
            for (auto file : files) {
                if (file.find(fileName) != std::string::npos) {
                    uint32_t countUniforms = 0U;
                    auto file_content      = FileHelper::Instance()->LoadFileToString(path + FileHelper::Instance()->puSlashType + file);
                    const auto& arr        = ct::splitStringToVector(file_content, '\n');
                    for (const auto& a : arr) {
                        if (a.find("UniformSection") == std::string::npos) {
                            ++countUniforms;
                        }
                    }

                    ct::replaceString(file, fileName + "_", "");
                    ct::replaceString(file, ".conf", "");
                    if (!file.empty()) {
                        ConfInfos ci;
                        ci.confName            = file;
                        ci.countUniforms       = countUniforms;
                        finalFilesToSave[file] = ci;
                    }
                }
            }

            // save summary file
            SaveUniformConfigSummaryFile(filePathName, finalFilesToSave);
        }
    }

    return res;
}

bool ConfigSwitcherUnit::Prepare(CodeTreePtr vCodeTree, ShaderKeyPtr vShaderKey) {
    bool res = false;

    prCodeTree  = vCodeTree;
    prShaderKey = vShaderKey;

    if (prCodeTree && prShaderKey) {
        const auto pPath = FileHelper::Instance()->ParsePathFileName(prShaderKey->puKey);
        if (pPath.isOk) {
            puShaderName = pPath.name;
        } else {
            puShaderName = prShaderKey->puKey;
        }

        res = LoadUniformConfigSummaryFile(prShaderKey->puKey);
    }

    return res;
}

bool ConfigSwitcherUnit::prNewConfigField(const float& /*aw*/) {
    bool change = false;

    if (prCodeTree && prShaderKey && prShowNewField) {
        ImGui::PushID("##new");

        const std::string configName = prNewConfigNameBuffer;
        const bool isOk              = (puUniformSettings.find(configName) == puUniformSettings.end()) && (!configName.empty());
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(isOk ? 1 : -1));
        ImGui::InputText("#ConfigName", prNewConfigNameBuffer, 1023);
        ImGui::PopStyleColor();
        if (isOk)  // non existant
        {
            if (ImGui::ContrastedButton("OK", nullptr)) {
                change = true;

                uint32_t countUniforms = (uint32_t)prShaderKey->puUniformsDataBase.size();
                AddConfigName(configName, countUniforms);
                Sort();

                if (prShaderKey->puIsInclude) {
                    if (prCodeTree->puIncludeUniformsList.find(prShaderKey->puKey) != prCodeTree->puIncludeUniformsList.end()) {
                        prCodeTree->SaveConfigIncludeFile(prShaderKey->puKey, &prCodeTree->puIncludeUniformsList[prShaderKey->puKey], configName);
                    }
                } else {
                    prShaderKey->SaveConfigShaderFile(prShaderKey->puKey, CONFIG_TYPE_Enum::CONFIG_TYPE_UNIFORM, configName);
                }

                SaveUniformConfigSummaryFile(prShaderKey->puKey);

                prShowNewField = false;
            }
            ImGui::SameLine();
        }

        if (ImGui::ContrastedButton("Cancel", nullptr)) {
            prShowNewField = false;
        }

        ImGui::PopID();
    }

    return change;
}

/*bool ConfigSwitcherUnit::prRenameConfigField(const float& aw)
{
    bool change = false;

    if (prCodeTree && prShaderKey) {
        if (!puConfigNameToRename.empty()) {
            ImGui::PushID("##rename");

            ImGui::Text("To Rename : %s", puConfigNameToRename.c_str());

            std::string configName = puConfigNameToRenameBuffer;
            const bool isOk        = (puUniformSettings.find(configName) == puUniformSettings.end()) && (!configName.empty());
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(isOk ? 1 : -1));
            ImGui::InputText("##New Name", puConfigNameToRenameBuffer, 1023);
            ImGui::PopStyleColor();
            if (isOk)  // non existant
            {
                if (ImGui::ContrastedButton("OK", nullptr)) {
                    configName = puConfigNameToRenameBuffer;

                    if (!configName.empty()) {
                        change = true;

                        const std::string oldConfigFile = CodeTree::GetConfigFileName(prShaderKey->puKey, puConfigNameToRename);
                        const std::string newConfigFile = CodeTree::GetConfigFileName(prShaderKey->puKey, configName);

                        if (!oldConfigFile.empty() && !newConfigFile.empty()) {
                            if (std::rename(oldConfigFile.c_str(), newConfigFile.c_str())) {
                                LogVarError("Can't rename %s to %s", oldConfigFile.c_str(), newConfigFile.c_str());
                            } else {
                                RemoveConfigName(puConfigNameToRename);
                                AddConfigName(configName);
                                Sort();
                            }

                            prShaderKey->puConfigSwitcherSelectedConfig = configName;

                            SaveUniformConfigSummaryFile(prShaderKey->puKey);

                            prShowRenameField = false;
                        }
                    }
                }
                ImGui::SameLine();
            }

            if (ImGui::ContrastedButton("Cancel", nullptr)) {
                prShowRenameField = false;
            }

            ImGui::PopID();
        }
    }

    return change;
}
*/

bool ConfigSwitcherUnit::prToggleButton(const char* vLabelOn, const char* vLabelOff, const char* vHelp, bool* vToggled, ImFont* imfont) {
    bool pressed = false;

    assert(vToggled);

    if (*vToggled) {
        ImVec4 bua = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
        // ImVec4 buh = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
        // ImVec4 bu = ImGui::GetStyleColorVec4(ImGuiCol_Button);
        ImVec4 te = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        ImGui::PushStyleColor(ImGuiCol_Button, te);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, te);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, te);
        ImGui::PushStyleColor(ImGuiCol_Text, bua);

        pressed = ImGui::ContrastedButton(vLabelOn, nullptr, imfont);

        ImGui::PopStyleColor(4);  //-V112
    } else {
        pressed = ImGui::ContrastedButton(vLabelOff, nullptr, imfont);
    }

    if (vHelp && ImGui::IsItemHovered()) {
        ImGui::SetTooltip(vHelp);
    }

    if (pressed) *vToggled = !*vToggled;

    return pressed;
}

RenderingModeEnum ConfigSwitcherUnit::GetRenderingMode() {
    return RenderingModeEnum::RENDERING_MODE_PICTURES;
}

std::string ConfigSwitcherUnit::GetRenderingFilePathNameForCurrentFrame() {
    return puRenderingFileName;
}

void ConfigSwitcherUnit::SelectConfigByName(const std::string& vName) {
    if (prResetNotConcernedUniforms) {
        prShaderKey->ResetUniformsToTheirDefaultValue();
    }

    if (prShaderKey->puIsInclude) {
        prCodeTree->LoadConfigIncludeFile(prShaderKey->puKey, CONFIG_TYPE_Enum::CONFIG_TYPE_UNIFORM, vName);
    } else {
        prShaderKey->LoadConfigShaderFile(prShaderKey->puKey, CONFIG_TYPE_Enum::CONFIG_TYPE_UNIFORM, vName);
    }

    prShaderKey->puConfigSwitcherSelectedConfig = vName;

    prCodeTree->ReScaleMouseUniforms(UniformHelper::FBOSize);
}

bool ConfigSwitcherUnit::DrawConfigSwitcher(const float& vWidth, const float& vHeight, ImFont* vImFontSymbol, bool vHideLabel) {
    bool change = false;

    // on desactive l'export
    puRendering = false;

    if (prCodeTree && prShaderKey && vImFontSymbol) {
        float posy = ImGui::GetCursorPosY();

        ImGui::PushID(prShaderKey.get());

        if (!vHideLabel) ImGui::FramedGroupText("%s", puShaderName.c_str());

        prToggleButton(ICON_NDPTB_HEXAGON_SLICE_6 "##ResetNCWidgetsON", ICON_NDPTB_HEXAGON_OUTLINE "##ResetNCWidgetsOFF", "Reset Not Concerned Widgets (Available but not saved in a config)", &prResetNotConcernedUniforms, vImFontSymbol);

        ImGui::SameLine();

        // TOOLBAR
        if (ImGui::ContrastedButton(ICON_NDPTB_LAYERS_PLUS "##NewConfig", "New", vImFontSymbol)) {
            change         = true;
            prShowNewField = true;
            ct::ResetBuffer(prNewConfigNameBuffer);
            ct::AppendToBuffer(prNewConfigNameBuffer, 1023, prShaderKey->puConfigSwitcherSelectedConfig);
        }

        if (!prShaderKey->puConfigSwitcherSelectedConfig.empty()) {
            ImGui::SameLine();

            if (ImGui::ContrastedButton(ICON_NDPTB_LAYERS "##SaveConfig", "Save", vImFontSymbol)) {
                change = true;

                // update uniform count
                auto it = searchForConfNameInContainer(prShaderKey->puConfigSwitcherSelectedConfig, puOrderedStrings);
                if (it != puOrderedStrings.end()) {
                    it->countUniforms = (uint32_t)prShaderKey->puUniformsDataBase.size();
                }

                if (prShaderKey->puIsInclude) {
                    if (prCodeTree->puIncludeUniformsList.find(prShaderKey->puKey) != prCodeTree->puIncludeUniformsList.end()) {
                        prCodeTree->SaveConfigIncludeFile(prShaderKey->puKey, &prCodeTree->puIncludeUniformsList[prShaderKey->puKey], prShaderKey->puConfigSwitcherSelectedConfig);
                    }
                } else {
                    prShaderKey->SaveConfigShaderFile(prShaderKey->puKey, CONFIG_TYPE_Enum::CONFIG_TYPE_UNIFORM, prShaderKey->puConfigSwitcherSelectedConfig);
                }

                SaveUniformConfigSummaryFile(prShaderKey->puKey);
            }

            ImGui::SameLine();

            if (ImGui::ContrastedButton(ICON_NDPTB_LAYERS_REMOVE "##DeleteCongig", "Delete", vImFontSymbol)) {
                change = true;

                RemoveConfigName(prShaderKey->puConfigSwitcherSelectedConfig);
                Sort();
                SaveUniformConfigSummaryFile(prShaderKey->puKey);
                prShaderKey->puConfigSwitcherSelectedConfig.clear();
            }
        }

        // FIELDS
        // prRenameConfigField(vWidth);
        prNewConfigField(vWidth);

        // LIST
        float dispStart = 0.0f;
        if (ImGui::Selectable(ct::toStr("Current : %s", prShaderKey->puConfigSwitcherSelectedConfig.c_str()).c_str(), false)) {
            auto it = searchForConfNameInContainer(prShaderKey->puConfigSwitcherSelectedConfig, puOrderedStrings);
            if (it != puOrderedStrings.end()) {
                dispStart = (it - puOrderedStrings.begin()) * ImGui::GetTextLineHeightWithSpacing();
            }
        }

        static ImGuiTableFlags flags =        //
            ImGuiTableFlags_SizingFixedFit |  //
            ImGuiTableFlags_RowBg |           //
            ImGuiTableFlags_Hideable |        //
            ImGuiTableFlags_ScrollY |         //
            ImGuiTableFlags_NoHostExtendY;    //
        if (ImGui::BeginTable("##configtable", 3, flags, ImVec2(vWidth, -1.0f))) {
            ImGui::TableSetupScrollFreeze(0, 1);  // Make header always visible
            ImGui::TableSetupColumn("save", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("conf", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("count", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            prImGuiListClipper.Begin((int)puOrderedStrings.size());
            while (prImGuiListClipper.Step()) {
                for (int i = prImGuiListClipper.DisplayStart; i < prImGuiListClipper.DisplayEnd; i++) {
                    if (i < 0) {
                        continue;
                    }

                    ImGui::TableNextRow();

                    const auto& conf = puOrderedStrings[i];

                    if (ImGui::TableSetColumnIndex(0)) {
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
                        if (ImGui::ContrastedButton(ICON_NDPTB_FOLDER_IMAGE "##Savepicture", "Save Picture", vImFontSymbol)) {
                            if (!puRenderingPath.empty()) {
                                puRendering         = true;
                            } else {
                                ImGuiFileDialog::Instance()->OpenDialog("ConfigSwitcherExportPath", "Choose a path for export all your configs",  //
                                                                        nullptr, puRenderingPath, 1, (IGFD::UserDatas)this, ImGuiFileDialogFlags_Modal);          //
                            }
                            puRenderingFileName = conf.confName;
                            SelectConfigByName(conf.confName);
                        }
                        ImGui::PopStyleVar();
                    }
                    if (ImGui::TableSetColumnIndex(1)) {
                        if (ImGui::Selectable(conf.confName.c_str(), prShaderKey->puConfigSwitcherSelectedConfig == conf.confName)) {
                            change = true;

                            SelectConfigByName(conf.confName);
                        }
                    }
                    if (ImGui::TableSetColumnIndex(2)) {
                        ImGui::Text("%u", conf.countUniforms);
                    }
                }
            }

            prImGuiListClipper.End();

            if (dispStart > 0.0f) {
                const float rangeY = (prImGuiListClipper.DisplayEnd - prImGuiListClipper.DisplayStart) * ImGui::GetTextLineHeightWithSpacing();
                dispStart -= rangeY * 0.5f;
                ImGui::SetScrollY(dispStart);
            }

            ImGui::EndTable();
        }

        ImGui::PopID();
    }

    return change;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

std::string GetUnifiedSwitcherLayoutEnumToString(const UnifiedSwitcherLayoutEnum& vUnifiedSwitcherLayoutEnum) {
    switch (vUnifiedSwitcherLayoutEnum) {
        case UnifiedSwitcherLayoutEnum::HORIZONTAL_LAYOUT: return "Horizontal";
        case UnifiedSwitcherLayoutEnum::VERTICAL_LAYOUT: return "Vertical";
        default: break;
    }
    return "";
}

UnifiedSwitcherLayoutEnum GetStringToUnifiedSwitcherLayoutEnum(const std::string& vString) {
    if (vString == "Horizontal") {
        return UnifiedSwitcherLayoutEnum::HORIZONTAL_LAYOUT;
    }
    return UnifiedSwitcherLayoutEnum::VERTICAL_LAYOUT;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

ShaderKeyConfigSwitcherUnified::ShaderKeyConfigSwitcherUnified() {
}

ShaderKeyConfigSwitcherUnified::~ShaderKeyConfigSwitcherUnified() {
}

void ShaderKeyConfigSwitcherUnified::Activate(bool vFlag, CodeTreePtr vCodeTree, RenderPackWeak vRenderPack, bool vForce) {
    if (vFlag)  // si nouvelle activation et n'etait pas active
    {
        if (!prActivated || vForce) {
            Prepare(vCodeTree, vRenderPack);
        }
    }

    prActivated = vFlag;
}

bool ShaderKeyConfigSwitcherUnified::IsActivated() {
    return prActivated;
}

void ShaderKeyConfigSwitcherUnified::Prepare(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack) {
    auto rpPtr = vRenderPack.lock();
    if (vCodeTree && rpPtr) {
        prSwitchers.clear();
        prOneConfigExistAtleast = false;

        auto mainKeyPtr = rpPtr->GetShaderKey();
        if (mainKeyPtr) {
            ConfigSwitcherUnit csu;
            if (csu.Prepare(vCodeTree, mainKeyPtr)) {
                prOneConfigExistAtleast = true;
            }

            // set rendering path
            csu.SetRenderingPath(puRenderingPath);

            // pour le moment ajout toujours dans prSwitchers,
            // car il faut pouvoir montrer les widget de creation meme si c'est vide
            // sinon comment on fait pour initier des nouvelles conf sans les widgets ?
            prSwitchers.push_back(csu);
        }

        for (auto bufWeak : rpPtr->puBuffers) {
            auto bufPtr = bufWeak.lock();
            if (bufPtr) {
                auto subKey = bufPtr->GetShaderKey();
                if (subKey) {
                    ConfigSwitcherUnit csu;
                    if (csu.Prepare(vCodeTree, subKey)) prOneConfigExistAtleast = true;

                    // pour le moment ajout toujours dans prSwitchers,
                    // car il faut pouvoir montrer les widget de creation meme si c'est vide
                    // sinon comment on fait pour initier des nouvelles conf sans les widgets ?
                    prSwitchers.push_back(csu);
                }
            }
        }

        // will be applied only if the var is selected
        HideEmptyPanes();
    }
}

bool ShaderKeyConfigSwitcherUnified::IsRendering() {
    for (auto csu : prSwitchers) {
        if (csu.IsRendering()) {
            return true;
        }
    }
    return false;
}

RenderingModeEnum ShaderKeyConfigSwitcherUnified::GetRenderingMode() {
    return RenderingModeEnum::RENDERING_MODE_PICTURES;
}

std::string ShaderKeyConfigSwitcherUnified::GetRenderingFilePathNameForCurrentFrame() {
    for (auto csu : prSwitchers) {
        if (csu.IsRendering()) {
            std::string file_path_name = puRenderingPath + "/" + csu.GetRenderingFilePathNameForCurrentFrame() + ".png";
            return FileHelper::Instance()->CorrectSlashTypeForFilePathName(file_path_name);
        }
    }
    return "";
}

void ShaderKeyConfigSwitcherUnified::ReGenerateConfigSummaryFilesByDirectoryScanning(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack) {
    // regen
    for (auto csu : prSwitchers) {
        csu.ReGenerateConfigSummaryFileByDirectoryScanning();
    }

    // reouverture
    Prepare(vCodeTree, vRenderPack);
}

bool ShaderKeyConfigSwitcherUnified::DrawConfigSwitcher(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack, ImFont* vImFontSymbol) {
    bool change = false;

    if (vCodeTree && !vRenderPack.expired()) {
        // if (prOneConfigExistAtleast) // meme si il yen a plus on doit pouoir le regenerer quand meme
        {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Export")) {
                    if (ImGui::MenuItem("Set export path")) {
                        ImGuiFileDialog::Instance()->OpenDialog("ConfigSwitcherExportPath", "Choose a path for export all your configs",  //
                                                                nullptr, puRenderingPath, 1, nullptr, ImGuiFileDialogFlags_Modal);        //
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("General")) {
                    if (ImGui::MenuItem("Scan Dir and Re Generate Config Summary Files")) {
                        ReGenerateConfigSummaryFilesByDirectoryScanning(vCodeTree, vRenderPack);
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Layouts")) {
                    if (ImGui::MenuItem("Horizontal Table", nullptr, prUnifiedSwitcherLayoutEnum == UnifiedSwitcherLayoutEnum::HORIZONTAL_LAYOUT)) prUnifiedSwitcherLayoutEnum = UnifiedSwitcherLayoutEnum::HORIZONTAL_LAYOUT;
                    if (ImGui::MenuItem("Vertical Collapsing Headers", nullptr, prUnifiedSwitcherLayoutEnum == UnifiedSwitcherLayoutEnum::VERTICAL_LAYOUT)) prUnifiedSwitcherLayoutEnum = UnifiedSwitcherLayoutEnum::VERTICAL_LAYOUT;

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Show/Hide Panes")) {
                    if (ImGui::MenuItem("Show All Switchers")) {
                        ShowAllPanes();
                    }

                    if (ImGui::MenuItem("Hide All Switchers")) {
                        HideAllPanes();
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Hide Empty Switchers" /*, nullptr, &prHideEmptyPanes */)) {
                        HideEmptyPanes();
                    }

                    ImGui::Separator();

                    for (auto& sw : prSwitchers) {
                        ImGui::MenuItem(sw.puShaderName.c_str(), nullptr, &sw.puIsVisible);
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
        }

        if (prUnifiedSwitcherLayoutEnum == UnifiedSwitcherLayoutEnum::HORIZONTAL_LAYOUT) {
            const ImVec2 aw = ImGui::GetContentRegionAvail() - ImGui::GetStyle().FramePadding;

            int count = 0;
            for (auto& sw : prSwitchers) {
                if (sw.puIsVisible) count++;
            }

            if (count) {
                static ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedFit | ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide;
                if (ImGui::BeginTable("##ConfigSwitcherPanes", count, tableFlags, aw)) {
                    ImGui::TableSetupScrollFreeze(0, 1);  // Make header always visible

                    int idx = 0;
                    for (const auto& sw : prSwitchers) {
                        if (sw.puIsVisible) {
                            ImGui::TableSetupColumn(sw.puShaderName.c_str(), ImGuiTableColumnFlags_WidthStretch, -1, idx++);
                        }
                    }

                    ImGui::TableNextRow();

                    idx = 0;
                    for (auto& sw : prSwitchers) {
                        if (sw.puIsVisible) {
                            if (ImGui::TableSetColumnIndex(idx++)) {
                                change |= sw.DrawConfigSwitcher(-1.0f, -1.0f, vImFontSymbol);
                            }
                        }
                    }

                    ImGui::EndTable();
                }
            }
        } else if (prUnifiedSwitcherLayoutEnum == UnifiedSwitcherLayoutEnum::VERTICAL_LAYOUT) {
            ImGuiContext& g          = *GImGui;
            const ImGuiStyle& style  = g.Style;
            const ImVec2 label_size  = ImGui::CalcTextSize("TOTO");
            const float frame_height = ImMax(g.FontSize, label_size.y) + style.FramePadding.y * 2 + style.ItemSpacing.y;
            float listBoxHeight      = ImGui::GetContentRegionAvail().y;
            int count                = 0;
            for (auto& sw : prSwitchers) {
                if (sw.puIsVisible) {
                    listBoxHeight -= frame_height;

                    if (sw.puIsOpened) {
                        count++;
                    }
                }
            }

            if (count) {
                listBoxHeight += style.ItemSpacing.y;
                listBoxHeight /= (float)count;
            }

            for (auto& sw : prSwitchers) {
                if (sw.puIsVisible) {
                    sw.puIsOpened = ImGui::CollapsingHeader(sw.puShaderName.c_str());
                    if (sw.puIsOpened) {
                        change |= sw.DrawConfigSwitcher(-1.0f, listBoxHeight, vImFontSymbol, true);
                    }
                }
            }
        }
    }

    return change;
}

void ShaderKeyConfigSwitcherUnified::ShowAllPanes() {
    for (auto& sw : prSwitchers) {
        sw.puIsVisible = true;
    }
}

void ShaderKeyConfigSwitcherUnified::HideAllPanes() {
    for (auto& sw : prSwitchers) {
        sw.puIsVisible = false;
    }
}

void ShaderKeyConfigSwitcherUnified::HideEmptyPanes() {
    for (auto& sw : prSwitchers) {
        if (sw.IsEmpty()) {
            sw.puIsVisible = false;
        }
    }
}

void ShaderKeyConfigSwitcherUnified::DrawDialog() {
    if (ImGuiFileDialog::Instance()->Display("ConfigSwitcherExportPath", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            puRenderingPath = ImGuiFileDialog::Instance()->GetCurrentPath();
            puRenderingMode = RenderingModeEnum::RENDERING_MODE_PICTURES;
            if (ImGuiFileDialog::Instance()->GetUserDatas()) {
                auto* config_switcher_unit_ptr = (ConfigSwitcherUnit*)(ImGuiFileDialog::Instance()->GetUserDatas());
                config_switcher_unit_ptr->SetRenderingPath(puRenderingPath);
                if (!config_switcher_unit_ptr->GetRenderingFilePathNameForCurrentFrame().empty()) {
                    config_switcher_unit_ptr->UseRendering();
                }
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

//////////////// CONFIGURATION XML //////////////////////////////////

std::string ShaderKeyConfigSwitcherUnified::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    std::string str;

    str += vOffset + "<SwitcherUnified>\n";
    str += vOffset + "\t<layout>" + GetUnifiedSwitcherLayoutEnumToString(prUnifiedSwitcherLayoutEnum) + "</layout>\n";
    str += vOffset + "\t<export>" + puRenderingPath + "</export>\n";
    str += vOffset + "</SwitcherUnified>\n";

    return str;
}

bool ShaderKeyConfigSwitcherUnified::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    // The value of this child identifies the name of this element
    std::string strName       = "";
    std::string strValue      = "";
    std::string strParentName = "";

    strName = vElem->Value();
    if (vElem->GetText()) strValue = vElem->GetText();
    if (vParent != nullptr) strParentName = vParent->Value();

    if (strParentName == "SwitcherUnified") {
        if (strName == "layout") {
            prUnifiedSwitcherLayoutEnum = GetStringToUnifiedSwitcherLayoutEnum(strValue);
        } else if (strName == "export") {
            puRenderingPath = strValue;
        }
    }

    return true;
}
