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

#include "SectionCode.h"
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <CodeTree/CodeTree.h>
#include <uTypes/uTypes.h>
#include <CodeTree/SyntaxErrors/LineFileErrors.h>
#include <ctools/Logger.h>
#include <CodeTree/ShaderKey.h>
#include <CodeTree/Parsing/UniformParsing.h>

// #define VERBOSE_DEBUG
// #define USE_UNIFORMS_BUFFER

////////////////////////////////////////////////////////////////////////
//// pour relativiser les chemins de fichier a l'application ///////////
////////////////////////////////////////////////////////////////////////

#include <cstdio> /* defines FILENAME_MAX */
#ifdef _WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

CodeTagStruct SectionCode::codeTags = CodeTagStruct();

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

std::shared_ptr<SectionCode> SectionCode::Create() {
    std::shared_ptr<SectionCode> res = std::make_shared<SectionCode>();
    res->m_This = res;
    return res;
}

std::shared_ptr<SectionCode> SectionCode::Create(ShaderKeyPtr vParentkey,
                                                 std::shared_ptr<SectionCode> vParentSection,
                                                 const std::string& vFile,
                                                 const std::string& vCode,
                                                 const std::string& vType,
                                                 const std::string& vParentType,
                                                 const std::string& vName,
                                                 const std::string& vInFileBufferName,
                                                 const size_t& vSourceCodeStartLine,
                                                 const size_t& vSourceCodeEndLine,
                                                 const size_t& vLevel,
                                                 const bool& vIsInclude) {
    std::shared_ptr<SectionCode> res = std::make_shared<SectionCode>(
        vParentkey, vParentSection, vFile, vCode, vType, vParentType, vName, vInFileBufferName, vSourceCodeStartLine, vSourceCodeEndLine, vLevel, vIsInclude);
    res->m_This = res;
    return res;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

SectionCode::SectionCode() {
    orphan = true;
    isInclude = false;

    parentKey.reset();
    parentSection = nullptr;

    level = 0;

    code.clear();
    name.clear();
    inFileBufferName.clear();

    sourceCodeStartLine = 0;
    sourceCodeEndLine = 0;

    finalCodeStartLine.clear();
    finalCodeEndLine.clear();

    parentType = "NONE";
    currentType = "FULL";
}

SectionCode::SectionCode(std::weak_ptr<ShaderKey> vParentKey,
                         std::shared_ptr<SectionCode> vParentSection,
                         const std::string& vAbsoluteFile,
                         std::string vCode,
                         std::string vType,
                         std::string vParentType,
                         const std::string& vSectionName,
                         const std::string& vInFileBufferName,
                         size_t vSourceCodeStartLine,
                         size_t vSourceCodeEndLine,
                         size_t vLevel,
                         bool vIsInclude)
    : SectionCode() {
    orphan = true;
    isInclude = vIsInclude;

    parentKey = vParentKey;
    parentSection = vParentSection;

    if (!vAbsoluteFile.empty()) {
        // LogAssert(FileHelper::Instance()->IsAbsolutePath(vAbsoluteFile), "%s","vAbsoluteFile is not absolute");

        absoluteFile = vAbsoluteFile;

        // on va le relativiser
        relativeFile = vAbsoluteFile;
        char cCurrentPath[FILENAME_MAX];
        if (GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
            std::string currentDirectory = cCurrentPath + FileHelper::Instance()->puSlashType;
            size_t pos = relativeFile.find(currentDirectory, 0);
            if (pos != std::string::npos) {
                ct::replaceString(relativeFile, currentDirectory, "");
            }
        }
    }

    level = vLevel;

    code = vCode;
    name = vSectionName;
    inFileBufferName = vInFileBufferName;

    sourceCodeStartLine = vSourceCodeStartLine;
    sourceCodeEndLine = vSourceCodeEndLine;

    finalCodeStartLine.clear();
    finalCodeEndLine.clear();

    parentType = vParentType;
    currentType = vType;
}

SectionCode::~SectionCode() = default;

void SectionCode::Clear() {
    subSections.clear();
    errors.clear();
}

void SectionCode::DefineSubSection(const std::string& vNewCode, size_t vStartPos, size_t vEndPos, std::string vType, std::string vParentType, bool vIsInclude) {
    if (parentKey.expired())
        return;
    auto pkeyPtr = parentKey.lock();
    if (pkeyPtr.use_count()) {
        pkeyPtr->AddShaderStageName(vType);

        // isInclude = vIsInclude;
        std::weak_ptr<ShaderKey> includeKey;
        std::string _extractCode = vNewCode.substr(vStartPos, vEndPos - vStartPos);
        std::string _sectionName;
        std::vector<std::string> _inFileBufferNames;
        std::string _absoluteFile = absoluteFile;
        size_t sl = ct::GetCountOccurenceInSection(vNewCode, 0, vStartPos, '\n');
        size_t slSub = 0;
        size_t el = ct::GetCountOccurenceInSection(vNewCode, 0, vEndPos, '\n');
        size_t elSub = 0;
        bool needToCreateThisSectionEvenIfCodeEmpty = false;
        // Section qui ne contient pas de code
        if (vType == "PROJECT" || vType == "FRAMEBUFFER") {
            //_endLine += ct::GetCountOccurence(_extractCode, "\n");
            std::string _configLine = _extractCode;  // .substr(0, _endLine - 0);

            // on vire ce code car dans le bloc d'apres les COUNT et SIZE n'etait jamais trouvés
            // ct::replaceString(_configLine, "\n", " ");
            // ct::replaceString(_configLine, "\r", "");

            /*
            @FRAMEBUFFER
            //FORMAT(float or byte)
            COUNT(5)
            //WRAP(clamp or repeat or mirror)
            //FILTER(linear or nearest)
            //MIPMAP(false or true)
            SIZE(1000)
            //RATIO(1.5 or picture:file.jpeg)

            count et size n'etait jamais trouvé
            */

            bool useable = m_ShaderStageParsing.ParseSectionConfig_ReturnSectionName_SayIfWeCanUse(
                m_This, _configLine, vType, sourceCodeStartLine + sl, forcedStageName, &_sectionName, &_inFileBufferNames);

            ++sl;

            if (!useable)
                _extractCode.clear();

            ++sl;  // todo : pourquoi 2 sl ??
        } else if (vType == "INCLUDE") {
            // extractCode contient le code depuis la fin du tag @CONFIG_START
            // donc il faut parser la ligne pour recup le nom de la config. il doit y en avoir un, sinon erreur

            if (!_extractCode.empty()) {
                includeKey = m_ShaderStageParsing.ParseIncludeLine(m_This, _extractCode, sourceCodeStartLine + sl);

                auto incKeyPtr = includeKey.lock();
                if (incKeyPtr.use_count()) {
                    // comment peux t'on verifier qu'on ne fasse pas une inclusion infinie
                    _extractCode = incKeyPtr->puMainSection->code;
#ifdef _DEBUG
                    auto ps = FileHelper::Instance()->ParsePathFileName(incKeyPtr->puMainSection->relativeFile);
                    if (ps.isOk) {
                        FileHelper::Instance()->SaveToFile(_extractCode, "include_" + ps.name + ".txt", (int)FILE_LOCATION_Enum::FILE_LOCATION_DEBUG);
                    }
#endif
                } else {
                    _extractCode.clear();
                }

                if (!_extractCode.empty()) {
                    slSub = sourceCodeStartLine + sl;
                    elSub = sourceCodeStartLine + el;
                    // sl -= sourceCodeStartLine;
                    //  vu que el est un size_t, si el vaut 0 et que je lui soustrait une valeur sup a 0, j'obtient un el qui vaut std::string::npos ...
                    el = ct::GetCountOccurence(_extractCode, "\n");  // -sourceCodeStartLine;
                }
            }
        } else if (vType == "CONFIG") {
            // extractCode contient le code depuis la fin du tag @CONFIG_START
            // donc il faut parser la ligne pour recup le nom de la config. il doit y en avoir un, sinon erreur

            size_t _endLine = _extractCode.find('\n', 0);
            if (_endLine != std::string::npos) {
                _sectionName = _extractCode.substr(0, _endLine - 0);
                // ct::replaceString(_blockName, " ", "");

                if (_inFileBufferNames.empty()) {
                    if (!inFileBufferName.empty()) {
                        _inFileBufferNames.push_back(inFileBufferName);
                    } else {
                        _inFileBufferNames.emplace_back("");
                    }
                }

                for (auto& it : _inFileBufferNames) {
                    if (vParentType == "VERTEX" || vParentType == "GEOMETRY" || vParentType == "FRAGMENT" || vParentType == "TESSCONTROL" || vParentType == "TESSEVAL" ||
                        vParentType == "COMPUTE") {
                        pkeyPtr->AddConfigName(vParentType, it, name, _sectionName, true);
                    } else if (vParentType == "UNIFORMS" || vParentType == "FRAMEBUFFER") {
                        pkeyPtr->AddConfigName("UNIFORMS", it, name, _sectionName, true);
                        if (pkeyPtr->puIsVertexShaderPresent)
                            pkeyPtr->AddConfigName("VERTEX", it, name, _sectionName, true);
                        if (pkeyPtr->puIsFragmentShaderPresent)
                            pkeyPtr->AddConfigName("FRAGMENT", it, name, _sectionName, true);
                        if (pkeyPtr->puIsGeometryShaderPresent)
                            pkeyPtr->AddConfigName("GEOMETRY", it, name, _sectionName, true);
                        if (pkeyPtr->puIsTesselationControlShaderPresent)
                            pkeyPtr->AddConfigName("TESSCONTROL", it, name, _sectionName, true);
                        if (pkeyPtr->puIsTesselationEvalShaderPresent)
                            pkeyPtr->AddConfigName("TESSEVAL", it, name, _sectionName, true);
                        if (pkeyPtr->puIsComputeShaderPresent)
                            pkeyPtr->AddConfigName("COMPUTE", it, name, _sectionName, true);
                    } else if (vParentType == "COMMON") {
                        pkeyPtr->AddConfigName(_sectionName, it, name, _sectionName, true);
                        if (pkeyPtr->puIsVertexShaderPresent)
                            pkeyPtr->AddConfigName("VERTEX", it, name, _sectionName, true);
                        if (pkeyPtr->puIsFragmentShaderPresent)
                            pkeyPtr->AddConfigName("FRAGMENT", it, name, _sectionName, true);
                        if (pkeyPtr->puIsGeometryShaderPresent)
                            pkeyPtr->AddConfigName("GEOMETRY", it, name, _sectionName, true);
                        if (pkeyPtr->puIsTesselationControlShaderPresent)
                            pkeyPtr->AddConfigName("TESSCONTROL", it, name, _sectionName, true);
                        if (pkeyPtr->puIsTesselationEvalShaderPresent)
                            pkeyPtr->AddConfigName("TESSEVAL", it, name, _sectionName, true);
                        if (pkeyPtr->puIsComputeShaderPresent)
                            pkeyPtr->AddConfigName("COMPUTE", it, name, _sectionName, true);
                    }
                }

                _endLine += 1;
                if (_endLine < _extractCode.size())
                    _extractCode = _extractCode.substr(_endLine);
                else
                    _extractCode.clear();
            }

            if (_sectionName.empty()) {
                // error
            }

            ++sl;
            --el;
        } else if (vType == "REPLACE_CODE") {
            // extractCode contient le code depuis la fin du tag @CONFIG_START
            // donc il faut parser la ligne pour recup le nom de la config. il doit y en avoir un, sinon erreur

            if (!_extractCode.empty() && currentType != "NOTE") {
                auto replaceCode = m_ShaderStageParsing.ParseReplaceCode(m_This, _extractCode, sourceCodeStartLine + sl);
                _sectionName = replaceCode.name;
                pkeyPtr->AddReplaceCode(vParentType, replaceCode, true);
                m_ReplaceCode = pkeyPtr->GetReplaceCodeByName(vParentType, _sectionName);
                if (!m_ReplaceCode.name.empty()) {
                    if (!m_ReplaceCode.newCode.empty())
                        _extractCode = m_ReplaceCode.newCode;
                    else
                        _extractCode = m_ReplaceCode.defCode;
                } else {
                    _extractCode.clear();
                }

                needToCreateThisSectionEvenIfCodeEmpty = true;
            }

            if (_sectionName.empty()) {
                // error
            }
        } else if (vType == "UNIFORM") {
            std::string _codeToParse = _extractCode;
            ct::replaceString(_codeToParse, "[", " [");  // pour les uniform vec3 aaa[10];
            UniformParsedStruct uniformParsed = UniformParsing::ParseUniformString(_codeToParse, sourceCodeStartLine + sl, m_This);
            if (uniformParsed.isOk()) {
                std::string uniformName = uniformParsed.name;

                if (isInclude) {
                    if (pkeyPtr->puParentCodeTree)
                        pkeyPtr->puParentCodeTree->AddUniformNameForIncludeFile(uniformName, relativeFile);
                } else if (vParentType == "REPLACE_CODE") {
                    if (parentSection) {
                        if (!parentSection->m_ReplaceCode.newCodeFromKey.empty()) {
                            uniformParsed.isPiloted = true;
                            // ca peut merder ca, car si on est viens d'une config, et bien parentType sera la config et pas le stage
                            // il faudrait ce dire qu'uns ection de stage est forcemment en depth 1 dans la hierarchie
                            // donc asuver quand en depth 1 le stage
                            // todo: car peut merder ca
                            uniformParsed.pilotStage = parentType;
                            uniformParsed.pilotKey = parentSection->m_ReplaceCode.newCodeFromKey;
                        }
                    }
                }

#ifdef USE_UNIFORMS_BUFFER
                _extractCode = "//" + _extractCode;
#else
                if (uniformParsed.notUploadableToGPU) {
                    _extractCode = "//" + _extractCode;
                } else {
                    _extractCode = codeTags.UniformTemporaryTag + uniformName + ")\n";

                    //	_extractCode = "//" + _extractCode;
                    //_extractCode.clear();
                    // opengl : _extractCode = uniformParsed.GetUniformHeaderString();
                }
#endif

                pkeyPtr->AddUniformToDataBase(m_This, uniformParsed);
            } else {
                // format invalide
                // formatError.uniformMsg = ShaderMsg::SHADER_MSG_ERROR;
                // formatError.uniformError += "Bad uniform format : " + uni_line + "\n";
                // uniform_pos++; // pour pas bloquer la boucle
                _extractCode = "//" + _extractCode;
            }

            if (el > 0)
                --el;
            else
                el = 0;
        } else if (vType == "TEXT") {
            if (el > 0)
                --el;
            else
                el = 0;
        } else  // tout ca peu contenir du code, les trucs un peut spéciaux ont été faits avant
        /*if (
            vType == "UNIFORMS" ||
            vType == "COMMON" ||
            vType == "VERTEX" ||
            vType == "GEOMETRY" ||
            vType == "TESSCONTROL" ||
            vType == "TESSEVAL" ||
            vType == "FRAGMENT" ||
            vType == "NOTE" ||
            vType == "COMPUTE")*/
        {
            stageName = vType;
            size_t _endLine = _extractCode.find('\n', 0);
            if (_endLine != std::string::npos) {
                std::string _configLine = _extractCode.substr(0, _endLine - 0);
                bool useable = m_ShaderStageParsing.ParseSectionConfig_ReturnSectionName_SayIfWeCanUse(
                    m_This, _configLine, vType, sourceCodeStartLine + sl, forcedStageName, &_sectionName, &_inFileBufferNames);
                _endLine += 1;
                ++sl;
                if (_endLine < _extractCode.size())
                    _extractCode = _extractCode.substr(_endLine);
                else
                    _extractCode.clear();
                if (!useable)
                    _extractCode.clear();
                ++sl;
            }

            if (vType == "COMMON")
                pkeyPtr->puIsCommonShaderPartPresent = true;
            if (vType == "VERTEX")
                pkeyPtr->puIsVertexShaderPresent = true;
            if (vType == "GEOMETRY")
                pkeyPtr->puIsGeometryShaderPresent = true;
            if (vType == "TESSCONTROL")
                pkeyPtr->puIsTesselationControlShaderPresent = true;
            if (vType == "TESSEVAL")
                pkeyPtr->puIsTesselationEvalShaderPresent = true;
            if (vType == "FRAGMENT")
                pkeyPtr->puIsFragmentShaderPresent = true;
            if (vType == "COMPUTE")
                pkeyPtr->puIsComputeShaderPresent = true;
        }

        if (!_extractCode.empty() || needToCreateThisSectionEvenIfCodeEmpty) {
            subSections[vStartPos].clear();
            if (_inFileBufferNames.empty()) {
                if (!inFileBufferName.empty())
                    _inFileBufferNames.push_back(inFileBufferName);
                else
                    _inFileBufferNames.emplace_back("");
            }
            for (auto& it : _inFileBufferNames) {
                auto _section = SectionCode::Create(pkeyPtr,
                                                    m_This,
                                                    _absoluteFile,
                                                    _extractCode,
                                                    vType,
                                                    vParentType,
                                                    _sectionName,
                                                    it,
                                                    sourceCodeStartLine + sl - slSub,
                                                    sourceCodeStartLine + el - elSub,
                                                    level + 1,
                                                    vIsInclude);

                if (!stageName.empty())
                    _section->stageName = stageName;

                // LogVarDebug("l:%u t|pt:%s|%s => inc:%s", (uint32_t)level, vType.c_str(), vParentType.c_str(), (vIsInclude ? "true" : "false"));

                // on propage le forcedSectionName
                // attention par contre la section force doit juste servir au fichier pas a ses enfants inclus
                if (!forcedSectionName.empty() && _section->forcedSectionName.empty())
                    _section->forcedSectionName = forcedSectionName;

                // on propage le forcedStageName
                // attention par contre le stage force doit juste servir au fichier pas a ses enfants inclus
                if (!forcedStageName.empty() && _section->forcedStageName.empty())
                    _section->forcedStageName = forcedStageName;

                auto incKeyPtr = includeKey.lock();
                if (incKeyPtr.use_count()) {
                    // comme on veut faire attention par contre la section forcee doit juste servir au fichier pas a ses enfants inclus
                    // c'est ici qu'on reecrit le forecedsectionname via celui du parnet inclu
                    _section->forcedSectionName = incKeyPtr->puMainSection->forcedSectionName;
                    _section->forcedStageName = incKeyPtr->puMainSection->forcedStageName;
                    _section->absoluteFile = incKeyPtr->puMainSection->absoluteFile;
                    _section->relativeFile = incKeyPtr->puMainSection->relativeFile;
                }

                subSections[vStartPos].push_back(_section);
            }
        }

        orphan = false;
    }
}

bool SectionCode::ParseConfigs(const std::string& codeToParse, size_t& vLastBlockPos) {
    bool configStartFound = false;

#ifdef _DEBUG
    FileHelper::Instance()->SaveStringToFile(codeToParse, "SectionCode_ParseConfigs.txt");
#endif

    // les configs
    size_t nextPos = 0;
    while (nextPos != std::string::npos) {
        nextPos = ShaderStageParsing::GetTagPos(codeToParse, codeTags.ConfigStartTag, nextPos, false);
        if (nextPos != std::string::npos) {
            if (!ShaderStageParsing::IfInCommentZone(codeToParse, nextPos)) {
                // error on peut pas ouvir une config sans fermer la precedente
                configStartFound = true;
                size_t posWithoutTag = nextPos;
                size_t pos = nextPos + codeTags.ConfigStartTag.size();

                size_t sl = ct::GetCountOccurenceInSection(codeToParse, 0, vLastBlockPos, '\n');
                size_t el = ct::GetCountOccurenceInSection(codeToParse, 0, posWithoutTag, '\n');

                if (el > sl)  // on fait une section text que si elle fait au moin une ligne
                {
                    DefineSubSection(codeToParse, vLastBlockPos, posWithoutTag, "TEXT", this->currentType, isInclude);
                }

                size_t nextOpenTag = ShaderStageParsing::GetTagPos(codeToParse, codeTags.ConfigStartTag, pos, false);
                nextPos = ShaderStageParsing::GetTagPos(codeToParse, codeTags.ConfigEndTag, pos, false);
                if (nextPos != std::string::npos) {
                    if (nextOpenTag != std::string::npos) {
                        if (nextOpenTag < nextPos) {
                            SetSyntaxError(relativeFile,
                                           "Config Syntax Error",
                                           true,
                                           "you must close a config flag, before open a new one",
                                           ct::GetCountOccurenceInSection(codeToParse, 0, nextOpenTag, '\n') + this->sourceCodeStartLine);
                        }
                    }
                    if (!ShaderStageParsing::IfInCommentZone(codeToParse, nextPos)) {
                        if (pos) {
                            sl = ct::GetCountOccurenceInSection(codeToParse, 0, pos, '\n') + this->sourceCodeStartLine;
                            el = ct::GetCountOccurenceInSection(codeToParse, 0, nextPos, '\n') + this->sourceCodeStartLine;

                            DefineSubSection(codeToParse, pos, nextPos, "CONFIG", this->currentType, isInclude);
                            vLastBlockPos = ShaderStageParsing::GetTagPos(codeToParse, "\n", nextPos + codeTags.ConfigEndTag.size());
                            if (vLastBlockPos != std::string::npos)
                                ++vLastBlockPos;
                            else
                                vLastBlockPos = nextPos + codeTags.ConfigEndTag.size();
                        }
                    }
                }
            }
            if (nextPos != std::string::npos) {
                ++nextPos;  // pour pas bloquer la boucle
            }
        }
    }

    return configStartFound;
}

bool SectionCode::ParseIncludes(const std::string& codeToParse, size_t& vLastBlockPos) {
    bool includeFound = false;

    //  on fait les includes
    // lastBlockPos = 0;
    size_t nextPos = 0;
    while (nextPos != std::string::npos) {
        nextPos = ShaderStageParsing::GetTagPos(codeToParse, codeTags.IncludeWordTag, nextPos, false);
        if (nextPos != std::string::npos) {
            if (!ShaderStageParsing::IfInCommentZone(codeToParse, nextPos)) {
                includeFound = true;

                size_t posWithoutTag = nextPos;
                size_t pos = nextPos + codeTags.IncludeWordTag.size();
                size_t firstQuote = ShaderStageParsing::GetTagPos(codeToParse, "\"", pos);  // pour l'incluison de includeFile
                if (firstQuote != std::string::npos) {
                    // firstQuote += 1;
                    size_t endQuote = codeToParse.find_first_of("\"\n", firstQuote + 1);
                    if (endQuote != std::string::npos) {
                        if (codeToParse[endQuote] == '\n') {
                            SetSyntaxError(relativeFile,
                                           "Include Syntax Error",
                                           true,
                                           "you must quote the include file on the same line.",
                                           ct::GetCountOccurenceInSection(codeToParse, 0, pos, '\n') + this->sourceCodeStartLine);
                        } else {
                            // le char \n => fin de ligne
                            // le char / => un commentaire
                            endQuote = codeToParse.find_first_of("/\n", endQuote + 1);
                            if (endQuote != std::string::npos) {
                                size_t sl = ct::GetCountOccurenceInSection(codeToParse, 0, vLastBlockPos, '\n');
                                size_t el = ct::GetCountOccurenceInSection(codeToParse, 0, posWithoutTag, '\n');

                                if (el > sl)  // on fait une section text que si elle fait au moin une ligne
                                {
                                    DefineSubSection(codeToParse, vLastBlockPos, posWithoutTag, "TEXT", this->currentType, isInclude);
                                }

                                DefineSubSection(codeToParse, firstQuote, endQuote, "INCLUDE", this->currentType, true);

                                vLastBlockPos = ShaderStageParsing::GetTagPos(codeToParse, "\n", endQuote);
                                if (vLastBlockPos != std::string::npos)
                                    ++vLastBlockPos;
                                else
                                    vLastBlockPos = endQuote;
                            }
                        }
                    }
                } else {
                    SetSyntaxError(relativeFile,
                                   "Include Syntax Error",
                                   true,
                                   "you must quote the include file.",
                                   ct::GetCountOccurenceInSection(codeToParse, 0, pos, '\n') + this->sourceCodeStartLine);
                }
            }

            if (nextPos != std::string::npos) {
                ++nextPos;  // pour pas bloquer la boucle
            }
        }
    }

    return includeFound;
}

bool SectionCode::ParseUniforms(const std::string& codeToParse, size_t& vLastBlockPos) {
    bool uniformFound = false;

    size_t nextPos = 0;
    while (nextPos != std::string::npos) {
        nextPos = ShaderStageParsing::GetTagPos(codeToParse, codeTags.UniformWordTag, nextPos, false);
        if (nextPos != std::string::npos) {
            size_t endUniformWordTagPos = nextPos + codeTags.UniformWordTag.size();
            size_t isUniformsPos = codeToParse.find_first_of("\t (\n", endUniformWordTagPos);
            if (isUniformsPos == endUniformWordTagPos) {
                if (!ShaderStageParsing::IfInCommentZone(codeToParse, nextPos)) {
                    uniformFound = true;

                    size_t coma_pos = codeToParse.find(';', nextPos);
                    if (coma_pos != std::string::npos) {
                        // on test si cet uniform est layouté et bindé comme dans un compute
                        // dans ce cas on va pas l'utliser et le laisser tel quel dans le code

                        size_t pos_layout = codeToParse.rfind("layout", nextPos);
                        size_t pos_binding = codeToParse.rfind("binding", nextPos);
                        if (pos_layout != std::string::npos && pos_binding != std::string::npos) {
                            size_t last_coma_pos = codeToParse.rfind(';', nextPos);
                            if (last_coma_pos == std::string::npos)
                                last_coma_pos = pos_layout;

                            if (nextPos > pos_binding && pos_binding > pos_layout && pos_layout >= last_coma_pos) {
                                uniformFound = false;
                            }
                        }

                        if (uniformFound) {
                            size_t first_endline_pos = codeToParse.find('\n', coma_pos);
                            if (first_endline_pos != std::string::npos) {
                                first_endline_pos += 1;

                                size_t sl = ct::GetCountOccurenceInSection(codeToParse, 0, vLastBlockPos, '\n');
                                size_t el = ct::GetCountOccurenceInSection(codeToParse, 0, nextPos, '\n');

                                if (el > sl)  // on fait une section text que si elle fait au moin une ligne
                                {
                                    DefineSubSection(codeToParse, vLastBlockPos, nextPos, "TEXT", this->currentType, isInclude);
                                }

                                DefineSubSection(codeToParse, nextPos, first_endline_pos, "UNIFORM", this->currentType, isInclude);

                                vLastBlockPos = first_endline_pos;
                            }
                        }
                    }
                }
            }
            if (nextPos != std::string::npos) {
                ++nextPos;  // pour pas bloquer la boucle
            }
        }
    }

    return uniformFound;
}

bool SectionCode::ParseReplaceCode(const std::string& codeToParse, size_t& vLastBlockPos) {
    bool replaceCodeStartFound = false;

    // les configs
    size_t nextPos = 0;
    while (nextPos != std::string::npos) {
        nextPos = ShaderStageParsing::GetTagPos(codeToParse, codeTags.CodeToReplaceStartTag, nextPos, false);
        if (nextPos != std::string::npos) {
            if (!ShaderStageParsing::IfInCommentZone(codeToParse, nextPos) && stageName != "NOTE") {
                // error on peut pas ouvir un replace code sans fermer le precedent
                replaceCodeStartFound = true;

                size_t posWithoutTag = nextPos;
                size_t pos = nextPos + codeTags.CodeToReplaceStartTag.size();

                size_t sl = ct::GetCountOccurenceInSection(codeToParse, 0, vLastBlockPos, '\n');
                size_t el = ct::GetCountOccurenceInSection(codeToParse, 0, posWithoutTag, '\n');

                if (el > sl)  // on fait une section text que si elle fait au moin une ligne
                {
                    DefineSubSection(codeToParse, vLastBlockPos, posWithoutTag, "TEXT", this->currentType, isInclude);
                }

                auto nextOpenTag = ShaderStageParsing::GetTagPos(codeToParse, codeTags.CodeToReplaceStartTag, pos, false);
                nextPos = ShaderStageParsing::GetTagPos(codeToParse, codeTags.CodeToReplaceEndTag, pos, false);
                if (nextPos != std::string::npos) {
                    if (nextOpenTag != std::string::npos) {
                        if (nextOpenTag < nextPos) {
                            SetSyntaxError(relativeFile,
                                           "Replace Code Syntax Error",
                                           true,
                                           "you must close a Replace flag, before open a new one",
                                           ct::GetCountOccurenceInSection(codeToParse, 0, nextOpenTag, '\n') + this->sourceCodeStartLine);
                        }
                    }
                    if (!ShaderStageParsing::IfInCommentZone(codeToParse, nextPos)) {
                        if (pos) {
                            sl = ct::GetCountOccurenceInSection(codeToParse, 0, pos, '\n') + this->sourceCodeStartLine;
                            el = ct::GetCountOccurenceInSection(codeToParse, 0, nextPos, '\n') + this->sourceCodeStartLine;

                            DefineSubSection(codeToParse, pos, nextPos, "REPLACE_CODE", this->currentType, isInclude);

                            vLastBlockPos = ShaderStageParsing::GetTagPos(codeToParse, "\n", nextPos + codeTags.CodeToReplaceEndTag.size());
                            if (vLastBlockPos != std::string::npos)
                                ++vLastBlockPos;
                            else
                                vLastBlockPos = nextPos + codeTags.CodeToReplaceEndTag.size();
                        }
                    }
                }
            }
            if (nextPos != std::string::npos) {
                ++nextPos;  // pour pas bloquer la boucle
            }
        }
    }

    return replaceCodeStartFound;
}

bool SectionCode::ParseFragColorLayouts(const std::string& codeToParse) {
    bool layoutFound = false;

    /*
    can be :
    layout
    ( location = 0 )
    out vec4 fragColor
    ;

    or

    layout(location = 1) out vec4 fragPos;
    */
    if (parentType == "FRAGMENT") {
        if (parentKey.expired())
            return false;
        auto pkeyPtr = parentKey.lock();
        if (pkeyPtr.use_count()) {
            size_t nextPos = 0;
            while (nextPos != std::string::npos) {
                nextPos = ShaderStageParsing::GetTagPos(codeToParse, codeTags.LayoutWordTag, nextPos, true);
                if (nextPos != std::string::npos) {
                    size_t endLinePos = codeToParse.find(';', nextPos);
                    if (endLinePos != std::string::npos) {
                        endLinePos += 1U;
                        std::string _code = codeToParse.substr(nextPos, endLinePos - nextPos);
                        ct::replaceString(_code, "\t", "");
                        ct::replaceString(_code, "\r", "");  // unix
                        ct::replaceString(_code, "\n", "");

                        size_t locationPos = _code.find("location");
                        if (locationPos != std::string::npos) {
                            size_t equalPos = _code.find("=", locationPos);
                            if (equalPos != std::string::npos) {
                                equalPos += 1U;
                                size_t endParPos = _code.find(")", equalPos);
                                if (endParPos != std::string::npos) {
                                    size_t outPos = _code.find("out", endParPos);
                                    if (outPos != std::string::npos) {
                                        size_t vec4Pos = _code.find("vec4", outPos);
                                        if (vec4Pos != std::string::npos) {
                                            vec4Pos += 4U;

                                            size_t firstChar = _code.find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", vec4Pos);
                                            if (firstChar != std::string::npos) {
                                                size_t endSpaceOrSemiColon = _code.find_first_of(" ;", firstChar);
                                                if (endSpaceOrSemiColon != std::string::npos) {
                                                    layoutFound = true;

                                                    auto _number = _code.substr(equalPos, endParPos - equalPos);
                                                    ct::replaceString(_number, " ", "");
                                                    uint8_t locationID =
                                                        (uint8_t)ct::StringToNumber<int>(_number);  // en utilisant ct::StringToNumber<uint8_t> il m'a renvoyé des char
                                                                                                    // avec un '0' valant 48... CONNARD !!!

                                                    auto _name = _code.substr(firstChar, endSpaceOrSemiColon - firstChar);

                                                    pkeyPtr->AddFragColorName(locationID, _name);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                if (nextPos != std::string::npos) {
                    ++nextPos;  // pour pas bloquer la boucle
                }
            }
        }
    }

    return layoutFound;
}

inline std::map<size_t, std::string> GetSectionsFromCode(const std::string vCode, CodeTagStruct vCodeTags) {
    std::map<size_t, std::string> sections;

    size_t lastFound = 0;

    while (lastFound != std::string::npos) {
        auto tag = ShaderStageParsing::GetTagIfFound(vCode, lastFound, &lastFound, true);
        if (!tag.empty()) {
            sections[lastFound] = tag;
        }

        if (lastFound != std::string::npos)
            ++lastFound;
    }

    return sections;
}

void SectionCode::Parse() {
    if (!code.empty()) {
        Clear();

        std::string codeToParse = code;

        // on va remplacer les blocks CodeInsert avant le parse.
        // c'est pouruqoi on creer une copie du code qu'on utilisera apres
        // pour pas changer le code qui sert dans le processus de maj
        // std::string codeToParse = parentKey->CompleteShaderCodeWithReplaceCode(code);

        FileHelper::Instance()->SaveToFile(codeToParse, "SectionCode_Parse.glsl", (int)FILE_LOCATION_Enum::FILE_LOCATION_DEBUG);

        if (codeToParse == "#include \"bin\\scripts\\kifs\\ifs.glsl\":@SDF\n") {
            // CTOOL_DEBUG_BREAK;
        }

        auto sections = GetSectionsFromCode(codeToParse, codeTags);
        if (!sections.empty()) {
            // pour faire une iteration de plus pour parser le dernier bloc
            sections[codeToParse.size()] = "FULL";

            size_t last_idx = 0;
            std::string last_tag;

            for (auto& section : sections) {
                size_t idx = section.first;
                auto tag = section.second;

                if (idx > 0) {
                    if (!last_tag.empty()) {
                        // last_tag is the section name without the @ so we add 1 +> last_tag.size() + 1
                        DefineSubSection(codeToParse, last_idx + last_tag.size() + 1, idx, last_tag, parentType, isInclude);
                    }
                }

                last_idx = idx;
                last_tag = tag;
            }
        } else {
            // on va parser les fichier includes et les configs en boucle
            // this
            size_t lastBlockPos = 0;

            // l'ordre ici est tres important
            if (!ParseReplaceCode(codeToParse, lastBlockPos)) {
                if (!ParseConfigs(codeToParse, lastBlockPos)) {
                    if (!ParseIncludes(codeToParse, lastBlockPos)) {
                        ParseUniforms(codeToParse, lastBlockPos);
                        ParseFragColorLayouts(codeToParse);
                    }
                }
            }

            if (lastBlockPos > 0) {
                size_t lastSectionPos = codeToParse.size();
                if (lastBlockPos < lastSectionPos) {
                    DefineSubSection(codeToParse, lastBlockPos, lastSectionPos, "TEXT", this->currentType, isInclude);
                }
            }
        }

        // parse sub sections
        for (auto subSection : subSections) {
            for (auto it2 : subSection.second) {
                it2->Parse();
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SectionCode::GetShaderPart(std::shared_ptr<SectionCode> /*vSectionCodeParent*/,
                                       const std::string& vSectionType,
                                       size_t vCurrentFinalLine,
                                       std::string vRootType,
                                       const std::string& vDesiredStageName,
                                       const std::string& vDesiredInFileBufferName,
                                       const std::string& vDesiredSectionName,
                                       const std::string& vDesiredConfigName,
                                       std::map<KEY_TYPE_Enum, std::set<std::string>>* vUsedFiles,
                                       bool vFound,
                                       int vLevel) {
    std::string res;

    std::string _desiredSectionName = vDesiredSectionName;
    std::string _desiredStageName = vDesiredStageName;

    if (!vFound) {
        if (currentType == vRootType) {
            vFound = true;
        }

        if (currentType == "COMMON") {
            if (vRootType == "VERTEX" || vRootType == "GEOMETRY" || vRootType == "FRAGMENT" || vRootType == "TESSCONTROL" || vRootType == "TESSEVAL")
                vFound = true;
        } else if (!_desiredStageName.empty()) {
            if (vRootType != "UNIFORMS" && vRootType != "COMMON") {
                vFound = (currentType == _desiredStageName);
            }
        }
    }

    if (currentType == "INCLUDE") {
        vFound = false;
    }

    // si on est pas dans la bonne config, on quitte
    if (currentType == "CONFIG") {
        if (!name.empty() && !vDesiredConfigName.empty() && name != vDesiredConfigName) {
            ResetFinalLineMarks(vSectionType);
            return res;
        }
    } else {
        if (!vDesiredInFileBufferName.empty() && !inFileBufferName.empty() && vDesiredInFileBufferName != inFileBufferName) {
            return res;
        }

        if (!_desiredSectionName.empty()) {
            if (!name.empty() && _desiredSectionName != name) {
                return res;
            }
        }
    }

    if (!subSections.empty()) {
        size_t finalLine = vCurrentFinalLine;
        for (auto& subSection : subSections) {
            for (auto sec : subSection.second) {
                std::string _sectionType = vSectionType;

                // si la section parent possede un nom de section force, alors ca remplace le choix de l'utilisateur
                if (!forcedSectionName.empty()) {
                    _desiredSectionName = forcedSectionName;
                }

                if (!forcedStageName.empty()) {
                    _desiredStageName = forcedStageName;
                }

                std::string _code = sec->GetShaderPart(m_This,
                                                       _sectionType,
                                                       finalCodeStartLine[_sectionType] + finalLine,
                                                       vRootType,
                                                       _desiredStageName,
                                                       vDesiredInFileBufferName,
                                                       _desiredSectionName,
                                                       vDesiredConfigName,
                                                       vUsedFiles,
                                                       vFound,
                                                       vLevel + 1);

                if (!_code.empty()) {
                    finalLine += ct::GetCountOccurence(_code, "\n");
                    res += _code;
                }
            }
        }
    } else if (vFound) {
        if (isInclude)
            (*vUsedFiles)[KEY_TYPE_Enum::KEY_TYPE_INCLUDE].emplace(relativeFile);
        else
            (*vUsedFiles)[KEY_TYPE_Enum::KEY_TYPE_SHADER].emplace(relativeFile);

        finalCodeStartLine[vSectionType] = vCurrentFinalLine;
        finalCodeEndLine[vSectionType] = finalCodeStartLine[vSectionType] + ct::GetCountOccurence(this->code, "\n") - 1;

        printf("l:%u t|pt:%s|%s\n", (uint32_t)vLevel, currentType.c_str(), vRootType.c_str());

        res += code;
    }

    return res;
}

std::string SectionCode::GetSectionPart(const std::string& vSectionType,
                                        size_t vCurrentFinalLine,
                                        std::string vOriginType,
                                        std::string vParentType,
                                        const std::string& vDesiredInFileBufferName,
                                        const std::string& vDesiredSectionName,
                                        const std::string& vDesiredConfigName,
                                        std::map<KEY_TYPE_Enum, std::set<std::string>>* vUsedFiles) {
    std::string res;

    if (!vSectionType.empty() && !subSections.empty()) {
        size_t finalLine = vCurrentFinalLine;
        for (auto& subSection : subSections) {
            for (auto sec : subSection.second) {
                if (sec->currentType == vOriginType || sec->currentType == vParentType || sec->currentType == "COMMON") {
                    std::string _code = sec->GetShaderPart(m_This,
                                                           vSectionType,
                                                           finalCodeStartLine[vSectionType] + finalLine,
                                                           vParentType,
                                                           std::string(),  // pas de desired stage par default
                                                           vDesiredInFileBufferName,
                                                           vDesiredSectionName,
                                                           vDesiredConfigName,
                                                           vUsedFiles,
                                                           false,
                                                           1);

                    if (!_code.empty()) {
                        finalLine += ct::GetCountOccurence(_code, "\n");
                        res += _code;
                    }
                }
            }
        }
    }

    return res;
}

std::shared_ptr<SectionCode> SectionCode::GetSectionCodeForTargetLine(const std::string& vSectionName, size_t vLine) {
    std::shared_ptr<SectionCode> res = nullptr;

    if (!subSections.empty()) {
        for (auto& subSection : subSections) {
            // comme les sous section it2 sont des repetitions du meme code, pas besoin d'iterer pour la conversion de ligne
            // for (const auto& it2 : it->second)
            {
                auto sec = (*subSection.second.begin());
                res = sec->GetSectionCodeForTargetLine(vSectionName, vLine);
                if (res)
                    break;
            }
        }
    } else {
        if (vLine >= finalCodeStartLine[vSectionName] && vLine <= finalCodeEndLine[vSectionName]) {
            res = m_This;
        }
    }

    return res;
}

void SectionCode::ResetFinalLineMarks(const std::string& vSectionName) {
    finalCodeStartLine[vSectionName] = 0;
    finalCodeEndLine[vSectionName] = 0;

    for (auto& subSection : subSections) {
        // comme les sous section it2 sont des repetitions du meme code, pas besoin d'iterer pour la conversion de ligne
        // for (const auto& it2 : it->second)
        {
            auto sec = (*subSection.second.begin());
            sec->ResetFinalLineMarks(vSectionName);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

void SectionCode::SetSyntaxError(const std::string& vKey, const std::string& vErrorType, bool vErrorOrWarnings, const std::string& vError, size_t vLine) {
    auto pkeyPtr = parentKey.lock();
    if (pkeyPtr.use_count() && pkeyPtr->puParentCodeTree) {
        auto key = pkeyPtr->puParentCodeTree->GetKey(vKey);
        if (key) {
            SetSyntaxError(key, vErrorType, vErrorOrWarnings, vError, vLine);
        }
    }
}

void SectionCode::SetSyntaxError(std::weak_ptr<ShaderKey> vKey, const std::string& vErrorType, bool vErrorOrWarnings, const std::string& vError, size_t vLine) {
    auto vKeyPtr = vKey.lock();
    if (vKeyPtr.use_count()) {
        auto pkeyPtr = parentKey.lock();
        if (pkeyPtr.use_count()) {
            if (vErrorOrWarnings) {
                pkeyPtr->puParseSuccess = ShaderMsg::SHADER_MSG_ERROR;
            } else {
                pkeyPtr->puParseSuccess = ShaderMsg::SHADER_MSG_WARNING;
            }
        }

        std::string fromFile = vKeyPtr->puKey;
        if (!vKeyPtr->puInFileBufferFromKey.empty())
            fromFile = vKeyPtr->puInFileBufferFromKey;
        vKeyPtr->GetSyntaxErrors()->SetSyntaxError(vKeyPtr, "ERROR", vErrorType, vErrorOrWarnings, LineFileErrors(vLine, fromFile, vError));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////