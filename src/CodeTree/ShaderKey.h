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
#include <CodeTree/Parsing/SectionCode.h>
#include <Uniforms/UniformVariant.h>
#include <CodeTree/SyntaxErrors/SyntaxErrors.h>
#include <CodeTree/Parsing/UniformParsing.h>
#include <Systems/TimeLineSystem.h>
#include <Headers/RenderPackHeaders.h>

#include <string>
#include <unordered_map>

class ShaderKey;

struct ConfigGuiStruct {
    std::string sectionType;
    std::vector<std::string> puConfigArray;
    // const char* puConfigArray[MAX_CONFIG_COUNT_PER_SHADER_TYPE];
    int puConfigArrayIndex = -1;
    // int puConfigArrayCount = 0;

    ConfigGuiStruct();
    ConfigGuiStruct(std::string vSectioType);
    void Clear();
    void PrepareConfigsComboBox(ShaderKeyPtr vParentKey, const std::string& vInFileBufferName, const std::string& vSelectedSectionName);
    bool IsThereSomeConfigs();
    bool DrawConfigComboBox(ShaderKeyPtr vParentKey, const std::string& vSectionType, const std::string& vInFileBufferName);
};

enum class CONFIG_TYPE_Enum { CONFIG_TYPE_ALL = 0, CONFIG_TYPE_SHADER = 1, CONFIG_TYPE_UNIFORM = 2 };

struct ShaderGlobalSettings {
    size_t countInstances = 1U;     // value
    size_t countIterations = 1U;    // value
    size_t countFramesToJump = 0U;  // value
    size_t countVertices = 1U;      // value
    bool showUnUsedUniforms = false;
    bool showCustomUniforms = false;
    std::string searchUniformName;  // foe search input
    std::string displayMode = "triangles";
    float lineWidth = 1.0f;
    bool showFlag = true;
    bool useZBuffer = true;
    bool useTransparent = false;
    bool useCulling = false;
    bool useBlending = false;
    bool useGeometryShaderIfPresent = false;
    bool useTesselationShaderIfPresent = false;

    ShaderGlobalSettings() {
        lineWidth = 1.0f;
        showFlag = true;
        useZBuffer = true;
        useTransparent = false;
        useCulling = false;
        useBlending = false;
        useGeometryShaderIfPresent = false;
        useTesselationShaderIfPresent = false;
        countInstances = 1;     // value
        countIterations = 1;    // value
        countFramesToJump = 0;  // value
        countVertices = 1;      // value
        showUnUsedUniforms = false;
        showCustomUniforms = false;
        displayMode = "triangles";
    }

    std::string getConfig() {
        std::string res;

        // doit etre du style => name:value1;value2;value3 etc..
        res += "linewidth:" + ct::toStr(lineWidth) + "\n";
        res += "showflag:" + ct::toStr(showFlag ? "true" : "false") + "\n";
        res += "usezbuffer:" + ct::toStr(useZBuffer ? "true" : "false") + "\n";
        res += "usetransparent:" + ct::toStr(useTransparent ? "true" : "false") + "\n";
        res += "useculling:" + ct::toStr(useCulling ? "true" : "false") + "\n";
        res += "useblending:" + ct::toStr(useBlending ? "true" : "false") + "\n";
        res += "usegeometryshaderifpresent:" + ct::toStr(useGeometryShaderIfPresent ? "true" : "false") + "\n";
        res += "usetesselationshaderifpresent:" + ct::toStr(useTesselationShaderIfPresent ? "true" : "false") + "\n";
        res += "displaymode:" + ct::toStr(displayMode) + "\n";
        res += "countinstances:" + ct::toStr(countInstances) + "\n";
        res += "countvertices:" + ct::toStr(countVertices) + "\n";
        res += "countiterations:" + ct::toStr(countIterations) + "\n";
        res += "countframestojump:" + ct::toStr(countFramesToJump) + "\n";
        res += "showUnUsedUniforms:" + ct::toStr(showUnUsedUniforms ? "true" : "false") + "\n";
        res += "showCustomUniforms:" + ct::toStr(showCustomUniforms ? "true" : "false") + "\n";

        return res;
    }

    void LoadConfigLine(std::vector<std::string> vConfigLineArray) {
        if (!vConfigLineArray.empty()) {
            if (vConfigLineArray[0] == "usetesselationshaderifpresent") {
                if (vConfigLineArray.size() > 1) {
                    useTesselationShaderIfPresent = ct::ivariant(vConfigLineArray[1]).GetB();
                }
            } else if (vConfigLineArray[0] == "usegeometryshaderifpresent") {
                if (vConfigLineArray.size() > 1) {
                    useGeometryShaderIfPresent = ct::ivariant(vConfigLineArray[1]).GetB();
                }
            } else if (vConfigLineArray[0] == "useblending") {
                if (vConfigLineArray.size() > 1) {
                    useBlending = ct::ivariant(vConfigLineArray[1]).GetB();
                }
            } else if (vConfigLineArray[0] == "useculling") {
                if (vConfigLineArray.size() > 1) {
                    useCulling = ct::ivariant(vConfigLineArray[1]).GetB();
                }
            } else if (vConfigLineArray[0] == "usetransparent") {
                if (vConfigLineArray.size() > 1) {
                    useTransparent = ct::ivariant(vConfigLineArray[1]).GetB();
                }
            } else if (vConfigLineArray[0] == "usezbuffer") {
                if (vConfigLineArray.size() > 1) {
                    useZBuffer = ct::ivariant(vConfigLineArray[1]).GetB();
                }
            } else if (vConfigLineArray[0] == "showflag") {
                if (vConfigLineArray.size() > 1) {
                    showFlag = ct::ivariant(vConfigLineArray[1]).GetB();
                }
            } else if (vConfigLineArray[0] == "linewidth") {
                if (vConfigLineArray.size() > 1) {
                    lineWidth = ct::fvariant(vConfigLineArray[1]).GetF();
                }
            } else if (vConfigLineArray[0] == "displaymode") {
                if (vConfigLineArray.size() > 1) {
                    displayMode = vConfigLineArray[1];
                }
            } else if (vConfigLineArray[0] == "countinstances") {
                if (vConfigLineArray.size() > 1) {
                    countInstances = ct::ivariant(vConfigLineArray[1]).GetI();
                }
            } else if (vConfigLineArray[0] == "countiterations") {
                if (vConfigLineArray.size() > 1) {
                    countIterations = ct::ivariant(vConfigLineArray[1]).GetI();
                }
            } else if (vConfigLineArray[0] == "countframestojump") {
                if (vConfigLineArray.size() > 1) {
                    countFramesToJump = ct::ivariant(vConfigLineArray[1]).GetI();
                }
            } else if (vConfigLineArray[0] == "countvertices") {
                if (vConfigLineArray.size() > 1) {
                    countVertices = ct::ivariant(vConfigLineArray[1]).GetI();
                }
            } else if (vConfigLineArray[0] == "showUnUsedUniforms") {
                if (vConfigLineArray.size() > 1) {
                    showUnUsedUniforms = ct::fvariant(vConfigLineArray[1]).GetB();
                }
            } else if (vConfigLineArray[0] == "showCustomUniforms") {
                if (vConfigLineArray.size() > 1) {
                    showCustomUniforms = ct::fvariant(vConfigLineArray[1]).GetB();
                }
            }
        }
    }
};

struct ReplaceFuncNameStruct {
    size_t start = 0;
    size_t end = 0;
    std::string oldName;
    std::string newName;
};

struct FinalUniformStruct {
    std::string uniformString;
    UniformParsedStruct infos;

    FinalUniformStruct() = default;

    FinalUniformStruct(const std::string& vUniformString, const UniformParsedStruct& vInfos) {
        uniformString = vUniformString;
        infos = vInfos;
    }
};

class CodeTree;
class RenderPack;
struct OpenGlVersionStruct;
class ShaderKey {
public:
    static ShaderKeyPtr Create();

public:
    // custom widgets uniform of this application, not filled here in RenderPack
    std::unordered_map<uType::uTypeEnum, std::unordered_map<std::string, int>> puCustomWidgets;

    CodeTreePtr puParentCodeTree = nullptr;
    ShaderKeyPtr m_This = nullptr;
    std::shared_ptr<SectionCode> puMainSection = nullptr;

    // error
    ShaderMsg puCompilationSuccess;
    ShaderMsg puParseSuccess;

    KEY_TYPE_Enum puKeyType = KEY_TYPE_Enum::KEY_TYPE_SHADER;

    // Replace func name Struct : section name, func name start, replacing infos
    std::map<std::string, std::map<size_t, ReplaceFuncNameStruct>> puReplaceFuncNames;

    // le tag dans le code, sert a generer des templates,
    // genre dans le code ce serais "\"space3d.glsl\"",
    // comme ca je sais que c'est ca qu'on remplace par [[include.1]]
    std::string puIncludeFileInCode;

    // include
    bool puIsInclude = false;  // necessaire car un include n'est pas proprietaire des ces uniforms donc il ne dois pas les detruire

    // derniere update
    bool puCodeUpdated = false;

    // file based key
    std::string puKey;
    // std::string puFilePathName;
    std::string puPath;

public:
    // need to save the conf file
    // quand :
    // - un uniform est modifié
    // - un config est modifié
    // ce flag va servir a Save Includes files
    // pour pas, sauver plusieurs fois inutilement
    // dans CodeTree::SaveConfigIncludeFiles()
    bool puNeedSaving = false;

    // string based key
    std::string puFileString;

    // les fichiers conf multiples qui sont scann� dans le repertoire de la clef
    // ShaderKeyConfigSwitcher puShaderKeyConfigSwitcher;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // used by pour voir qui utilisent cet includes
    // car si un includes est mis a jour il faudra recompiler tout les shaders qui l'utilisent
    std::set<std::string> puUsedByKeys;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // liste des includes file names
    std::map<std::string,  // include name
             std::string>
        puIncludeFileNames;  // corresponding valid path
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // pareil mais  liste que ceux utilis� par les dernieres compilation
    // type genre shader / include puis les nom de fichiers
    std::map<KEY_TYPE_Enum,  // sahder/include type key
             std::set<std::string>>
        puUsedFileNames;  // file name
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::map<std::string,                    // section type
             std::map<std::string,           // in file buffer name
                      std::map<std::string,  // section name
                               std::set<std::string>>>>
        puConfigNames;  // config names
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // section name
    std::set<std::string> puSectionNames;
    // shader stages name
    std::set<std::string> puShaderStagesNames;

    std::string puInFileBufferName;     // current buffer name
    std::string puInFileBufferFromKey;  // original file Path name
    // in file buffer names
    std::set<std::string> puBufferNames;

    // in file buffer name, section name, uniform name, uniform // virtual mean create in nodegraph
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, UniformParsedStruct>>> puVirtualUniformParsedDataBase;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::map<std::string,                                      // in file buffer name
             std::map<std::string,                             // stage name
                      std::map<std::string,                    // section name
                               std::map<std::string,           // config name
                                        std::map<std::string,  // uniform name
                                                 UniformParsedStruct>>>>>
        puUniformParsedDataBase;  // uniform
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // uniform name, uniform
    std::unordered_map<std::string, UniformVariantPtr> puUniformsDataBase;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::map<std::string,           // shader stage name
             std::map<std::string,  // uniform name
                      FinalUniformStruct>>
        puFinalUniformsCode;  // contient les formes final des uniforms
                              // ( remplie par CreateUniformsHeaderFromParsedStruct)
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // les uniforms pr�sent dans le code hors includes
    std::unordered_map<std::string, std::string> puUsedUniformsInCode;

    // uniform name, uniform list
    std::unordered_map<std::string, std::list<UniformVariantPtr>> puUniformWidgetsDataBase;
    std::map<std::string, std::list<UniformVariantPtr>> puUniformSectionDataBase;  // display
    std::unordered_map<std::string, bool> puUniformSectionOpened;

    // buffer name, section name, section config
    std::unordered_map<std::string, std::unordered_map<std::string, SectionConfigStruct>> puBufferSectionConfig;
    SectionConfigStruct puLastSelectedSectionConfig;

    // selected config in config switcher pane
    std::string puConfigSwitcherSelectedConfig;

    std::unordered_map<uint8_t, std::string> m_FragColorNames;  // one fragColor name fro each fragment buffer

    TimeLineInfos puTimeLine;

    GLenum puGeometryOutputRenderMode = 0;
    GLenum puGeometryInputRenderMode = 0;
    bool puIsVertexShaderPresent = false;
    bool puIsGeometryShaderPresent = false;
    bool puIsTesselationControlShaderPresent = false;
    bool puIsTesselationEvalShaderPresent = false;
    bool puIsCommonShaderPartPresent = false;
    bool puIsFragmentShaderPresent = false;  // pas obligatoire pour du Transform Feedback
    bool puIsComputeShaderPresent = false;

    // errors
    SyntaxErrors puSyntaxErrors;

    // settings
    ShaderGlobalSettings puShaderGlobalSettings;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////// CODE TO REPLACE ///////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::unordered_map<std::string,                            // shader stage
                       std::unordered_map<std::string,         // repalce name
                                          ReplaceCodeStruct>>  // replace code structure
        puReplaceCodesByName;

private:  // configs
    // ConfigGuiStruct puUniformConfigGuiStruct;
    ConfigGuiStruct puVertexConfigGuiStruct;
    ConfigGuiStruct puGeometryConfigGuiStruct;
    ConfigGuiStruct puTesselationControlConfigGuiStruct;
    ConfigGuiStruct puTesselationEvalConfigGuiStruct;
    ConfigGuiStruct puFragmentConfigGuiStruct;
    ConfigGuiStruct puComputeConfigGuiStruct;
    ConfigGuiStruct puSectionConfigGuiStruct;

    // section type, inFileBufferName, section name, selected configs
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> puShaderConfigSelectedName;

private:  // sections
    std::vector<std::string> puSectionArray;
    int puSectionArrayIndex = -1;
    std::string puCurrentSectionName;

public:
    SyntaxErrors* GetSyntaxErrors() {
        return &puSyntaxErrors;
    }

public:
    ShaderKey();
    ~ShaderKey();

    // clear key
    void Clear();

    // update if needed
    bool UpdateIfChange(bool vForce, bool vIsInclude, bool vResetConfigs);

    // sahder code retrieving
    ShaderParsedStruct GetComputeShader(const std::string& vInFileBufferName, OpenGlVersionStruct* vGLVersion = nullptr);
    ShaderParsedStruct GetFullShader(const std::string& vInFileBufferName,
                                     bool vUseGeometryShaderIfPresent,
                                     bool vUseTesselationIfPresent,
                                     OpenGlVersionStruct* vGLVersion = nullptr);
    ShaderParsedStruct GetPartialShader(const std::string& vInFileBufferName, const std::string& vShaderStageName, OpenGlVersionStruct* vGLVersion);
    ShaderParsedCodeStruct GetShaderSection(const std::string& vInFileBufferName,
                                            const std::string& vShaderStageName,
                                            const std::string& vSectionName,
                                            const std::string& vConfigName,
                                            OpenGlVersionStruct* vGLVersion = nullptr);
    std::string GetNoteSection(const std::string& vInFileBufferName, const std::string& vSectionName, const std::string& vConfigName);

    // parce code
    void StartParse();

    // configNames
    void ClearConfigs();
    std::set<std::string>* GetConfigNames(const std::string& vShaderStageName, const std::string& vInFileBufferName, const std::string& vSectionName);
    void AddConfigName(const std::string& vShaderStageName,
                       const std::string& vInFileBufferName,
                       const std::string& vSectionName,
                       const std::string& vConfigName,
                       bool vPropagateToParents);
    void PrepareConfigsComboBox(const std::string& vInFileBufferName);
    std::string GetSelectedConfigName(const std::string& vSectionType, const std::string& vInFileBufferName);
    void SelectConfigName(const std::string& vSectionType, const std::string& vInFileBufferName, const std::string& vName);
    bool DrawConfigComboBox(const std::string& vSectionType, const std::string& vInFileBufferName);
    bool IsThereSomeConfigs();

    // Shader stage name after the char '@' like VERTEX, FRAGMENT, COMMON, and others etc
    void ClearShaderStageNames();
    void AddShaderStageName(const std::string& vShaderSectionName);
    bool IsShaderStageNameExist(const std::string& vShaderSectionName);

    // FragColorNames
    void AddFragColorName(const uint8_t& vLocationID, const std::string& vName);
    std::string GetFragColorName(const uint8_t& vLocationID) const;
    void ClearFragColorNames();

    // sectionNames like
    void ClearSections();
    std::set<std::string>* GetSectionNames();
    void AddSectionName(const std::string& vSectionName, bool vPropagateToParents);
    void PrepareSectionsComboBox();
    std::string GetSelectedSectionName();
    void SelectSectionName(const std::string& vSectionName);
    bool DrawSectionComboBox();
    bool IsThereSomeSectionConfigs();

    // buffer name -> multipass feature in one file
    void AddBufferName(const std::string& vBufferName, bool vPropagateToParents);

    // uniforms
    void DestroyUniformsWithOwner(ShaderKeyPtr vOwner);
    void ClearUniforms(bool vExceptFinalUniforms = false);
    void CreateUniformsHeader(const std::string& vInFileBufferName, const std::string& vStageName, const std::string& vSectionName, const std::string& vConfigName);
    void CreateUniformsHeaderFromParsedStruct(const std::string& vSectionType,
                                              const UniformParsedStruct& vUniformParsedStruct,
                                              std::map<std::string, std::string>* vTempDico);
    void CreateUniforms(RenderPackWeak vRenderPack);
    std::string getFinalUniformsCode(std::string voriginalUniformsCode, const std::string& vSectionType);
    void AddUniformToDataBase(std::shared_ptr<SectionCode> vSectionCode, const UniformParsedStruct& vUniformParsedStruct);
    void AddUniform(UniformVariantPtr vUniform);
    void ResetUniformsToTheirDefaultValue();
    void SerializeUniformsAsConstToClipBoard(const GuiBackend_Window& vHandleForClipboard);
    void SerializeUniformsAsWidgetToClipBoard(const GuiBackend_Window& vHandleForClipboard);
    void SerializeUniformAsWidgetToClipBoard(const GuiBackend_Window& vHandleForClipboard, UniformVariantPtr vUniform);
    std::string GetAvailableUniformName(const std::string& vName);
    void CheckWhatUniformIsUsedInCode();
    UniformVariantPtr GetUniformByName(const std::string& vName);
    UniformVariantPtr GetUsedUniformByName(const std::string& vName);
    std::list<UniformVariantPtr>* GetUniformsByWidget(const std::string& vWidget);
    std::list<UniformVariantPtr>* GetUniformsBySection(const std::string& vSection);
    UniformParsedStruct GetUniformParsedStructByName(const std::string& vName);

    // custom widgets
    void AddCustomWidgetName(uType::uTypeEnum vGlslType, const std::string& vName, int vArrayCount = 0);
    bool IsCustomWidgetName(uType::uTypeEnum vGlslType, const std::string& vName);
    int GetArrayCountOfCustomWidgetName(const std::string& vName);
    void ClearCustomWidgetNames();
    void CompleteWithCustomWidgetsFromKey(ShaderKeyPtr vKey);

    // replace code
    void AddReplaceCode(const std::string& vShaderCodePartEnum, const ReplaceCodeStruct& vReplaceCodeStruct, bool vOnlyIfNotAlreadyDefined = true);
    ReplaceCodeStruct GetReplaceCodeByName(const std::string& vShaderCodePartEnum, const std::string& vName);
    std::vector<ReplaceCodeStruct> GetReplaceCodesByType(const std::string& vShaderCodePartEnum, const std::string& vType);
    void RemoveReplaceCode(const std::string& vShaderCodePartEnum, const std::string& vName);
    bool SetNewCodeInReplaceCodes(const std::string& vShaderCodePartEnum,
                                  const std::string& vName,
                                  const std::string& vCodeToReplace,
                                  std::string vNewCodeFromKey,
                                  bool vOnlyIfNotAlreadyDefined = false);
    void ClearReplaceCodes();

    // config files *.conf
    void LoadConfigShaderFile(const std::string& vShaderFileName, const CONFIG_TYPE_Enum& vConfigType, const std::string& vUniformConfigName, bool vIsConfigFile = false);
    void SaveConfigShaderFile(std::string vShaderFileName, CONFIG_TYPE_Enum vConfigType, std::string vUniformConfigName, bool vIsConfigFile = false);
    void LoadRenderPackConfig(CONFIG_TYPE_Enum vConfigType);
    void LoadConfigIncludeFile(std::string vUniformConfigName);
    void SaveRenderPackConfig(CONFIG_TYPE_Enum vConfigType);
    void SaveDependConfigIncludeFile(std::string vUniformConfigName);
    void LoadTimeLineConfigFile(std::string vShaderFileName, std::string vUniformConfigName);
    void SaveTimeLineConfigFile(std::string vShaderFileName, std::string vUniformConfigName);

    // section configs
    SectionConfigStruct GetSectionConfig(const std::string& vInFileBufferName = "");

    // replace func names // node graph
    void AddReplacingFuncName(ShaderTypeEnum vType, size_t vStart, size_t vEnd, std::string vOldFuncName, std::string vNewFuncNames);
    void RemoveReplacingFuncName(ShaderTypeEnum vType, std::string vNewFuncNames);
    void ClearReplacingFuncNames();

    // paths
    std::string GetPath();
    void Setpath(std::string vPath);

    // open file key
    void OpenFileKey();

public:  // errors line ( goal of theses class )
    LineFileErrors GetShaderErrorWithGoodLineNumbers(ShaderPtr vShader, ShaderTypeEnum vType, bool vErrorOrWarnings);
    ErrorLineFragment ConvertLineFromTargetToSourceForShadertype(const std::string& vSectionName, size_t vLine);

private:
    ErrorLine ParseErrorLine(const std::string& vSectionName, const std::string& vErrorLine);
    // ct::uvec3 => x:error line, y:start por in text, z:end pos in text
    std::vector<ct::uvec3> GetErrorLineNumbersFromErrorLine(const std::string& vLineToParse);

public:
    UniformVariantPtr CreateUniform(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed);

    void ParseUniformSection(UniformVariantPtr vUniform);
    void FinaliseUniformSectionParsing(UniformVariantPtr vUniform);

    void Complete_Uniform_Float(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Vec2(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Vec3(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Vec4(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);

    void Complete_Uniform_Int(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_IVec2(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_IVec3(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_IVec4(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);

    void Complete_Uniform_UInt(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_UVec2(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_UVec3(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_UVec4(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);

    void Complete_Uniform_Bool(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_BVec2(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_BVec3(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_BVec4(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);

    void Complete_Uniform_Mat2(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Mat3(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Mat4(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);

    void Complete_Uniform_Sampler1D(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Sampler2D(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Sampler3D(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_SamplerCube(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);

    void Complete_Uniform_Float_Array(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Int_Array(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Vec2_Array(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Vec3_Array(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Vec4_Array(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Mat4_Array(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Sampler1D_Array(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Sampler2D_Array(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Sampler3D_Array(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_SamplerCube_Array(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);

    void Complete_Uniform_VR(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);

    void Complete_Uniform_Shadertoy(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Vertex(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Geometry(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Tesselation(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Camera(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Mouse(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Slider(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Color(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Time(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Text(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Radio(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Button(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Combobox(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Checkbox(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Buffer(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Picture(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Depth(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Volume(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_Compute(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_CubeMap(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_TextureSound(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
    void Complete_Uniform_TextureVideo(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);

    void Complete_Uniform_Picture_With_Texture(UniformVariantPtr vUniform);
    void Complete_Uniform_Buffer_With_Buffer(RenderPackWeak vRenderPack, UniformVariantPtr vUniform, bool vBufferConfigExist = false);
    void Complete_Uniform_Sound_With_Sound(const GuiBackend_Window& vGLFWwindow, UniformVariantPtr vUniform, bool vSoundConfigExist = false);
};
