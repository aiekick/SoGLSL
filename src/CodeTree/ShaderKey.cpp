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

#include "ShaderKey.h"
#include <Renderer/RenderPack.h>
#include <Renderer/Shader.h>
#include <CodeTree/CodeTree.h>
#include <Gui/CustomGuiWidgets.h>
#include <Systems/GizmoSystem.h>
#include <Texture/Texture2D.h>
#include <Texture/TextureCube.h>
#include <Texture/Texture3D.h>
#include <Texture/TextureSound.h>
#include <Systems/GamePadSystem.h>
#include <Systems/MidiSystem.h>
#include <Systems/SoundSystem.h>
#include <Helper/NaturalSort.h>
#include <ctools/GLVersionChecker.h>
#include <ctools/Logger.h>
#include <Uniforms/UniformHelper.h>
#include <CodeTree/Parsing/UniformParsing.h>
#include <VR/Backend/VRBackend.h>
#include <locale>

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ConfigGuiStruct::ConfigGuiStruct() {
    Clear();
}

ConfigGuiStruct::ConfigGuiStruct(std::string vSectioType) {
    sectionType = vSectioType;
    Clear();
}

void ConfigGuiStruct::Clear() {
    puConfigArray.clear();
    puConfigArrayIndex = -1;
}

void ConfigGuiStruct::PrepareConfigsComboBox(ShaderKeyPtr vParentKey, const std::string& vInFileBufferName, const std::string& vSelectedSectionName) {
    if (vParentKey && !sectionType.empty()) {
        Clear();

        puConfigArrayIndex = 0;

        auto verts = vParentKey->GetConfigNames(sectionType, vInFileBufferName, vSelectedSectionName);
        if (verts) {
            puConfigArrayIndex = -1;
            std::string selected = vParentKey->GetSelectedConfigName(sectionType, vInFileBufferName);
            if (verts->find(selected) == verts->end())  // non trouve
            {
                selected.clear();
            }

            if (!verts->empty()) {
                for (auto config : *verts) {
                    if (config == selected) {
                        puConfigArrayIndex = (int)puConfigArray.size();
                    }
                    puConfigArray.push_back(config);
                }

                if (selected.empty()) {
                    vParentKey->SelectConfigName(sectionType, vInFileBufferName, ((puConfigArrayIndex != -1) ? puConfigArray[puConfigArrayIndex] : puConfigArray[0]));
                }
            } else {
                CTOOL_DEBUG_BREAK;
            }
        }
    }
}

bool ConfigGuiStruct::IsThereSomeConfigs() {
    return !puConfigArray.empty();
}

bool ConfigGuiStruct::DrawConfigComboBox(ShaderKeyPtr vParentKey, const std::string& vSectionType, const std::string& vInFileBufferName) {
    bool change = false;

    if (!puConfigArray.empty() && vParentKey && !sectionType.empty() && vSectionType == sectionType) {
        if (puConfigArrayIndex < 0) {
            const std::string name = vParentKey->GetSelectedConfigName(vSectionType, vInFileBufferName);
            puConfigArrayIndex = 0;
            for (size_t i = 0; i < puConfigArray.size(); i++) {
                if (name == puConfigArray[i]) {
                    puConfigArrayIndex = (int)i;
                    break;
                }
            }
        }

        if (ImGui::ContrastedComboVectorDefault(180, vSectionType.c_str(), &puConfigArrayIndex, puConfigArray, (int)puConfigArray.size(), 0)) {
            change |= true;
            vParentKey->SelectConfigName(vSectionType, vInFileBufferName, puConfigArray[puConfigArrayIndex]);
        }
    }

    return change;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ShaderKeyPtr ShaderKey::Create() {
    auto res = std::make_shared<ShaderKey>();
    res->m_This = res;
    return res;
}

ShaderKey::ShaderKey() {
    puMainSection = SectionCode::Create();
    puMainSection->parentKey = m_This;
    puParentCodeTree = nullptr;

    puIsInclude = false;
    puCodeUpdated = true;  // pour le demarrage

    puCompilationSuccess = ShaderMsg::SHADER_MSG_OK;
    puParseSuccess = ShaderMsg::SHADER_MSG_OK;

    puGeometryOutputRenderMode = 0;
    puGeometryInputRenderMode = 0;
    puIsGeometryShaderPresent = false;

    puIsTesselationControlShaderPresent = false;
    puIsTesselationEvalShaderPresent = false;

    // puUniformConfigGuiStruct;
    puVertexConfigGuiStruct = ConfigGuiStruct("VERTEX");
    puGeometryConfigGuiStruct = ConfigGuiStruct("GEOMETRY");
    puTesselationControlConfigGuiStruct = ConfigGuiStruct("TESSCONTROL");
    puTesselationEvalConfigGuiStruct = ConfigGuiStruct("TESSEVAL");
    puFragmentConfigGuiStruct = ConfigGuiStruct("FRAGMENT");
    puComputeConfigGuiStruct = ConfigGuiStruct("COMPUTE");
    puSectionConfigGuiStruct = ConfigGuiStruct("SECTION");
}

ShaderKey::~ShaderKey() {
    Clear();
}

void ShaderKey::Clear() {
    puCompilationSuccess = ShaderMsg::SHADER_MSG_OK;
    puParseSuccess = ShaderMsg::SHADER_MSG_OK;
    puCustomWidgets.clear();
    puConfigNames.clear();
    puUniformParsedDataBase.clear();
    puMainSection->Clear();
    puSyntaxErrors.clear();
    ClearUniforms();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::AddCustomWidgetName(uType::uTypeEnum vGlslType, const std::string& vName, int vArrayCount) {
    puCustomWidgets[vGlslType][vName] = vArrayCount;
}

bool ShaderKey::IsCustomWidgetName(uType::uTypeEnum vGlslType, const std::string& vName) {
    if (puCustomWidgets.find(vGlslType) != puCustomWidgets.end())  // found
    {
        if (puCustomWidgets[vGlslType].find(vName) != puCustomWidgets[vGlslType].end())  // found
        {
            return true;
        }
    }
    return false;
}

int ShaderKey::GetArrayCountOfCustomWidgetName(const std::string& vName) {
    for (auto it = puCustomWidgets.begin(); it != puCustomWidgets.end(); ++it) {
        if (it->second.find(vName) != it->second.end())  // found
        {
            return it->second[vName];
        }
    }
    return -1;
}

void ShaderKey::ClearCustomWidgetNames() {
    puCustomWidgets.clear();
}

void ShaderKey::CompleteWithCustomWidgetsFromKey(ShaderKeyPtr vKey) {
    if (vKey) {
        for (auto itType = vKey->puCustomWidgets.begin(); itType != vKey->puCustomWidgets.end(); ++itType) {
            for (auto itName = itType->second.begin(); itName != itType->second.end(); ++itName) {
                puCustomWidgets[itType->first][itName->first] = itName->second;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderKey::UpdateIfChange(bool vForce, bool vIsInclude, bool vResetConfigs) {
    bool res = false;

    // laod code file
    std::string code;

    if (!puFileString.empty())
        code = puFileString;
    else
        code = FileHelper::Instance()->LoadFileToString(puKey, true);

    if (!code.empty()) {
        if (puMainSection->code != code) {
            res = true;
        }

        if (res || vForce) {
            puCodeUpdated = true;

            puSyntaxErrors.clear();

            if (vResetConfigs || vForce) {
                ClearConfigs();
                ClearSections();
            }

            // ClearUniforms();

            puIncludeFileNames.clear();

            puUsedByKeys.clear();
            puIsGeometryShaderPresent = false;

            // vu qu'on va redeclencher la compilation et qu'on flag cette valleur que quand ca merde
            // on les mets par defaut a true
            puParseSuccess = ShaderMsg::SHADER_MSG_OK;
            puCompilationSuccess = ShaderMsg::SHADER_MSG_OK;

            std::string keyId = puKey;
            if (!puInFileBufferFromKey.empty())
                keyId = puInFileBufferFromKey;
            puMainSection = SectionCode::Create(m_This, nullptr, keyId, code, "NONE", "FULL", "", "", 0, 0, 0, false);

            if (vIsInclude) {
            } else {
                StartParse();
                PrepareConfigsComboBox(puInFileBufferName);
                PrepareSectionsComboBox();
            }

            if (puSyntaxErrors.isThereSomeSyntaxMessages(m_This, true)) {
                puParseSuccess = ShaderMsg::SHADER_MSG_ERROR;
            } else if (puSyntaxErrors.isThereSomeSyntaxMessages(m_This, false)) {
                puParseSuccess = ShaderMsg::SHADER_MSG_WARNING;
            }

            res = true;
        }
    }

    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief
/// va retourner le code glsl final du stage
/// l'algo est simple :
/// on va parcourir tout les noeuds jusqu'a trouver la section qu'on cherche
/// une fois trouvee on va accumuler tout les noued orpheluns et recuperer le code
/// apres il suffit de sortir et de recommencer
/// @param vInFileBufferName c'est les balise BUFFER ou MAIN dans un shader multipass en 1 fichier
/// @param vShaderStageName stage name
/// @param vSectionName stage section selected in availables sections
/// @param vConfigName config used for the stage
/// @param vGLVersion opengl struct
/// @return
ShaderParsedCodeStruct ShaderKey::GetShaderSection(const std::string& vInFileBufferName,
                                                   const std::string& vShaderStageName,
                                                   const std::string& vSectionName,
                                                   const std::string& vConfigName,
                                                   OpenGlVersionStruct* vGLVersion) {
    ShaderParsedCodeStruct res;

    if (!vShaderStageName.empty()) {
        /*
        l'algo est simple :
        on va parcourir tout les noeuds jusqu'a trouver la section qu'on cherche
        une fois trouve on va accumuler tout les noued orpheluns et recuperer le code
        apres il suffit de sortir et de recommencer
        */

        std::string infileBufferName = vInFileBufferName;
        std::string sectionName = vSectionName;
        std::string configName = vConfigName;

        puMainSection->ResetFinalLineMarks(vShaderStageName);

        size_t finalLine = 0;

        if (sectionName.empty())  // si pas de section selectionnée on prend la 1ere de la liste
        {
            auto sections = GetSectionNames();
            if (sections)
                if (sections->size() > 0)
                    sectionName = *sections->begin();
        }

        if (configName.empty())  // si pas de config selectionnée on prend la 1ere de la liste
        {
            auto configs = GetConfigNames(vShaderStageName, infileBufferName, sectionName);
            if (configs)
                if (configs->size() > 0)
                    configName = *configs->begin();
        }

        if (vGLVersion) {
            res.header += vGLVersion->DefineCode + "\n";
            if (vGLVersion->attribLayoutSupportedExtention) {
                res.header += "#extension GL_ARB_explicit_attrib_location : enable\n";
                finalLine++;  // la ligne d'apres
            }
        } else {
            res.header += GLVersionChecker::Instance()->GetGlslVersionHeader() + "\n";
            if (GLVersionChecker::Instance()->m_AttribLayoutSupportedExtention) {
                res.header += "#extension GL_ARB_explicit_attrib_location : enable\n";
                finalLine++;  // la ligne d'apres
            }
        }

        // attention cette ligne a une tres gorssse importance sur le parsing via le node graph
        // edit 26/01/2020 : en fait peut etre que non, il semble que c'est de ca que vient le decalage de 2 lignes
        // sur les retour d'erreurs
        // res.header += "#line 1\n";
        // res.header += "\n";
        finalLine += 3;  // la ligne d'apres

        res.shaderCodePart = "NONE";

        if (vShaderStageName == "VERTEX")
            res.shaderCodePart = "VERTEX";
        else if (vShaderStageName == "GEOMETRY")
            res.shaderCodePart = "GEOMETRY";
        else if (vShaderStageName == "FRAGMENT")
            res.shaderCodePart = "FRAGMENT";
        else if (vShaderStageName == "COMPUTE")
            res.shaderCodePart = "COMPUTE";
        else if (vShaderStageName == "TESSCONTROL")
            res.shaderCodePart = "TESSCONTROL";
        else if (vShaderStageName == "TESSEVAL")
            res.shaderCodePart = "TESSEVAL";
        else if (vShaderStageName == "COMMON")
            res.shaderCodePart = "COMMON";

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////// UNIFORMS ///////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        // ce code n'est plus voué a fonctionner avec les ubo's,
        // et pourtant il va falloir trouver un moyen.
        // car la section UNIFORMS defini un ubo commun à tous les stages,
        // la ou les uniforms definis dans chaque stages seront specifique a ces stages
        std::string uniformPart =
            puMainSection->GetSectionPart(vShaderStageName, finalLine, res.shaderCodePart, "UNIFORMS", infileBufferName, sectionName, configName, &puUsedFileNames);
        if (!uniformPart.empty()) {
            FileHelper::Instance()->SaveToFile(uniformPart, "parsed_uniforpunames.txt", (int)FILE_LOCATION_Enum::FILE_LOCATION_DEBUG);

            uniformPart = getFinalUniformsCode(uniformPart, vShaderStageName);
            res.uniforms = uniformPart;
            finalLine += ct::GetCountOccurence(uniformPart, "\n");

            FileHelper::Instance()->SaveToFile(uniformPart, "finals_uniforpunames.txt", (int)FILE_LOCATION_Enum::FILE_LOCATION_DEBUG);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////// SPECIFIC TO EACH SECTIONS //////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        if (res.shaderCodePart != "NONE" && !res.shaderCodePart.empty()) {
            // res.uniforms = CreateUniformBuffer(vShaderStageName);
            // finalLine += ct::GetCountOccurence(res.uniforms, "\n");
            res.code = puMainSection->GetSectionPart(
                vShaderStageName, finalLine, res.shaderCodePart, res.shaderCodePart, infileBufferName, sectionName, configName, &puUsedFileNames);
            finalLine += ct::GetCountOccurence(res.code, "\n");
        }

        FileHelper::Instance()->SaveToFile(res.code, "final_code_without_uniforms.text", (int)FILE_LOCATION_Enum::FILE_LOCATION_DEBUG);

        /*
        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////// REPLACE FUNC NAMES FOR  NODE GRAPH /////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        // pour eviter qu'une ecriture decale la suivante, on va ecrire de la fin vers le debut
        for (auto it = puReplaceFuncNames[vSectionType].rbegin(); it != puReplaceFuncNames[vSectionType].rend(); ++it)
        {
            size_t start = it->second.start;
            if (it->second.end > 0)
            {
                size_t end = it->second.end;
                size_t size = res.code.size();

                if (start < size && end < size)
                {
                    //std::string str = res.code.substr(start, end - start);
                    //std::string oldStr = it->second.oldName;
                    //if (str == oldStr)
                    {
                        std::string newStr = it->second.newName;
                        res.code.replace(start, end - start, newStr);
                    }
                    //else
                    //{
                    //	LogVar(str + " != " + it->second.oldName);
                    //	puSyntaxErrors.SetSyntaxError(m_This, ERROR_TYPE_Enum::NODEGRAPH_COMPILATION_ERROR_TYPE,
                    //		<NodeGraph/NodeGraphSystem Compile fail", false,
                    //		LineFileErrors(start, m_This->puKey, <NodeGraph/NodeGraphSystem Compile fail, " + str + " != " + it->second.oldName));
                    //}
                }
                else
                {
                    if (start == std::string::npos)
                    {
                        std::string fromFile = puKey;
                        if (!m_This->puInFileBufferFromKey.empty())
                            fromFile = m_This->puInFileBufferFromKey;
                        LogVar("start > res.code.size()");
                        puSyntaxErrors.SetSyntaxError(m_This, ERROR_TYPE_Enum::NODEGRAPH_COMPILATION_ERROR_TYPE,
                            "NodeGraphSystem Compile fail", false,
                            LineFileErrors(start, fromFile, "NodeGraphSystem Compile fail, start > res.code.size()"));
                    }
                    if (end == std::string::npos)
                    {
                        std::string fromFile = puKey;
                        if (!m_This->puInFileBufferFromKey.empty())
                            fromFile = m_This->puInFileBufferFromKey;
                        LogVar("end > res.code.size()");
                        puSyntaxErrors.SetSyntaxError(m_This, ERROR_TYPE_Enum::NODEGRAPH_COMPILATION_ERROR_TYPE,
                            "NodeGraphSystem Compile fail", false,
                            LineFileErrors(start, fromFile, "NodeGraphSystem Compile fail, end > res.code.size()"));
                    }
                }

            }
            else
            {
                if (it->second.end == 0)
                {
                    std::string fromFile = puKey;
                    if (!m_This->puInFileBufferFromKey.empty())
                        fromFile = m_This->puInFileBufferFromKey;
                    LogVar("end = 0");
                    puSyntaxErrors.SetSyntaxError(m_This, ERROR_TYPE_Enum::NODEGRAPH_COMPILATION_ERROR_TYPE,
                        "NodeGraphSystem Compile fail", false,
                        LineFileErrors(start, fromFile, "NodeGraphSystem Compile fail, end = 0"));
                }
            }
        }
        */

        FileHelper::Instance()->SaveToFile(res.code, "final_code_without_uniforms_after_node_graph_change.text", (int)FILE_LOCATION_Enum::FILE_LOCATION_DEBUG);

        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        puCodeUpdated = false;
    }

    return res;
}

std::string ShaderKey::GetNoteSection(const std::string& vInFileBufferName, const std::string& vSectionName, const std::string& vConfigName) {
    std::string res;

    /*
    l'algo est simple :
    on va parcourir tout les noeuds jusqu'a trouver la section qu'on cherche
    une fois found on va accumuler tout les noued orpheluns et recuperer le code
    apres il suffit de sortir et de recommencer
    */
    std::string sectionName = vSectionName;
    std::string configName = vConfigName;

    if (sectionName.empty())  // si pas de configs on prend la 1ere de la liste
    {
        auto sections = GetSectionNames();
        if (sections)
            if (sections->size() > 0)
                sectionName = *sections->begin();
    }

    if (configName.empty())  // si pas de configs on prend la 1ere de la liste
    {
        auto configs = GetConfigNames("NOTE", puInFileBufferName, sectionName);
        if (configs)
            if (configs->size() > 0)
                configName = *configs->begin();
    }

    const size_t finalLine = 0;

    const std::string correspondingType = "NOTE";

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////// NOTE ///////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::string notePart = puMainSection->GetSectionPart("NOTE", finalLine, correspondingType, "NOTE", vInFileBufferName, sectionName, configName, &puUsedFileNames);
    if (!notePart.empty()) {
        res = notePart;
    }

    return res;
}

ShaderParsedStruct ShaderKey::GetComputeShader(const std::string& vInFileBufferName, OpenGlVersionStruct* vGLVersion) {
    ShaderParsedStruct res;

    const std::string currentSection = GetSelectedSectionName();

    puFinalUniformsCode.clear();
    puUsedFileNames.clear();

    CreateUniformsHeader(vInFileBufferName, "COMPUTE", currentSection, GetSelectedConfigName("COMPUTE", puInFileBufferName));

    res.shader["COMPUTE"] = GetShaderSection(vInFileBufferName, "COMPUTE", currentSection, GetSelectedConfigName("COMPUTE", puInFileBufferName), vGLVersion);

    res.ParseNote(GetNoteSection(vInFileBufferName, currentSection, GetSelectedConfigName("NOTE", puInFileBufferName)));

    puCodeUpdated = false;

    return res;
}

ShaderParsedStruct ShaderKey::GetFullShader(const std::string& vInFileBufferName,
                                            bool vUseGeometryShaderIfPresent,
                                            bool vUseTesselationIfPresent,
                                            OpenGlVersionStruct* vGLVersion) {
    ShaderParsedStruct res;

    const std::string currentSection = GetSelectedSectionName();

    puFinalUniformsCode.clear();
    puUsedFileNames.clear();

    //////////////////////////////////

    {
        const auto& selectedConfig = GetSelectedConfigName("VERTEX", puInFileBufferName);
        CreateUniformsHeader(vInFileBufferName, "VERTEX", currentSection, selectedConfig);
        res.shader["VERTEX"] = GetShaderSection(vInFileBufferName, "VERTEX", currentSection, selectedConfig, vGLVersion);
    }

    //////////////////////////////////

    if (vUseGeometryShaderIfPresent && puIsGeometryShaderPresent) {
        const auto& selectedConfig = GetSelectedConfigName("GEOMETRY", puInFileBufferName);
        CreateUniformsHeader(vInFileBufferName, "GEOMETRY", currentSection, selectedConfig);
        res.shader["GEOMETRY"] = GetShaderSection(vInFileBufferName, "GEOMETRY", currentSection, selectedConfig, vGLVersion);
    }

    //////////////////////////////////

    if (vUseTesselationIfPresent && puIsTesselationControlShaderPresent) {
        const auto& selectedConfig = GetSelectedConfigName("TESSCONTROL", puInFileBufferName);
        CreateUniformsHeader(vInFileBufferName, "TESSCONTROL", currentSection, selectedConfig);
        res.shader["TESSCONTROL"] = GetShaderSection(vInFileBufferName, "TESSCONTROL", currentSection, selectedConfig, vGLVersion);
    }

    //////////////////////////////////

    if (vUseTesselationIfPresent && puIsTesselationEvalShaderPresent) {
        const auto& selectedConfig = GetSelectedConfigName("TESSEVAL", puInFileBufferName);
        CreateUniformsHeader(vInFileBufferName, "TESSEVAL", currentSection, selectedConfig);
        res.shader["TESSEVAL"] = GetShaderSection(vInFileBufferName, "TESSEVAL", currentSection, selectedConfig, vGLVersion);
    }

    //////////////////////////////////

    if (puIsFragmentShaderPresent) {
        const auto& selectedConfig = GetSelectedConfigName("FRAGMENT", puInFileBufferName);
        CreateUniformsHeader(vInFileBufferName, "FRAGMENT", currentSection, selectedConfig);
        res.shader["FRAGMENT"] = GetShaderSection(vInFileBufferName, "FRAGMENT", currentSection, selectedConfig, vGLVersion);
    }

    //////////////////////////////////

    res.ParseNote(GetNoteSection(vInFileBufferName, currentSection, GetSelectedConfigName("NOTE", puInFileBufferName)));

    puCodeUpdated = false;

    return res;
}

ShaderParsedStruct ShaderKey::GetPartialShader(const std::string& vInFileBufferName, const std::string& vShaderStageName, OpenGlVersionStruct* vGLVersion) {
    ShaderParsedStruct res;

    const std::string currentSection = GetSelectedSectionName();

    //////////////////////////////////

    CreateUniformsHeader(vInFileBufferName, vShaderStageName, currentSection, GetSelectedConfigName(vShaderStageName, puInFileBufferName));

    res.shader[vShaderStageName] =
        GetShaderSection(vInFileBufferName, vShaderStageName, currentSection, GetSelectedConfigName(vShaderStageName, puInFileBufferName), vGLVersion);

    //////////////////////////////////

    puCodeUpdated = false;

    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::StartParse() {
    puUniformParsedDataBase.clear();
    puIncludeFileNames.clear();

    puSyntaxErrors.clear();
    puCompilationSuccess = ShaderMsg::SHADER_MSG_OK;
    puParseSuccess = ShaderMsg::SHADER_MSG_OK;

    ClearShaderStageNames();
    ClearFragColorNames();

    puMainSection->Parse();

    // CheckWhatUniformIsUsedInCode();

    /*if (!puBufferNames.empty())
    {
        if (puFileString.empty())
        {
            if (puBufferNames.find("MAIN") != puBufferNames.end())
            {
                puInFileBufferName = "MAIN";
            }
        }
    }*/
}

void ShaderKey::CheckWhatUniformIsUsedInCode() {
    puUsedUniformsInCode.clear();
    for (auto itUni = puUniformsDataBase.begin(); itUni != puUniformsDataBase.end(); ++itUni) {
        UniformVariantPtr v = itUni->second;

        const size_t n = ct::GetCountOccurence(puMainSection->code, v->name);

        if (n > 0) {
            puUsedUniformsInCode[v->name];
        }
    }

    CTOOL_DEBUG_BREAK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::DestroyUniformsWithOwner(ShaderKeyPtr vOwner) {
    bool res = false;

    std::vector<std::string> uniToRemove;
    for (auto itUni = puUniformsDataBase.begin(); itUni != puUniformsDataBase.end(); ++itUni) {
        UniformVariantPtr v = itUni->second;
#ifdef DEBUG_UNIFORMS
        res = UniformVariant::destroy(v, vOwner, std::string(__FILE__) + '_' + std::string(__FUNCTION__));
#else
        res = UniformVariant::destroy(v, vOwner);
#endif
        if (res) {
            uniToRemove.emplace_back(itUni->first);
        }
    }

    for (auto itLst = uniToRemove.begin(); itLst != uniToRemove.end(); ++itLst) {
        if (puUniformsDataBase.find(*itLst) != puUniformsDataBase.end())  // found
        {
            puUniformsDataBase.erase(*itLst);
        }
    }
    uniToRemove.clear();
}

void ShaderKey::ClearUniforms(bool vExceptFinalUniforms) {
    std::vector<std::string> uniformsToErase;
    for (auto itUni = puUniformsDataBase.begin(); itUni != puUniformsDataBase.end(); ++itUni) {
        UniformVariantPtr v = itUni->second;
        if (
#ifdef DEBUG_UNIFORMS
            UniformVariant::destroy(v, m_This, std::string(__FILE__) + '_' + std::string(__FUNCTION__))
#else
            UniformVariant::destroy(v, m_This)
#endif
        ) {
            uniformsToErase.emplace_back(itUni->first);
        }
    }
    // comme les unfirom ne sera pas forcemment detruits, il ne faut pas effacer la bd bettement
    // il faut seulement virer ceux qui ont �t� d�truits
    for (auto it = uniformsToErase.begin(); it != uniformsToErase.end(); ++it) {
        puUniformsDataBase.erase(*it);
    }
    // puUniformsDataBase.clear();

    for (auto incFileName : puIncludeFileNames) {
        if (puParentCodeTree) {
            auto key = puParentCodeTree->GetIncludeKey(incFileName.first);
            if (key) {
                key->DestroyUniformsWithOwner(m_This);
            }
        }
    }

    puUniformWidgetsDataBase.clear();
    puUniformSectionDataBase.clear();
    puUniformSectionOpened.clear();

    if (!vExceptFinalUniforms)
        puFinalUniformsCode.clear();
}

void ShaderKey::AddUniformToDataBase(std::shared_ptr<SectionCode> vSectionCode, const UniformParsedStruct& vUniformParsedStruct) {
    if (vSectionCode) {
        std::string inFileBufferName;
        std::string sectionName;
        std::string configName;
        std::string stageName = vSectionCode->currentType;

        if (vSectionCode->currentType == "CONFIG") {
            configName = vSectionCode->name;
        } else if (vSectionCode->currentType == "COMMON" ||       //
                   vSectionCode->currentType == "VERTEX" ||       //
                   vSectionCode->currentType == "FRAGMENT" ||     //
                   vSectionCode->currentType == "FRAMEBUFFER" ||  //
                   vSectionCode->currentType == "GEOMETRY" ||     //
                   vSectionCode->currentType == "TESSCONTROL" ||  //
                   vSectionCode->currentType == "TESSEVAL" ||     //
                   vSectionCode->currentType == "UNIFORMS" ||     //
                   vSectionCode->currentType == "COMPUTE") {
            inFileBufferName = vSectionCode->inFileBufferName;
            sectionName = vSectionCode->name;
        } else {
            // LogVarLightInfo("stageName cleared for %s", stageName.c_str());
            stageName.clear();  // on ne defini pas le stage name, car ces uniforms devront donc apparaitre dans tout les stages (car common, uniforms ou autres)
        }

        stageName = vSectionCode->stageName;

        if (!vSectionCode->forcedSectionName.empty())  // si ya un force alors mettre name a "" assurera que l'uniform sera prit a tout les coup
            configName.clear();

        puUniformParsedDataBase[inFileBufferName][stageName][sectionName][configName][vUniformParsedStruct.name] = vUniformParsedStruct;
    }
}

/*UniformParsedStruct* ShaderKey::GetUniformStruct(std::string vUniformName)
{
    if (puUniformParsedDataBase.find(vUniformName) != puUniformParsedDataBase.end()) // found
    {
        return &puUniformParsedDataBase[vUniformName];
    }

    return 0;
}*/

void ShaderKey::AddUniform(UniformVariantPtr vUniform) {
    if (vUniform != nullptr && !vUniform->name.empty()) {
        // si on ecrase le pointeur du precedent uniform
        // d'un certaine facon on ne pourra pas le detruire.
        // ce sera donc de la memoire allou� pour rien

        if (puUniformsDataBase.find(vUniform->name) != puUniformsDataBase.end())  // found
        {
            UniformVariantPtr current = puUniformsDataBase[vUniform->name];
            if (current != vUniform) {
                // on va d'abord detruire le current point�
                // if (current->owner != m_This)
                //	LogVar("Uniform not created here, so we will not destroy it cr�� ici...");
#ifdef DEBUG_UNIFORMS
                UniformVariant::destroy(current, m_This, std::string(__FILE__) + '_' + std::string(__FUNCTION__));
#else
                UniformVariant::destroy(current, m_This);
#endif
                // maintenant on le remplace pas le nouveau
                puUniformsDataBase[vUniform->name] = vUniform;
            }
        } else  // non found
        {
            puUniformsDataBase[vUniform->name] = vUniform;
        }

        if (!vUniform->widget.empty()) {
            auto lst = &puUniformWidgetsDataBase[vUniform->widget];
            lst->emplace_back(vUniform);
            lst->unique();
        }

        ParseUniformSection(vUniform);
    }
}

inline bool sort_section_order(UniformVariantPtr a, UniformVariantPtr b) {
    if (a && b) {
        return a->sectionOrder < b->sectionOrder;
    }
    return false;
}

void ShaderKey::ParseUniformSection(UniformVariantPtr vUniform) {
    if (vUniform) {
        const std::string sectionString = vUniform->sectionName;

        //()
        //(name:order:condition)
        //(order:name:condition)
        // name is string
        // order is number
        // condition is bool, contain name==name

        // 2 truc a definir
        // vUniform->sectionName
        // vUniform->sectionOrder
        // puis :
        // vUniform->useVisCheckCond = true;
        // vUniform->uniCheckCondName = cond;
        // vUniform->uniCheckCond = true;
        // vUniform->uniCheckCondPtr = 0;
        // vUniform->sectionName

        std::string sectionName = "default";
        std::string sectionOrder;
        std::string sectionCond;

        if (!sectionString.empty()) {
            auto vec = ct::splitStringToVector(sectionString, ":", false);
            if (!vec.empty()) {
                for (auto it = vec.begin(); it != vec.end(); ++it) {
                    std::string param = *it;

                    if (!param.empty()) {
                        // est ce un order / on accepte les nombre negatif
                        size_t pos = param.find_first_not_of("-0123456789");
                        if (pos == std::string::npos) {
                            sectionOrder = param;
                        } else {
                            // est un bool
                            pos = param.find_first_of("!<>=");
                            if (pos != std::string::npos) {
                                sectionCond = param;
                            } else {  // est un name
                                sectionName = param;
                            }
                        }
                    }
                }

                // name
                if (!sectionName.empty()) {
                    vUniform->sectionName = sectionName;
                }

                // order
                if (!sectionOrder.empty()) {
                    // c'est un order
                    vUniform->sectionOrder = ct::fvariant(sectionOrder).GetI();

                    puUniformSectionDataBase[sectionName].emplace_back(vUniform);
                    puUniformSectionDataBase[sectionName].unique();
                    puUniformSectionDataBase[sectionName].sort(sort_section_order);
                } else {
                    puUniformSectionDataBase[sectionName].emplace_back(vUniform);
                    puUniformSectionDataBase[sectionName].unique();
                }

                // cond
                if (!sectionCond.empty()) {
                    ct::replaceString(sectionCond, " ", "");
                    ct::replaceString(sectionCond, "\t", "");
                    ct::replaceString(sectionCond, "\r", "");
                    ct::replaceString(sectionCond, "\n", "");

                    vUniform->sectionCond = sectionCond;
                }
            }
        } else {
            puUniformSectionDataBase[sectionName].emplace_back(vUniform);
            puUniformSectionDataBase[sectionName].unique();
        }
    }
}

void ShaderKey::FinaliseUniformSectionParsing(UniformVariantPtr vUniform) {
    if (vUniform) {
        // cond
        if (!vUniform->sectionCond.empty()) {
            LineFileErrors err;

            std::string var0;
            std::string var1;

            std::string op = "eq";
            size_t condPos = vUniform->sectionCond.find("==");
            if (condPos == std::string::npos) {
                condPos = vUniform->sectionCond.find("!=");
                op = "neq";
            }
            if (condPos == std::string::npos) {
                condPos = vUniform->sectionCond.find('>');
                op = "sup";
            }
            if (condPos == std::string::npos) {
                condPos = vUniform->sectionCond.find('<');
                op = "inf";
            }
            if (condPos == std::string::npos) {
                condPos = vUniform->sectionCond.find("<=");
                op = "infeq";
            }
            if (condPos == std::string::npos) {
                condPos = vUniform->sectionCond.find(">=");
                op = "supeq";
            }
            if (condPos != std::string::npos) {
                var0 = vUniform->sectionCond.substr(0, condPos);
                var1 = vUniform->sectionCond.substr(condPos + 2);
            }

            if (!var0.empty() && !var1.empty()) {
                auto uni0 = GetUniformByName(var0);
                auto uni1 = GetUniformByName(var1);

                // if (uni0 && uni1)
                //{
                //	puSyntaxErrors.SetSyntaxError(m_This, "Parsing Error :", "Uniform Section Error",
                //		false, "Condition is bad, cant be two uniform names", vUniform->SourceLinePos);
                // }
                // else
                if (!uni0 && !uni1) {
                    std::string fromFile = m_This->puKey;
                    if (!m_This->puInFileBufferFromKey.empty())
                        fromFile = m_This->puInFileBufferFromKey;
                    puSyntaxErrors.SetSyntaxError(
                        m_This,
                        "Parsing error : ",
                        "Uniform Section Error",
                        false,
                        LineFileErrors(vUniform->SourceLinePos, fromFile, "The Condition '" + vUniform->sectionCond + "' is bad, no uniform to compare to a value"));
                } else {
                    UniformVariantPtr uni = uni0;
                    std::string var = var1;

                    if (uni0 && !uni1) {
                        uni = uni0;
                        var = var1;
                    } else if (uni1 && !uni0) {
                        uni = uni1;
                        var = var0;
                    }

                    if (uni && !var.empty()) {
                        if (uni->widget == "checkbox") {
                            // le bool c'est valable que pour la checkbox
                            if (var == "true") {
                                vUniform->useVisCheckCond = true;
                                vUniform->uniCheckCondName = uni->name;

                                if (op == "eq")
                                    vUniform->uniCheckCond = true;
                                else if (op == "neq")
                                    vUniform->uniCheckCond = false;

                                vUniform->uniCheckCondPtr = &(uni->x);
                            } else if (var == "false") {
                                vUniform->useVisCheckCond = true;
                                vUniform->uniCheckCondName = uni->name;

                                if (op == "eq")
                                    vUniform->uniCheckCond = false;
                                else if (op == "neq")
                                    vUniform->uniCheckCond = true;

                                vUniform->uniCheckCondPtr = &(uni->x);
                            } else {
                                std::string fromFile = m_This->puKey;
                                if (!m_This->puInFileBufferFromKey.empty())
                                    fromFile = m_This->puInFileBufferFromKey;
                                puSyntaxErrors.SetSyntaxError(
                                    m_This,
                                    "Parsing error : ",
                                    "Uniform Section Error",
                                    false,
                                    LineFileErrors(vUniform->SourceLinePos, fromFile, "Condition value true or false (boolean) must be used with checbox widget only"));
                            }
                        } else if (uni->widget == "combobox") {
                            // sinon on a la combobox
                            // on va voir si on trouve le choix par mi les chocies
                            bool found = false;
                            int idx = 0;
                            for (auto it = uni->choices.begin(); it != uni->choices.end(); ++it) {
                                if (*it == var) {
                                    found = true;
                                    break;
                                }

                                idx++;
                            }
                            if (found) {
                                vUniform->useVisComboCond = true;
                                vUniform->uniComboCondName = var;
                                vUniform->uniComboCondPtr = &(uni->ix);
                                vUniform->uniComboCond = idx;

                                if (op == "eq")
                                    vUniform->uniComboCondDir = true;
                                else if (op == "neq")
                                    vUniform->uniComboCondDir = false;
                            } else {
                                std::string fromFile = m_This->puKey;
                                if (!m_This->puInFileBufferFromKey.empty())
                                    fromFile = m_This->puInFileBufferFromKey;
                                puSyntaxErrors.SetSyntaxError(m_This,
                                                              "Parsing error : ",
                                                              "Uniform Section Error",
                                                              false,
                                                              LineFileErrors(vUniform->SourceLinePos,
                                                                             fromFile,
                                                                             "The Condition '" + vUniform->sectionCond + "' is bad, the combobox choice " + var +
                                                                                 " can't be found is the comobobox " + uni->name));
                            }
                        } else if (uni->glslType == uType::uTypeEnum::U_FLOAT) {
                            if (op == "sup")
                                vUniform->useVisOpCond = 1;
                            else if (op == "supeq")
                                vUniform->useVisOpCond = 2;
                            else if (op == "inf")
                                vUniform->useVisOpCond = 3;
                            else if (op == "infeq")
                                vUniform->useVisOpCond = 4;

                            vUniform->uniOpCondThreshold = ct::fvariant(var).GetF();
                            vUniform->uniCondPtr = &(uni->x);
                        } else {
                            std::string fromFile = m_This->puKey;
                            if (!m_This->puInFileBufferFromKey.empty())
                                fromFile = m_This->puInFileBufferFromKey;
                            puSyntaxErrors.SetSyntaxError(
                                m_This,
                                "Parsing error : ",
                                "Uniform Section Error",
                                false,
                                LineFileErrors(vUniform->SourceLinePos,
                                               fromFile,
                                               "Condition " + var + "for the uniform widget " + uni->name + ", is only supported with checkbox or combobox"));
                        }
                    }
                }
            } else {
                std::string fromFile = m_This->puKey;
                if (!m_This->puInFileBufferFromKey.empty())
                    fromFile = m_This->puInFileBufferFromKey;
                puSyntaxErrors.SetSyntaxError(m_This,
                                              "Parsing error : ",
                                              "Uniform Section Error",
                                              false,
                                              LineFileErrors(vUniform->SourceLinePos, fromFile, "The Condition '" + vUniform->sectionCond + "' is incomplete"));
            }
        }
    }
}

void ShaderKey::SerializeUniformsAsConstToClipBoard(const GuiBackend_Window& vHandleForClipboard) {
    std::string res;

    for (auto itSection = puUniformSectionDataBase.begin(); itSection != puUniformSectionDataBase.end(); ++itSection) {
        std::string section = itSection->first;

        if (!itSection->second.empty()) {
            if (itSection != puUniformSectionDataBase.begin())
                res += "\n";

            res += "// " + section + "\n";
        }

        if (section != "hidden") {
            for (auto itLst = itSection->second.begin(); itLst != itSection->second.end(); ++itLst) {
                UniformVariantPtr uni = *itLst;

                if (uni && uni->canWeSave) {
                    res += UniformHelper::SerializeUniformAsConst(uni);
                }
            }
        }
    }

    /*for (auto it = puUniformsDataBase.begin(); it != puUniformsDataBase.end(); ++it)
    {
        UniformVariantPtr uni = it->second;

        if (uni)
        {
            res += UniformHelper::SerializeUniformAsConst(uni);
        }
    }*/

    if (!res.empty()) {
        // save to clipboard
        GuiBackend::Instance()->SetClipboardString(vHandleForClipboard, res.c_str());
    }
}

/// <summary>
/// will reset uniforms to their default value
/// </summary>
void ShaderKey::ResetUniformsToTheirDefaultValue() {
    if (puParentCodeTree) {
        for (const auto& itSection : puUniformSectionDataBase) {
            // const auto& section = itSection.first;

            for (auto uniPtr : itSection.second) {
                puParentCodeTree->ResetUniformWidgetToTheirDefaultValue(uniPtr);
            }
        }
    }
}

void ShaderKey::SerializeUniformsAsWidgetToClipBoard(const GuiBackend_Window& vHandleForClipboard) {
    std::string res;

    for (const auto& itSection : puUniformSectionDataBase) {
        const auto& section = itSection.first;

        if (!itSection.second.empty()) {
            if (section != puUniformSectionDataBase.begin()->first)
                res += "\n";

            res += "// " + section + "\n";
        }

        if (section != "hidden") {
            for (auto uniPtr : itSection.second) {
                if (uniPtr && uniPtr->canWeSave) {
                    res += UniformHelper::UniformHelper::SerializeUniformAsWidget(uniPtr);
                }
            }
        }
    }

    if (!res.empty()) {
        // save to clipboard
        GuiBackend::Instance()->SetClipboardString(vHandleForClipboard, res.c_str());
    }
}

void ShaderKey::SerializeUniformAsWidgetToClipBoard(const GuiBackend_Window& vHandleForClipboard, UniformVariantPtr vUniform) {
    const std::string res = UniformHelper::SerializeUniformAsWidget(vUniform);
    if (!res.empty()) {
        // save to clipboard
        GuiBackend::Instance()->SetClipboardString(vHandleForClipboard, res.c_str());
    }
}

std::string ShaderKey::GetAvailableUniformName(const std::string& vName) {
    std::string name = vName;

    int idx = 1;
    while (puUniformsDataBase.find(name) != puUniformsDataBase.end()) {
        name = vName + ct::toStr(idx);
        idx++;
    }

    return name;
}

void ShaderKey::CreateUniformsHeaderFromParsedStruct(const std::string& vShaderStageName,
                                                     const UniformParsedStruct& vUniformParsedStruct,
                                                     std::map<std::string, std::string>* vTempDico) {
    UniformParsedStruct uniformParsed = vUniformParsedStruct;

    if (uniformParsed.isOk() && vTempDico) {
        std::string uniformType = uniformParsed.type;
        std::string uniformParams = uniformParsed.params;
        std::string uniformName = uniformParsed.name;
        std::string uniformArray = uniformParsed.array;

        if (vTempDico->find(uniformName) == vTempDico->end())  // non found
        {
            uType::uTypeEnum glslType = uType::GetGlslTypeFromString(uniformType, !uniformArray.empty());

            int arrayCount = 1;

            if (glslType != uType::uTypeEnum::U_VOID) {
                arrayCount = uType::GetCountChannelForType(glslType);

                if (!uniformParams.empty()) {
                    std::vector<std::string> arr = ct::splitStringToVector(uniformParams, ':');

                    if (glslType == uType::uTypeEnum::U_SAMPLER2D_ARRAY || glslType == uType::uTypeEnum::U_SAMPLER3D_ARRAY ||
                        glslType == uType::uTypeEnum::U_FLOAT_ARRAY || glslType == uType::uTypeEnum::U_VEC2_ARRAY || glslType == uType::uTypeEnum::U_VEC3_ARRAY ||
                        glslType == uType::uTypeEnum::U_VEC4_ARRAY || glslType == uType::uTypeEnum::U_INT_ARRAY) {
                        // c'étais utile pour SdfFontDesigner car on pouvait faire des declarations de ce style
                        // uniform int countArr;
                        // uniform vec3 toto[countArr];
                        // mais countArr etait constant et defini par le program car ne supportait pas un resize durant le tuning
                        // on verra a le refaire plus tard
                        /*if (IsCustomWidgetName(glslType, uniformParams) || IsCustomWidgetName(glslType, arr[0]))
                        {
                            int count = GetArrayCountOfCustomWidgetName(uniformArray);
                            if (count > 0)
                            {
                                arrayCount = count;
                            }
                            else
                            {
                                count = ct::fvariant(uniformArray).GetI();
                                if (count <= 0)
                                    arrayCount = 1;
                                else
                                    arrayCount = count;
                            }
                        }*/
                    }
                }
            }

            (*vTempDico)[uniformName] = uniformName;

            // write uniform
            std::string uniformString;
            std::string typeStr = ConvertUniformsTypeEnumToString(glslType);
            if (glslType == uType::uTypeEnum::U_FLOAT_ARRAY || glslType == uType::uTypeEnum::U_VEC2_ARRAY || glslType == uType::uTypeEnum::U_VEC3_ARRAY ||
                glslType == uType::uTypeEnum::U_VEC4_ARRAY)
                uniformString = "uniform " + typeStr + " " + uniformName + "[" + ct::toStr(arrayCount) + "];";
            else
                uniformString = "uniform " + typeStr + " " + uniformName + ";";

            if (vUniformParsedStruct.notUploadableToGPU) {
                // m_NoUploadableUniformsName[uniformName] = typeStr;
                uniformString.clear();
            }

            puFinalUniformsCode[vShaderStageName][uniformName] = FinalUniformStruct(uniformString, uniformParsed);
        }
    }
}

// will fill puFinalUniformsCode and will not destroy uniforms
void ShaderKey::CreateUniformsHeader(const std::string& vInFileBufferName,
                                     const std::string& vStageName,
                                     const std::string& vSectionName,
                                     const std::string& vConfigName) {
    std::string emptyBuffer;
    std::string emptySection;
    std::string emptyConfig;

    const std::string& bufferName = vInFileBufferName;
    const std::string& stageName = vStageName;
    const std::string& sectionName = vSectionName;
    const std::string& configName = vConfigName;

    std::map<std::string, std::string> _TempUniformsDataBase;

    for (const auto& itBuffer : puUniformParsedDataBase) {
        if (itBuffer.first == bufferName || itBuffer.first == emptyBuffer)
        //! itBase->first->forcedSectionName.empty()) // s'il y a un forcedSectionName alors on le prend, peu importe si le nom de section n'est pas le meme)
        {
            for (const auto& itStage : itBuffer.second) {
                if (itStage.first == stageName ||
                    itStage.first == "UNIFORMS" ||  // pour les uniforms qui sont dans la section UNIFORMS et qui sont dont communs a toutes les autres sections
                    itStage.first == emptyConfig)
                //! itBase->first->forcedSectionName.empty()) // s'il y a un forcedSectionName alors on le prend, peu importe si le nom de section n'est pas le meme)
                {
                    for (const auto& itSection : itStage.second) {
                        if (itSection.first == sectionName || itSection.first == emptySection)
                        //! itBase->first->forcedSectionName.empty()) // s'il y a un forcedSectionName alors on le prend, peu importe si le nom de section n'est pas le
                        //! meme)
                        {
                            for (const auto& itConfig : itSection.second) {
                                if (itConfig.first == configName || itConfig.first == emptyConfig)
                                //! itBase->first->forcedSectionName.empty()) // s'il y a un forcedSectionName alors on le prend, peu importe si le nom de section n'est
                                //! pas le meme)
                                {
                                    for (const auto& itUni : itConfig.second) {
                                        CreateUniformsHeaderFromParsedStruct(stageName, itUni.second, &_TempUniformsDataBase);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /*
    // les virtuals => cree dans le nodegraph
    for (auto & itBuffer : m_VirtualUniformParsedDataBase)
    {
        if (itBuffer.first == bufferName || itBuffer.first == emptyBuffer)
            //!itBase->first->forcedSectionName.empty()) // s'il y a un forcedSectionName alors on le prend, peu importe si le nom de section n'est pas le meme)
        {
            for (auto & itSection : itBuffer.second)
            {
                if (itSection.first == sectionName || itSection.first == emptySection)
                    //!itBase->first->forcedSectionName.empty()) // s'il y a un forcedSectionName alors on le prend, peu importe si le nom de section n'est pas le meme)
                {
                    for (auto & itUni : itSection.second)
                    {
                        CreateUniformsHeaderFromParsedStruct(stageName, itUni.second, &m_TempUniformsDataBase);
                    }
                }
            }
        }
    }
    */
}

void ShaderKey::CreateUniforms(RenderPackWeak vRenderPack) {
    ClearUniforms(true);

    for (auto itType = puFinalUniformsCode.begin(); itType != puFinalUniformsCode.end(); ++itType) {
        for (auto itUni = itType->second.begin(); itUni != itType->second.end(); ++itUni) {
            if (puUniformsDataBase.find(itUni->first) == puUniformsDataBase.end())  // non found
            {
                UniformVariantPtr v = CreateUniform(vRenderPack, itUni->second.infos);

                AddUniform(v);
            }
        }
    }
}

UniformParsedStruct ShaderKey::GetUniformParsedStructByName(const std::string& vName) {
    UniformParsedStruct res;

    for (auto itType = puFinalUniformsCode.begin(); itType != puFinalUniformsCode.end(); ++itType) {
        for (auto itUni = itType->second.begin(); itUni != itType->second.end(); ++itUni) {
            if (vName == itUni->first)  // non found
            {
                return itUni->second.infos;
            }
        }
    }

    return res;
}

// remplace les PARSED_UNIFORM_NAME(uTime) par le formes finale genre : uniform float uTime;
std::string ShaderKey::getFinalUniformsCode(std::string vOriginalUniformsCode, const std::string& vSectionType) {
    std::string res;

    if (!vOriginalUniformsCode.empty()) {
        res = vOriginalUniformsCode;

        std::set<std::string> alreadyInsertedUniformNames;

        if (!puVirtualUniformParsedDataBase.empty()) {
            for (auto itUni = puVirtualUniformParsedDataBase[""].begin(); itUni != puVirtualUniformParsedDataBase[""].end(); ++itUni) {
                res += SectionCode::codeTags.UniformTemporaryTag + itUni->first + ")\n";
            }
        }

        size_t nextPos = 0;
        while ((nextPos = res.find(SectionCode::codeTags.UniformTemporaryTag, nextPos)) != std::string::npos) {
            const size_t firstParenthesis = nextPos + SectionCode::codeTags.UniformTemporaryTag.size();

            const size_t lastParenthesis = res.find(')', firstParenthesis);
            if (lastParenthesis != std::string::npos) {
                std::string uniformName = res.substr(firstParenthesis, lastParenthesis - firstParenthesis);

                if (puFinalUniformsCode[vSectionType].find(uniformName) != puFinalUniformsCode[vSectionType].end())  // found
                {
                    // on efface le tag
                    res.erase(nextPos, lastParenthesis + 1 - nextPos);

                    if (alreadyInsertedUniformNames.find(uniformName) == alreadyInsertedUniformNames.end())  // non found
                    {
                        alreadyInsertedUniformNames.emplace(uniformName);

                        // on insert le vrai code de l'uniform
                        std::string uniformToInsert = puFinalUniformsCode[vSectionType][uniformName].uniformString;

                        res.insert(nextPos, uniformToInsert);
                    } else  // deja inserer
                    {
                        // on va juste inserer le vrai code de l'uniform mais comment�
                        // pour garder une trace et le bon comptage des lignes
                        res.insert(nextPos, "//" + puFinalUniformsCode[vSectionType][uniformName].uniformString);
                    }
                }
            }

            nextPos++;
        }
    }

    return res;
}

UniformVariantPtr ShaderKey::GetUniformByName(const std::string& vName) {
    if (!vName.empty()) {
        if (puUniformsDataBase.find(vName) != puUniformsDataBase.end()) {
            return puUniformsDataBase[vName];
        }
    }
    return nullptr;
}

UniformVariantPtr ShaderKey::GetUsedUniformByName(const std::string& vName) {
    if (!vName.empty()) {
        if (puUniformsDataBase.find(vName) != puUniformsDataBase.end()) {
            UniformVariantPtr uni = puUniformsDataBase[vName];
            if (uni->loc != -1) {
                return uni;
            }
        }
    }
    return nullptr;
}

std::list<UniformVariantPtr>* ShaderKey::GetUniformsByWidget(const std::string& vWidget) {
    if (!vWidget.empty()) {
        if (puUniformWidgetsDataBase.find(vWidget) != puUniformWidgetsDataBase.end()) {
            return &puUniformWidgetsDataBase[vWidget];
        }
    }
    return nullptr;
}

std::list<UniformVariantPtr>* ShaderKey::GetUniformsBySection(const std::string& vSection) {
    if (!vSection.empty()) {
        if (puUniformSectionDataBase.find(vSection) != puUniformSectionDataBase.end()) {
            return &puUniformSectionDataBase[vSection];
        }
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::AddReplaceCode(const std::string& vShaderCodePartEnum, const ReplaceCodeStruct& vReplaceCodeStruct, bool vOnlyIfNotAlreadyDefined) {
    if (!vReplaceCodeStruct.name.empty()) {
        if (vOnlyIfNotAlreadyDefined) {
            auto res = GetReplaceCodeByName(vShaderCodePartEnum, vReplaceCodeStruct.name);
            if (!res.name.empty())  // found so quit
                return;
        }
        puReplaceCodesByName[vShaderCodePartEnum][vReplaceCodeStruct.name] = vReplaceCodeStruct;
    }
}

ReplaceCodeStruct ShaderKey::GetReplaceCodeByName(const std::string& vShaderCodePartEnum, const std::string& vName) {
    ReplaceCodeStruct res;

    if (puReplaceCodesByName.find(vShaderCodePartEnum) != puReplaceCodesByName.end())  // trouve
    {
        if (puReplaceCodesByName[vShaderCodePartEnum].find(vName) != puReplaceCodesByName[vShaderCodePartEnum].end())  // trouve
        {
            res = puReplaceCodesByName[vShaderCodePartEnum][vName];
        }
    }

    return res;
}

std::vector<ReplaceCodeStruct> ShaderKey::GetReplaceCodesByType(const std::string& vShaderCodePartEnum, const std::string& vType) {
    std::vector<ReplaceCodeStruct> res;

    if (puReplaceCodesByName.find(vShaderCodePartEnum) != puReplaceCodesByName.end())  // trouve
    {
        for (auto map : puReplaceCodesByName[vShaderCodePartEnum]) {
            if (map.second.type == vType) {
                res.push_back(map.second);
            }
        }
    }

    return res;
}

void ShaderKey::RemoveReplaceCode(const std::string& vShaderCodePartEnum, const std::string& vName) {
    if (puReplaceCodesByName.find(vShaderCodePartEnum) != puReplaceCodesByName.end())  // trouve
    {
        if (puReplaceCodesByName[vShaderCodePartEnum].find(vName) != puReplaceCodesByName[vShaderCodePartEnum].end())  // trouve
        {
            puReplaceCodesByName[vShaderCodePartEnum].erase(vName);
            if (puReplaceCodesByName[vShaderCodePartEnum].empty())
                puReplaceCodesByName.erase(vShaderCodePartEnum);
        }
    }
}

/// @brief set the content of a replace code
/// @param vShaderCodePartEnum shader stage
/// @param vName name of th replace code
/// @param vCodeToReplace the content code to set
/// @param vFromKey the origin shader key where the new code come from
/// @param vOnlyIfNotAlreadyDefined will be set only is not altready defined (so must be clear before set again)
/// @return true if new code can be inserted or if replace code name not found or if shader stage name not found. todo: to explan
bool ShaderKey::SetNewCodeInReplaceCodes(const std::string& vShaderCodePartEnum,
                                         const std::string& vName,
                                         const std::string& vCodeToReplace,
                                         std::string vNewCodeFromKey,
                                         bool vOnlyIfNotAlreadyDefined) {
    bool res = false;

    bool canwedefine = true;

    // on va checker le dico avec la bonne methode pour eviter d'ajouter un truc vide en voulant checker
    if (puReplaceCodesByName.find(vShaderCodePartEnum) != puReplaceCodesByName.end())  // found
    {
        if (puReplaceCodesByName[vShaderCodePartEnum].find(vName) != puReplaceCodesByName[vShaderCodePartEnum].end())  // found
        {
            if (puReplaceCodesByName[vShaderCodePartEnum][vName].newCode != vCodeToReplace) {
                res = true;
            }

            if (vOnlyIfNotAlreadyDefined) {
                canwedefine = false;
            }
        } else {
            res = true;
        }
    } else {
        res = true;
    }

    if (canwedefine) {
        puReplaceCodesByName[vShaderCodePartEnum][vName].newCode = vCodeToReplace;
        puReplaceCodesByName[vShaderCodePartEnum][vName].newCodeFromKey = vNewCodeFromKey;
    }

    return res;
}

void ShaderKey::ClearReplaceCodes() {
    puReplaceCodesByName.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ct::uvec3 => x:error line, y:start por in text, z:end pos in text
std::vector<ct::uvec3> ShaderKey::GetErrorLineNumbersFromErrorLine(const std::string& vLineToParse) {
    // ct::uvec3 => x:error line, y:start por in text, z:end pos in text
    std::vector<ct::uvec3> errorLines;
    size_t pos = 0;

    // https://stackoverflow.com/questions/25043861/parsing-glsl-error-messages
    // nvidia : <glShaderSource array index>(line) -> 0(33)
    // intel : <glShaderSource array index>:line: -> 0:33:
    // intel sur linux : <glShaderSource array index>:line(1): ->0:33(1):
    // ati : <glShaderSource array index>:line: -> 0:33:

    std::string startTag = "0(";
    std::string endTag = ")";

    if (vLineToParse.find(startTag, 0) == std::string::npos) {
        startTag = "0:";
        endTag = "(:";
    }

    while ((pos = vLineToParse.find(startTag, pos)) != std::string::npos) {
        pos += 2;
        const size_t lastPos = vLineToParse.find_first_of(endTag, pos);
        if (lastPos != std::string::npos) {
            std::string errorLineStr = vLineToParse.substr(pos, lastPos - pos);

            bool success = false;
            const auto errorLine = ct::uvariant(errorLineStr).GetU(&success);
            if (success) {
                // ct::uvec3 => x:error line, y:start por in text, z:end pos in text
                errorLines.emplace_back(ct::uvec3(errorLine, (uint32_t)(pos - 2U), (uint32_t)(lastPos + 1U)));
            } else {
                LogVarError("string to number fail at pos %u of %s", (uint32_t)pos, vLineToParse.c_str());
            }
        }
    }

    return errorLines;
}

ErrorLine ShaderKey::ParseErrorLine(const std::string& vSectionName, const std::string& vErrorLine) {
    ErrorLine res;

    if (vErrorLine.size() < 3)  // todo : a virer, c'est pas normal de devoir faire ca sur linux
        return res;

    auto locs = GetErrorLineNumbersFromErrorLine(vErrorLine);

    ErrorLineFragment frag;
    ErrorLineFragment errorFileLine;

    if (locs.size() == 1) {
        ct::uvec3 loc = locs[0];

        errorFileLine = ConvertLineFromTargetToSourceForShadertype(vSectionName, loc.x);

        errorFileLine.line = ct::maxi<size_t>(errorFileLine.line - 1, 0u);

        frag.file = errorFileLine.file;
        frag.line = errorFileLine.line;
        frag.error = "(" + ct::toStr(frag.line) + ")";
        // if (!frag.error.empty())
        {
            // res.fragments.push_back(frag); // le numero de ligne avec un lien de fichier

            // frag = ErrorLineFragment();
            frag.error += vErrorLine.substr(loc.z, vErrorLine.size() - loc.z);
            ct::replaceString(frag.error, "\"", "");
            ct::replaceString(frag.error, "\n", "");
            ct::replaceString(frag.error, "'", "");

            if (!frag.error.empty()) {
                res.fragments.push_back(frag);  // le reste de l'erreur
            }
        }
    } else if (locs.size() == 2) {
        ct::uvec3 loc0 = locs[0];
        errorFileLine = ConvertLineFromTargetToSourceForShadertype(vSectionName, loc0.x);

        frag.file = errorFileLine.file;
        frag.line = errorFileLine.line;
        frag.error = "(" + ct::toStr(frag.line) + ")";
        if (!frag.error.empty()) {
            res.fragments.push_back(frag);  // le numero de ligne avec un lien de fichier

            ct::uvec3 loc1 = locs[1];
            errorFileLine = ConvertLineFromTargetToSourceForShadertype(vSectionName, loc1.x);

            frag = ErrorLineFragment();
            frag.error = vErrorLine.substr(loc0.z, loc1.y - loc0.z);
            if (!frag.error.empty()) {
                res.fragments.push_back(frag);  // l'erreur suivante

                frag.file = errorFileLine.file;
                frag.line = errorFileLine.line;
                frag.error = "(" + ct::toStr(frag.line) + ")";
                if (!frag.error.empty()) {
                    res.fragments.push_back(frag);  // le numero de ligne avec un lien de fichier

                    frag = ErrorLineFragment();
                    frag.error = vErrorLine.substr(loc1.z, vErrorLine.size() - loc1.z);
                    if (!frag.error.empty()) {
                        res.fragments.push_back(frag);  // l'erreur suivante
                    }
                }
            }
        }
    } else  // failed to parse error line
    {
        errorFileLine = ConvertLineFromTargetToSourceForShadertype(vSectionName, 0);
        frag.file = errorFileLine.file;
        frag.line = errorFileLine.line;
        frag.error = vErrorLine;
        res.fragments.push_back(frag);  // le numero de ligne avec un lien de fichier
    }

    return res;
}

LineFileErrors ShaderKey::GetShaderErrorWithGoodLineNumbers(ShaderPtr vShader, ShaderTypeEnum vType, bool vErrorOrWarnings) {
    LineFileErrors res;

    if (vShader != nullptr) {
        std::string err;

        // error : 0(line) :
        if (vErrorOrWarnings)
            err = vShader->GetLastShaderErrorString(vType);
        else
            err = vShader->GetLastShaderWarningsString(vType);

        std::string sectionName;
        if (vType == ShaderTypeEnum::SHADER_TYPE_VERTEX)
            sectionName = GetShaderTypeEnumString(ShaderTypeEnum::SHADER_TYPE_VERTEX);
        if (vType == ShaderTypeEnum::SHADER_TYPE_GEOMETRY)
            sectionName = GetShaderTypeEnumString(ShaderTypeEnum::SHADER_TYPE_GEOMETRY);
        if (vType == ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL)
            sectionName = GetShaderTypeEnumString(ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL);
        if (vType == ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL)
            sectionName = GetShaderTypeEnumString(ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL);
        if (vType == ShaderTypeEnum::SHADER_TYPE_FRAGMENT)
            sectionName = GetShaderTypeEnumString(ShaderTypeEnum::SHADER_TYPE_FRAGMENT);
        if (vType == ShaderTypeEnum::SHADER_TYPE_COMPUTE)
            sectionName = GetShaderTypeEnumString(ShaderTypeEnum::SHADER_TYPE_COMPUTE);

        std::unordered_map<std::string, ShaderMsg> puKeyStatus;

        auto lines = ct::splitStringToVector(err, "\n");
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            auto err2 = ParseErrorLine(sectionName, *it);
            if (err2.fragments.size()) {
                auto frag = *err2.fragments.begin();
                if (!frag.file.empty()) {
                    if (vErrorOrWarnings)
                        puKeyStatus[frag.file] = ShaderMsg::SHADER_MSG_ERROR;
                    else
                        puKeyStatus[frag.file] = ShaderMsg::SHADER_MSG_WARNING;
                }

                std::string fileToApp = FileHelper::Instance()->GetPathRelativeToApp(frag.file);

                res.Set(frag.line, fileToApp, err2);
            }
        }

        for (auto it = puKeyStatus.begin(); it != puKeyStatus.end(); ++it) {
            puParentCodeTree->SetCompilationStatusForKey(it->first, it->second);
        }
    }

    return res;
}

ErrorLineFragment ShaderKey::ConvertLineFromTargetToSourceForShadertype(const std::string& vSectionName, size_t vLine) {
    ErrorLineFragment res;

    std::shared_ptr<SectionCode> sec = puMainSection->GetSectionCodeForTargetLine(vSectionName, vLine);
    if (sec) {
        const size_t sourceStartLine = sec->sourceCodeStartLine;
        const size_t finalStartLine = sec->finalCodeStartLine[vSectionName];
        const size_t finalEndLine = sec->finalCodeEndLine[vSectionName];

        if (finalEndLine >= finalStartLine) {
            res.line = vLine - finalStartLine + sourceStartLine;

            std::string fromFile = sec->relativeFile;
            if (!m_This->puInFileBufferFromKey.empty())
                fromFile = m_This->puInFileBufferFromKey;

            res.file = fromFile;

            // LogVarDebug("----------------------------------");
            // LogVarDebug("Found Shader File : %s", sec->relativeFile.c_str());
            // LogVarDebug("type : %s", sec->type.c_str());
            // LogVarDebug("Source Line : %zu", vLine);
            // LogVarDebug("Target Line : %zu", res.line);
        } else {
            LogVarDebugError("Shader Not Found : %s", sec->relativeFile.c_str());
        }
    }

    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::ClearConfigs() {
    puConfigNames.clear();
}

std::set<std::string>* ShaderKey::GetConfigNames(const std::string& vShaderStageName, const std::string& vInFileBufferName, const std::string& vSectionName) {
    std::set<std::string>* res = nullptr;

    if (puConfigNames.find(vShaderStageName) != puConfigNames.end())  // found
    {
        std::string inFile = vInFileBufferName;
        bool existing = puConfigNames[vShaderStageName].find(inFile) != puConfigNames[vShaderStageName].end();
        if (!existing) {
            inFile.clear();
            existing = puConfigNames[vShaderStageName].find(inFile) != puConfigNames[vShaderStageName].end();
        }

        if (existing)  // found
        {
            if (puConfigNames[vShaderStageName][inFile].find(vSectionName) != puConfigNames[vShaderStageName][inFile].end())  // found
            {
                res = &puConfigNames[vShaderStageName][inFile][vSectionName];
            } else if (puConfigNames[vShaderStageName][inFile].find("") != puConfigNames[vShaderStageName][inFile].end())  // found
            {
                res = &puConfigNames[vShaderStageName][inFile][""];
            }
        }
    }

    return res;
}

void ShaderKey::AddConfigName(const std::string& vShaderStageName,
                              const std::string& vInFileBufferName,
                              const std::string& vSectionName,
                              const std::string& vConfigName,
                              bool vPropagateToParents) {
    puConfigNames[vShaderStageName][vInFileBufferName][vSectionName].emplace(vConfigName);

    if (vPropagateToParents) {
        for (auto& usedKey : puUsedByKeys) {
            auto key = puParentCodeTree->GetKey(usedKey);  // .lock();
            if (key)
                key->AddConfigName(vShaderStageName, vInFileBufferName, vSectionName, vConfigName, vPropagateToParents);
        }
    }
}

void ShaderKey::PrepareConfigsComboBox(const std::string& vInFileBufferName) {
    puVertexConfigGuiStruct.PrepareConfigsComboBox(m_This, vInFileBufferName, puCurrentSectionName);
    puGeometryConfigGuiStruct.PrepareConfigsComboBox(m_This, vInFileBufferName, puCurrentSectionName);
    puTesselationControlConfigGuiStruct.PrepareConfigsComboBox(m_This, vInFileBufferName, puCurrentSectionName);
    puTesselationEvalConfigGuiStruct.PrepareConfigsComboBox(m_This, vInFileBufferName, puCurrentSectionName);
    puFragmentConfigGuiStruct.PrepareConfigsComboBox(m_This, vInFileBufferName, puCurrentSectionName);
    puComputeConfigGuiStruct.PrepareConfigsComboBox(m_This, vInFileBufferName, puCurrentSectionName);
    puSectionConfigGuiStruct.PrepareConfigsComboBox(m_This, vInFileBufferName, puCurrentSectionName);
}

bool ShaderKey::IsThereSomeConfigs() {
    return  // on fait les shaders les plus courant en 1er
        puVertexConfigGuiStruct.IsThereSomeConfigs() || puFragmentConfigGuiStruct.IsThereSomeConfigs() || puComputeConfigGuiStruct.IsThereSomeConfigs() ||
        puSectionConfigGuiStruct.IsThereSomeConfigs() || puGeometryConfigGuiStruct.IsThereSomeConfigs() || puTesselationControlConfigGuiStruct.IsThereSomeConfigs() ||
        puTesselationEvalConfigGuiStruct.IsThereSomeConfigs();
}

bool ShaderKey::DrawConfigComboBox(const std::string& vSectionType, const std::string& vInFileBufferName) {
    bool change = false;

    // if (puConfigVertexArrayCount || puConfigGeometryArrayCount || puConfigFragmentArrayCount || puConfigComputeArrayCount || puConfigSectionArrayCount)
    //	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Comfigs");

    // il faut mettre des else pour economiser du temps quand un gui a �t� utilis�
    // voir le type de code dans codeetree imguiuniformswidgets
    change |= puVertexConfigGuiStruct.DrawConfigComboBox(m_This, vSectionType, vInFileBufferName);
    change |= puGeometryConfigGuiStruct.DrawConfigComboBox(m_This, vSectionType, vInFileBufferName);
    change |= puTesselationControlConfigGuiStruct.DrawConfigComboBox(m_This, vSectionType, vInFileBufferName);
    change |= puTesselationEvalConfigGuiStruct.DrawConfigComboBox(m_This, vSectionType, vInFileBufferName);
    change |= puFragmentConfigGuiStruct.DrawConfigComboBox(m_This, vSectionType, vInFileBufferName);
    change |= puComputeConfigGuiStruct.DrawConfigComboBox(m_This, vSectionType, vInFileBufferName);
    change |= puSectionConfigGuiStruct.DrawConfigComboBox(m_This, vSectionType, vInFileBufferName);

    return change;
}

void ShaderKey::SelectConfigName(const std::string& vSectionType, const std::string& vInFileBufferName, const std::string& vName) {
    if (!vName.empty()) {
        const std::string sectionName = GetSelectedSectionName();

        // puCurrentConfigName = vName;
        puShaderConfigSelectedName[vSectionType][vInFileBufferName][sectionName] = vName;

        PrepareConfigsComboBox(vInFileBufferName);
    }
}

std::string ShaderKey::GetSelectedConfigName(const std::string& vSectionType, const std::string& vInFileBufferName) {
    if (puShaderConfigSelectedName.find(vSectionType) != puShaderConfigSelectedName.end())  // found
    {
        const std::string sectionName = GetSelectedSectionName();

        std::string inFile = vInFileBufferName;
        bool existing = puShaderConfigSelectedName[vSectionType].find(inFile) != puShaderConfigSelectedName[vSectionType].end();
        if (!existing) {
            inFile = "";
            existing = puShaderConfigSelectedName[vSectionType].find(inFile) != puShaderConfigSelectedName[vSectionType].end();
        }

        if (existing)  // trouve
        {
            if (puShaderConfigSelectedName[vSectionType][inFile].find(sectionName) != puShaderConfigSelectedName[vSectionType][inFile].end())  // found
            {
                return puShaderConfigSelectedName[vSectionType][inFile][sectionName];
            }
        }
    }
    return "";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::ClearShaderStageNames() {
    puShaderStagesNames.clear();
}

void ShaderKey::AddShaderStageName(const std::string& vShaderSectionName) {
    puShaderStagesNames.emplace(vShaderSectionName);
}

bool ShaderKey::IsShaderStageNameExist(const std::string& vShaderSectionName) {
    return (puShaderStagesNames.find(vShaderSectionName) != puShaderStagesNames.end());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::AddFragColorName(const uint8_t& vLocationID, const std::string& vName) {
    m_FragColorNames[vLocationID] = vName;
}

std::string ShaderKey::GetFragColorName(const uint8_t& vLocationID) const {
    if (m_FragColorNames.find(vLocationID) != m_FragColorNames.end()) {
        return m_FragColorNames.at(vLocationID);
    }
    return "";
}

void ShaderKey::ClearFragColorNames() {
    m_FragColorNames.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::ClearSections() {
    puSectionNames.clear();
}

std::set<std::string>* ShaderKey::GetSectionNames() {
    return &puSectionNames;
}

void ShaderKey::AddSectionName(const std::string& vSectionName, bool vPropagateToParents) {
    puSectionNames.emplace(vSectionName);

    if (vPropagateToParents) {
        for (auto& usedKey : puUsedByKeys) {
            auto key = puParentCodeTree->GetKey(usedKey);  // .lock();
            if (key)
                key->AddSectionName(vSectionName, vPropagateToParents);
        }
    }
}

void ShaderKey::PrepareSectionsComboBox() {
    puSectionArray.clear();
    puSectionArrayIndex = -1;

    auto sect = GetSectionNames();

    std::string selected = GetSelectedSectionName();
    if (selected.empty()) {
        if (!puSectionNames.empty()) {
            selected = *puSectionNames.begin();
            SelectSectionName(selected);
        }
    } else {
        if (puSectionNames.find(selected) == puSectionNames.end())  // non found
        {
            if (!puSectionNames.empty()) {
                selected = *puSectionNames.begin();
                SelectSectionName(selected);
            } else {
                selected.clear();
            }
        }
    }

    if (!sect->empty()) {
        for (const auto& sec : *sect) {
            if (sec == selected) {
                puSectionArrayIndex = (int)puSectionArray.size();
            }
            puSectionArray.push_back(sec);
        }

        if (selected.empty()) {
            SelectSectionName(((puSectionArrayIndex != -1) ? puSectionArray[puSectionArrayIndex] : puSectionArray[0]));
        }
    }
}

bool ShaderKey::IsThereSomeSectionConfigs() {
    return !puSectionArray.empty();
}

bool ShaderKey::DrawSectionComboBox() {
    bool change = false;

    if (!puSectionArray.empty()) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Sections");

        if (!puSectionArray.empty()) {
            if (puSectionArrayIndex < 0) {
                const std::string name = GetSelectedSectionName();
                ;
                puSectionArrayIndex = 0;
                for (size_t i = 0; i < puSectionArray.size(); i++) {
                    if (name == puSectionArray[i]) {
                        puSectionArrayIndex = (int)i;
                        break;
                    }
                }
            }

            if (ImGui::ContrastedComboVectorDefault(180, "Section", &puSectionArrayIndex, puSectionArray, (int)puSectionArray.size(), 0)) {
                change |= true;
                SelectSectionName(puSectionArray[puSectionArrayIndex]);
            }
        }
    }

    return change;
}

void ShaderKey::SelectSectionName(const std::string& vSectionName) {
    if (!vSectionName.empty()) {
        puCurrentSectionName = vSectionName;

        if (puParentCodeTree) {
            ShaderKeyPtr key = puParentCodeTree->GetParentkeyRecurs(puKey);
            if (key) {
                key->PrepareConfigsComboBox(puInFileBufferName);
            }
        }
    }
}

std::string ShaderKey::GetSelectedSectionName() {
    return puCurrentSectionName;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::AddBufferName(const std::string& vBufferName, bool vPropagateToParents) {
    puBufferNames.emplace(vBufferName);

    if (vPropagateToParents) {
        for (auto& usedKey : puUsedByKeys) {
            auto key = puParentCodeTree->GetKey(usedKey);  // .lock();
            if (key)
                key->AddBufferName(vBufferName, vPropagateToParents);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::LoadConfigShaderFile(const std::string& vShaderFileName, const CONFIG_TYPE_Enum& vConfigType, const std::string& vUniformConfigName) {
    std::string configFile = CodeTree::GetConfigFileName(vShaderFileName, vUniformConfigName);

    if (!configFile.empty()) {
        std::string file;

        std::ifstream docFile(configFile, std::ios::in);
        if (docFile.is_open()) {
            std::stringstream strStream;

            strStream << docFile.rdbuf();  // read the file

            file = strStream.str();
            ct::replaceString(file, "\r\n", "\n");

            docFile.close();
        }

        // LogVar("-------------------------------------------");
        // LogVar("Load Shader from Params File :" + configFile);

        std::vector<std::string> lines = ct::splitStringToVector(file, "\n");

        for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
            std::string line = *it;

            std::vector<std::string> arr = ct::splitStringToVector(line, ":");

            if (arr.size() > 1)  // two fields mini
            {
                std::string name = arr[0];

                if (vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_SHADER || vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_ALL) {
                    if (name == "Section") {
                        SelectSectionName(arr[1]);
                    } else if (name == "VertexConfig") {
                        SelectConfigName("VERTEX", puInFileBufferName, arr[1]);
                    } else if (name == "GeomConfig") {
                        SelectConfigName("GEOMETRY", puInFileBufferName, arr[1]);
                    } else if (name == "TessControlConfig") {
                        SelectConfigName("TESSCONTROL", puInFileBufferName, arr[1]);
                    } else if (name == "TessEvalConfig") {
                        SelectConfigName("TESSEVAL", puInFileBufferName, arr[1]);
                    } else if (name == "FragmentConfig") {
                        SelectConfigName("FRAGMENT", puInFileBufferName, arr[1]);
                    } else if (name == "ComputeConfig") {
                        SelectConfigName("COMPUTE", puInFileBufferName, arr[1]);
                    } else if (name == "SwitcherSelectedConfig") {
                        puConfigSwitcherSelectedConfig = arr[1];
                    } else {
                        puShaderGlobalSettings.LoadConfigLine(arr);
                    }
                }
                if (vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_UNIFORM || vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_ALL) {
                    UniformVariantPtr v = nullptr;

                    if (name == "UniformSection" && arr.size() > 2) {
                        puUniformSectionOpened[arr[1]] = ct::ivariant(arr[2]).GetB();
                    } else if (name == "UniformLocked" && arr.size() > 1) {
                        if (puUniformsDataBase.find(arr[1]) != puUniformsDataBase.end()) {  // found
                            puUniformsDataBase.at(arr[1])->lockedAgainstConfigLoading = true;
                        }
                    } else {
                        if (puUniformsDataBase.find(name) != puUniformsDataBase.end()) {  // found
                            v = puUniformsDataBase.at(name);
                        }

                        if (v != nullptr) {
                            UniformHelper::DeSerializeUniform(m_This, v, arr);

                            if (v->sectionName.empty())
                                CTOOL_DEBUG_BREAK;

                            if (puUniformSectionOpened.find(v->sectionName) == puUniformSectionOpened.end())  // non found
                            {
                                puUniformSectionOpened[v->sectionName] = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

void ShaderKey::SaveConfigShaderFile(std::string vShaderFileName, CONFIG_TYPE_Enum vConfigType, std::string vUniformConfigName) {
    if (!vShaderFileName.empty()) {
        std::string configFile = puParentCodeTree->GetConfigFileName(vShaderFileName, vUniformConfigName);

        if (!configFile.empty()) {
            std::string shaderStream;
            std::string uniformStream;
            std::string uniformSectionStream;
            std::string uniformLockedStream;

            if (vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_SHADER || vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_ALL) {
                // sections
                if (!puSectionArray.empty())
                    shaderStream += "Section:" + GetSelectedSectionName() + "\n";

                // configs
                if (puVertexConfigGuiStruct.IsThereSomeConfigs())
                    shaderStream += "VertexConfig:" + GetSelectedConfigName("VERTEX", puInFileBufferName) + "\n";
                if (puGeometryConfigGuiStruct.IsThereSomeConfigs())
                    shaderStream += "GeomConfig:" + GetSelectedConfigName("GEOMETRY", puInFileBufferName) + "\n";
                if (puTesselationControlConfigGuiStruct.IsThereSomeConfigs())
                    shaderStream += "TessControlConfig:" + GetSelectedConfigName("TESSCONTROL", puInFileBufferName) + "\n";
                if (puTesselationEvalConfigGuiStruct.IsThereSomeConfigs())
                    shaderStream += "TessEvalConfig:" + GetSelectedConfigName("TESSEVAL", puInFileBufferName) + "\n";
                if (puFragmentConfigGuiStruct.IsThereSomeConfigs())
                    shaderStream += "FragmentConfig:" + GetSelectedConfigName("FRAGMENT", puInFileBufferName) + "\n";
                if (puComputeConfigGuiStruct.IsThereSomeConfigs())
                    shaderStream += "ComputeConfig:" + GetSelectedConfigName("COMPUTE", puInFileBufferName) + "\n";

                // selected config in config switcher
                shaderStream += "SwitcherSelectedConfig:" + puConfigSwitcherSelectedConfig + "\n";

                // global settings
                shaderStream += puShaderGlobalSettings.getConfig();
            }

            if (vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_UNIFORM || vConfigType == CONFIG_TYPE_Enum::CONFIG_TYPE_ALL) {
                std::unordered_map<std::string, int> sectionUsed;

                // uniforms
                for (auto it = puUniformsDataBase.begin(); it != puUniformsDataBase.end(); ++it) {
                    if (it->second) {
                        // on vaut sauver que les uniforms qui ne sont que dans ce shader, donc pas utilise dans des includes
                        if (puParentCodeTree->puIncludeUniformNames.find(it->second->name) == puParentCodeTree->puIncludeUniformNames.end()) {  // non found
                            uniformStream += UniformHelper::SerializeUniform(it->second);
                            if (it->second->lockedAgainstConfigLoading) {
                                uniformLockedStream += "UniformLocked:" + it->second->name + "\n";
                            }
                            sectionUsed[it->second->sectionName] = 0;
                        }
                    }
                }

                // uniform sections
                for (auto itSection = puUniformSectionOpened.begin(); itSection != puUniformSectionOpened.end(); ++itSection) {
                    if (sectionUsed.find(itSection->first) != sectionUsed.end()) {
                        uniformSectionStream += "UniformSection:" + itSection->first + ":" + ct::toStr(itSection->second ? "true" : "false") + "\n";
                    }
                }
            }

            std::ofstream configFileWriter(configFile, std::ios::out);

            if (configFileWriter.bad() == false) {
                configFileWriter << uniformStream;
                configFileWriter << uniformSectionStream;  // toujours apres les uniforms pour faciliter l'init au chargement si pas found
                configFileWriter << uniformLockedStream;   // toujours apres les uniforms pour faciliter l'init au chargement si pas found
                configFileWriter << shaderStream;
                configFileWriter.close();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::LoadRenderPackConfig(CONFIG_TYPE_Enum vConfigType) {
    // puShaderKeyConfigSwitcher.LoadUniformConfigSummaryFile(puKey);
    LoadConfigIncludeFile("");
    LoadConfigShaderFile(puKey, vConfigType, "");
    LoadTimeLineConfigFile(puKey, "");
}

void ShaderKey::LoadConfigIncludeFile(std::string vUniformConfigName) {
    for (const auto& incFileName : puIncludeFileNames) {
        LoadConfigShaderFile(incFileName.first, CONFIG_TYPE_Enum::CONFIG_TYPE_UNIFORM, vUniformConfigName);
    }
}

void ShaderKey::SaveRenderPackConfig(CONFIG_TYPE_Enum vConfigType) {
    // puShaderKeyConfigSwitcher.SaveUniformConfigSummaryFile(puKey);
    puParentCodeTree->SaveConfigIncludeFiles();
    SaveConfigShaderFile(puKey, vConfigType, "");
    // SaveDependConfigIncludeFile("");
    SaveTimeLineConfigFile(puKey, "");
}

void ShaderKey::SaveDependConfigIncludeFile(std::string vUniformConfigName) {
    if (puParentCodeTree) {
        for (const auto& incFileName : puIncludeFileNames) {
            SaveConfigShaderFile(incFileName.first, CONFIG_TYPE_Enum::CONFIG_TYPE_UNIFORM, vUniformConfigName);
        }
    }
}

void ShaderKey::LoadTimeLineConfigFile(std::string vShaderFileName, std::string vUniformConfigName) {
    const std::string configFile = CodeTree::GetConfigFileName(vShaderFileName, vUniformConfigName, ".timeline");

    if (!configFile.empty()) {
        puTimeLine = TimeLineSystem::Instance()->LoadTimeLineConfig(configFile);
    }
}

void ShaderKey::SaveTimeLineConfigFile(std::string vShaderFileName, std::string vUniformConfigName) {
    if (!vShaderFileName.empty()) {
        const std::string configFile = puParentCodeTree->GetConfigFileName(vShaderFileName, vUniformConfigName, ".timeline");

        if (!configFile.empty()) {
            if (!TimeLineSystem::Instance()->SaveTimeLineConfig(configFile, puTimeLine)) {
                // le fichier ne sert plus, s'il existe il sera detruit
                FileHelper::Instance()->DestroyFile(configFile);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

SectionConfigStruct ShaderKey::GetSectionConfig(const std::string& vInFileBufferName) {
    SectionConfigStruct res;

    // pouruqoi on fait ca ?
    // ca insere une section ""
    res = puBufferSectionConfig[vInFileBufferName][""];

    const std::string currentSection = GetSelectedSectionName();
    if (currentSection != "" && puBufferSectionConfig[vInFileBufferName].find(currentSection) != puBufferSectionConfig[vInFileBufferName].end()) {
        puBufferSectionConfig[vInFileBufferName][currentSection].Complete(&res);
    }

    res.CheckChangeWith(&puLastSelectedSectionConfig);

    puLastSelectedSectionConfig = res;

    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::AddReplacingFuncName(ShaderTypeEnum vType, size_t vStart, size_t vEnd, std::string vOldFuncName, std::string vNewFuncNames) {
    std::string sectionName;

    if (vType == ShaderTypeEnum::SHADER_TYPE_VERTEX)
        sectionName = "VERTEX";
    if (vType == ShaderTypeEnum::SHADER_TYPE_GEOMETRY)
        sectionName = "GEOMETRY";
    if (vType == ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL)
        sectionName = "TESSCONTROL";
    if (vType == ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL)
        sectionName = "TESSEVAL";
    if (vType == ShaderTypeEnum::SHADER_TYPE_FRAGMENT)
        sectionName = "FRAGMENT";
    if (vType == ShaderTypeEnum::SHADER_TYPE_COMPUTE)
        sectionName = "COMPUTE";

    if (!sectionName.empty()) {
        ReplaceFuncNameStruct res;
        res.start = vStart;
        res.end = vEnd;
        res.oldName = vOldFuncName;
        res.newName = vNewFuncNames;

        puReplaceFuncNames[sectionName][vStart] = res;
    }
}

void ShaderKey::RemoveReplacingFuncName(ShaderTypeEnum vType, std::string vNewFuncNames) {
    std::string sectionName;

    if (vType == ShaderTypeEnum::SHADER_TYPE_VERTEX)
        sectionName = "VERTEX";
    if (vType == ShaderTypeEnum::SHADER_TYPE_GEOMETRY)
        sectionName = "GEOMETRY";
    if (vType == ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL)
        sectionName = "TESSCONTROL";
    if (vType == ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL)
        sectionName = "TESSEVAL";
    if (vType == ShaderTypeEnum::SHADER_TYPE_FRAGMENT)
        sectionName = "FRAGMENT";
    if (vType == ShaderTypeEnum::SHADER_TYPE_COMPUTE)
        sectionName = "COMPUTE";

    if (!sectionName.empty()) {
        std::vector<size_t> posToErase;

        for (auto it = puReplaceFuncNames[sectionName].begin(); it != puReplaceFuncNames[sectionName].end(); ++it) {
            if (it->second.newName == vNewFuncNames) {
                posToErase.emplace_back(it->first);
            }
        }

        for (auto it = posToErase.begin(); it != posToErase.end(); ++it) {
            puReplaceFuncNames[sectionName].erase(*it);
        }

        posToErase.clear();
    }
}

void ShaderKey::ClearReplacingFuncNames() {
    puReplaceFuncNames.clear();
}

std::string ShaderKey::GetPath() {
    if (puPath.empty()) {
        const auto p = FileHelper::Instance()->ParsePathFileName(puKey);
        if (p.isOk) {
            puPath = p.path;
        }
    }
    return puPath;
}
void ShaderKey::Setpath(std::string vPath) {
    puPath = vPath;
}

void ShaderKey::OpenFileKey() {
    if (!puMainSection->relativeFile.empty()) {
        // if (!puInFileBufferFromKey.empty())
        //	FileHelper::Instance()->OpenFile(puInFileBufferFromKey);
        // else
        FileHelper::Instance()->OpenFile(puMainSection->relativeFile);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

UniformVariantPtr ShaderKey::CreateUniform(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed) {
    UniformVariantPtr v = nullptr;

    const auto& sectionParams = vUniformParsed.sectionParams;
    const auto& uniformType = vUniformParsed.type;
    // const auto& uniformParams    = vUniformParsed.params;
    const auto& uniformName = vUniformParsed.name;
    const auto& uniformArray = vUniformParsed.array;
    auto uniformComment = vUniformParsed.comment;
    const size_t& sourceCodeLine = vUniformParsed.sourceCodeLine;

#ifdef DEBUG_UNIFORMS
    v = UniformVariant::create(m_This, uniformName, std::string(__FILE__) + "_" + std::string(__FUNCTION__));
#else
    v = UniformVariant::create(m_This, uniformName);
#endif

    if (!sectionParams.empty()) {
        v->sectionName = sectionParams;  // sectionName vaut "default", donc si sectionParams est vide pas besoin de mettre sectionName � 0
    }

    v->noExport = vUniformParsed.noExport;
    v->SourceLinePos = sourceCodeLine;
    v->glslType = uType::GetGlslTypeFromString(uniformType, !uniformArray.empty());

    if (v->glslType != uType::uTypeEnum::U_VOID) {
        switch (v->glslType) {
            case uType::uTypeEnum::U_FLOAT: Complete_Uniform_Float(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_VEC2: Complete_Uniform_Vec2(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_VEC3: Complete_Uniform_Vec3(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_VEC4: Complete_Uniform_Vec4(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_INT: Complete_Uniform_Int(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_IVEC2: Complete_Uniform_IVec2(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_IVEC3: Complete_Uniform_IVec3(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_IVEC4: Complete_Uniform_IVec4(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_UINT: Complete_Uniform_UInt(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_UVEC2: Complete_Uniform_UVec2(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_UVEC3: Complete_Uniform_UVec3(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_UVEC4: Complete_Uniform_UVec4(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_BOOL: Complete_Uniform_Bool(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_BVEC2: Complete_Uniform_BVec2(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_BVEC3: Complete_Uniform_BVec3(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_BVEC4: Complete_Uniform_BVec4(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_MAT2: Complete_Uniform_Mat2(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_MAT3: Complete_Uniform_Mat3(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_MAT4: Complete_Uniform_Mat4(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_TEXT: Complete_Uniform_Text(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_SAMPLER1D: Complete_Uniform_Sampler1D(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_SAMPLER2D: Complete_Uniform_Sampler2D(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_SAMPLER3D: Complete_Uniform_Sampler3D(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_SAMPLERCUBE: Complete_Uniform_SamplerCube(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_SAMPLER1D_ARRAY: Complete_Uniform_Sampler1D_Array(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_SAMPLER2D_ARRAY: Complete_Uniform_Sampler2D_Array(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_SAMPLER3D_ARRAY: Complete_Uniform_Sampler3D_Array(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_FLOAT_ARRAY: Complete_Uniform_Float_Array(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_VEC2_ARRAY: Complete_Uniform_Vec2_Array(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_VEC3_ARRAY: Complete_Uniform_Vec3_Array(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_VEC4_ARRAY: Complete_Uniform_Vec4_Array(vRenderPack, vUniformParsed, v); break;
            case uType::uTypeEnum::U_INT_ARRAY: Complete_Uniform_Int_Array(vRenderPack, vUniformParsed, v); break;
            default: break;
        }
    }

    if (!uniformComment.empty()) {
        ct::replaceString(uniformComment, "\\n", "\n");
        ct::replaceString(uniformComment, "\r", "");
        v->comment = uniformComment;
    }

    return v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::Complete_Uniform_Float(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 1;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "time") {
        Complete_Uniform_Time(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "deltatime") {
        Complete_Uniform_Shadertoy(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "checkbox") {
        Complete_Uniform_Checkbox(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "button") {
        Complete_Uniform_Button(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "usegeometry") {
        Complete_Uniform_Geometry(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "usetesscontrol") {
        Complete_Uniform_Tesselation(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "usetesseval") {
        Complete_Uniform_Tesselation(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "maxpoints") {
        Complete_Uniform_Vertex(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "maxinstances") {
        Complete_Uniform_Vertex(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "useculling") {
        GizmoSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "gamepad") {
        GamePadSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "midi") {
        MidiSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    }
#ifdef USE_VR
    else if (vUniform->widgetType == "vr") {
        VRBackend::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    }
#endif
    else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_Vec2(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 2;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "buffer") {
        Complete_Uniform_Buffer(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "checkbox") {
        Complete_Uniform_Checkbox(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "button") {
        Complete_Uniform_Button(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "radio") {
        Complete_Uniform_Radio(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "mouse") {
        Complete_Uniform_Mouse(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "compute") {
        Complete_Uniform_Compute(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "picture") {
        Complete_Uniform_Picture(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "midi") {
        MidiSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    }
#ifdef USE_VR
    else if (vUniform->widgetType == "vr") {
        VRBackend::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    }
#endif
    else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_Vec3(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 3;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "buffer") {
        Complete_Uniform_Buffer(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "checkbox") {
        Complete_Uniform_Checkbox(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "button") {
        Complete_Uniform_Button(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "radio") {
        Complete_Uniform_Radio(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "volume") {
        Complete_Uniform_Volume(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "compute") {
        Complete_Uniform_Compute(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "color") {
        Complete_Uniform_Color(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "gizmo") {
        GizmoSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "gamepad") {
        GamePadSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "midi") {
        MidiSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    }
#ifdef USE_VR
    else if (vUniform->widgetType == "vr") {
        VRBackend::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    }
#endif
    else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_Vec4(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 4;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "checkbox") {
        Complete_Uniform_Checkbox(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "button") {
        Complete_Uniform_Button(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "radio") {
        Complete_Uniform_Radio(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "mouse") {
        Complete_Uniform_Mouse(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "date") {
        Complete_Uniform_Shadertoy(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "gamepad") {
        GamePadSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "color")  // widget time
    {
        Complete_Uniform_Color(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "midi") {
        MidiSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    } else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_Int(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 1;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "combobox") {
        Complete_Uniform_Combobox(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "frame")  // widget deltatime
    {
        Complete_Uniform_Shadertoy(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "cullingtype") {
        GizmoSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    }
#ifdef USE_VR
    else if (vUniform->widgetType == "vr") {
        VRBackend::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    }
#endif
    else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_IVec2(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 2;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_IVec3(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 3;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_IVec4(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 4;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_UInt(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 1;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    }
#ifdef USE_VR
    else if (vUniform->widgetType == "vr") {
        VRBackend::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    }
#endif
    else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_UVec2(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 2;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_UVec3(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 3;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_UVec4(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 4;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_Bool(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 1;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "checkbox") {
        Complete_Uniform_Checkbox(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "button") {
        Complete_Uniform_Button(vRenderPack, vUniformParsed, vUniform);
    }
#ifdef USE_VR
    else if (vUniform->widgetType == "vr") {
        VRBackend::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    }
#endif
    else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_BVec2(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 2;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "checkbox") {
        Complete_Uniform_Checkbox(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "button") {
        Complete_Uniform_Button(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "radio") {
        Complete_Uniform_Radio(vRenderPack, vUniformParsed, vUniform);
    } else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_BVec3(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 3;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "checkbox") {
        Complete_Uniform_Checkbox(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "button") {
        Complete_Uniform_Button(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "radio") {
        Complete_Uniform_Radio(vRenderPack, vUniformParsed, vUniform);
    } else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_BVec4(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 4;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "checkbox") {
        Complete_Uniform_Checkbox(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "button") {
        Complete_Uniform_Button(vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "radio") {
        Complete_Uniform_Radio(vRenderPack, vUniformParsed, vUniform);
    } else {
        Complete_Uniform_Slider(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_Mat2(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 1;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else {
        vUniform->mat2 = glm::mat2(1.0f);
    }
}

void ShaderKey::Complete_Uniform_Mat3(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 1;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else {
        vUniform->mat3 = glm::mat3(1.0f);
    }
}

void ShaderKey::Complete_Uniform_Mat4(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->count = 1;
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "gizmo") {
        GizmoSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "culling") {
        GizmoSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "camera") {
        Complete_Uniform_Camera(vRenderPack, vUniformParsed, vUniform);
    } else {
        vUniform->mat4 = glm::mat4(1.0f);
    }
}

void ShaderKey::Complete_Uniform_Sampler1D(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "sound") {
        SoundSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    } else if (vUniform->widgetType == "mic") {
        SoundSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
    } else {
        Complete_Uniform_Picture(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_Sampler2D(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (!vUniformParsed.paramsArray.empty()) {
        if (vUniform->widgetType == "buffer") {
            Complete_Uniform_Buffer(vRenderPack, vUniformParsed, vUniform);
        } else if (vUniform->widgetType == "sound") {
            SoundSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
        } else if (vUniform->widgetType == "mic") {
            SoundSystem::Instance()->Complete_Uniform(m_This, vRenderPack, vUniformParsed, vUniform);
        } else if (vUniform->widgetType == "picture") {
            Complete_Uniform_Picture(vRenderPack, vUniformParsed, vUniform);
        } else if (vUniform->widgetType == "depth") {
            Complete_Uniform_Depth(vRenderPack, vUniformParsed, vUniform);
        } else if (vUniform->widgetType == "compute") {
            Complete_Uniform_Compute(vRenderPack, vUniformParsed, vUniform);
        }
    } else {
        Complete_Uniform_Picture(vRenderPack, vUniformParsed, vUniform);
    }
}

void ShaderKey::Complete_Uniform_Sampler3D(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (!vUniformParsed.paramsArray.empty()) {
        if (vUniform->widgetType == "compute") {
            Complete_Uniform_Compute(vRenderPack, vUniformParsed, vUniform);
        } else if (vUniform->widgetType == "volume") {
            Complete_Uniform_Volume(vRenderPack, vUniformParsed, vUniform);
        }
    }
}

void ShaderKey::Complete_Uniform_SamplerCube(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
    } else if (vUniform->widgetType == "cubemap") {
        Complete_Uniform_CubeMap(vRenderPack, vUniformParsed, vUniform);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::Complete_Uniform_Float_Array(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
        int count = GetArrayCountOfCustomWidgetName(vUniformParsed.array);
        if (count > 0) {
            vUniform->count = count;
        } else {
            count = ct::fvariant(vUniformParsed.array).GetI();
            if (count <= 0)
                vUniform->count = 1;
            else
                vUniform->count = count;
        }
    }
}

void ShaderKey::Complete_Uniform_Int_Array(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
        int count = GetArrayCountOfCustomWidgetName(vUniformParsed.array);
        if (count > 0) {
            vUniform->count = count;
        } else {
            count = ct::fvariant(vUniformParsed.array).GetI();
            if (count <= 0)
                vUniform->count = 1;
            else
                vUniform->count = count;
        }
    }
}

void ShaderKey::Complete_Uniform_Vec2_Array(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
        int count = GetArrayCountOfCustomWidgetName(vUniformParsed.array);
        if (count > 0) {
            vUniform->count = count;
        } else {
            count = ct::fvariant(vUniformParsed.array).GetI();
            if (count <= 0)
                vUniform->count = 1;
            else
                vUniform->count = count;
        }
    }
}

void ShaderKey::Complete_Uniform_Vec3_Array(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
        int count = GetArrayCountOfCustomWidgetName(vUniformParsed.array);
        if (count > 0) {
            vUniform->count = count;
        } else {
            count = ct::fvariant(vUniformParsed.array).GetI();
            if (count <= 0)
                vUniform->count = 1;
            else
                vUniform->count = count;
        }
    }
}

void ShaderKey::Complete_Uniform_Vec4_Array(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
        int count = GetArrayCountOfCustomWidgetName(vUniformParsed.array);
        if (count > 0) {
            vUniform->count = count;
        } else {
            count = ct::fvariant(vUniformParsed.array).GetI();
            if (count <= 0)
                vUniform->count = 1;
            else
                vUniform->count = count;
        }
    }
}

void ShaderKey::Complete_Uniform_Mat4_Array(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
        int count = GetArrayCountOfCustomWidgetName(vUniformParsed.array);
        if (count > 0) {
            vUniform->count = count;
        } else {
            count = ct::fvariant(vUniformParsed.array).GetI();
            if (count <= 0)
                vUniform->count = 1;
            else
                vUniform->count = count;
        }
    }
}

void ShaderKey::Complete_Uniform_Sampler1D_Array(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
        int count = GetArrayCountOfCustomWidgetName(vUniformParsed.array);
        if (count > 0) {
            vUniform->count = count;
        } else {
            count = ct::fvariant(vUniformParsed.array).GetI();
            if (count <= 0)
                vUniform->count = 1;
            else
                vUniform->count = count;
        }
    }
}

void ShaderKey::Complete_Uniform_Sampler2D_Array(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
        int count = GetArrayCountOfCustomWidgetName(vUniformParsed.array);
        if (count > 0) {
            vUniform->count = count;
        } else {
            count = ct::fvariant(vUniformParsed.array).GetI();
            if (count <= 0)
                vUniform->count = 1;
            else
                vUniform->count = count;
        }
    }
}

void ShaderKey::Complete_Uniform_Sampler3D_Array(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
        int count = GetArrayCountOfCustomWidgetName(vUniformParsed.array);
        if (count > 0) {
            vUniform->count = count;
        } else {
            count = ct::fvariant(vUniformParsed.array).GetI();
            if (count <= 0)
                vUniform->count = 1;
            else
                vUniform->count = count;
        }
    }
}

void ShaderKey::Complete_Uniform_SamplerCube_Array(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (!vUniformParsed.paramsArray.empty())
        vUniform->widgetType = vUniformParsed.paramsArray[0];
    if (IsCustomWidgetName(vUniform->glslType, vUniformParsed.params) || IsCustomWidgetName(vUniform->glslType, vUniform->widgetType)) {
        vUniform->widget = vUniformParsed.params;
        vUniform->constant = true;
        int count = GetArrayCountOfCustomWidgetName(vUniformParsed.array);
        if (count > 0) {
            vUniform->count = count;
        } else {
            count = ct::fvariant(vUniformParsed.array).GetI();
            if (count <= 0)
                vUniform->count = 1;
            else
                vUniform->count = count;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderKey::Complete_Uniform_VR(RenderPackWeak vRenderPack, const UniformParsedStruct& /*vUniformParsed*/, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;
}

void ShaderKey::Complete_Uniform_Vertex(RenderPackWeak vRenderPack, const UniformParsedStruct& /*vUniformParsed*/, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    if (vUniform->widgetType == "maxpoints")  // checkbox
    {
        vUniform->widget = "maxpoints";
        vUniform->x = 0.0f;

        auto rpPtr = vRenderPack.lock();
        if (rpPtr) {
            auto modelPtr = rpPtr->GetModel().lock();
            if (modelPtr) {
                if (modelPtr->GetMeshType() == BaseMeshEnum::PRIMITIVE_TYPE_POINTS) {
                    vUniform->x = (float)modelPtr->GetVerticesCount();
                }
            }
        }
    } else if (vUniform->widgetType == "maxinstances")  // checkbox
    {
        vUniform->widget = "maxinstances";
        vUniform->x = 0.0f;
        auto rpPtr = vRenderPack.lock();
        if (rpPtr) {
            auto modelPtr = rpPtr->GetModel().lock();
            if (modelPtr) {
                vUniform->x = (float)modelPtr->GetInstancesCount();
            }
        }
    }
}
void ShaderKey::Complete_Uniform_Geometry(RenderPackWeak vRenderPack, const UniformParsedStruct& /*vUniformParsed*/, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    if (vUniform->widgetType == "usegeometry")  // use geometry
    {
        vUniform->widget = "usegeometry";
        vUniform->bx = puShaderGlobalSettings.useGeometryShaderIfPresent && puIsGeometryShaderPresent;
    }
}
void ShaderKey::Complete_Uniform_Tesselation(RenderPackWeak vRenderPack, const UniformParsedStruct& /*vUniformParsed*/, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    if (vUniform->widgetType == "usetesscontrol")  // use tesselation control
    {
        vUniform->widget = "usetesscontrol";
        vUniform->bx = puShaderGlobalSettings.useTesselationShaderIfPresent && puIsTesselationControlShaderPresent;
    } else if (vUniform->widgetType == "usetesseval")  // use tesselation eval
    {
        vUniform->widget = "usetesseval";
        vUniform->bx = puShaderGlobalSettings.useTesselationShaderIfPresent && puIsTesselationEvalShaderPresent;
    }
}

void ShaderKey::Complete_Uniform_Shadertoy(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    if (vUniform->widgetType == "deltatime")  // widget deltatime
    {
        // evite le listing de cet uniform sauf si un widget dedie est prevu
        vUniform->widget = "deltatime";
        vUniform->x = 0.0f;
    } else if (vUniform->widgetType == "date") {
        vUniform->widget = vUniformParsed.params;
        vUniform->x = 0.0f;
        vUniform->y = 0.0f;
        vUniform->z = 0.0f;
        vUniform->w = 0.0f;
    } else if (vUniform->widgetType == "frame")  // widget deltatime
    {
        // evite le listing de cet uniform sauf si un widget d�di� est pr�vu
        vUniform->widget = vUniform->widgetType;
        vUniform->ix = 0;
    }
}

void ShaderKey::Complete_Uniform_Camera(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    vUniform->widget = vUniformParsed.params;
    if (vUniformParsed.paramsArray.size() > 1) {
        if (vUniformParsed.paramsArray[1] == "mvp") {
            vUniform->widget = "camera:mvp";
            if (vUniformParsed.paramsArray.size() > 2) {
                vUniform->camDefRotXY = ct::fvec2(vUniformParsed.paramsArray[2], ',');
                vUniform->camRotXY = vUniform->camDefRotXY;
            }
            if (vUniformParsed.paramsArray.size() > 3) {
                vUniform->camDefZoom = ct::fvariant(vUniformParsed.paramsArray[3]).GetF();
                vUniform->camZoom = vUniform->camDefZoom;
            }
            if (vUniformParsed.paramsArray.size() > 4) {
                vUniform->camDefTranslateXYZ = ct::fvec3(vUniformParsed.paramsArray[4], ',');
                vUniform->camTranslateXYZ = vUniform->camDefTranslateXYZ;
            }
        }
    }
}

void ShaderKey::Complete_Uniform_Mouse(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    vUniform->widget = vUniformParsed.params;
    vUniform->x = 0.0f;
    vUniform->y = 0.0f;
    vUniform->z = 0.0f;
    vUniform->w = 0.0f;
    vUniform->timeLineSupported = true;
}

void ShaderKey::Complete_Uniform_Slider(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    vUniform->timeLineSupported = true;
    vUniform->count = uType::GetCountChannelForType(vUniform->glslType);

    if (!vUniformParsed.params.empty()) {
        std::vector<std::string> arr = ct::splitStringToVector(vUniformParsed.params, ":");
        if (vUniform->count)  // slider
        {
            // si n = 4 et la string vaut 0.5,0.2, on aura xy=0.5 et zw = 0.2, c'est juste pour ca qu'on fait ca dessous
            if (!arr.empty())  // inf
                vUniform->inf = ct::fvec4(arr[0], ',', vUniform->count);
            if (arr.size() > 1)  // sup
                vUniform->sup = ct::fvec4(arr[1], ',', vUniform->count);
            if (arr.size() > 2)  // default
                vUniform->def = ct::fvec4(arr[2], ',', vUniform->count);
            if (arr.size() > 3)  // step
                vUniform->step = ct::fvec4(arr[3], ',', vUniform->count);
        }
    }

    switch (vUniform->glslType) {
        case uType::uTypeEnum::U_BOOL:
        case uType::uTypeEnum::U_BVEC2:
        case uType::uTypeEnum::U_BVEC3:
        case uType::uTypeEnum::U_BVEC4:
            vUniform->bx = (bool)vUniform->def.x;
            vUniform->by = (bool)vUniform->def.y;
            vUniform->bz = (bool)vUniform->def.z;
            vUniform->bw = (bool)vUniform->def.w;
            break;

        case uType::uTypeEnum::U_FLOAT:
        case uType::uTypeEnum::U_VEC2:
        case uType::uTypeEnum::U_VEC3:
        case uType::uTypeEnum::U_VEC4:
            vUniform->x = vUniform->def.x;
            vUniform->y = vUniform->def.y;
            vUniform->z = vUniform->def.z;
            vUniform->w = vUniform->def.w;
            break;

        case uType::uTypeEnum::U_INT:
        case uType::uTypeEnum::U_IVEC2:
        case uType::uTypeEnum::U_IVEC3:
        case uType::uTypeEnum::U_IVEC4:
            vUniform->ix = (int32_t)vUniform->def.x;
            vUniform->iy = (int32_t)vUniform->def.y;
            vUniform->iz = (int32_t)vUniform->def.z;
            vUniform->iw = (int32_t)vUniform->def.w;
            break;

        case uType::uTypeEnum::U_UINT:
        case uType::uTypeEnum::U_UVEC2:
        case uType::uTypeEnum::U_UVEC3:
        case uType::uTypeEnum::U_UVEC4:
            vUniform->ux = (uint32_t)vUniform->def.x;
            vUniform->uy = (uint32_t)vUniform->def.y;
            vUniform->uz = (uint32_t)vUniform->def.z;
            vUniform->uw = (uint32_t)vUniform->def.w;
            break;

        default: break;
    }
}

void ShaderKey::Complete_Uniform_Color(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    vUniform->widget = "color";
    vUniform->timeLineSupported = true;

    if (vUniform->glslType == uType::uTypeEnum::U_VEC3) {
        if (vUniformParsed.paramsArray.size() > 1) {
            vUniform->def = ct::fvec4(vUniformParsed.paramsArray[1], ',', vUniform->count);
            vUniform->x = vUniform->def.x;
            vUniform->y = vUniform->def.y;
            vUniform->z = vUniform->def.z;
        }
    } else if (vUniform->glslType == uType::uTypeEnum::U_VEC4) {
        if (vUniformParsed.paramsArray.size() > 1) {
            vUniform->def = ct::fvec4(vUniformParsed.paramsArray[1], ',', vUniform->count);
            vUniform->x = vUniform->def.x;
            vUniform->y = vUniform->def.y;
            vUniform->z = vUniform->def.z;
            vUniform->w = vUniform->def.w;
        }
    }
}

void ShaderKey::Complete_Uniform_Time(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    vUniform->widget = "time";
    vUniform->timeLineSupported = true;

    if (vUniform->glslType == uType::uTypeEnum::U_FLOAT) {
        for (size_t i = 1; i < vUniformParsed.paramsArray.size(); ++i) {
            auto& valStr = vUniformParsed.paramsArray[i];
            if (valStr == "true") {
                vUniform->bx = true;
                vUniform->def.x = 1.0f;
            } else if (valStr == "false") {
                vUniform->bx = false;
                vUniform->def.x = 0.0f;
            } else {
                // def.y contain the period
                vUniform->def.y = std::abs(ct::fvariant(valStr).GetF());
            }
        }
    }
}

void ShaderKey::Complete_Uniform_Text(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    vUniform->widget = "text";

    if (vUniform->glslType == uType::uTypeEnum::U_TEXT) {
        vUniform->text = vUniformParsed.originalParams;
        ct::replaceString(vUniform->text, "\\n", "\n");
        vUniform->uiOnly = true;
    }
}

void ShaderKey::Complete_Uniform_Radio(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    vUniform->widget = "radio";
    vUniform->timeLineSupported = true;

    if (vUniform->glslType == uType::uTypeEnum::U_VEC2 || vUniform->glslType == uType::uTypeEnum::U_BVEC2) {
        vUniform->bx = false;
        vUniform->by = false;
        if (vUniformParsed.paramsArray.size() == 2) {
            const int i = ct::fvariant(vUniformParsed.paramsArray[1]).GetI();
            if (i == 0)
                vUniform->bx = true;
            if (i == 1)
                vUniform->by = true;
        } else {
            vUniform->bx = true;
        }
        vUniform->bdef.x = vUniform->bx;
        vUniform->bdef.y = vUniform->by;
    } else if (vUniform->glslType == uType::uTypeEnum::U_VEC3 || vUniform->glslType == uType::uTypeEnum::U_BVEC3) {
        vUniform->bx = false;
        vUniform->by = false;
        vUniform->bz = false;
        if (vUniformParsed.paramsArray.size() == 2) {
            const int i = ct::fvariant(vUniformParsed.paramsArray[1]).GetI();
            if (i == 0)
                vUniform->bx = true;
            if (i == 1)
                vUniform->by = true;
            if (i == 2)
                vUniform->bz = true;
        } else {
            vUniform->bx = true;
        }
        vUniform->bdef.x = vUniform->bx;
        vUniform->bdef.y = vUniform->by;
        vUniform->bdef.z = vUniform->bz;
    } else if (vUniform->glslType == uType::uTypeEnum::U_VEC4 || vUniform->glslType == uType::uTypeEnum::U_BVEC4) {
        vUniform->bx = false;
        vUniform->by = false;
        vUniform->bz = false;
        vUniform->bw = false;
        if (vUniformParsed.paramsArray.size() == 2) {
            const int i = ct::fvariant(vUniformParsed.paramsArray[1]).GetI();
            if (i == 0)
                vUniform->bx = true;
            if (i == 1)
                vUniform->by = true;
            if (i == 2)
                vUniform->bz = true;
            if (i == 3)
                vUniform->bw = true;
        } else {
            vUniform->bx = true;
        }
        vUniform->bdef.x = vUniform->bx;
        vUniform->bdef.y = vUniform->by;
        vUniform->bdef.z = vUniform->bz;
        vUniform->bdef.w = vUniform->bw;
    }
}

void ShaderKey::Complete_Uniform_Button(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    vUniform->widget = "button";
    vUniform->timeLineSupported = false;
    vUniform->canWeSave = false;

    vUniform->bx = false;
    vUniform->by = false;
    vUniform->bz = false;
    vUniform->bw = false;

    if (vUniform->glslType == uType::uTypeEnum::U_FLOAT || vUniform->glslType == uType::uTypeEnum::U_BOOL) {
        if (vUniformParsed.originalParamsArray.size() == 2) {
            auto arr = ct::splitStringToVector(vUniformParsed.originalParamsArray[1], ',');
            if (!arr.empty())
                vUniform->buttonName0 = arr[0];
        }
    } else if (vUniform->glslType == uType::uTypeEnum::U_VEC2 || vUniform->glslType == uType::uTypeEnum::U_BVEC2) {
        if (vUniformParsed.originalParamsArray.size() == 2) {
            auto arr = ct::splitStringToVector(vUniformParsed.originalParamsArray[1], ',');
            if (!arr.empty())
                vUniform->buttonName0 = arr[0];
            if (arr.size() > 1)
                vUniform->buttonName1 = arr[1];
        }
    } else if (vUniform->glslType == uType::uTypeEnum::U_VEC3 || vUniform->glslType == uType::uTypeEnum::U_BVEC3) {
        if (vUniformParsed.originalParamsArray.size() == 2) {
            auto arr = ct::splitStringToVector(vUniformParsed.originalParamsArray[1], ',');
            if (!arr.empty())
                vUniform->buttonName0 = arr[0];
            if (arr.size() > 1)
                vUniform->buttonName1 = arr[1];
            if (arr.size() > 2)
                vUniform->buttonName2 = arr[2];
        }
    } else if (vUniform->glslType == uType::uTypeEnum::U_VEC4 || vUniform->glslType == uType::uTypeEnum::U_BVEC4) {
        if (vUniformParsed.originalParamsArray.size() == 2) {
            auto arr = ct::splitStringToVector(vUniformParsed.originalParamsArray[1], ',');
            if (!arr.empty())
                vUniform->buttonName0 = arr[0];
            if (arr.size() > 1)
                vUniform->buttonName1 = arr[1];
            if (arr.size() > 2)
                vUniform->buttonName2 = arr[2];
            if (arr.size() > 3)
                vUniform->buttonName3 = arr[3];
        }
    }
}
void ShaderKey::Complete_Uniform_Combobox(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    vUniform->widget = "combobox";
    if (vUniformParsed.originalParamsArray.size() > 1) {
        // std::string str;
        // size_t len = 0;
        vUniform->choices = ct::splitStringToVector(vUniformParsed.originalParamsArray[1], ",");
        vUniform->ix = 0;
    }
    if (vUniformParsed.originalParamsArray.size() > 2) {
        const std::string key = vUniformParsed.originalParamsArray[2];

        bool found = false;
        int idx = 0;
        for (auto it = vUniform->choices.begin(); it != vUniform->choices.end(); ++it) {
            if (*it == key) {
                found = true;
                break;
            }

            idx++;
        }
        if (found) {
            vUniform->def.x = (float)idx;
            vUniform->ix = idx;
        }
    }
    vUniform->timeLineSupported = true;
}

void ShaderKey::Complete_Uniform_Checkbox(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    vUniform->widget = "checkbox";
    vUniform->timeLineSupported = true;

    if (vUniform->glslType == uType::uTypeEnum::U_FLOAT || vUniform->glslType == uType::uTypeEnum::U_BOOL) {
        vUniform->bx = false;
        if (vUniformParsed.paramsArray.size() == 2)  // vec3 or bvec3(checkbox:false)
        {
            auto arr = ct::splitStringToVector(vUniformParsed.paramsArray[1], ',');
            if (!arr.empty())
                vUniform->bx = ct::ivariant(arr[0]).GetB();
        }
        vUniform->bdef.x = vUniform->bx;
    } else if (vUniform->glslType == uType::uTypeEnum::U_VEC2 || vUniform->glslType == uType::uTypeEnum::U_BVEC2) {
        vUniform->bx = false;
        vUniform->by = false;
        if (vUniformParsed.paramsArray.size() == 2)  // vec3 or bvec3(checkbox:false)
        {
            auto arr = ct::splitStringToVector(vUniformParsed.paramsArray[1], ',');
            if (!arr.empty())
                vUniform->bx = ct::ivariant(arr[0]).GetB();
            if (arr.size() > 1)
                vUniform->by = ct::ivariant(arr[1]).GetB();
        }
        vUniform->bdef.x = vUniform->bx;
        vUniform->bdef.y = vUniform->by;
    } else if (vUniform->glslType == uType::uTypeEnum::U_VEC3 || vUniform->glslType == uType::uTypeEnum::U_BVEC3) {
        vUniform->bx = false;
        vUniform->by = false;
        vUniform->bz = false;
        if (vUniformParsed.paramsArray.size() == 2)  // vec3 or bvec3(checkbox:false)
        {
            auto arr = ct::splitStringToVector(vUniformParsed.paramsArray[1], ',');
            if (!arr.empty())
                vUniform->bx = ct::ivariant(arr[0]).GetB();
            if (arr.size() > 1)
                vUniform->by = ct::ivariant(arr[1]).GetB();
            if (arr.size() > 2)
                vUniform->bz = ct::ivariant(arr[2]).GetB();
        }
        vUniform->bdef.x = vUniform->bx;
        vUniform->bdef.y = vUniform->by;
        vUniform->bdef.z = vUniform->bz;
    } else if (vUniform->glslType == uType::uTypeEnum::U_VEC4 || vUniform->glslType == uType::uTypeEnum::U_BVEC4) {
        vUniform->bx = false;
        vUniform->by = false;
        vUniform->bz = false;
        vUniform->bw = false;
        if (vUniformParsed.paramsArray.size() == 2)  // vec4 or bvec4(checkbox:false)
        {
            auto arr = ct::splitStringToVector(vUniformParsed.paramsArray[1], ',');
            if (!arr.empty())
                vUniform->bx = ct::ivariant(arr[0]).GetB();
            if (arr.size() > 1)
                vUniform->by = ct::ivariant(arr[1]).GetB();
            if (arr.size() > 2)
                vUniform->bz = ct::ivariant(arr[2]).GetB();
            if (arr.size() > 3)
                vUniform->bw = ct::ivariant(arr[3]).GetB();
        }
        vUniform->bdef.x = vUniform->bx;
        vUniform->bdef.y = vUniform->by;
        vUniform->bdef.z = vUniform->bz;
        vUniform->bdef.w = vUniform->bw;
    }
}

void ShaderKey::Complete_Uniform_Buffer(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    if (vRenderPack.expired() || !vUniform)
        return;

    vUniform->widget = "buffer";
    vUniform->attachment = 0;

    const bool bufferConfigExist = false;

    for (auto it = vUniformParsed.paramsDico.begin(); it != vUniformParsed.paramsDico.end(); ++it) {
        std::string key = it->first;

        if (key == "target") {
            if (it->second.size() == 1) {
                vUniform->attachment = ct::ivariant(*it->second.begin()).GetI();
            }
        }
        if (key == "choice") {
            if (it->second.size() == 1) {
                vUniform->bufferShaderName = *it->second.begin();
                auto rpPtr = vRenderPack.lock();
                if (rpPtr) {
                    if (rpPtr->puName == vUniform->bufferShaderName)
                        vUniform->bufferShaderName.clear();
                }
            }

            // une choose box a afficher
            vUniform->bufferChoiceActivated = true;
        }
        if (key == "buffer") {
            if (it->second.size() == 1) {
                vUniform->inFileBufferName = *it->second.begin();
                vUniform->bufferShaderName = puKey;
                auto rpPtr = vRenderPack.lock();
                if (rpPtr) {
                    if (rpPtr->GetShaderKey()) {
                        if (!rpPtr->GetShaderKey()->puInFileBufferFromKey.empty()) {
                            vUniform->bufferShaderName = rpPtr->GetShaderKey()->puInFileBufferFromKey;
                        }
                    }
                    if (rpPtr->puName == vUniform->inFileBufferName)
                        vUniform->inFileBufferName.clear();
                }
            }
        }
        if (key == "file") {
            if (it->second.size() == 1) {
                vUniform->bufferShaderName = *it->second.begin();

                const size_t fileTag = vUniformParsed.originalParams.find("file");
                if (fileTag != std::string::npos) {
                    const size_t equalTag = vUniformParsed.originalParams.find('=', fileTag + 1);
                    if (equalTag != std::string::npos) {
                        const size_t endTag = vUniformParsed.originalParams.find_first_of(")\n:", equalTag + 1);
                        if (endTag != std::string::npos) {
                            vUniform->bufferFileInCode = vUniformParsed.originalParams.substr(fileTag, endTag - fileTag);
                        } else {
                            vUniform->bufferFileInCode = vUniformParsed.originalParams.substr(fileTag);
                        }
                    }
                }

                // vUniform->bufferFileChoosebox = false;
            } else {
                // une choose box a afficher
                // vUniform->bufferFileChoosebox = true;
            }

            // une choose box a afficher
            // vUniform->bufferChoiceActivated = false;
        }
    }

    Complete_Uniform_Buffer_With_Buffer(vRenderPack, vUniform, bufferConfigExist);
}

void ShaderKey::Complete_Uniform_Buffer_With_Buffer(RenderPackWeak vRenderPack, UniformVariantPtr vUniform, bool vBufferConfigExist) {
    if (vUniform) {
        if (vUniform->glslType == uType::uTypeEnum::U_SAMPLER2D) {
            if (!vUniform->bufferShaderName.empty() /* && vUniform->name != puName*/) {
                auto rpPtr = vRenderPack.lock();
                if (rpPtr) {
                    const auto rp = rpPtr->CreateChildBuffer(vUniform->name, vUniform->bufferShaderName, vUniform->inFileBufferName);
                    if (rp)
                        vUniform->bufferShaderName = rp->puName;
                }
                vUniform->pipe = nullptr;
            } else  // une config pour ce buffer
            {
                if (vBufferConfigExist) {
                    auto rpPtr = vRenderPack.lock();
                    if (rpPtr) {
                        rpPtr->puTexParamCustomized = true;

                        GLenum wrap = GL_CLAMP_TO_EDGE;
                        if (vUniform->wrap == "clamp")
                            wrap = GL_CLAMP_TO_EDGE;
                        else if (vUniform->wrap == "repeat")
                            wrap = GL_REPEAT;
                        else if (vUniform->wrap == "mirror")
                            wrap = GL_MIRRORED_REPEAT;

                        GLenum min = GL_LINEAR;
                        if (vUniform->mipmap) {
                            if (vUniform->filter == "linear")
                                min = GL_LINEAR_MIPMAP_LINEAR;
                            else if (vUniform->filter == "nearest")
                                min = GL_NEAREST_MIPMAP_NEAREST;
                            rpPtr->puTexParams.maxMipMapLvl = vUniform->maxmipmaplvl;
                        } else {
                            if (vUniform->filter == "linear")
                                min = GL_LINEAR;
                            else if (vUniform->filter == "nearest")
                                min = GL_NEAREST;
                            rpPtr->puTexParams.maxMipMapLvl = 0;
                        }

                        GLenum mag = GL_LINEAR;
                        if (vUniform->filter == "linear")
                            mag = GL_LINEAR;
                        if (vUniform->filter == "nearest")
                            mag = GL_NEAREST;

                        rpPtr->puTexParams.useMipMap = (int)vUniform->mipmap;
                        rpPtr->puTexParams.wrapS = wrap;
                        rpPtr->puTexParams.wrapT = wrap;
                        rpPtr->puTexParams.minFilter = min;
                        rpPtr->puTexParams.magFilter = mag;
                    }
                }
            }
        }

        // si c'est un enfant d'un multiloc, alors on propage
        if (vUniform->parentMultiLoc) {
            vUniform->parentMultiLoc->Propagate();
        }
    }
}

void ShaderKey::Complete_Uniform_Picture(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->widget = "picture";
    vUniform->attachment = 0;

    // default params
    vUniform->textureChoiceActivated = true;

    for (auto it = vUniformParsed.paramsDico.begin(); it != vUniformParsed.paramsDico.end(); ++it) {
        std::string key = it->first;

        if (key == "target") {
            if (it->second.size() == 1) {
                vUniform->target = *it->second.begin();
            } else {
                // une choose box a afficher
            }
        } else if (key == "choice") {
            if (it->second.size() == 1) {
                vUniform->filePathNames.clear();
                vUniform->filePathNames.emplace_back(*it->second.begin());
            }

            // une choose box a afficher
            vUniform->textureChoiceActivated = true;
        } else if (key == "file") {
            if (it->second.size() == 1) {
                vUniform->filePathNames.clear();
                vUniform->filePathNames.emplace_back(*it->second.begin());
                vUniform->textureFileChoosebox = false;
            } else {
                // une choose box a afficher
                vUniform->textureFileChoosebox = true;
            }

            vUniform->textureChoiceActivated = false;
        } else if (key == "flip") {
            if (it->second.size() == 1) {
                vUniform->flip = ct::fvariant(*it->second.begin()).GetB();
                vUniform->textureFlipChoosebox = false;
            } else {
                // une choose box a afficher
                vUniform->textureFlipChoosebox = true;
            }
        } else if (key == "wrap") {
            if (it->second.size() == 1) {
                vUniform->wrap = *it->second.begin();
                vUniform->textureWrapChoosebox = false;
            } else {
                // une choose box a afficher
                vUniform->textureWrapChoosebox = true;
            }
        } else if (key == "filter") {
            if (it->second.size() == 1) {
                vUniform->filter = *it->second.begin();
                vUniform->textureFilterChoosebox = false;
            } else {
                // une choose box a afficher
                vUniform->textureFilterChoosebox = true;
            }
        } else if (key == "mipmap") {
            if (it->second.size() == 1) {
                vUniform->mipmap = ct::fvariant(*it->second.begin()).GetB();
                vUniform->textureMipmapChoosebox = false;
            } else {
                // une choose box a afficher
                vUniform->textureMipmapChoosebox = true;
            }
        } else if (key == "mipmaplvl") {
            if (it->second.size() == 1) {
                vUniform->maxmipmaplvl = ct::fvariant(*it->second.begin()).GetI();
            } else {
            }
        }
    }

    Complete_Uniform_Picture_With_Texture(vUniform);
}

void ShaderKey::Complete_Uniform_Picture_With_Texture(UniformVariantPtr vUniform) {
    if (vUniform) {
        vUniform->texture_ptr.reset();

        if (vUniform->target.empty()) {
            if (!vUniform->filePathNames.empty()) {
                std::string _filepathName = vUniform->filePathNames[0];
                if (!FileHelper::Instance()->IsFileExist(_filepathName, true))
                    _filepathName = FileHelper::Instance()->GetAbsolutePathForFileLocation(_filepathName, (int)FILE_LOCATION_Enum::FILE_LOCATION_ASSET_TEXTURE_2D);

                vUniform->texture_ptr =
                    Texture2D::createFromFile(_filepathName.c_str(), GL_TEXTURE_2D, vUniform->flip, vUniform->mipmap, vUniform->wrap, vUniform->filter);
            } else {
                vUniform->texture_ptr = Texture2D::createEmpty(GL_TEXTURE_2D);
            }
        }

        if (vUniform->texture_ptr) {
            vUniform->ownTexture = true;

            if (vUniform->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                vUniform->uSampler2D = vUniform->texture_ptr->getBack()->glTex;
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC2) {
                vUniform->x = (float)vUniform->texture_ptr->getSize().x;
                vUniform->y = (float)vUniform->texture_ptr->getSize().y;
            }
        } else {
            vUniform->ownTexture = false;

            if (vUniform->glslType == uType::uTypeEnum::U_SAMPLER2D) {
                vUniform->uSampler2D = -1;
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC2) {
                vUniform->x = 0.0f;
                vUniform->y = 0.0f;
            }
        }

        // si c'est un enfant d'un multiloc, alors on propage
        if (vUniform->parentMultiLoc) {
            vUniform->parentMultiLoc->Propagate();
        }
    }
}

void ShaderKey::Complete_Uniform_Depth(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& /*vUniformParsed*/, UniformVariantPtr vUniform) {
    vUniform->widget = "depth";
}

void ShaderKey::Complete_Uniform_Volume(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->widget = "volume";
    vUniform->attachment = 0;

    for (auto it = vUniformParsed.paramsDico.begin(); it != vUniformParsed.paramsDico.end(); ++it) {
        std::string key = it->first;

        if (key == "file") {
            if (it->second.size() == 1) {
                vUniform->filePathNames.emplace_back(*it->second.begin());
            }
        } else if (key == "flip") {
            if (it->second.size() == 1) {
                vUniform->flip = ct::fvariant(*it->second.begin()).GetB();
            }
        } else if (key == "wrap") {
            if (it->second.size() == 1) {
                vUniform->wrap = *it->second.begin();
            }
        } else if (key == "filter") {
            if (it->second.size() == 1) {
                vUniform->filter = *it->second.begin();
            }
        } else if (key == "mipmap") {
            if (it->second.size() == 1) {
                vUniform->mipmap = ct::fvariant(*it->second.begin()).GetB();
            }
        } else if (key == "mipmaplvl") {
            if (it->second.size() == 1) {
                vUniform->maxmipmaplvl = ct::fvariant(*it->second.begin()).GetI();
            }
        } else if (key == "format") {
            if (it->second.size() == 1) {
                vUniform->volumeFormat = *it->second.begin();
            }
        }
    }

    if (!vUniform->filePathNames.empty()) {
        std::string _filepathName = vUniform->filePathNames[0];
        if (!FileHelper::Instance()->IsFileExist(_filepathName, true))
            _filepathName = FileHelper::Instance()->GetAbsolutePathForFileLocation(_filepathName, (int)FILE_LOCATION_Enum::FILE_LOCATION_ASSET_TEXTURE_3D);

        vUniform->volume_ptr = Texture3D::createFromFile(_filepathName.c_str(), vUniform->volumeFormat, vUniform->mipmap, vUniform->wrap, vUniform->filter);

        if (vUniform->volume_ptr) {
            vUniform->ownVolume = true;

            if (vUniform->glslType == uType::uTypeEnum::U_SAMPLER3D) {
                vUniform->uSampler3D = vUniform->volume_ptr->getBack()->glTex;
                vUniform->uImage3D = vUniform->volume_ptr->getFront()->glTex;
            } else if (vUniform->glslType == uType::uTypeEnum::U_VEC3) {
                vUniform->x = (float)vUniform->volume_ptr->getSize().x;
                vUniform->y = (float)vUniform->volume_ptr->getSize().y;
                vUniform->z = (float)vUniform->volume_ptr->getSize().z;
            }
        }
    }
}

void ShaderKey::Complete_Uniform_Compute(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    auto rpPtr = vRenderPack.lock();
    if (rpPtr) {
        // on cible la texture d'un compute
        // uniform sampler3D(compute:file=sdf_compute:target=uTexSDf) uTex;

        // on cree une texture dans un compute
        // uniform sampler3D(compute:format=rgba32f:flip=true:wrap=repeat:filter=nearest:loc=0) uTexSDf;

        vUniform->widget = "compute";
        vUniform->attachment = 0;

        for (auto it = vUniformParsed.paramsDico.begin(); it != vUniformParsed.paramsDico.end(); ++it) {
            std::string key = it->first;

            if (key == "file") {
                if (it->second.size() == 1) {
                    vUniform->computeShaderName = *it->second.begin();
                }
            } else if (key == "flip") {
                if (it->second.size() == 1) {
                    vUniform->flip = ct::fvariant(*it->second.begin()).GetB();
                }
            } else if (key == "wrap") {
                if (it->second.size() == 1) {
                    vUniform->wrap = *it->second.begin();
                }
            } else if (key == "filter") {
                if (it->second.size() == 1) {
                    vUniform->filter = *it->second.begin();
                }
            } else if (key == "mipmap") {
                if (it->second.size() == 1) {
                    vUniform->mipmap = ct::fvariant(*it->second.begin()).GetB();
                }
            } else if (key == "mipmaplvl") {
                if (it->second.size() == 1) {
                    vUniform->maxmipmaplvl = ct::fvariant(*it->second.begin()).GetI();
                }
            } else if (key == "format") {
                if (it->second.size() == 1) {
                    vUniform->volumeFormat = *it->second.begin();
                }
            } else if (key == "target") {
                if (it->second.size() == 1) {
                    vUniform->target = *it->second.begin();
                }
            } else if (key == "binding") {
                if (it->second.size() == 1) {
                    vUniform->slot = ct::fvariant(*it->second.begin()).GetI();
                }
            }
        }

        if (vUniform->glslType == uType::uTypeEnum::U_SAMPLER3D) {
            if (!vUniform->computeShaderName.empty()) {
                auto computePack = rpPtr->CreateChildCompute(vUniform->name, vUniform->computeShaderName);
                if (computePack) {
                    if (computePack->GetShaderKey()) {
                        auto uni = computePack->GetShaderKey()->GetUniformByName(vUniform->target);
                        if (uni) {
                            // vUniform->volume_ptr = uni->volume;
                            // vUniform->ownVolume = false;
                            if (uni->volume_ptr)
                                vUniform->uSampler3D = uni->volume_ptr->getBack()->glTex;
                        }
                    }
                }
            } else  // on cree une texture 3d vide
            {
                vUniform->volume_ptr =
                    Texture3D::createComputeVolume(vUniform->volumeFormat, rpPtr->puSectionConfig.computeConfig.size, vUniform->mipmap, vUniform->wrap, vUniform->filter);

                if (vUniform->volume_ptr) {
                    vUniform->ownVolume = true;

                    vUniform->uSampler3D = vUniform->volume_ptr->getBack()->glTex;
                    vUniform->uImage3D = vUniform->volume_ptr->getFront()->glTex;
                    vUniform->computeTextureFormat = vUniform->volume_ptr->getBack()->glinternalformat;
                }
            }
        }
        if (vUniform->glslType == uType::uTypeEnum::U_SAMPLER2D) {
            if (!vUniform->computeShaderName.empty()) {
                auto computePack = rpPtr->CreateChildCompute(vUniform->name, vUniform->computeShaderName);
                if (computePack) {
                    if (computePack->GetShaderKey()) {
                        auto uni = computePack->GetShaderKey()->GetUniformByName(vUniform->target);
                        if (uni) {
                            // vUniform->texture_ptr = uni->texture;
                            // vUniform->ownTexture = false;
                            if (uni->texture_ptr)
                                vUniform->uSampler2D = uni->texture_ptr->getBack()->glTex;
                        }
                    }
                }
            } else  // on cree une texture 2d vide
            {
                ct::ivec3 size = rpPtr->puSectionConfig.computeConfig.size;
                vUniform->texture_ptr = Texture2D::createComputeTexture(vUniform->volumeFormat, size.xy(), vUniform->mipmap, vUniform->wrap, vUniform->filter);

                if (vUniform->texture_ptr) {
                    vUniform->ownTexture = true;

                    vUniform->uSampler2D = vUniform->texture_ptr->getBack()->glTex;
                    vUniform->uImage2D = vUniform->texture_ptr->getFront()->glTex;
                    vUniform->computeTextureFormat = vUniform->texture_ptr->getBack()->glinternalformat;
                }
            }
        } else if (vUniform->glslType == uType::uTypeEnum::U_VEC3) {
            if (!vUniform->computeShaderName.empty()) {
                auto computePack = rpPtr->puBuffers.get(vUniform->computeShaderName).lock();
                if (computePack) {
                    if (computePack->GetShaderKey()) {
                        auto uni = computePack->GetShaderKey()->GetUniformByName(vUniform->target);
                        if (uni) {
                            if (uni->volume_ptr) {
                                const ct::ivec3 size = uni->volume_ptr->getSize();
                                vUniform->x = (float)size.x;
                                vUniform->y = (float)size.y;
                                vUniform->z = (float)size.z;
                            }
                        }
                    }
                }
            } else {
                vUniform->x = 0.0f;
                vUniform->y = 0.0f;
                vUniform->z = 0.0f;
            }
        } else if (vUniform->glslType == uType::uTypeEnum::U_VEC2) {
            if (!vUniform->computeShaderName.empty()) {
                auto computePack = rpPtr->puBuffers.get(vUniform->computeShaderName).lock();
                if (computePack) {
                    if (computePack->GetShaderKey()) {
                        auto uni = computePack->GetShaderKey()->GetUniformByName(vUniform->target);
                        if (uni) {
                            if (uni->texture_ptr) {
                                const ct::ivec2 size = uni->texture_ptr->getSize();
                                vUniform->x = (float)size.x;
                                vUniform->y = (float)size.y;
                            }
                        }
                    }
                }
            } else {
                vUniform->x = 0.0f;
                vUniform->y = 0.0f;
            }
        }
    }
}

void ShaderKey::Complete_Uniform_CubeMap(RenderPackWeak /*vRenderPack*/, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    vUniform->widget = "cubemap";
    vUniform->attachment = 0;

    for (auto it = vUniformParsed.paramsDico.begin(); it != vUniformParsed.paramsDico.end(); ++it) {
        std::string key = it->first;

        if (key == "files") {
            for (auto itLst = it->second.begin(); itLst != it->second.end(); ++itLst) {
                vUniform->filePathNames.emplace_back(*itLst);
            }
        } else if (key == "flip") {
            if (it->second.size() == 1) {
                vUniform->flip = ct::fvariant(*it->second.begin()).GetB();
            }
        } else if (key == "wrap") {
            if (it->second.size() == 1) {
                vUniform->wrap = *it->second.begin();
            }
        } else if (key == "filter") {
            if (it->second.size() == 1) {
                vUniform->filter = *it->second.begin();
            }
        } else if (key == "mipmap") {
            if (it->second.size() == 1) {
                vUniform->mipmap = ct::fvariant(*it->second.begin()).GetB();
            }
        } else if (key == "mipmaplvl") {
            if (it->second.size() == 1) {
                vUniform->maxmipmaplvl = ct::fvariant(*it->second.begin()).GetI();
            }
        }
    }

    if (vUniform->filePathNames.size() == 6) {
        vUniform->cubemap_ptr =
            TextureCube::create(FileHelper::Instance()->GetAbsolutePathForFileLocation(vUniform->filePathNames, (int)FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP));

        if (vUniform->cubemap_ptr != nullptr) {
            vUniform->ownCubeMap = true;
            vUniform->uSamplerCube = vUniform->cubemap_ptr->getCubeMapId();
        }
    }
}

void ShaderKey::Complete_Uniform_TextureSound(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    auto rpPtr = vRenderPack.lock();
    if (rpPtr) {
        vUniform->widget = "sound";

        for (auto it = vUniformParsed.paramsDico.begin(); it != vUniformParsed.paramsDico.end(); ++it) {
            std::string key = it->first;

            if (key == "choice") {
                if (it->second.size() == 1) {
                    vUniform->filePathNames.clear();
                    vUniform->filePathNames.emplace_back(*it->second.begin());
                }

                // une choose box a afficher
                vUniform->soundChoiceActivated = true;
            } else if (key == "file") {
                if (it->second.size() == 1) {
                    vUniform->filePathNames.clear();
                    vUniform->filePathNames.emplace_back(*it->second.begin());
                    vUniform->soundFileChoosebox = false;
                } else {
                    // une choose box a afficher
                    vUniform->soundFileChoosebox = true;
                }

                vUniform->soundChoiceActivated = false;
            } else if (key == "loop") {
                if (it->second.size() == 1) {
                    vUniform->soundLoop = ct::ivariant(*it->second.begin()).GetB();
                }
            } else if (key == "histo") {
                if (it->second.size() == 1) {
                    vUniform->soundHisto = ct::ivariant(*it->second.begin()).GetI();
                }
            }
        }

        Complete_Uniform_Sound_With_Sound(rpPtr->puWindow, vUniform);
    }
}

void ShaderKey::Complete_Uniform_TextureVideo(RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) {
    UNUSED(vRenderPack);
    UNUSED(vUniformParsed);
    UNUSED(vUniform);
}

void ShaderKey::Complete_Uniform_Sound_With_Sound(const GuiBackend_Window& /*vWindow*/, UniformVariantPtr /*vUniform*/, bool /*vSoundConfigExist*/) {
    CTOOL_DEBUG_BREAK;

    /*if (vUniform->filePathNames.size() == 1U)
    {
        std::string _filepathName = vUniform->filePathNames[0];
        if (!FileHelper::Instance()->IsFileExist(_filepathName, true))
            _filepathName = FileHelper::Instance()->GetAbsolutePathForFileLocation(_filepathName, (int)FILE_LOCATION_Enum::FILE_LOCATION_ASSET_SOUND);

        if (FileHelper::Instance()->IsFileExist(_filepathName, true))
        {
            vUniform->sound_histo_ptr.reset();
            vUniform->sound = TextureSound::Create(_filepathName, vUniform->soundHisto);
            if (vUniform->sound)
            {
                if (!vSoundConfigExist)
                {
                    vUniform->bx = false;
                    vUniform->x = 0.0f;
                }
                else
                {
                    vUniform->bx ? vUniform->sound->Play() : vUniform->sound->Pause();
                }

                vUniform->attachment = 0;
                vUniform->ownSound = true;
                vUniform->pipe = nullptr;// vUniform->sound->GetPipe();
                vUniform->sound->SetLoopPlayBack(vUniform->soundLoop);
                vUniform->sound->SetPosInPercentOfTotalLength(vUniform->y);
                vUniform->sound->SetVolume(vUniform->soundVolume);
            }
        }
        else
        {
            std::string fromFile = m_This->puKey;
            if (!m_This->puInFileBufferFromKey.empty())
                fromFile = m_This->puInFileBufferFromKey;
            puSyntaxErrors.SetSyntaxError(m_This, "Asset not found :", "Sound file not found", false,
                LineFileErrors(vUniform->SourceLinePos, fromFile, "The file Condition '" + vUniform->filePathNames[0] + "' not found (" + _filepathName + ")"));
        }
    }*/
}