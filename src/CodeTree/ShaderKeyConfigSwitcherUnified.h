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

#pragma once

#include <string>
#include <set>
#include <vector>
#include <memory>
#include <unordered_map>
#include <ImGuiPack.h>
#include <ctools/ConfigAbstract.h>
#include <Headers/RenderPackHeaders.h>
#include <Interfaces/RenderingInterface.h>

struct ConfInfos {
    std::string confName;
    uint32_t countUniforms = 0U;
};

class CodeTree;
class ShaderKey;
class ConfigSwitcherUnit : public RenderingInterface {
private:
    std::string puConfigNameToRename;
    char puConfigNameToRenameBuffer[1024] = "\0";
    std::unordered_map<std::string, ConfInfos> puUniformSettings;
    std::vector<ConfInfos> puOrderedStrings;  // will be sorted with alpha num
    CodeTreePtr prCodeTree = nullptr;
    ShaderKeyPtr prShaderKey = nullptr;
    bool prShowRenameField = false;
    bool prShowNewField = false;
    ImGuiListClipper prImGuiListClipper;
    // this ser=ttings will control a spoecifi mode
    // when all uniforms who are not in the preset to be loaded will be rested to their default value @davidar
    bool prResetNotConcernedUniforms = false;
    char prNewConfigNameBuffer[1024] = "";

public:
    std::string puShaderName;
    bool puIsVisible = true;
    bool puIsOpened = false;  // in case of callapsing header

public:
    bool Prepare(CodeTreePtr vCodeTree = nullptr, ShaderKeyPtr vShaderKey = nullptr);

    bool IsEmpty();
    void Clear(const std::string& vKey);
    void Sort();
    void AddConfigName(const std::string& vName, const uint32_t& vCountUniforms);
    void RemoveConfigName(const std::string& vName);
    void SelectConfigByName(const std::string& vName);
    void SaveUniformConfigSummaryFile(const std::string& vKey);
    void SaveUniformConfigSummaryFile(const std::string& vKey, const std::unordered_map<std::string, ConfInfos>& vFilesToSave);
    bool LoadUniformConfigSummaryFile(const std::string& vKey);
    bool DrawConfigSwitcher(const float& vWidth, const float& vHeight, ImFont* vImFontSymbol, bool vHideLabel = false);
    bool ReGenerateConfigSummaryFileByDirectoryScanning();

public:  // Rendering flag
    RenderingModeEnum GetRenderingMode() override;
    std::string GetRenderingFilePathNameForCurrentFrame() override;

private:
    bool prNewConfigField(const float& aw);
    // bool prRenameConfigField(const float& aw);
    bool prToggleButton(const char* vLabelOn, const char* vLabelOff, const char* vHelp, bool* vToggled, ImFont* imfont);
};

enum class UnifiedSwitcherLayoutEnum { HORIZONTAL_LAYOUT, VERTICAL_LAYOUT };

static std::string GetUnifiedSwitcherLayoutEnumToString(const UnifiedSwitcherLayoutEnum& vUnifiedSwitcherLayoutEnum);
static UnifiedSwitcherLayoutEnum GetStringToUnifiedSwitcherLayoutEnum(const std::string& vString);

class ShaderKeyConfigSwitcherUnified : public conf::ConfigAbstract, public RenderingInterface {
private:
    std::vector<ConfigSwitcherUnit> prSwitchers;
    bool prActivated = false;
    RenderPackWeak prRefRenderPack;
    bool prOneConfigExistAtleast = false;
    UnifiedSwitcherLayoutEnum prUnifiedSwitcherLayoutEnum = UnifiedSwitcherLayoutEnum::HORIZONTAL_LAYOUT;
    bool prHideEmptyPanes = false;

public:
    ShaderKeyConfigSwitcherUnified();
    ~ShaderKeyConfigSwitcherUnified();

    void Activate(bool vFlag, CodeTreePtr vCodeTree = nullptr, RenderPackWeak vRenderPack = RenderPackWeak(), bool vForce = false);
    bool IsActivated();

private:
    void Prepare(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack);
    void ReGenerateConfigSummaryFilesByDirectoryScanning(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack);
    void ShowAllPanes();
    void HideAllPanes();
    void HideEmptyPanes();

public:  // ImGui
    void DrawDialog();
    bool DrawConfigSwitcher(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack, ImFont* vImFontSymbol);

public:  // Rendering flag
    bool IsRendering() override;
    RenderingModeEnum GetRenderingMode() override;
    std::string GetRenderingFilePathNameForCurrentFrame() override;

public:
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

public:
    static ShaderKeyConfigSwitcherUnified* Instance() {
        static ShaderKeyConfigSwitcherUnified _instance;
        return &_instance;
    }
};
