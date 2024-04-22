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

#include <Headers/RenderPackHeaders.h>
#include <ctools/cTools.h>
#include <CodeTree/ShaderKey.h>
#include <Uniforms/UniformVariant.h>
#include <Uniforms/UniformsMultiLoc.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack/ImGuiPack.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <set>

class AssetManager;
class CameraSystem;
class UniformVariant;
class CodeTree {
public:
    static std::string GetConfigFileName(const std::string& vBaseFileName, const std::string& vConfigName, const std::string& vExt = ".conf");
    static CodeTreePtr Create();

public:  // gizmo
    static UniformVariantPtr puCurrentGizmo;

public:
    std::unordered_map<std::string, ShaderKeyPtr> puShaderKeys;
    std::unordered_map<std::string, ShaderKeyPtr> puIncludeKeys;
    CodeTreePtr m_This = nullptr;

private:
    std::function<void(std::set<std::string>)> puChangeFunc;
    std::set<std::string> puPathsToTrack;  // map, juste pour avoir la recherche binaire et unicit�

private:
    AssetManager* puAssetManager = nullptr;

public:
    bool puDontSaveConfigFiles = false;
    bool puShowUnUsedUniforms = false;
    bool puShowCustomUniforms = false;
    bool m_ShowCustomCheckBox = false;  // show/hide custom checkbox for puShowCustomUniforms

private:
    ImWidgets::InputText m_SearchInputText;

public:
    static ShaderKeyPtr puShaderKeyToEditPopup;
    static ShaderKeyPtr puShaderKeyWhereCreateUniformsConfig;
    static ShaderKeyPtr puShaderKeyWhereRenameUniformsConfig;
    static ShaderKeyPtr puShaderKeyUniformsConfigToSwitch;

public: /* texture choosebox */
    // bool puShowPictureDialog;
    static UniformVariantPtr puPictureChooseUniform;
    std::string puPictureFilePath;
    std::string puPictureFilePathName;
    bool textureChoiceActivated = false;
    bool textureFlipChoosebox = false;
    bool textureMipmapChoosebox = false;
    bool textureFilterChoosebox = false;
    bool textureWrapChoosebox = false;
    bool puFlipTexture = false;
    bool puMipmapTexture = false;
    std::string puWrapTexture;
    std::string puFilterTexture;
    void InitTextureChooseDialogWithUniform(UniformVariantPtr vUniform);
    void DrawTextureOptions(const char* vFilter, IGFDUserDatas vUserDatas, bool* vCantContinue);

public: /* texture right menu popup */
    void TexturePopupInit();
    bool TexturePopupCheck(UniformVariantPtr vUniform);
    bool DrawTexturePopup(RenderPackWeak vRenderPack);
    static UniformVariantPtr puPicturePopupUniform;

public:
    void ApplyTextureChange(RenderPackWeak vRenderPack, UniformVariantPtr vUniform);
    void ResetTexture(RenderPackWeak vRenderPack, UniformVariantPtr vUniform);
    void EmptyTexture(RenderPackWeak vRenderPack, UniformVariantPtr vUniform);

public:  // config switcher
    bool DrawEditCollapsingHeaderPopup(RenderPackWeak vRenderPack);

public: /* buffer choosebox */
    static UniformVariantPtr puBufferChooseUniform;
    std::string puBufferFilePath;
    std::string puBufferFilePathName;
    bool bufferChoiceActivated;
    bool bufferFlipChoosebox;
    bool bufferMipmapChoosebox;
    bool bufferFilterChoosebox;
    bool bufferWrapChoosebox;
    bool puFlipBuffer;
    bool puMipmapBuffer;
    std::string puWrapBuffer;
    std::string puFilterBuffer;
    void InitBufferChooseDialogWithUniform(UniformVariantPtr vUniform);
    void DrawBufferOptions(const char* vFilter, IGFDUserDatas vUserDatas, bool* vCantContinue);

public: /* buffer right menu popup */
    void BufferPopupInit();
    bool BufferPopupCheck(UniformVariantPtr vUniform);
    bool DrawBufferPopup(RenderPackWeak vRenderPack);
    static UniformVariantPtr puBufferPopupUniform;

public:
    void ApplyBufferChange(RenderPackWeak vRenderPack, UniformVariantPtr vUniform);
    void ResetBuffer(RenderPackWeak vRenderPack, UniformVariantPtr vUniform);

public: /* Sound choosebox */
    // bool puShowPictureDialog;
    static UniformVariantPtr puSoundChooseUniform;
    std::string puSoundFilePath;
    std::string puSoundFilePathName;
    bool soundChoiceActivated;
    bool soundLoopChoosebox;
    bool puSoundLoop;
    void InitSoundChooseDialogWithUniform(UniformVariantPtr vUniform);
    void DrawSoundOptions(const char* vFilter, IGFDUserDatas vUserDatas, bool* vCantContinue);

public: /* Sound right menu popup */
    void SoundPopupInit();
    bool SoundPopupCheck(UniformVariantPtr vUniform);
    bool DrawSoundPopup(RenderPackWeak vRenderPack);
    static UniformVariantPtr puSoundPopupUniform;

public:
    void ApplySoundChange(RenderPackWeak vRenderPack, UniformVariantPtr vUniform);
    void ResetSound(RenderPackWeak vRenderPack, UniformVariantPtr vUniform);

public:
    bool DrawPopups(RenderPackWeak vRenderPack);
    bool DrawDialogs(RenderPackWeak vRenderPack, ct::ivec2 vScreenSize);

public:
    std::map<KEY_TYPE_Enum, std::unordered_map<ShaderKeyPtr, std::string>> puFilesUsedFromLastShadersConstruction;
    std::map<std::string, std::unordered_map<std::string, UniformsMultiLoc*>> puIncludeUniformsList;
    std::map<std::string, std::map<std::string, std::list<UniformsMultiLoc*>>> puIncludeUniformSectionUniformsList;  // display
    std::unordered_map<std::string, std::unordered_map<std::string, bool>> puIncludeUniformSectionOpened;
    std::unordered_map<std::string, std::string> puIncludeUniformNames;  // pour evitert d'itere a chaque frame tout les includes

public:
    CodeTree();
    ~CodeTree();

    void RemoveKey(const std::string& vKey);

    void Clear();
    void Clear(const std::unordered_set<std::string>& vExceptions);
    void ClearShaders();
    void ClearShaders(const std::unordered_set<std::string>& vExceptions);
    void ClearIncludes();
    void ClearIncludes(const std::unordered_set<std::string>& vExceptions);

    ShaderKeyPtr LoadFromFile(const std::string& vFilePathName, KEY_TYPE_Enum vFileType);
    ShaderKeyPtr LoadFromString(const std::string& vKey,
                                const std::string& vFileString,
                                const std::string& vOriginalFilePathName,  // like the oriignal file from where the current code (vFileString) come from
                                const std::string& vInFileBufferName,      // like MAIN or CHILD_A, CHILD_B
                                KEY_TYPE_Enum vFileType);

    ShaderKeyPtr AddKey(const std::string& vKey, bool vIsInclude);
    ShaderKeyPtr AddShaderKey(const std::string& vKey, bool vFilebased = true);
    ShaderKeyPtr AddIncludeKey(const std::string& vKey, bool vFilebased = true);

    ShaderKeyPtr GetKey(const std::string& vKey);
    ShaderKeyPtr GetShaderKey(const std::string& vKey);
    ShaderKeyPtr GetIncludeKey(const std::string& vKey);

    ShaderKeyPtr AddOrUpdateFromFileAndGetKey(const std::string& vFilePathName, bool vResetConfigs, bool vResetReplaceCodes, bool vIsInclude);
    ShaderKeyPtr AddOrUpdateFromStringAndGetKey(const std::string& vKey,
                                                const std::string& vFileString,
                                                const std::string& vOriginalFilePathName,  // like the oriignal file from where the current code (vFileString) come from
                                                const std::string& vInFileBufferName,      // like MAIN or CHILD_A, CHILD_B
                                                bool vResetConfigs,
                                                bool vResetReplaceCodes,
                                                bool vIsInclude);
    ShaderKeyPtr AddOrUpdateFromFileAndGetKeyIfModified(const std::string& vFilePathName, bool vResetConfigs, bool vResetReplaceCodes, bool vIsInclude);

    bool AddOrUpdateFromFile(ShaderKeyPtr vKey, bool vResetConfigs, bool vResetReplaceCodes, bool vIsInclude);
    bool AddOrUpdateFromFile(const std::string& vFilePathName, bool vResetConfigs, bool vResetReplaceCodes, bool vIsInclude);
    bool AddOrUpdateFromString(const std::string& vKey,
                               const std::string& vFileString,
                               const std::string& vOriginalFilePathName,  // like the oriignal file from where the current code (vFileString) come from
                               const std::string& vInFileBufferName,      // like MAIN or CHILD_A, CHILD_B
                               bool vResetConfigs,
                               bool vResetReplaceCodes,
                               bool vIsInclude);

    ShaderParsedStruct GetFullShader(const std::string& vKey, const std::string& vInFileBufferName, const std::string& vSectionName, const std::string& vConfigName);

    std::unordered_map<std::string, UniformVariantPtr>* GetUniformsFromIncludeFileName(const std::string& vIncludeFileName);

    void RecordToTimeLine(ShaderKeyPtr vKey, UniformVariantPtr vUniPtr, int vComponent = 0);
    bool DrawUniformName(ShaderKeyPtr vKey, UniformVariantPtr vUniPtr, int vComponent = 0, const char* vTxt = nullptr);
    void DrawUniformComment(UniformVariantPtr vUniPtr);
    bool CheckUniformVisiblity(UniformVariantPtr vUniPtr, bool vShowUnUsed);
    bool DrawImGuiUniformWidget(ShaderKeyPtr vKey,
                                float vFirstColumnWidth,
                                RenderPackWeak vRenderPack = RenderPackWeak(),
                                bool vShowUnUsed = true,
                                bool vShowCustom = false);
    bool DrawImGuiIncludesUniformWidget(RenderPackWeak vMainRenderPack, float vFirstColumnWidth, ct::ivec2 vScreenSize);
    bool DrawImGuiIncludesUniformWidget(UniformsMultiLoc* vUniLoc,
                                        float vFirstColumnWidth,
                                        RenderPackWeak vMainRenderPack = RenderPackWeak(),
                                        bool vShowUnUsed = true,
                                        bool vShowCustom = false);
    bool DrawImGuiUniformWidgetForPanes(UniformVariantPtr vUniPtr,
                                        float vMaxWidth,
                                        float vFirstColumnWidth,
                                        RenderPackWeak vRenderPack = RenderPackWeak(),
                                        bool vShowUnUsed = true,
                                        bool vShowCustom = false);

    void ResetUniformWidgetToTheirDefaultValue(UniformVariantPtr vUniPtr);

    void ReScaleMouseUniforms(ShaderKeyPtr vKey, ct::fvec2 vNewSize);
    void ReScaleMouseUniforms(ct::fvec2 vNewSize);
    void ReScaleUniformOfTimeLine(ShaderKeyPtr vKey, const std::string& vUniformName, ct::fvec2 vScale);

    void ShowCustomCheckbox(const bool& vFlag);

    // code replacement
    // void SetCodeToReplaceInShadertype(const std::string& vKey, const std::string& vKeyInCode, const std::string& vCodeToReplace, const std::string& vShaderType, bool
    // vForceReParse); void RemoveKeyToReplaceInShadertype(const std::string& vKey, const std::string& vKeyInCode, const std::string& vShaderType);

    void SetCompilationStatusForKey(const std::string& vKey, ShaderMsg vStatus);
    ShaderMsg GetCompilationStatusForKey(const std::string& vKey);

    // includes
    void DestroyIncludeFileList();
    void FillIncludeFileList();
    void FillIncludeFileList(std::list<ShaderKeyPtr> vShaderKey);
    void FillIncludeFileListOfShaderKey(ShaderKeyPtr vShaderKey);
    void SaveConfigIncludeFiles();
    void SaveConfigIncludeFile(const std::string& vKey, std::unordered_map<std::string, UniformsMultiLoc*>* vMultiLocs, const std::string& vUniformConfigName);
    void LoadConfigIncludeFile(const std::string& vShaderFileName, const CONFIG_TYPE_Enum& vConfigType, const std::string& vUniformConfigName);
    void AddUniformNameForIncludeFile(const std::string& vUniformName, const std::string& vIncludeFile);
    void SetUniformPointerForIncludeUniformName(ShaderKeyPtr vShaderKey, UniformVariantPtr vUniformPointer, const std::string& vUniformName);

    // Uniforms Config Switcher
    bool DrawUniformsConfigSwitcher();
    void CloseUniformsConfigSwitcher(ShaderKeyPtr vKeyToCompare = nullptr);

public:  // file tracker
    void InitFilesTracker(std::function<void(std::set<std::string>)> vChangeFunc, std::list<std::string> vPathsToTrack);
    void AddPathToTrack(std::string vPathToTrack, bool vCreateDirectoryIfNotExist);  // pas de const std::string& ici
    void CheckIfTheseAreSomeFileChanges();
    ShaderKeyPtr GetParentkeyRecurs(const std::string& vKey);
    std::set<std::string> ReParseFilesIfChange(std::set<std::string> vFiles);

public:
    AssetManager* GetAssetManager();

private:
    std::shared_ptr<SectionCode> GetSectionCode(const std::string& vKey);
    bool AddOrUpdateKeyWithCode(ShaderKeyPtr vKey,
                                const std::string& vKeyId,
                                const std::string& vCode,
                                const std::string& vOriginalFilePathName,  // like the oriignal file from where the current code (vFileString) come from
                                const std::string& vInFileBufferName,      // like MAIN or CHILD_A, CHILD_B
                                bool vResetConfigs,
                                bool vIsInclude,
                                bool vIsStringBased);
};
