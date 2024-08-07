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

#include "CodeTree.h"

#include <ImGuiPack.h>

#include <Renderer/RenderPack.h>
#include <Gui/CustomGuiWidgets.h>
#include <Res/CustomFont.h>
#include <Res/CustomFont2.h>
#include <ctools/FileHelper.h>
#include <Manager/AssetManager.h>
#include <Systems/CameraSystem.h>
#include <Uniforms/UniformVariant.h>
#include <CodeTree/ShaderKey.h>
#include <Systems/GizmoSystem.h>
#include <Systems/GamePadSystem.h>
#include <Systems/MidiSystem.h>
#include <Systems/SoundSystem.h>
#include <VR/Backend/VRBackend.h>
#include <Texture/Texture2D.h>
#include <Texture/TextureCube.h>
#include <Texture/Texture3D.h>
#include <Texture/TextureSound.h>
#include <Systems/TimeLineSystem.h>
#include <Systems/FilesTrackerSystem.h>
#include <ctools/Logger.h>
#include <Uniforms/UniformHelper.h>
#include <CodeTree/Parsing/UniformParsing.h>
#include <CodeTree/ShaderKeyConfigSwitcherUnified.h>
#include <Uniforms/UniformWidgets.h>

using namespace std::placeholders;

// #define VERBOSE

#ifdef _DEBUG
// #define PARSED_UNIFORM_VERBOSE
#endif

/////////////////////////////////////////////////////////////////////////////////////////
//// STATIC /////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

UniformVariantPtr CodeTree::puPictureChooseUniform = nullptr;
UniformVariantPtr CodeTree::puPicturePopupUniform = nullptr;
static int _TextureWrap = 0;
static int _TextureFilter = 0;

UniformVariantPtr CodeTree::puBufferChooseUniform = nullptr;
UniformVariantPtr CodeTree::puBufferPopupUniform = nullptr;
static int _BufferWrap = 0;
static int _BufferFilter = 0;

UniformVariantPtr CodeTree::puSoundChooseUniform = nullptr;
UniformVariantPtr CodeTree::puSoundPopupUniform = nullptr;

ShaderKeyPtr CodeTree::puShaderKeyToEditPopup = nullptr;
ShaderKeyPtr CodeTree::puShaderKeyWhereCreateUniformsConfig = nullptr;
ShaderKeyPtr CodeTree::puShaderKeyWhereRenameUniformsConfig = nullptr;
ShaderKeyPtr CodeTree::puShaderKeyUniformsConfigToSwitch = nullptr;

UniformVariantPtr CodeTree::puCurrentGizmo = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

CodeTree::CodeTree() {
    puDontSaveConfigFiles = false;
    puShowUnUsedUniforms = false;
    puShowCustomUniforms = false;

    InitTextureChooseDialogWithUniform(nullptr);
    TexturePopupInit();
}

CodeTree::~CodeTree() {
    Clear();
}

void CodeTree::Clear() {
    ClearIncludes();
    ClearShaders();
    DestroyIncludeFileList();
    AssetManager::Instance()->Clear();
}

void CodeTree::Clear(const std::unordered_set<std::string>& vExceptions) {
    ClearIncludes(vExceptions);
    ClearShaders(vExceptions);
    DestroyIncludeFileList();
    AssetManager::Instance()->Clear();
}

void CodeTree::ClearShaders() {
    CloseUniformsConfigSwitcher();
    puShaderKeys.clear();
}

void CodeTree::ClearShaders(const std::unordered_set<std::string>& vExceptions) {
    std::list<std::string> keysToErase;

    for (auto it = puShaderKeys.begin(); it != puShaderKeys.end(); ++it) {
        if (vExceptions.find(ct::toLower(it->first)) == vExceptions.end())  // non found
        {
            keysToErase.emplace_back(it->first);
        }
    }

    for (auto it = keysToErase.begin(); it != keysToErase.end(); ++it) {
        if (!puDontSaveConfigFiles) {
            puShaderKeys[*it]->SaveRenderPackConfig(CONFIG_TYPE_Enum::CONFIG_TYPE_ALL);
        }
        puShaderKeys.erase(*it);
    }

    keysToErase.clear();
}

void CodeTree::ClearIncludes() {
    CloseUniformsConfigSwitcher();
    puIncludeKeys.clear();
}

void CodeTree::ClearIncludes(const std::unordered_set<std::string>& vExceptions) {
    CloseUniformsConfigSwitcher();

    std::list<std::string> keysToErase;

    SaveConfigIncludeFiles();

    for (auto it = puIncludeKeys.begin(); it != puIncludeKeys.end(); ++it) {
        if (vExceptions.find(ct::toLower(it->first)) == vExceptions.end())  // non found
        {
            keysToErase.emplace_back(it->first);
        }
    }

    for (auto it = keysToErase.begin(); it != keysToErase.end(); ++it) {
        if (!puDontSaveConfigFiles) {
            puIncludeKeys[*it]->SaveRenderPackConfig(CONFIG_TYPE_Enum::CONFIG_TYPE_ALL);
        }
        puIncludeKeys.erase(*it);
    }

    keysToErase.clear();
}

void CodeTree::RemoveKey(const std::string& vKey) {
    ShaderKeyPtr key = GetShaderKey(vKey);
    if (key) {
        CloseUniformsConfigSwitcher(key);
        puShaderKeys.erase(vKey);
    }
    key = GetIncludeKey(vKey);
    if (key) {
        CloseUniformsConfigSwitcher(key);
        puIncludeKeys.erase(vKey);
    }
}

ShaderKeyPtr CodeTree::LoadFromFile(const std::string& vFilePathName, KEY_TYPE_Enum vFileType) {
    const std::string rel = FileHelper::Instance()->GetPathRelativeToApp(vFilePathName);

    if (vFileType == KEY_TYPE_Enum::KEY_TYPE_SHADER)
        return AddOrUpdateFromFileAndGetKey(rel, true, true, false);
    else if (vFileType == KEY_TYPE_Enum::KEY_TYPE_INCLUDE)  //-V547
        return AddOrUpdateFromFileAndGetKey(rel, true, true, true);

    return nullptr;
}

ShaderKeyPtr CodeTree::LoadFromString(const std::string& vKey,
                                      const std::string& vFileString,
                                      const std::string& vOriginalFilePathName,  // like the oriignal file from where the current code (vFileString) come form
                                      const std::string& vInFileBufferName,      // like MAIN or CHILD_A, CHILD_B
                                      KEY_TYPE_Enum vFileType) {
    if (vFileType == KEY_TYPE_Enum::KEY_TYPE_SHADER)
        return AddOrUpdateFromStringAndGetKey(vKey, vFileString, vOriginalFilePathName, vInFileBufferName, true, true, false);
    else if (vFileType == KEY_TYPE_Enum::KEY_TYPE_INCLUDE)  //-V547
        return AddOrUpdateFromStringAndGetKey(vKey, vFileString, vOriginalFilePathName, vInFileBufferName, true, true, true);

    return nullptr;
}

ShaderKeyPtr CodeTree::AddKey(const std::string& vKey, bool vIsInclude) {
    ShaderKeyPtr key = nullptr;

    if (vIsInclude)
        key = AddIncludeKey(vKey);
    else
        key = AddShaderKey(vKey);

    return key;
}

ShaderKeyPtr CodeTree::AddShaderKey(const std::string& vKey, bool vFilebased) {
    if (puShaderKeys.find(vKey) == puShaderKeys.end())  // non trouve
    {
        ShaderKeyPtr key = ShaderKey::Create();
        key->puParentCodeTree = m_This;
        key->puKeyType = KEY_TYPE_Enum::KEY_TYPE_SHADER;

        if (!vKey.empty() && vFilebased)  // file based
        {
            const PathStruct p = FileHelper::Instance()->ParsePathFileName(vKey);
            if (p.isOk) {
                key->puKey = vKey;
                // key.puFilePathName = vKey;
                key->Setpath(p.path);

                AddPathToTrack(p.path, false);

                puShaderKeys[key->puKey] = key;
                return puShaderKeys[key->puKey];
            }
        } else  // string based
        {
            key->puKey = vKey;
            puShaderKeys[vKey] = key;
            return puShaderKeys[vKey];
        }

        key->puSyntaxErrors.puParentKeyName = key->puKey;
    } else {
        return puShaderKeys[vKey];
    }

    return nullptr;
}

ShaderKeyPtr CodeTree::GetShaderKey(const std::string& vKey) {
    if (puShaderKeys.find(vKey) != puShaderKeys.end())  // trouve
    {
        return puShaderKeys[vKey];
    }
    return nullptr;
}

ShaderKeyPtr CodeTree::AddIncludeKey(const std::string& vKey, bool vFilebased) {
    if (puIncludeKeys.find(vKey) == puIncludeKeys.end())  // non found
    {
        ShaderKeyPtr key = ShaderKey::Create();
        key->puParentCodeTree = m_This;
        key->puKeyType = KEY_TYPE_Enum::KEY_TYPE_INCLUDE;

        key->puIsInclude = true;

        if (!vKey.empty() && vFilebased)  // file based
        {
            const PathStruct p = FileHelper::Instance()->ParsePathFileName(vKey);
            if (p.isOk) {
                key->puKey = vKey;
                // key.puFilePathName = vKey;
                key->Setpath(p.path);

                AddPathToTrack(p.path, false);

                puIncludeKeys[key->puKey] = key;
                return puIncludeKeys[key->puKey];
            }
        } else  // string based
        {
            key->puKey = vKey;
            puIncludeKeys[vKey] = key;
            return puIncludeKeys[vKey];
        }
    } else {
        return puIncludeKeys[vKey];
    }
    return nullptr;
}

ShaderKeyPtr CodeTree::GetIncludeKey(const std::string& vKey) {
    if (puIncludeKeys.find(vKey) != puIncludeKeys.end())  // found
    {
        return puIncludeKeys[vKey];
    }

    return nullptr;
}

ShaderKeyPtr CodeTree::GetKey(const std::string& vKey) {
    ShaderKeyPtr key = nullptr;

    key = GetShaderKey(vKey);
    if (!key)
        key = GetIncludeKey(vKey);

    return key;
}

ShaderKeyPtr CodeTree::AddOrUpdateFromStringAndGetKey(
    const std::string& vKey,
    const std::string& vFileString,
    const std::string& vOriginalFilePathName,  // like the oriignal file from where the current code (vFileString) come form
    const std::string& vInFileBufferName,      // like MAIN or CHILD_A, CHILD_B
    bool vResetConfigs,
    bool vResetReplaceCodes,
    bool vIsInclude) {
    ShaderKeyPtr key = nullptr;

    AddOrUpdateFromString(vKey, vFileString, vOriginalFilePathName, vInFileBufferName, vResetConfigs, vResetReplaceCodes, vIsInclude);

    if (vIsInclude) {
        key = GetIncludeKey(vKey);
    } else {
        key = GetShaderKey(vKey);
    }

    return key;
}

ShaderKeyPtr CodeTree::AddOrUpdateFromFileAndGetKey(const std::string& vFilePathName, bool vResetConfigs, bool vResetReplaceCodes, bool vIsInclude) {
    ShaderKeyPtr key = nullptr;

    AddOrUpdateFromFile(vFilePathName, vResetConfigs, vResetReplaceCodes, vIsInclude);

    if (vIsInclude) {
        key = GetIncludeKey(vFilePathName);
    } else {
        key = GetShaderKey(vFilePathName);
    }

    return key;
}

// renvoie la key que s'il y a une maj
ShaderKeyPtr CodeTree::AddOrUpdateFromFileAndGetKeyIfModified(const std::string& vFilePathName, bool vResetConfigs, bool vResetReplaceCodes, bool vIsInclude) {
    ShaderKeyPtr key = nullptr;

    if (AddOrUpdateFromFile(vFilePathName, vResetConfigs, vResetReplaceCodes, vIsInclude)) {
        if (vIsInclude) {
            key = GetIncludeKey(vFilePathName);
        } else {
            key = GetShaderKey(vFilePathName);
        }
    }

    return key;
}

bool CodeTree::AddOrUpdateFromFile(ShaderKeyPtr vKey, bool vResetConfigs, bool vResetReplaceCodes, bool vIsInclude) {
    bool res = false;

    res = AddOrUpdateFromFile(vKey->puKey, vResetConfigs, vResetReplaceCodes, vIsInclude);

    return res;
}

bool CodeTree::AddOrUpdateFromString(const std::string& vKey,
                                     const std::string& vFileString,
                                     const std::string& vOriginalFilePathName,  // like the oriignal file from where the current code (vFileString) come form
                                     const std::string& vInFileBufferName,      // like MAIN or CHILD_A, CHILD_B
                                     bool vResetConfigs,
                                     bool /*vResetReplaceCodes*/,
                                     bool vIsInclude) {
    bool res = false;

    ShaderKeyPtr key = nullptr;

    if (vIsInclude) {
        key = GetIncludeKey(vKey);
    } else {
        key = GetShaderKey(vKey);
    }

    // laod code file
    res = AddOrUpdateKeyWithCode(key, vKey, vFileString, vOriginalFilePathName, vInFileBufferName, vResetConfigs, vIsInclude, true);

    return res;
}

bool CodeTree::AddOrUpdateFromFile(const std::string& vFilePathName, bool vResetConfigs, bool /*vResetReplaceCodes*/, bool vIsInclude) {
    bool res = false;

    ShaderKeyPtr key = nullptr;

    if (vIsInclude) {
        key = GetIncludeKey(vFilePathName);
    } else {
        key = GetShaderKey(vFilePathName);
    }

    const auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
    if (ps.isOk) {
        FilesTrackerSystem::Instance()->addWatch(ps.path);
    }

    // load code file
    const std::string code = FileHelper::Instance()->LoadFileToString(vFilePathName, true);
    std::string mainStr;
    const size_t bufferPos = code.find("BUFFER(MAIN)");
    if (bufferPos != std::string::npos) {
        mainStr = "MAIN";
    }
    res = AddOrUpdateKeyWithCode(key, vFilePathName, code, "", mainStr, vResetConfigs, vIsInclude, false);

    return res;
}

std::shared_ptr<SectionCode> CodeTree::GetSectionCode(const std::string& vKey) {
    std::shared_ptr<SectionCode> res = nullptr;

    ShaderKeyPtr key = GetShaderKey(vKey);
    if (!key)
        key = GetIncludeKey(vKey);

    if (key) {
        res = key->puMainSection;
    }

    return res;
}

ShaderParsedStruct CodeTree::GetFullShader(const std::string& vKey,
                                           const std::string& vInFileBufferName,
                                           const std::string& /*vSectionName*/,
                                           const std::string& /*vConfigName*/) {
    ShaderParsedStruct res;

    ShaderKeyPtr key = GetShaderKey(vKey);
    if (!key)
        key = GetIncludeKey(vKey);

    if (key) {
        res = key->GetFullShader(vInFileBufferName, true, true);
    }

    return res;
}

std::unordered_map<std::string, UniformVariantPtr>* CodeTree::GetUniformsFromIncludeFileName(const std::string& vIncludeFileName) {
    ShaderKeyPtr key = GetIncludeKey(vIncludeFileName);
    if (key != nullptr) {
        return &key->puUniformsDataBase;
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////

bool CodeTree::DrawImGuiUniformWidget(ShaderKeyPtr vKey, float vFirstColumnWidth, RenderPackWeak vRenderPack, bool vShowUnUsed, bool vShowCustom) {
    bool change = false;

    if (vKey != nullptr) {
        char nodeTitle[256] = "";
        const float PaneWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2.0f;
        for (auto itSec = vKey->puUniformSectionDataBase.begin(); itSec != vKey->puUniformSectionDataBase.end(); ++itSec) {
            std::string section = itSec->first;
            auto uniforms = &itSec->second;

            if (section != "hidden") {
                static bool _existingUniforms = false;
                _existingUniforms = false;

                // if sone search, we hide section widget
                for (auto itLst = uniforms->begin(); itLst != uniforms->end(); ++itLst) {
                    UniformVariantPtr v = *itLst;

                    if (!vKey->puShaderGlobalSettings.searchUniformName.empty()) {
                        if (v->nameForSearch.find(vKey->puShaderGlobalSettings.searchUniformName) == std::string::npos) {
                            continue;
                        }
                    }

                    // on a remplie ce map avec tout les noms d'uniforms des includes
                    // donc pas besoin d'ietrer chaque include, juste de tester si le nom existe pas dans cette liste
                    // s'il existe pas alors on l'affiche car ici on affiche que les uniforms des sahder or includes
                    // if (vKey->puUsedUniformsInCode.find(v->name) == vKey->puUsedUniformsInCode.end()) // non trouve
                    if (puIncludeUniformNames.find(v->name) == puIncludeUniformNames.end()) {  // non trouve
                        if ((!vShowUnUsed && v->loc > -1) || vShowUnUsed) {
                            _existingUniforms = true;  // permet d'eviter d'afficher une section mais pas un seul uniform de cette section
                            break;
                        }
                    }
                }

                if (_existingUniforms) {
                    snprintf(nodeTitle, 255, "%s", section.c_str());
                    // pas besoin  de chercher si ca existe avant de prendre, ca va auto initialiser
                    // par defautl ce sera true (true vaut n'importe quoi et false vaut 0). ca tombe bien c'est ce qu'on veut
                    bool* opened = &vKey->puUniformSectionOpened[section];  // pas besoin  de chercher si ca existe avant de prendre, ca va auto initialiser
                    if (ImGui::CollapsingHeader_SmallHeight(nodeTitle, 0.8f, -1, true, opened)) {
                        for (auto itLst = uniforms->begin(); itLst != uniforms->end(); ++itLst) {
                            UniformVariantPtr v = *itLst;

                            if (!vKey->puShaderGlobalSettings.searchUniformName.empty()) {
                                if (v->nameForSearch.find(vKey->puShaderGlobalSettings.searchUniformName) == std::string::npos) {
                                    continue;
                                }
                            }

                            // on a remplie ce map avec tout les noms d'uniforms des includes
                            // donc pas besoin d'ietrer chaque include, juste de tester si le nom existe pas dans cette liste
                            // s'il existe pas alors on l'affiche car ici on affiche que les uniforms des sahder or includes
                            // if (vKey->puUsedUniformsInCode.find(v->name) == vKey->puUsedUniformsInCode.end()) // non trouve
                            if (puIncludeUniformNames.find(v->name) == puIncludeUniformNames.end())  // non trouve
                            {
                                change |= UniformWidgets::drawImGuiUniformWidgetForPanes(m_This, v, PaneWidth, vFirstColumnWidth, vRenderPack, vShowUnUsed, vShowCustom);
                            }
                        }
                    }
                }
            }
        }
    }

    return change;
}

void CodeTree::ResetUniformWidgetToTheirDefaultValue(UniformVariantPtr vUniPtr) {
    if (vUniPtr && vUniPtr->loc) {
        UniformVariantPtr v = vUniPtr;

        if (v->lockedAgainstConfigLoading) {
            return;
        }

        // GizmoSystem::Instance()->DrawWidget(m_This, v, vMaxWidth, vFirstColumnWidth, vRenderPack, vShowUnUsed, vShowCustom, false, &change))
        // GamePadSystem::Instance()->DrawWidget(m_This, v, vMaxWidth, vFirstColumnWidth, vRenderPack, vShowUnUsed, vShowCustom, false, &change))
        // MidiSystem::Instance()->DrawWidget(m_This, v, vMaxWidth, vFirstColumnWidth, vRenderPack, vShowUnUsed, vShowCustom, false, &change))

        if (v->widgetType == "time") {
            v->bx = (v->def.x > 0.5f);
            v->x = 0.0f;

        } else if (v->widgetType == "sound") {
            /*if (v->sound)
            {
                v->sound->Reset();
            }*/
            v->x = 0.0f;
        } else if (!v->constant) {
            v->bx = v->bdef.x;
            v->by = v->bdef.y;
            v->bz = v->bdef.z;
            v->bw = v->bdef.w;

            v->x = v->def.x;
            v->y = v->def.y;
            v->z = v->def.z;
            v->w = v->def.w;

            v->ix = (int)v->def.x;
            v->iy = (int)v->def.y;
            v->iz = (int)v->def.z;
            v->iw = (int)v->def.w;

            v->ux = (uint32_t)v->def.x;
            v->uy = (uint32_t)v->def.y;
            v->uz = (uint32_t)v->def.z;
            v->uw = (uint32_t)v->def.w;

            v->mat2 = v->mat2Def;
            v->mat3 = v->mat3Def;
            v->mat4 = v->mat4Def;
        }
    }
}

void CodeTree::RecordToTimeLine(ShaderKeyPtr vKey, UniformVariantPtr vUniPtr, int vComponent) {
    if (vUniPtr) {
        if (TimeLineSystem::Instance()->CanWeRecord() || TimeLineSystem::Instance()->IsKeyExistForCurrentFrame(vKey, vUniPtr, vComponent)) {
            TimeLineSystem::Instance()->AddKeyForCurrentFrame(vKey, vUniPtr, vComponent);
        }
    }
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

/*
void CodeTree::SetCodeToReplaceInShadertype(const std::string& vKey, const std::string& vKeyInCode, const std::string& vCodeToReplace, const std::string& vShaderType,
bool vForceReParse)
{
    ShaderKeyPtr key = GetShaderKey(vKey);
    if (!key)
        key = GetIncludeKey(vKey);

    if (key)
    {
        if (key->SetCodeToReplaceInShadertype(vKeyInCode, vCodeToReplace, vShaderType) || vForceReParse)
        {
            key->StartParse();
        }
    }
}

void CodeTree::RemoveKeyToReplaceInShadertype(const std::string& vKey, const std::string& vKeyInCode, const std::string& vShaderType)
{
    ShaderKeyPtr key = GetShaderKey(vKey);
    if (!key)
        key = GetIncludeKey(vKey);

    if (key)
    {
        key->RemoveKeyToReplaceInShadertype(vKeyInCode, vShaderType);
    }
}
*/

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

void CodeTree::SetCompilationStatusForKey(const std::string& vKey, ShaderMsg vStatus) {
    ShaderKeyPtr key = GetKey(vKey);

    if (key) {
        key->puCompilationSuccess = vStatus;
    }
}

ShaderMsg CodeTree::GetCompilationStatusForKey(const std::string& vKey) {
    ShaderKeyPtr key = GetKey(vKey);

    if (key) {
        return key->puCompilationSuccess;
    }

    return ShaderMsg::SHADER_MSG_OK;
}

///////////////////////////////////////////////////////
//// GLOBAL INCLUDES FILES ////////////////////////////
///////////////////////////////////////////////////////

void CodeTree::DestroyIncludeFileList() {
    for (auto it = puIncludeUniformsList.begin(); it != puIncludeUniformsList.end(); ++it) {
        for (auto itLst = it->second.begin(); itLst != it->second.end(); ++itLst) {
            UniformsMultiLoc* uniLoc = itLst->second;

            // on en va pas detruire les pointeurs sur Uniforms car ils ne sont pas  � nous
            SAFE_DELETE(uniLoc);
        }
    }

    for (auto it = puIncludeKeys.begin(); it != puIncludeKeys.end(); ++it) {
        // std::string keyName = it->first;
        ShaderKeyPtr incKey = it->second;

        for (auto itLst = incKey->puUniformsDataBase.begin(); itLst != incKey->puUniformsDataBase.end(); ++itLst) {
            itLst->second.reset();
            itLst->second = nullptr;
        }
    }

    puIncludeUniformsList.clear();
    puIncludeUniformSectionUniformsList.clear();
    puIncludeUniformSectionOpened.clear();
    puFilesUsedFromLastShadersConstruction.clear();
    puIncludeUniformNames.clear();
}

void CodeTree::FillIncludeFileList() {
    DestroyIncludeFileList();

    for (auto it = puShaderKeys.begin(); it != puShaderKeys.end(); ++it) {
        // std::string keyName = it->first;
        ShaderKeyPtr key = it->second;

        FillIncludeFileListOfShaderKey(key);
    }

    // on propage une fois pour avoir la bonne couleur des uniforms
    for (auto it = puIncludeUniformsList.begin(); it != puIncludeUniformsList.end(); ++it) {
        for (auto itLoc = it->second.begin(); itLoc != it->second.end(); ++itLoc) {
            itLoc->second->Propagate();
        }
    }
}

void CodeTree::FillIncludeFileList(std::list<ShaderKeyPtr> vShaderKey) {
    DestroyIncludeFileList();

    for (auto it = vShaderKey.begin(); it != vShaderKey.end(); ++it) {
        // std::string keyName = it->first;
        ShaderKeyPtr key = *it;

        FillIncludeFileListOfShaderKey(key);
    }

    // on propage une fois pour avoir la bonne couleur des uniforms
    for (auto it = puIncludeUniformsList.begin(); it != puIncludeUniformsList.end(); ++it) {
        for (auto itLoc = it->second.begin(); itLoc != it->second.end(); ++itLoc) {
            itLoc->second->Propagate();
        }
    }
}

void CodeTree::FillIncludeFileListOfShaderKey(ShaderKeyPtr vShaderKey) {
    if (vShaderKey) {
        for (auto it = vShaderKey->puUsedFileNames.begin(); it != vShaderKey->puUsedFileNames.end(); ++it) {
            const auto& type = it->first;
            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                const auto& filename = *it2;
                auto keyPtr = GetKey(filename);
                if (keyPtr) {
                    if (keyPtr->puFileString.empty())
                        puFilesUsedFromLastShadersConstruction[type][keyPtr];

                    if (type == KEY_TYPE_Enum::KEY_TYPE_INCLUDE) {
                        for (auto itDB = vShaderKey->puUniformsDataBase.begin(); itDB != vShaderKey->puUniformsDataBase.end(); ++itDB) {
                            SetUniformPointerForIncludeUniformName(keyPtr, itDB->second, itDB->first);

                            /*if (key->puUniformsDataBase.find(itDB->first) != key->puUniformsDataBase.end()) // found
                            {
                                if (vShaderKey->puUniformSectionOpened.find(itDB->second->sectionName) != vShaderKey->puUniformSectionOpened.end())
                                {
                                    puIncludeUniformSectionOpened[key->puKey][itDB->second->sectionName] = vShaderKey->puUniformSectionOpened[itDB->second->sectionName];
                                }
                            }*/
                        }

                        m_This->LoadConfigIncludeFile(filename, CONFIG_TYPE_Enum::CONFIG_TYPE_UNIFORM, "");
                    }
                }
            }
        }
    }
}

// met le pointer pour tout les uniforms de ce nom
void CodeTree::SetUniformPointerForIncludeUniformName(ShaderKeyPtr vIncludeKey, UniformVariantPtr vUniformPointer, const std::string& vUniformName) {
    if (vUniformPointer && vIncludeKey) {
        // for (auto it = puIncludeKeys.begin(); it != puIncludeKeys.end(); ++it)
        //{
        // std::string keyName = it->first;
        // ShaderKeyPtr incKey = &it->second;

        // if (vShaderKey->puUsedFileNames.find(incKey->puKey) != vShaderKey->puUsedFileNames.end())
        //{
        if (vIncludeKey->puUniformsDataBase.find(vUniformName) != vIncludeKey->puUniformsDataBase.end())  // found
        {
            if (vIncludeKey->puKeyType == KEY_TYPE_Enum::KEY_TYPE_INCLUDE && vIncludeKey->puUniformsDataBase[vUniformName]) {
                if (vUniformPointer->loc >= 0) {
                    if (vUniformPointer->loc != vIncludeKey->puUniformsDataBase[vUniformName]->loc) {
                        vIncludeKey->puUniformsDataBase[vUniformName] = vUniformPointer;
                    }
                }

                // deja existant on va completer les UniformsMultiLoc
                for (auto itLoc = puIncludeUniformsList[vIncludeKey->puKey].begin(); itLoc != puIncludeUniformsList[vIncludeKey->puKey].end(); ++itLoc) {
                    if (itLoc->first == vUniformName) {
                        itLoc->second->linkedUniforms[vUniformPointer] = true;
                    }
                }
            } else {
                vIncludeKey->puUniformsDataBase[vUniformName] = vUniformPointer;

                // todo : mettre un shared_ptr ici
                UniformsMultiLoc* mloc = new UniformsMultiLoc(vUniformPointer);

                puIncludeUniformNames[vUniformName];

                // liste des unfiroms par includes files
                puIncludeUniformsList[vIncludeKey->puKey][mloc->name] = mloc;

                if (vUniformPointer->sectionName.empty()) {
                    vUniformPointer->sectionName = "default";
                }

                // liste des uniforms par includes files puis par section et par ordre de section
                if (vUniformPointer->sectionOrder >= (int)puIncludeUniformSectionUniformsList[vIncludeKey->puKey][vUniformPointer->sectionName].size())
                    puIncludeUniformSectionUniformsList[vIncludeKey->puKey][vUniformPointer->sectionName].emplace_back(mloc);
                else {
                    auto itIncSec = puIncludeUniformSectionUniformsList[vIncludeKey->puKey][vUniformPointer->sectionName].begin();
                    for (int i = 0; i < vUniformPointer->sectionOrder; i++)
                        itIncSec++;
                    puIncludeUniformSectionUniformsList[vIncludeKey->puKey][vUniformPointer->sectionName].insert(itIncSec, mloc);
                }

                puIncludeUniformSectionOpened[vIncludeKey->puKey][vUniformPointer->sectionName] = true;
            }
        }
        //}
        //}
    }
}

void CodeTree::SaveConfigIncludeFiles() {
    for (auto it = puIncludeUniformsList.begin(); it != puIncludeUniformsList.end(); ++it) {
        // std::string key = it->first;

        SaveConfigIncludeFile(it->first, &it->second, "");
    }
}

void CodeTree::SaveConfigIncludeFile(const std::string& vKey,
                                     std::unordered_map<std::string, UniformsMultiLoc*>* vMultiLocs,
                                     const std::string& vUniformConfigName,
                                     const bool vIsConfigFile) {
    std::string configFile = CodeTree::GetConfigFileName(vKey, vUniformConfigName);

    if (!configFile.empty()) {
        std::string uniformStream;
        std::string uniformSectionStream;
        std::string uniformLockedStream;
        // global settings
        // fileStream += puShaderGlobalSettings.getConfig();

        ShaderKeyPtr key = GetKey(vKey);
        if (key) {
            // key->puShaderKeyConfigSwitcher.SaveUniformConfigSummaryFile(vKey);

            std::unordered_map<std::string, int> sectionUsed;

            // uniforms
            for (auto itLoc = vMultiLocs->begin(); itLoc != vMultiLocs->end(); ++itLoc) {
                UniformsMultiLoc* loc = itLoc->second;

                uniformStream += UniformHelper::SerializeUniform(loc->uniform);
                if (!vIsConfigFile && loc->uniform->lockedAgainstConfigLoading) {
                    uniformLockedStream += "UniformLocked:" + loc->uniform->name + "\n";
                }
                sectionUsed[loc->uniform->sectionName] = 0;
            }

            // uniform sections
            for (auto itSection = puIncludeUniformSectionOpened[vKey].begin(); itSection != puIncludeUniformSectionOpened[vKey].end(); ++itSection) {
                if (sectionUsed.find(itSection->first) != sectionUsed.end()) {
                    uniformSectionStream += "UniformSection:" + itSection->first + ":" + ct::toStr(itSection->second ? "true" : "false") + "\n";
                }
            }

            std::ofstream configFileWriter(configFile, std::ios::out);

            if (configFileWriter.bad() == false) {
                configFileWriter << uniformStream;
                configFileWriter << uniformSectionStream;  // toujours apres les uniforms pour faciliter l'init au chargement si pas found
                configFileWriter << uniformLockedStream;   // toujours apres les uniforms pour faciliter l'init au chargement si pas found
                configFileWriter.close();
            }
        }
    }
}

// [FIXME]
// je pige pas pourquoi je charge pas les multiloc actuellement et fait un propagate pluto que charger tout les includes...
// a refactoer dans la shaderkey
void CodeTree::LoadConfigIncludeFile(const std::string& vShaderFileName,
                                     const CONFIG_TYPE_Enum& vConfigType,
                                     const std::string& vUniformConfigName,
                                     const bool vIsConfigFile) {
    std::string configFile = CodeTree::GetConfigFileName(vShaderFileName, vUniformConfigName);

    if (!configFile.empty()) {
        if (puIncludeUniformsList.find(vShaderFileName) != puIncludeUniformsList.end()) {
            auto dico = &puIncludeUniformsList[vShaderFileName];
            ShaderKeyPtr key = GetKey(vShaderFileName);
            if (key && dico->size() > 0) {
                std::string file;

                std::ifstream docFile(configFile, std::ios::in);
                if (docFile.is_open()) {
                    std::stringstream strStream;

                    strStream << docFile.rdbuf();  // read the file

                    file = strStream.str();
                    ct::replaceString(file, "\r\n", "\n");

                    docFile.close();

                    std::vector<std::string> lines = ct::splitStringToVector(file, "\n");

                    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
                        std::string line = *it;

                        std::vector<std::string> arr = ct::splitStringToVector(line, ":");

                        if (arr.size() > 1)  // two fields mini
                        {
                            std::string name = arr[0];

                            if (vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_SHADER || vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_ALL) {
                                if (name == "Section") {
                                    key->SelectSectionName(arr[1]);
                                } else if (name == "VertexConfig") {
                                    key->SelectConfigName("VERTEX", key->puInFileBufferName, arr[1]);
                                } else if (name == "GeomConfig") {
                                    key->SelectConfigName("GEOMETRY", key->puInFileBufferName, arr[1]);
                                } else if (name == "FragmentConfig") {
                                    key->SelectConfigName("FRAGMENT", key->puInFileBufferName, arr[1]);
                                } else if (name == "ComputeConfig") {
                                    key->SelectConfigName("COMPUTE", key->puInFileBufferName, arr[1]);
                                } else {
                                    key->puShaderGlobalSettings.LoadConfigLine(arr);
                                }
                            }
                            if (vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_UNIFORM || vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_ALL) {
                                UniformVariantPtr v = nullptr;

                                if (name == "UniformSection" && arr.size() > 2) {
                                    puIncludeUniformSectionOpened[vShaderFileName][arr[1]] = ct::ivariant(arr[2]).GetB();
                                } else if (!vIsConfigFile && name == "UniformLocked" && arr.size() > 1) {
                                    if (vIsConfigFile) {
                                        dico->at(arr[1])->uniform->lockedAgainstConfigLoading = false;
                                    } else if (dico->find(arr[1]) != dico->end()) {  // found
                                        dico->at(arr[1])->uniform->lockedAgainstConfigLoading = true;
                                    }
                                } else {
                                    if (dico->find(name) != dico->end()) {  // found
                                        v = dico->at(name)->uniform;
                                    }

                                    if (v != nullptr) {
                                        UniformHelper::DeSerializeUniform(key, v, arr);
                                        dico->at(name)->Propagate();

                                        if (puIncludeUniformSectionOpened[vShaderFileName].find(v->sectionName) ==
                                            puIncludeUniformSectionOpened[vShaderFileName].end())  // non found
                                        {
                                            puIncludeUniformSectionOpened[vShaderFileName][v->sectionName] = true;
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

void CodeTree::AddUniformNameForIncludeFile(const std::string& vUniformName, const std::string& vIncludeFile) {
    auto key = GetIncludeKey(vIncludeFile);
    if (key) {
        key->puUniformsDataBase[vUniformName];
        puIncludeUniformNames[vUniformName];
    }
}

// mainRenderPack for gizmo
bool CodeTree::DrawImGuiIncludesUniformWidget(RenderPackWeak vMainRenderPack, float vFirstColumnWidth, ct::ivec2 vScreenSize) {
    bool change = false;

    ImGui::PushID(ImGui::IncPUSHID());
    ImGui::Checkbox("Show UnUsed", &puShowUnUsedUniforms);
    if (m_ShowCustomCheckBox) {
        ImGui::SameLine();
        ImGui::Checkbox("Show Custom", &puShowCustomUniforms);
    }
    ImGui::SameLine();
    if (ImGui::ContrastedButton(ICON_NDP_RESET, "Reset Search")) {
        m_SearchInputText.Clear();
    }
    ImGui::SameLine();
    m_SearchInputText.DisplayInputText(200.0f, ICON_NDP_SEARCH, "");
    ImGui::PopID();
    ImGui::Separator();

    char nodeTitle[256] = "";
    for (const auto& it : puIncludeUniformSectionUniformsList) {
        std::string includeFileName = it.first;

        ImGui::Indent();

        bool editCatched = false;
        bool headerOpened = false;

        auto rpPtr = vMainRenderPack.lock();
        if (rpPtr) {
            headerOpened = rpPtr->CollapsingHeader(includeFileName.c_str(), false, true, &editCatched);
        } else {
            headerOpened = ImGui::CollapsingHeader_Button(includeFileName.c_str(), -1, false, ICON_NDP_PENCIL_SQUARE_O, true, &editCatched);
        }

        if (headerOpened) {
            if (TimeLineSystem::Instance()->IsActive()) {
                if (ImGui::ContrastedButton("Add To TimeLine", "Put all uniforms as keys in TimeLine")) {
                    TimeLineSystem::Instance()->AddIncludeKeyForCurrentFrame(m_This, includeFileName);
                }

                ImGui::SameLine();

                if (ImGui::ContrastedButton("Remove From TimeLine", "Remove all uniform keys from TimeLine")) {
                    TimeLineSystem::Instance()->DelIncludeKeyForCurrentFrame(m_This, includeFileName);
                }
            }

            for (const auto& itSec : it.second) {
                std::string section = itSec.first;
                auto uniforms = &itSec.second;

                if (section != "hidden") {
                    snprintf(nodeTitle, 255, "%s", section.c_str());
                    bool* opened = &puIncludeUniformSectionOpened[includeFileName][section];
                    if (ImGui::CollapsingHeader_SmallHeight(nodeTitle, 0.8f, -1, true, opened)) {
                        for (auto itLoc = uniforms->begin(); itLoc != uniforms->end(); ++itLoc) {
                            change |= DrawImGuiIncludesUniformWidget((*itLoc), vFirstColumnWidth, vMainRenderPack, puShowUnUsedUniforms, puShowCustomUniforms);
                        }
                    }
                }
            }
        }

        if (editCatched) {
            ImGui::OpenPopup("EditCollapsingHeader");
            puShaderKeyToEditPopup = GetKey(includeFileName);
        }

        ImGui::Unindent();
    }
    change |= DrawPopups(vMainRenderPack);
    change |= DrawDialogs(vMainRenderPack, vScreenSize);

    return change;
}

bool CodeTree::DrawImGuiIncludesUniformWidget(UniformsMultiLoc* vUniLoc, float vFirstColumnWidth, RenderPackWeak vMainRenderPack, bool vShowUnUsed, bool vShowCustom) {
    bool change = false;

    if (vUniLoc != nullptr) {
        const float PaneWidth = ImGui::GetContentRegionAvail().x;

        UniformVariantPtr v = vUniLoc->uniform;

        const auto& searchText = m_SearchInputText.GetText();
        if (!searchText.empty()) {
            if (v->nameForSearch.find(searchText) == std::string::npos) {
                return false;
            }
        }

        if (UniformWidgets::drawImGuiUniformWidgetForPanes(m_This, v, PaneWidth, vFirstColumnWidth, vMainRenderPack, vShowUnUsed, vShowCustom)) {
            change = true;
            vUniLoc->Propagate(vMainRenderPack);
        } else {
            vUniLoc->Sync(vMainRenderPack);
        }
    }

    return change;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CodeTree::ReScaleMouseUniforms(ct::fvec2 vNewSize) {
    for (auto it = puShaderKeys.begin(); it != puShaderKeys.end(); ++it) {
        ReScaleMouseUniforms(it->second, vNewSize);
    }

    for (auto it = puIncludeKeys.begin(); it != puIncludeKeys.end(); ++it) {
        ReScaleMouseUniforms(it->second, vNewSize);
    }

    UniformHelper::FBOSizeForMouseUniformNormalization = vNewSize;
}

void CodeTree::ReScaleMouseUniforms(ShaderKeyPtr vKey, ct::fvec2 vNewSize) {
    // on va redim les mouse de type coordonn�e absolue
    if (vKey) {
        if (UniformHelper::FBOSizeForMouseUniformNormalization.x > 0.0f && UniformHelper::FBOSizeForMouseUniformNormalization.y > 0.0f) {
            const ct::fvec2 scale = vNewSize / UniformHelper::FBOSizeForMouseUniformNormalization;

            std::list<UniformVariantPtr>* lst = vKey->GetUniformsByWidget("mouse:2pos_2click");
            if (lst) {
                for (auto itLst = lst->begin(); itLst != lst->end(); ++itLst) {
                    UniformVariantPtr v = *itLst;
                    if (v) {
                        v->x *= scale.x;
                        v->y *= scale.y;
                        v->z *= scale.x;
                        v->w *= scale.y;
                        v->sup.x *= scale.x;
                        v->sup.y *= scale.y;
                        v->sup.z *= scale.x;
                        v->sup.w *= scale.y;

                        ReScaleUniformOfTimeLine(vKey, v->name, scale);
                    }
                }
            }

            lst = vKey->GetUniformsByWidget("mouse:2press_2click");
            if (lst) {
                for (auto itLst = lst->begin(); itLst != lst->end(); ++itLst) {
                    UniformVariantPtr v = *itLst;
                    if (v) {
                        v->x *= scale.x;
                        v->y *= scale.y;
                        v->z *= scale.x;
                        v->w *= scale.y;
                        v->sup.x *= scale.x;
                        v->sup.y *= scale.y;
                        v->sup.z *= scale.x;
                        v->sup.w *= scale.y;

                        ReScaleUniformOfTimeLine(vKey, v->name, scale);
                    }
                }
            }
        }
    }
}

void CodeTree::ReScaleUniformOfTimeLine(ShaderKeyPtr vKey, const std::string& vUniformName, ct::fvec2 vScale) {
    // on va redim les mouse de type coordonn�e absolue
    if (vKey) {
        // in the timeline
        if (!vKey->puTimeLine.timeLine.empty()) {
            if (vKey->puTimeLine.timeLine.find(vUniformName) != vKey->puTimeLine.timeLine.end())  // trouvé
            {
                auto str = &vKey->puTimeLine.timeLine[vUniformName];
                for (auto itChan = str->keys.begin(); itChan != str->keys.end(); ++itChan) {
                    const int chan = itChan->first;
                    for (auto frameKey : itChan->second) {
                        if (frameKey.second.use_count()) {
                            frameKey.second->xyzw[chan] = vScale[chan % 2];
                        }
                    }
                }

                TimeLineSystem::Instance()->ReComputeInterpolation(vKey, vUniformName);
            }
        }
    }
}

void CodeTree::ShowCustomCheckbox(const bool& vFlag) {
    m_ShowCustomCheckBox = vFlag;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CodeTree::AddPathToTrack(std::string vPathToTrack, bool vCreateDirectoryIfNotExist) {
    if (!vPathToTrack.empty()) {
        if (vCreateDirectoryIfNotExist) {
            FileHelper::Instance()->CreateDirectoryIfNotExist(vPathToTrack);
        }

        if (puPathsToTrack.find(vPathToTrack) == puPathsToTrack.end())  // non found
        {
            puPathsToTrack.emplace(vPathToTrack);
            FilesTrackerSystem::Instance()->addWatch(vPathToTrack);
        }
    }
}

void CodeTree::InitFilesTracker(std::function<void(std::set<std::string>)> vChangeFunc, std::list<std::string> vPathsToTrack) {
    puChangeFunc = vChangeFunc;

    for (const auto& path : vPathsToTrack) {
        AddPathToTrack(path, true);
    }

    FilesTrackerSystem::Instance()->Changes = false;
}

ShaderKeyPtr CodeTree::GetParentkeyRecurs(const std::string& vKey) {
    ShaderKeyPtr key = GetKey(vKey);
    if (key) {
        for (const auto& usedKeys : key->puUsedByKeys) {
            key = GetParentkeyRecurs(usedKeys);
            if (key) {
                break;
            }
        }
    }
    return key;
}

std::set<std::string> CodeTree::ReParseFilesIfChange(std::set<std::string> vFiles) {
    std::set<ShaderKeyPtr> parentKeysToUpdate;

    for (const auto& file : vFiles) {
        ShaderKeyPtr key = GetParentkeyRecurs(file);

        if (key) {
            parentKeysToUpdate.emplace(key);

            if (!key->puBufferNames.empty()) {
                std::string newCode = FileHelper::Instance()->LoadFileToString(key->puKey, true);
                auto ps = FileHelper::Instance()->ParsePathFileName(file);
                if (ps.isOk) {
                    for (const auto& bufName : key->puBufferNames) {
                        std::string f = ps.path + "/" + ps.name + "_" + bufName + "." + ps.ext;
                        f = FileHelper::Instance()->CorrectSlashTypeForFilePathName(f);
                        auto k = GetParentkeyRecurs(f);
                        if (k) {
                            k->puFileString = newCode;
                            parentKeysToUpdate.emplace(k);
                        }
                    }
                }
            }
        }
    }

    std::set<std::string> res;

    for (const auto& key : parentKeysToUpdate) {
        if (key) {
            res.emplace(key->puKey);

            // on met force a true, si un shader a été modifié, on doit forcement recharger son parent
            // donc on verifie pas si le code du parent a changé il faut forcer la maj
            key->UpdateIfChange(true, false, true);  // todo : pourquoi on a mit force a true ??
        }
    }

    parentKeysToUpdate.clear();

    return res;
}

void CodeTree::CheckIfTheseAreSomeFileChanges() {
    FilesTrackerSystem::Instance()->update();

    if (FilesTrackerSystem::Instance()->Changes) {
        FilesTrackerSystem::Instance()->files = ReParseFilesIfChange(FilesTrackerSystem::Instance()->files);

        puChangeFunc(FilesTrackerSystem::Instance()->files);

        FilesTrackerSystem::Instance()->files.clear();

        FilesTrackerSystem::Instance()->Changes = false;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

AssetManager* CodeTree::GetAssetManager() {
    return puAssetManager;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::string CodeTree::GetConfigFileName(const std::string& vBaseFileName, const std::string& vConfigName, const std::string& vExt) {
    const size_t p = vBaseFileName.find_last_of('.');
    std::string fileName = vBaseFileName.substr(0, p);

    if (!vConfigName.empty()) {
        std::string configName = vConfigName;
        ct::replaceString(configName, " ", "_");
        fileName += "_" + configName;
    }

    fileName += vExt;

    return fileName;
}

CodeTreePtr CodeTree::Create() {
    auto res = std::make_shared<CodeTree>();
    res->m_This = res;
    return res;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CodeTree::InitTextureChooseDialogWithUniform(UniformVariantPtr vUniform) {
    if (vUniform) {
        puFlipTexture = vUniform->flip;
        puMipmapTexture = vUniform->mipmap;
        puWrapTexture = vUniform->wrap;
        puFilterTexture = vUniform->filter;
        textureChoiceActivated = vUniform->textureChoiceActivated;
        textureFlipChoosebox = vUniform->textureFlipChoosebox;
        textureMipmapChoosebox = vUniform->textureMipmapChoosebox;
        textureFilterChoosebox = vUniform->textureFilterChoosebox;
        textureWrapChoosebox = vUniform->textureWrapChoosebox;
        puPictureChooseUniform = vUniform;
    } else {
        ImGuiFileDialog::Instance()->Close();
        // puShowPictureDialog = false;
        puPictureChooseUniform = nullptr;
        puPictureFilePath = FileHelper::Instance()->GetAbsolutePathForFileLocation("", (int)FILE_LOCATION_Enum::FILE_LOCATION_ASSET_TEXTURE_2D);
        puPictureFilePathName.clear();
        puFlipTexture = false;
        puMipmapTexture = true;
        puWrapTexture = "repeat";
        puWrapTexture = "linear";
        textureChoiceActivated = true;
        textureFlipChoosebox = true;
        textureMipmapChoosebox = true;
        textureFilterChoosebox = true;
        textureWrapChoosebox = true;
    }

    if (puWrapTexture == "clamp")
        _TextureWrap = 0;
    if (puWrapTexture == "repeat")
        _TextureWrap = 1;
    if (puWrapTexture == "mirror")
        _TextureWrap = 2;

    if (puFilterTexture == "linear")
        _TextureFilter = 0;
    if (puFilterTexture == "nearest")
        _TextureFilter = 1;
}

void CodeTree::DrawTextureOptions(const char* /*vFilter*/, IGFDUserDatas /*vUserDatas*/, bool* /*vCantContinue*/) {
    if (textureFlipChoosebox || textureChoiceActivated) {
        ImGui::Checkbox("Flip", &puFlipTexture);
    }
    if (textureMipmapChoosebox || textureChoiceActivated) {
        ImGui::Checkbox("Mipmap", &puMipmapTexture);
    }
    if (textureWrapChoosebox || textureChoiceActivated) {
        if (ImGui::Combo("Wrap", &_TextureWrap, "clamp\0repeat\0mirror\0\0")) {
            if (_TextureWrap == 0)
                puWrapTexture = "clamp";
            if (_TextureWrap == 1)
                puWrapTexture = "repeat";
            if (_TextureWrap == 2)
                puWrapTexture = "mirror";
        }
    }
    if (textureFilterChoosebox || textureChoiceActivated) {
        if (ImGui::Combo("Filter", &_TextureFilter, "linear\0nearest\0\0")) {
            if (_TextureFilter == 0)
                puFilterTexture = "linear";
            if (_TextureFilter == 1)
                puFilterTexture = "nearest";
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CodeTree::InitBufferChooseDialogWithUniform(UniformVariantPtr vUniform) {
    if (vUniform) {
        puFlipBuffer = vUniform->flip;
        puMipmapBuffer = vUniform->mipmap;
        puWrapBuffer = vUniform->wrap;
        puFilterBuffer = vUniform->filter;
        bufferChoiceActivated = vUniform->bufferChoiceActivated;
        bufferFlipChoosebox = vUniform->bufferFlipChoosebox;
        bufferMipmapChoosebox = vUniform->bufferMipmapChoosebox;
        bufferFilterChoosebox = vUniform->bufferFilterChoosebox;
        bufferWrapChoosebox = vUniform->bufferWrapChoosebox;
        puBufferChooseUniform = vUniform;
    } else {
        ImGuiFileDialog::Instance()->Close();
        puBufferChooseUniform = nullptr;
        puBufferFilePath = FileHelper::Instance()->GetAbsolutePathForFileLocation("", (int)FILE_LOCATION_Enum::FILE_LOCATION_SCRIPT);
        puBufferFilePathName.clear();
        puFlipBuffer = false;
        puMipmapBuffer = true;
        puWrapBuffer = "repeat";
        puWrapBuffer = "linear";
        bufferChoiceActivated = true;
        bufferFlipChoosebox = true;
        bufferMipmapChoosebox = true;
        bufferFilterChoosebox = true;
        bufferWrapChoosebox = true;
    }

    if (puWrapBuffer == "clamp")
        _BufferWrap = 0;
    if (puWrapBuffer == "repeat")
        _BufferWrap = 1;
    if (puWrapBuffer == "mirror")
        _BufferWrap = 2;

    if (puFilterBuffer == "linear")
        _BufferFilter = 0;
    if (puFilterBuffer == "nearest")
        _BufferFilter = 1;
}

void CodeTree::DrawBufferOptions(const char* /*vFilter*/, IGFDUserDatas /*vUserDatas*/, bool* vCantContinue) {
    if (bufferFlipChoosebox || bufferChoiceActivated) {
        ImGui::Checkbox("Flip", &puFlipBuffer);
    }
    if (bufferMipmapChoosebox || bufferChoiceActivated) {
        ImGui::Checkbox("Mipmap", &puMipmapBuffer);
    }
    if (bufferWrapChoosebox || bufferChoiceActivated) {
        if (ImGui::Combo("Wrap", &_BufferWrap, "clamp\0repeat\0mirror\0\0")) {
            if (_BufferWrap == 0)
                puWrapBuffer = "clamp";
            if (_BufferWrap == 1)
                puWrapBuffer = "repeat";
            if (_BufferWrap == 2)
                puWrapBuffer = "mirror";
        }
    }
    if (bufferFilterChoosebox || bufferChoiceActivated) {
        if (ImGui::Combo("Filter", &_BufferFilter, "linear\0nearest\0\0")) {
            if (_BufferFilter == 0)
                puFilterBuffer = "linear";
            if (_BufferFilter == 1)
                puFilterBuffer = "nearest";
        }
    }

    *vCantContinue = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CodeTree::InitSoundChooseDialogWithUniform(UniformVariantPtr vUniform) {
    if (vUniform) {
        soundChoiceActivated = vUniform->textureChoiceActivated;
        puSoundChooseUniform = vUniform;
        puSoundLoop = vUniform->soundLoop;
    } else {
        ImGuiFileDialog::Instance()->Close();
        // puShowPictureDialog = false;
        puSoundChooseUniform = nullptr;
        puSoundLoop = true;
        puSoundFilePath = FileHelper::Instance()->GetAbsolutePathForFileLocation("", (int)FILE_LOCATION_Enum::FILE_LOCATION_ASSET_SOUND);
        puSoundFilePathName.clear();
        soundChoiceActivated = true;
    }
}

void CodeTree::DrawSoundOptions(const char* /*vFilter*/, IGFDUserDatas /*vUserDatas*/, bool* /*vCantContinue*/) {
    if (soundLoopChoosebox || textureChoiceActivated) {
        ImGui::Checkbox("loop", &puFlipTexture);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int _currentImGuiFrame_ForLast_DrawDialogs_Call = 0;

bool CodeTree::DrawDialogs(RenderPackWeak vRenderPack, ct::ivec2 vScreenSize) {
    // pour eviter que ce soit appel� plusieurs fois
    // il faudrais que j'enrisgsite la frame et je compare le numero de frame enregistr� avec celui acutel
    // si'l est doiffeerent j'y vais sinon je quitte.
    // comme cela pas d'appel multiples a la memem focntion pednant le meme calcul de imgui

    if (_currentImGuiFrame_ForLast_DrawDialogs_Call == ImGui::GetCurrentContext()->FrameCount)
        return false;
    _currentImGuiFrame_ForLast_DrawDialogs_Call = ImGui::GetCurrentContext()->FrameCount;

    bool change = false;

    ImVec2 min = ImVec2(0, 0);
    ImVec2 max = ImVec2(FLT_MAX, FLT_MAX);
    if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) {
        max = ImVec2((float)vScreenSize.x, (float)vScreenSize.y);
        min = max * 0.5f;
    }

    // if (puPictureChooseUniform == vUniform)
    if (puPictureChooseUniform) {
        if (ImGuiFileDialog::Instance()->Display("PictureDialog", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                change = true;

                puPictureFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                puPictureFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                puPictureFilePathName = FileHelper::Instance()->GetPathRelativeToApp(puPictureFilePathName);

                puPictureChooseUniform->filePathNames.clear();
                puPictureChooseUniform->filePathNames.emplace_back(puPictureFilePathName);

                ApplyTextureChange(vRenderPack, puPictureChooseUniform);
            }

            puPictureChooseUniform = nullptr;
            ImGuiFileDialog::Instance()->Close();
        }
    }
    if (puSoundChooseUniform) {
        if (ImGuiFileDialog::Instance()->Display("SoundDialog", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                change = true;

                puSoundFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                puSoundFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                puSoundFilePathName = FileHelper::Instance()->GetPathRelativeToApp(puSoundFilePathName);

                puSoundChooseUniform->filePathNames.clear();
                puSoundChooseUniform->filePathNames.emplace_back(puSoundFilePathName);

                ApplySoundChange(vRenderPack, puSoundChooseUniform);
            }

            puSoundChooseUniform = nullptr;
            ImGuiFileDialog::Instance()->Close();
        }
    }
    // else if (puBufferChooseUniform == vUniform)
    if (puBufferChooseUniform) {
        if (ImGuiFileDialog::Instance()->Display("BufferDialog", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                change = true;

                puBufferFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                puBufferFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                puBufferFilePathName = FileHelper::Instance()->GetPathRelativeToApp(puBufferFilePathName);

                // vUniform->filePathNames.clear();
                // vUniform->filePathNames.emplace_back(puBufferFilePathName);

                // on va detruire le precednet buffer apres la creation du nouvea
                auto rpPtr = vRenderPack.lock();
                if (rpPtr) {
                    if (rpPtr->DestroyChildBuffer(puBufferChooseUniform->bufferShaderName)) {
                        // il faut qu'on trouve tout les uniforms qui ont le meme nom pour remplacer bufferShaderName par la nouvelle valeur
                        const std::string nameToSearch = puBufferChooseUniform->name;

                        if (puBufferChooseUniform->owner) {
                            auto uni = puBufferChooseUniform->owner->GetUniformByName(nameToSearch);
                            if (uni) {
                                uni->bufferShaderName = puBufferFilePathName;
                            }
                        }

                        // la on fait les shaderKey
                        for (auto itKey = puShaderKeys.begin(); itKey != puShaderKeys.end(); ++itKey) {
                            auto uni = itKey->second->GetUniformByName(nameToSearch);
                            if (uni) {
                                uni->bufferShaderName = puBufferFilePathName;
                            }
                        }

                        // la les includesKey
                        for (auto itKey = puIncludeKeys.begin(); itKey != puIncludeKeys.end(); ++itKey) {
                            auto uni = itKey->second->GetUniformByName(nameToSearch);
                            if (uni) {
                                uni->bufferShaderName = puBufferFilePathName;
                            }
                        }

                        // la on fait les multiloc
                        for (auto itIncludeList = puIncludeUniformsList.begin(); itIncludeList != puIncludeUniformsList.end(); ++itIncludeList) {
                            for (auto itLocList = itIncludeList->second.begin(); itLocList != itIncludeList->second.end(); ++itLocList) {
                                if (itLocList->first == nameToSearch) {
                                    itLocList->second->uniform->bufferShaderName = puBufferFilePathName;
                                    itLocList->second->Propagate(vRenderPack);
                                    break;
                                }
                            }
                        }

                        puBufferChooseUniform->bufferShaderName = puBufferFilePathName;
                        ApplyBufferChange(vRenderPack, puBufferChooseUniform);

                        if (puBufferChooseUniform->owner) {
                            auto uni = puBufferChooseUniform->owner->GetUniformByName(nameToSearch);
                            if (uni) {
                                uni->bufferShaderName = puBufferFilePathName;
                            }
                        }

                        // la on fait les shaderKey
                        for (auto itKey = puShaderKeys.begin(); itKey != puShaderKeys.end(); ++itKey) {
                            auto uni = itKey->second->GetUniformByName(nameToSearch);
                            if (uni) {
                                uni->bufferShaderName = puBufferFilePathName;
                            }
                        }

                        // la les includesKey
                        for (auto itKey = puIncludeKeys.begin(); itKey != puIncludeKeys.end(); ++itKey) {
                            auto uni = itKey->second->GetUniformByName(nameToSearch);
                            if (uni) {
                                uni->bufferShaderName = puBufferFilePathName;
                            }
                        }

                        // la on fait les multiloc
                        for (auto itIncludeList = puIncludeUniformsList.begin(); itIncludeList != puIncludeUniformsList.end(); ++itIncludeList) {
                            for (auto itLocList = itIncludeList->second.begin(); itLocList != itIncludeList->second.end(); ++itLocList) {
                                if (itLocList->first == nameToSearch) {
                                    itLocList->second->uniform->bufferShaderName = puBufferFilePathName;
                                    itLocList->second->Propagate(vRenderPack);
                                    break;
                                }
                            }
                        }

                        FillIncludeFileList();
                    }
                }
            }

            puBufferChooseUniform = nullptr;
            ImGuiFileDialog::Instance()->Close();
        }
    }
    change |= DrawUniformsConfigSwitcher();

    return change;
}

bool CodeTree::DrawUniformsConfigSwitcher() {
    bool change = false;

    /*if (puShaderKeyUniformsConfigToSwitch)
    {
        change |= puShaderKeyUniformsConfigToSwitch->puShaderKeyConfigSwitcher.ConfigSwitcher_Dialog(m_This,
            &puShaderKeyUniformsConfigToSwitch, &puShaderKeyWhereCreateUniformsConfig, &puShaderKeyWhereRenameUniformsConfig);
    }
    if (puShaderKeyWhereCreateUniformsConfig)
    {
        change |= puShaderKeyWhereCreateUniformsConfig->puShaderKeyConfigSwitcher.NewConfig_Dialog(m_This, &puShaderKeyWhereCreateUniformsConfig);
    }
    if (puShaderKeyWhereRenameUniformsConfig)
    {
        change |= puShaderKeyWhereRenameUniformsConfig->puShaderKeyConfigSwitcher.RenameConfig_Dialog(m_This, &puShaderKeyWhereRenameUniformsConfig);
    }*/

    return change;
}

void CodeTree::CloseUniformsConfigSwitcher(ShaderKeyPtr vKeyToCompare) {
    /*if (vKeyToCompare)
    {
        if (puShaderKeyUniformsConfigToSwitch && vKeyToCompare == puShaderKeyUniformsConfigToSwitch)
        {
            puShaderKeyUniformsConfigToSwitch->puShaderKeyConfigSwitcher.Clear(puShaderKeyUniformsConfigToSwitch->puKey);
            puShaderKeyUniformsConfigToSwitch = nullptr;
            puShaderKeyWhereCreateUniformsConfig = nullptr;
            puShaderKeyWhereRenameUniformsConfig = nullptr;
        }
    }
    else
    {
        if (puShaderKeyUniformsConfigToSwitch)
        {
            puShaderKeyUniformsConfigToSwitch->puShaderKeyConfigSwitcher.Clear(puShaderKeyUniformsConfigToSwitch->puKey);
        }
        puShaderKeyUniformsConfigToSwitch = nullptr;
        puShaderKeyWhereCreateUniformsConfig = nullptr;
        puShaderKeyWhereRenameUniformsConfig = nullptr;
    }*/
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CodeTree::TexturePopupInit() {
    puPicturePopupUniform = nullptr;
}

bool CodeTree::TexturePopupCheck(UniformVariantPtr vUniform) {
    bool res = false;

    if (vUniform) {
        if (ImGui::BeginPopupContextItem("TexturePopup", ImGuiPopupFlags_MouseButtonRight))  // bouton droit au dessus du dernier item
        {
            puFlipBuffer = vUniform->flip;
            puMipmapBuffer = vUniform->mipmap;
            puWrapBuffer = vUniform->wrap;
            puFilterBuffer = vUniform->filter;
            textureChoiceActivated = vUniform->textureChoiceActivated;
            textureFlipChoosebox = vUniform->textureFlipChoosebox;
            textureMipmapChoosebox = vUniform->textureMipmapChoosebox;
            textureFilterChoosebox = vUniform->textureFilterChoosebox;
            textureWrapChoosebox = vUniform->textureWrapChoosebox;
            puPicturePopupUniform = vUniform;

            if (puWrapBuffer == "clamp")
                _TextureWrap = 0;
            if (puWrapBuffer == "repeat")
                _TextureWrap = 1;
            if (puWrapBuffer == "mirror")
                _TextureWrap = 2;

            if (puFilterBuffer == "linear")
                _TextureFilter = 0;
            if (puFilterBuffer == "nearest")
                _TextureFilter = 1;

            res = true;

            ImGui::EndPopup();
        }
    }

    return res;
}

bool CodeTree::DrawTexturePopup(RenderPackWeak vRenderPack) {
    bool change = false;

    if (ImGui::BeginPopupContextItem("TexturePopup")) {
        if (puPicturePopupUniform) {
            ImGui::Text(puPicturePopupUniform->name.c_str());
            ImGui::Separator();

            if (textureFlipChoosebox || textureChoiceActivated) {
                if (ImGui::Checkbox("Flip", &puFlipTexture)) {
                    change = true;

                    ApplyTextureChange(vRenderPack, puPicturePopupUniform);
                }
            }
            if (textureMipmapChoosebox || textureChoiceActivated) {
                if (ImGui::Checkbox("Mipmap", &puMipmapTexture)) {
                    change = true;

                    ApplyTextureChange(vRenderPack, puPicturePopupUniform);
                }
            }
            if (textureWrapChoosebox || textureChoiceActivated) {
                ImGui::PushItemWidth(80.0f);
                if (ImGui::Combo("Wrap", &_TextureWrap, "clamp\0repeat\0mirror\0\0")) {
                    change = true;

                    if (_TextureWrap == 0)
                        puWrapTexture = "clamp";
                    if (_TextureWrap == 1)
                        puWrapTexture = "repeat";
                    if (_TextureWrap == 2)
                        puWrapTexture = "mirror";

                    ApplyTextureChange(vRenderPack, puPicturePopupUniform);
                }
                ImGui::PopItemWidth();
            }
            if (textureFilterChoosebox || textureChoiceActivated) {
                ImGui::PushItemWidth(80.0f);
                if (ImGui::Combo("Filter", &_TextureFilter, "linear\0nearest\0\0")) {
                    change = true;

                    if (_TextureFilter == 0)
                        puFilterTexture = "linear";
                    if (_TextureFilter == 1)
                        puFilterTexture = "nearest";

                    ApplyTextureChange(vRenderPack, puPicturePopupUniform);
                }
                ImGui::PopItemWidth();
            }
            if (ImGui::Selectable("Open Directory")) {
                change = true;

                if (!puPicturePopupUniform->filePathNames.empty()) {
                    std::string _filepathName = puPicturePopupUniform->filePathNames[0];
                    if (!FileHelper::Instance()->IsFileExist(_filepathName, true))
                        _filepathName = FileHelper::Instance()->GetAbsolutePathForFileLocation(_filepathName, (int)FILE_LOCATION_Enum::FILE_LOCATION_ASSET_TEXTURE_2D);

                    FileHelper::Instance()->SelectFile(_filepathName);
                }
            }
            if (ImGui::Selectable("Copy to Clipboard")) {
                change = true;

                auto rpPtr = vRenderPack.lock();
                if (rpPtr && rpPtr->GetShaderKey()) {
                    rpPtr->GetShaderKey()->SerializeUniformAsWidgetToClipBoard(rpPtr->puWindow, puPicturePopupUniform);
                }
            }
            if (ImGui::Selectable("Reset")) {
                change = true;

                ResetTexture(vRenderPack, puPicturePopupUniform);
            }
            if (ImGui::Selectable("Empty")) {
                change = true;

                EmptyTexture(vRenderPack, puPicturePopupUniform);
            }
        }

        ImGui::EndPopup();
    }

    return change;
}

bool CodeTree::DrawEditCollapsingHeaderPopup(RenderPackWeak vRenderPack) {
    bool change = false;

    UNUSED(vRenderPack);

    if (puShaderKeyToEditPopup) {
        if (ImGui::BeginPopup("EditCollapsingHeader")) {
            if (ImGui::MenuItem("Edit Shader")) {
                puShaderKeyToEditPopup->OpenFileKey();
            }

            if (TimeLineSystem::Instance()->IsActive()) {
                ImGui::Separator();

                if (ImGui::BeginMenu("From/To TimeLine")) {
                    if (ImGui::MenuItem("Add all Uniforms To TimeLine")) {
                        TimeLineSystem::Instance()->AddShaderKeyForCurrentFrame(puShaderKeyToEditPopup);
                    }

                    if (ImGui::MenuItem("Remove all uniforms From TimeLine")) {
                        TimeLineSystem::Instance()->DelShaderKeyForCurrentFrame(puShaderKeyToEditPopup);
                    }

                    ImGui::EndPopup();
                }
            }

            if (!vRenderPack.expired()) {
                if (ImGui::MenuItem("Reset Widgets")) {
                    puShaderKeyToEditPopup->ResetUniformsToTheirDefaultValue();
                }

                ImGui::Separator();

                if (ImGui::BeginMenu("Export Widgets")) {
                    if (ImGui::MenuItem("Put All Widgets syntax in Clipboard")) {
                        auto rpPtr = vRenderPack.lock();
                        if (rpPtr) {
                            puShaderKeyToEditPopup->SerializeUniformsAsWidgetToClipBoard(rpPtr->puWindow);
                        }
                    }

                    if (ImGui::MenuItem("Put all widgets as Const in Clipboard")) {
                        auto rpPtr = vRenderPack.lock();
                        if (rpPtr) {
                            puShaderKeyToEditPopup->SerializeUniformsAsConstToClipBoard(rpPtr->puWindow);
                        }
                    }

                    ImGui::EndPopup();
                }
            }

            /*if (ImGui::MenuItem("Export to ISF Shader File Format"))
            {

            }*/

            /*ImGui::Separator();

            if (ImGui::BeginMenu("Config Switching"))
            {
                if (ImGui::BeginMenu("Save Uniforms Config"))
                {
                    if (ImGui::MenuItem("to New Config"))
                    {
                        puShaderKeyWhereCreateUniformsConfig = puShaderKeyToEditPopup;
                    }
                    if (ImGui::BeginMenu("to Existing Config"))
                    {
                        puShaderKeyToEditPopup->puShaderKeyConfigSwitcher.SelectInListAndSaveConfig(m_This, puShaderKeyToEditPopup);

                        ImGui::EndMenu();
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Load Uniforms Config"))
                {
                    change |= puShaderKeyToEditPopup->puShaderKeyConfigSwitcher.SelectInListAndLoadConfig(m_This, puShaderKeyToEditPopup);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Erase Uniforms Config"))
                {
                    puShaderKeyToEditPopup->puShaderKeyConfigSwitcher.SelectInListAndEraseConfig(m_This, puShaderKeyToEditPopup);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Rename Uniforms Config"))
                {
                    puShaderKeyWhereRenameUniformsConfig = puShaderKeyToEditPopup->puShaderKeyConfigSwitcher.SelectInListAndRenameConfig(m_This, puShaderKeyToEditPopup);

                    ImGui::EndMenu();
                }

                ImGui::EndPopup();
            }*/

            ImGui::EndPopup();
        } else {
            puShaderKeyToEditPopup = nullptr;
        }
    }

    return change;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static int _currentImGuiFrame_ForLast_DrawPopups_Call = 0;
bool CodeTree::DrawPopups(RenderPackWeak vRenderPack) {
    // pour eviter que ce soit appel� plusieurs fois
    // il faudrais que j'enrisgsite la frame et je compare le numero de frame enregistr� avec celui acutel
    // si'l est doiffeerent j'y vais sinon je quitte.
    // comme cela pas d'appel multiples a la memem focntion pednant le meme calcul de imgui

    if (_currentImGuiFrame_ForLast_DrawPopups_Call == ImGui::GetCurrentContext()->FrameCount)
        return false;
    _currentImGuiFrame_ForLast_DrawPopups_Call = ImGui::GetCurrentContext()->FrameCount;

    bool res = DrawTexturePopup(vRenderPack);
    res |= DrawEditCollapsingHeaderPopup(vRenderPack);

    return res;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CodeTree::ApplyTextureChange(RenderPackWeak vRenderPack, UniformVariantPtr vUniform) {
    if (!vRenderPack.expired() && vUniform) {
        auto rpPtr = vRenderPack.lock();
        if (rpPtr && rpPtr->GetShaderKey()) {
            vUniform->flip = puFlipTexture;
            vUniform->mipmap = puMipmapTexture;
            vUniform->wrap = puWrapTexture;
            vUniform->filter = puFilterTexture;

            rpPtr->GetShaderKey()->Complete_Uniform_Picture_With_Texture(vUniform);
        }
    }
}

void CodeTree::ResetTexture(RenderPackWeak vRenderPack, UniformVariantPtr vUniform) {
    auto rpPtr = vRenderPack.lock();
    if (rpPtr && vUniform) {
        if (rpPtr->GetShaderKey()) {
            // on va trouver le parsedstruct qui correspond
            UniformParsedStruct uniParsed = rpPtr->GetShaderKey()->GetUniformParsedStructByName(vUniform->name);
            if (uniParsed.isOk()) {
                rpPtr->GetShaderKey()->Complete_Uniform_Picture(vRenderPack, uniParsed, vUniform);
            } else {
                vUniform->flip = false;
                vUniform->mipmap = true;
                vUniform->wrap = "repeat";
                vUniform->filter = "linear";

                vUniform->filePathNames.clear();

                // ca provoque une erreur onpengl (une seule). surement que l'id de texture �tait deja uplaod� quand j'ai reset la texture.
                // a voir comment on peut resoudre ca, c'est pas trop grave, mais c'est chiant, l'architecture de ce soft commence a faire spaghetti
                rpPtr->GetShaderKey()->Complete_Uniform_Picture_With_Texture(vUniform);
            }
        }
    }
}

void CodeTree::EmptyTexture(RenderPackWeak vRenderPack, UniformVariantPtr vUniform) {
    auto rpPtr = vRenderPack.lock();
    if (rpPtr && vUniform && rpPtr->GetShaderKey()) {
        // on va trouver le parsedstruct qui correspond
        UniformParsedStruct uniParsed = rpPtr->GetShaderKey()->GetUniformParsedStructByName(vUniform->name);
        if (uniParsed.isOk()) {
            vUniform->filePathNames.clear();
            rpPtr->GetShaderKey()->Complete_Uniform_Picture(rpPtr, uniParsed, vUniform);
        } else {
            vUniform->flip = false;
            vUniform->mipmap = true;
            vUniform->wrap = "repeat";
            vUniform->filter = "linear";

            vUniform->filePathNames.clear();

            // ca provoque une erreur onpengl (une seule). surement que l'id de texture �tait deja uplaod� quand j'ai reset la texture.
            // a voir comment on peut resoudre ca, c'est pas trop grave, mais c'est chiant, l'architecture de ce soft commence a faire spaghetti
            rpPtr->GetShaderKey()->Complete_Uniform_Picture_With_Texture(vUniform);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CodeTree::SoundPopupInit() {
    puSoundPopupUniform = nullptr;
}

bool CodeTree::SoundPopupCheck(UniformVariantPtr vUniform) {
    bool res = false;

    if (vUniform) {
        if (ImGui::BeginPopupContextItem("SoundPopup", ImGuiPopupFlags_MouseButtonRight))  // bouton droit au dessus du dernier item
        {
            puSoundLoop = vUniform->soundLoop;
            soundChoiceActivated = vUniform->soundChoiceActivated;
            soundLoopChoosebox = vUniform->soundLoopChoosebox;
            puSoundPopupUniform = vUniform;

            res = true;

            ImGui::EndPopup();
        }
    }

    return res;
}

bool CodeTree::DrawSoundPopup(RenderPackWeak vRenderPack) {
    bool change = false;

    if (ImGui::BeginPopupContextItem("SoundPopup")) {
        if (puSoundPopupUniform) {
            ImGui::Text(puSoundPopupUniform->name.c_str());
            ImGui::Separator();

            if (soundLoopChoosebox || soundChoiceActivated) {
                if (ImGui::Checkbox("Loop", &puSoundLoop)) {
                    change = true;

                    ApplySoundChange(vRenderPack, puSoundPopupUniform);
                }
            }

            if (ImGui::Selectable("Open Directory")) {
                change = true;

                if (!puSoundPopupUniform->filePathNames.empty()) {
                    FileHelper::Instance()->SelectFile(puSoundPopupUniform->filePathNames[0]);
                }
            }
            if (ImGui::Selectable("Copy to Clipboard")) {
                change = true;

                auto rpPtr = vRenderPack.lock();
                if (rpPtr && rpPtr->GetShaderKey()) {
                    rpPtr->GetShaderKey()->SerializeUniformAsWidgetToClipBoard(rpPtr->puWindow, puSoundPopupUniform);
                }
            }
            if (ImGui::Selectable("Reset")) {
                change = true;

                ResetSound(vRenderPack, puSoundPopupUniform);
            }
        }

        ImGui::EndPopup();
    }

    return change;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CodeTree::ApplySoundChange(RenderPackWeak vRenderPack, UniformVariantPtr vUniform) {
    auto rpPtr = vRenderPack.lock();
    if (rpPtr && vUniform && rpPtr->GetShaderKey()) {
        vUniform->soundLoop = puSoundLoop;

        rpPtr->GetShaderKey()->Complete_Uniform_Sound_With_Sound(rpPtr->puWindow, vUniform);
    }
}

void CodeTree::ResetSound(RenderPackWeak vRenderPack, UniformVariantPtr vUniform) {
    auto rpPtr = vRenderPack.lock();
    if (rpPtr && vUniform && rpPtr->GetShaderKey()) {
        // on va trouver le parsedstruct qui correspond
        UniformParsedStruct uniParsed = rpPtr->GetShaderKey()->GetUniformParsedStructByName(vUniform->name);
        if (uniParsed.isOk()) {
            rpPtr->GetShaderKey()->Complete_Uniform_TextureSound(vRenderPack, uniParsed, vUniform);
        } else {
            vUniform->soundLoop = true;
            vUniform->soundHisto = 0;
            vUniform->filePathNames.clear();

            // ca provoque une erreur onpengl (une seule). surement que l'id de texture �tait deja uplaod� quand j'ai reset la texture.
            // a voir comment on peut resoudre ca, c'est pas trop grave, mais c'est chiant, l'architecture de ce soft commence a faire spaghetti
            rpPtr->GetShaderKey()->Complete_Uniform_Sound_With_Sound(rpPtr->puWindow, vUniform);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CodeTree::BufferPopupInit() {
    puBufferPopupUniform = nullptr;
}

bool CodeTree::BufferPopupCheck(UniformVariantPtr vUniform) {
    bool res = false;

    if (vUniform) {
        if (ImGui::BeginPopupContextItem("BufferPopup", ImGuiPopupFlags_MouseButtonRight))  // bouton droit au dessus du dernier item
        {
            puFlipBuffer = vUniform->flip;
            puMipmapBuffer = vUniform->mipmap;
            puWrapBuffer = vUniform->wrap;
            puFilterBuffer = vUniform->filter;
            bufferChoiceActivated = vUniform->bufferChoiceActivated;
            bufferFlipChoosebox = vUniform->bufferFlipChoosebox;
            bufferMipmapChoosebox = vUniform->bufferMipmapChoosebox;
            bufferFilterChoosebox = vUniform->bufferFilterChoosebox;
            bufferWrapChoosebox = vUniform->bufferWrapChoosebox;
            puBufferPopupUniform = vUniform;

            if (puWrapBuffer == "clamp")
                _BufferWrap = 0;
            if (puWrapBuffer == "repeat")
                _BufferWrap = 1;
            if (puWrapBuffer == "mirror")
                _BufferWrap = 2;

            if (puFilterBuffer == "linear")
                _BufferFilter = 0;
            if (puFilterBuffer == "nearest")
                _BufferFilter = 1;

            res = true;

            ImGui::EndPopup();
        }
    }

    return res;
}

bool CodeTree::DrawBufferPopup(RenderPackWeak vRenderPack) {
    UNUSED(vRenderPack);

    // on desactive cette feature, je pense que ca ne sert a rien, car le buufer ne peut pas etre plus tune ici
    // que chez lui
    return false;

    /*bool change = false;

    if (ImGui::BeginPopupContextItem("BufferPopup"))
    {
        if (puBufferPopupUniform)
        {
            ImGui::Text(puBufferPopupUniform->name.c_str());
            ImGui::Separator();

            if (bufferFlipChoosebox || bufferChoiceActivated)
            {
                if (ImGui::Checkbox("Flip", &puFlipBuffer))
                {
                    change = true;

                    ApplyBufferChange(vRenderPack, puBufferPopupUniform);
                }
            }
            if (bufferMipmapChoosebox || bufferChoiceActivated)
            {
                if (ImGui::Checkbox("Mipmap", &puMipmapBuffer))
                {
                    change = true;

                    ApplyBufferChange(vRenderPack, puBufferPopupUniform);
                }
            }
            if (bufferWrapChoosebox || bufferChoiceActivated)
            {
                ImGui::PushItemWidth(80.0f);
                if (ImGui::Combo("Wrap", &_BufferWrap, "clamp\0repeat\0mirror\0\0"))
                {
                    change = true;

                    if (_BufferWrap == 0) puWrapBuffer = "clamp";
                    if (_BufferWrap == 1) puWrapBuffer = "repeat";
                    if (_BufferWrap == 2) puWrapBuffer = "mirror";

                    ApplyBufferChange(vRenderPack, puBufferPopupUniform);
                }
                ImGui::PopItemWidth();
            }
            if (bufferFilterChoosebox || bufferChoiceActivated)
            {
                ImGui::PushItemWidth(80.0f);
                if (ImGui::Combo("Filter", &_BufferFilter, "linear\0nearest\0\0"))
                {
                    change = true;

                    if (_BufferFilter == 0) puFilterBuffer = "linear";
                    if (_BufferFilter == 1) puFilterBuffer = "nearest";

                    ApplyBufferChange(vRenderPack, puBufferPopupUniform);
                }
                ImGui::PopItemWidth();
            }
            if (ImGui::Selectable("Open Directory"))
            {
                change = true;

                if (!puBufferPopupUniform->filePathNames.empty())
                {
                    FileHelper::Instance()->SelectFile(puBufferPopupUniform->filePathNames[0]);
                }
            }
            if (ImGui::Selectable("Copy to Clipboard"))
            {
                change = true;

                if (vRenderPack->GetShaderKey())
                {
                    vRenderPack->GetShaderKey()->SerializeUniformAsWidgetToClipBoard(vRenderPack->puWindow, puBufferPopupUniform);
                }
            }
            if (ImGui::Selectable("Reset"))
            {
                change = true;

                ResetBuffer(vRenderPack, puBufferPopupUniform);
            }
        }

        ImGui::EndPopup();
    }

    return change;*/
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CodeTree::ApplyBufferChange(RenderPackWeak vRenderPack, UniformVariantPtr vUniform) {
    auto rpPtr = vRenderPack.lock();
    if (rpPtr && vUniform && rpPtr->GetShaderKey()) {
        if (rpPtr->GetShaderKey()) {
            // ca sert a rien de faire ca
            // one ne peut modifier que le buffer courant
            // vUniform->flip = puFlipBuffer;
            // vUniform->mipmap = puMipmapBuffer;
            // vUniform->wrap = puWrapBuffer;
            // vUniform->filter = puFilterBuffer;

            rpPtr->GetShaderKey()->Complete_Uniform_Buffer_With_Buffer(vRenderPack, vUniform);
        }
    }
}

void CodeTree::ResetBuffer(RenderPackWeak vRenderPack, UniformVariantPtr vUniform) {
    auto rpPtr = vRenderPack.lock();
    if (rpPtr && vUniform && rpPtr->GetShaderKey()) {
        // on va trouver le parsedstruct qui correspond
        UniformParsedStruct uniParsed = rpPtr->GetShaderKey()->GetUniformParsedStructByName(vUniform->name);
        if (uniParsed.isOk()) {
            rpPtr->GetShaderKey()->Complete_Uniform_Buffer(vRenderPack, uniParsed, vUniform);
        } else {
            vUniform->flip = false;
            vUniform->mipmap = true;
            vUniform->wrap = "repeat";
            vUniform->filter = "linear";

            vUniform->filePathNames.clear();

            // ca provoque une erreur onpengl (une seule). surement que l'id de buffer �tait deja uplaod� quand j'ai reset la buffer.
            // a voir comment on peut resoudre ca, c'est pas trop grave, mais c'est chiant, l'architecture de ce soft commence a faire spaghetti
            rpPtr->GetShaderKey()->Complete_Uniform_Buffer_With_Buffer(vRenderPack, vUniform);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// private fucntion used by AddOrUpdateFromFile & AddOrUpdateFromFile & AddOrUpdateFromString
bool CodeTree::AddOrUpdateKeyWithCode(ShaderKeyPtr vKey,
                                      const std::string& vKeyId,
                                      const std::string& vCode,
                                      const std::string& vOriginalFilePathName,  // like the oriignal file from where the current code (vFileString) come from
                                      const std::string& vInFileBufferName,      // like MAIN or CHILD_A, CHILD_B
                                      bool vResetConfigs,
                                      bool vIsInclude,
                                      bool vIsStringBased) {
    bool res = false;

    if (!vCode.empty()) {
        if (vKey) {
            if (vKey->puMainSection->code != vCode) {
                res = true;
            }
        } else {
            res = true;
        }

        if (res) {
            if (vKey) {
                // clear configs if necessary
                if (vResetConfigs) {
                    vKey->ClearConfigs();
                    vKey->ClearSections();
                }

                // clear uniforms
                // key->ClearUniforms();
            }

            // add or replace ShaderKey
            if (!vKey) {
                if (vIsInclude) {
                    vKey = AddIncludeKey(vKeyId);
                } else {
                    vKey = AddShaderKey(vKeyId);
                }
            }

            if (vKey) {
                vKey->puInFileBufferName = vInFileBufferName;

                if (vIsStringBased)
                    vKey->puFileString = vCode;

                std::string keyId = vKeyId;
                if (!vOriginalFilePathName.empty()) {
                    keyId = vOriginalFilePathName;
                    vKey->puInFileBufferFromKey = vOriginalFilePathName;
                }
                vKey->GetSyntaxErrors()->clear();

                // key->puUsedByKeys.clear();
                vKey->puIsGeometryShaderPresent = false;
                vKey->puUniformParsedDataBase.clear();
                vKey->puCompilationSuccess = ShaderMsg::SHADER_MSG_OK;
                vKey->puParseSuccess = ShaderMsg::SHADER_MSG_OK;
                vKey->puIncludeFileNames.clear();
                vKey->puMainSection = SectionCode::Create(vKey, nullptr, keyId, vCode, "NONE", "FULL", "", "", 0, 0, 0, false);
                if (!vIsInclude) {
                    vKey->StartParse();

                    // if (!vKey->puBufferNames.empty() && vInFileBufferName.empty())
                    //	vKey->puInFileBufferName = "MAIN";

                    vKey->PrepareSectionsComboBox();
                    vKey->PrepareConfigsComboBox(vKey->puInFileBufferName);

                    if (vKey->GetSyntaxErrors()->isThereSomeSyntaxMessages(vKey, true)) {
                        vKey->puParseSuccess = ShaderMsg::SHADER_MSG_ERROR;
                    } else if (vKey->GetSyntaxErrors()->isThereSomeSyntaxMessages(vKey, false)) {
                        vKey->puParseSuccess = ShaderMsg::SHADER_MSG_WARNING;
                    }
                } else {
                    vKey->puUniformsDataBase.clear();
                }

                res = true;
            } else {
                if (!vKey)
                    LogVarError("Cant add key....");  //-V547
            }
        }
    }

    return res;
}