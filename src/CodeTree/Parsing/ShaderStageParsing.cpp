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

#include "ShaderStageParsing.h"

#include <ctools/FileHelper.h>
#include <CodeTree/Parsing/SectionCode.h>
#include <CodeTree/CodeTree.h>
#include <CodeTree/ShaderKey.h>
#include <ctools/Logger.h>
#include <filesystem>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// ShaderNoteStruct ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderNoteStruct::clear() {
    dico.clear();
    urls.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// ProjectConfigStruct /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProjectConfigStruct::ApplyDefault() {
}

void ProjectConfigStruct::Complete(ProjectConfigStruct* vConfigToComplete) {
    if (vConfigToComplete) {
        if (countChanges) {
        }
    }
}

void ProjectConfigStruct::CheckChangeWith(ProjectConfigStruct* vConfigToCcompareForChanges) {
    if (vConfigToCcompareForChanges) {
        needSceneUpdate = false;

        ApplyDefault();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// SceneConfigStruct ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneConfigStruct::ApplyDefault() {
    // apply default before check
    /*
    if (shaders.empty())
        shaders.emplace("main");
    */
}

void SceneConfigStruct::Complete(SceneConfigStruct* vConfigToComplete) {
    if (vConfigToComplete && countChanges) {
        static SceneConfigStruct _default;

        if (shaders != _default.shaders)
            vConfigToComplete->shaders = shaders;
    }
}

void SceneConfigStruct::CheckChangeWith(SceneConfigStruct* vConfigToCcompareForChanges) {
    if (vConfigToCcompareForChanges) {
        needSceneUpdate = false;

        ApplyDefault();

        for (auto& shader : shaders) {
            if (vConfigToCcompareForChanges->shaders.find(shader) == vConfigToCcompareForChanges->shaders.end()) {
                // not found
                needSceneUpdate |= true;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// FrameBufferShaderConfigStruct ///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameBufferShaderConfigStruct::Complete(FrameBufferShaderConfigStruct* vConfigToComplete) {
    if (vConfigToComplete) {
        if (countChanges) {
            static FrameBufferShaderConfigStruct _default;

            if (countIterations != _default.countIterations)
                vConfigToComplete->countIterations = countIterations;
            if (ratio > _default.ratio)
                vConfigToComplete->ratio = ratio;
            if (size != _default.size)
                vConfigToComplete->size = size;
            if (format != _default.format)
                vConfigToComplete->format = format;
            if (mipmap != _default.mipmap)
                vConfigToComplete->mipmap = true;
            if (wrap != _default.wrap)
                vConfigToComplete->wrap = wrap;
            if (filter != _default.filter)
                vConfigToComplete->filter = filter;
            if (count != _default.count)
                vConfigToComplete->count = count;
            if (bufferName != _default.bufferName)
                vConfigToComplete->bufferName = bufferName;
        }
    }
}

void FrameBufferShaderConfigStruct::CheckChangeWith(FrameBufferShaderConfigStruct* vConfigToCompareForChanges) {
    if (vConfigToCompareForChanges) {
        needSizeUpdate = false;

        if (IS_FLOAT_DIFFERENT(ratio, vConfigToCompareForChanges->ratio))
            needSizeUpdate |= true;
        if (size.x != vConfigToCompareForChanges->size.x)
            needSizeUpdate |= true;
        if (size.y != vConfigToCompareForChanges->size.y)
            needSizeUpdate |= true;

        needFBOUpdate = false;

        if (countIterations != vConfigToCompareForChanges->countIterations)
            needCountIterationsUpdate |= true;
        if (format != vConfigToCompareForChanges->format)
            needFBOUpdate |= true;
        if (mipmap != vConfigToCompareForChanges->mipmap)
            needFBOUpdate |= true;
        if (wrap != vConfigToCompareForChanges->wrap)
            needFBOUpdate |= true;
        if (filter != vConfigToCompareForChanges->filter)
            needFBOUpdate |= true;
        if (count != vConfigToCompareForChanges->count)
            needFBOUpdate |= true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// VertexShaderConfigStruct ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VertexShaderConfigStruct::ApplyDefault() {
    // apply default before check
    if (meshType == BaseMeshEnum::PRIMITIVE_TYPE_NONE)
        meshType = BaseMeshEnum::PRIMITIVE_TYPE_QUAD;
    if (defaultDisplayMode.empty())
        defaultDisplayMode = "TRIANGLES";
    if (displayMode.empty())
        displayMode[defaultDisplayMode] = ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES;
}

void VertexShaderConfigStruct::Complete(VertexShaderConfigStruct* vConfigToComplete) {
    if (vConfigToComplete && countChanges) {
        static VertexShaderConfigStruct _default;

        // not apply default here, else the completion become bad
        if (type != _default.type)
            vConfigToComplete->type = type;
        if (countInstances != _default.countInstances)
            vConfigToComplete->countInstances = countInstances;
        if (lineWidth != _default.lineWidth)
            vConfigToComplete->lineWidth = lineWidth;
        if (countVertexs != _default.countVertexs)
            vConfigToComplete->countVertexs = countVertexs;
        if (meshType != _default.meshType)
            vConfigToComplete->meshType = meshType;
        if (modelFileToLoad != _default.modelFileToLoad)
            vConfigToComplete->modelFileToLoad = modelFileToLoad;
        if (defaultDisplayMode != _default.defaultDisplayMode)
            vConfigToComplete->defaultDisplayMode = defaultDisplayMode;
        if (displayMode != _default.displayMode)
            vConfigToComplete->displayMode = displayMode;
    }
}

void VertexShaderConfigStruct::CheckChangeWith(VertexShaderConfigStruct* vConfigToCompareForChanges) {
    if (vConfigToCompareForChanges) {
        needLineWidthUpdate = false;
        needCountPointUpdate = false;
        needModelUpdate = false;
        needDisplayModeUpdate = false;

        ApplyDefault();

        if (lineWidth != vConfigToCompareForChanges->lineWidth)
            needLineWidthUpdate |= true;
        if (countInstances != vConfigToCompareForChanges->countInstances)
            needCountPointUpdate |= true;
        if (countVertexs != vConfigToCompareForChanges->countVertexs)
            needCountPointUpdate |= true;
        if (meshType != vConfigToCompareForChanges->meshType)
            needModelUpdate |= true;
        if (modelFileToLoad != vConfigToCompareForChanges->modelFileToLoad)
            needModelUpdate |= true;
        if (defaultDisplayMode != vConfigToCompareForChanges->defaultDisplayMode)
            needDisplayModeUpdate |= true;
        if (displayMode.size() != vConfigToCompareForChanges->displayMode.size())
            needDisplayModeUpdate |= true;
        if (!displayMode.empty() && !vConfigToCompareForChanges->displayMode.empty())
            if (displayMode[defaultDisplayMode] != vConfigToCompareForChanges->displayMode[vConfigToCompareForChanges->defaultDisplayMode])
                needDisplayModeUpdate |= true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// ComputeShaderConfigStruct ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ComputeShaderConfigStruct::ApplyDefault() {
}

void ComputeShaderConfigStruct::Complete(ComputeShaderConfigStruct* vConfigToComplete) {
    if (vConfigToComplete && countChanges) {
        static ComputeShaderConfigStruct _default;

        if (size != _default.size)
            vConfigToComplete->size = size;
        if (workgroups != _default.workgroups)
            vConfigToComplete->workgroups = workgroups;
        if (countIterations != _default.countIterations)
            vConfigToComplete->countIterations = countIterations;
    }
}

void ComputeShaderConfigStruct::CheckChangeWith(ComputeShaderConfigStruct* vConfigToCcompareForChanges) {
    if (vConfigToCcompareForChanges) {
        needSizeUpdate = false;
        needWorkgroupUpdate = false;
        needCountIterationsUpdate = false;

        ApplyDefault();

        if (size != vConfigToCcompareForChanges->size)
            needSizeUpdate |= true;
        if (workgroups != vConfigToCcompareForChanges->workgroups)
            needWorkgroupUpdate |= true;
        if (countIterations != vConfigToCcompareForChanges->countIterations)
            needCountIterationsUpdate |= true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// SectionConfigStruct /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SectionConfigStruct::Complete(SectionConfigStruct* vConfigToComplete) {
    if (vConfigToComplete) {
        sceneConfig.Complete(&vConfigToComplete->sceneConfig);
        framebufferConfig.Complete(&vConfigToComplete->framebufferConfig);
        vertexConfig.Complete(&vConfigToComplete->vertexConfig);
        computeConfig.Complete(&vConfigToComplete->computeConfig);
        projectConfig.Complete(&vConfigToComplete->projectConfig);
    }
}

void SectionConfigStruct::CheckChangeWith(SectionConfigStruct* vConfigToCcompareForChanges) {
    if (vConfigToCcompareForChanges) {
        sceneConfig.CheckChangeWith(&vConfigToCcompareForChanges->sceneConfig);
        framebufferConfig.CheckChangeWith(&vConfigToCcompareForChanges->framebufferConfig);
        vertexConfig.CheckChangeWith(&vConfigToCcompareForChanges->vertexConfig);
        computeConfig.CheckChangeWith(&vConfigToCcompareForChanges->computeConfig);
        projectConfig.CheckChangeWith(&vConfigToCcompareForChanges->projectConfig);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// STATIC FOR PARSING //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// todo : faire le test unitaire
size_t ShaderStageParsing::GetTagPos(const std::string& vCode, const std::string& vTag, size_t vStartPos, bool vCheckIfInCommentZone, bool vWordOnly) {
    size_t tagPos = std::string::npos;

    if (!vCode.empty()) {
        tagPos = vCode.find(vTag, vStartPos);
        if (tagPos != std::string::npos) {
            if (vWordOnly) {
                size_t endtag = tagPos + vTag.size();
                if (endtag < vCode.size())
                    if (vCode[endtag] != ' ' && vCode[endtag] != '\n' && vCode[endtag] != '\t')
                        tagPos = std::string::npos;
            }

            if (tagPos != std::string::npos) {
                if (vCheckIfInCommentZone)
                    if (IfInCommentZone(vCode, tagPos))
                        tagPos = std::string::npos;
            }
        }
    }

    return tagPos;
}

// todo : faire le test unitaire
std::string ShaderStageParsing::GetTagIfFound(const std::string& vCode, size_t vStartPos, size_t* vApproxPos, bool vCheckIfInCommentZone) {
    std::string tagFound;

    size_t tagPos = std::string::npos;

    if (!vCode.empty()) {
        tagPos = vCode.find('@', vStartPos);
        if (tagPos != std::string::npos) {
            ++tagPos;

            // on va juste verifier qu'on est pas attaché a un include
            if (tagPos > 1 && vCode[tagPos - 2] == ':') {
                // la on est sur un include
                // int i = 0;
            } else {
                size_t endtag = vCode.find_first_of(" \n\t\r", tagPos);
                if (endtag == std::string::npos)  // then end of file
                {
                    endtag = vCode.size();
                }
                if (endtag != std::string::npos) {
                    tagFound = vCode.substr(tagPos, endtag - tagPos);

                    // il y a des tags qui sont speciaux et que nous devons echapper
                    auto tagFoundToCompare = "@" + tagFound;
                    if (tagFoundToCompare == SectionCode::codeTags.ConfigStartTag || tagFoundToCompare == SectionCode::codeTags.ConfigEndTag ||
                        tagFoundToCompare == SectionCode::codeTags.BufferStartTag || tagFoundToCompare == SectionCode::codeTags.BufferEndTag) {
                        tagFound.clear();
                    }
                }

                if (!tagFound.empty()) {
                    if (vCheckIfInCommentZone) {
                        if (IfInCommentZone(vCode, tagPos))  // dans un commentaire alors on echappe
                        {
                            tagFound.clear();
                        }
                    }

                    // si un #include existe sur la meme ligne juste avant le @ alors on echappe
                    // utilsié apr ex : #include "shader.glsl":@SDF
                    if (IfOnIncludeLine(vCode, tagPos)) {
                        tagFound.clear();
                    }
                }
            }
        }

        if (vApproxPos) {
            *vApproxPos = tagPos - 1;
        }
    }

    return tagFound;
}

// todo : faire le test unitaire
bool ShaderStageParsing::IsTagFound(const std::string& vCode, const std::string& vTag, size_t vStartPos, size_t* vApproxPos, bool vCheckIfInCommentZone, bool vWordOnly) {
    bool tagFound = false;

    size_t tagPos = std::string::npos;

    if (!vCode.empty()) {
        tagPos = vCode.find(vTag, vStartPos);

        if (tagPos != std::string::npos) {
            tagFound = true;

            if (vWordOnly) {
                size_t endtag = tagPos + vTag.size();
                if (endtag < vCode.size()) {
                    if (vCode[endtag] != ' ' && vCode[endtag] != '\n' && vCode[endtag] != '\t') {
                        tagFound = false;
                    }
                }
            }

            if (tagFound) {
                if (vCheckIfInCommentZone) {
                    if (IfInCommentZone(vCode, tagPos)) {
                        tagFound = false;
                    }
                }
            }
        }

        if (vApproxPos) {
            *vApproxPos = tagPos;
        }
    }

    return tagFound;
}

// todo : faire le test unitaire
bool ShaderStageParsing::IfInCommentZone(const std::string& vCode, size_t vPos) {
    //// zone de commentaire => // est forcemment sur la meme ligne
    // size_t mark_comment = vCode->rfind("//", vPos);
    // if (mark_comment != std::string::npos)
    //{
    //	size_t mark_before = vCode->rfind("\n", vPos);
    //	if (mark_before != std::string::npos)
    //	{
    //		if (mark_before < mark_comment) // \n//#include => ok
    //		{
    //			return true;
    //		}
    //		else if (mark_before > mark_comment) // //\n#include => nok
    //		{
    //			return false;
    //		}
    //	}
    //	else // //#include => ok (juste ya pas de ligne avant :) )
    //	{
    //		return true;
    //	}
    // }
    //  zone de commentaire => /* */ est entre balise
    // size_t mark_before = vCode->rfind("/*", vPos);
    // if (mark_before != std::string::npos)
    //{
    //	size_t mark_after = vCode->find("*/", vPos);
    //	if (mark_after != std::string::npos)
    //	{
    //		return true; // /* #include */ => ok
    //	}
    // }
    // return false;

    size_t mark_comment = 0;

    // zone de commentaire //
    size_t lastLine = vCode.rfind('\n', vPos);
    if (lastLine != std::string::npos) {
        mark_comment = vCode.find("//", lastLine + 1);
    } else {
        mark_comment = vCode.rfind("//", vPos);
    }
    if (mark_comment != std::string::npos) {
        if (mark_comment < vPos) {
            return true;
        }
    }

    size_t mark_start_before = vCode.rfind("/*", vPos);
    if (mark_start_before != std::string::npos) {
        // on verifie que c'est le notre
        size_t mark_end_before = vCode.rfind("*/", vPos);
        if (mark_end_before != std::string::npos) {
            // zone de com deja fermee
            if (mark_start_before < mark_end_before) {
                return false;
            }
        }
        size_t mark_end_after = vCode.find("*/", vPos);
        if (mark_end_after != std::string::npos) {
            // on verifie que c'est le notre
            size_t mark_start_after = vCode.find("/*", vPos);
            if (mark_start_after != std::string::npos) {
                // zone de com deja fermee
                if (mark_start_after < mark_end_after) {
                    return false;
                }
            }

            return true;
        }
    }

    return false;
}

// todo : faire le test unitaire
bool ShaderStageParsing::IfOnIncludeLine(const std::string& vCode, size_t vPos) {
    size_t mark_include = 0;

    // zone de includ //
    size_t lastLine = vCode.rfind('\n', vPos);
    if (lastLine != std::string::npos) {
        mark_include = vCode.find(SectionCode::codeTags.IncludeWordTag, lastLine + 1);
    } else {
        mark_include = vCode.rfind(SectionCode::codeTags.IncludeWordTag, vPos);
    }
    if (mark_include != std::string::npos) {
        if (mark_include < vPos) {
            return true;
        }
    }

    return false;
}

// todo : faire le test unitaire
std::string ShaderStageParsing::supressCommentedCode(const std::string& vText) {
    std::string res;

    size_t lastPos = 0;
    while (lastPos != std::string::npos) {
        size_t posCommentLine = vText.find("//", lastPos);  // prioritaire
        size_t posCommentBlock = vText.find("/*", lastPos);

        if (posCommentLine < posCommentBlock) {
            if (posCommentLine != std::string::npos) {
                res += vText.substr(lastPos, posCommentLine - lastPos);

                size_t posEndLine = vText.find('\n', posCommentLine);
                if (posEndLine != std::string::npos) {
                    lastPos = posEndLine + 1;
                } else {
                    lastPos = vText.size();
                }
            }
        } else if (posCommentLine > posCommentBlock) {
            if (posCommentBlock != std::string::npos) {
                res += vText.substr(lastPos, posCommentBlock - lastPos);

                size_t posEndBlock = vText.find("*/", posCommentBlock);
                if (posEndBlock != std::string::npos) {
                    lastPos = posEndBlock + 2;
                } else {
                    // error
                    lastPos = vText.size();
                    // pas besoin de lancer une exception car ca veut dire que peu etre le */ est dans le code
                    // mais comme le /* est dans la conf il ne sera pas mit dans le code
                    // donc le driver du gpu va gueuler et bloquer la compil :)
                    // et si le */ est pas dans le code, on s'en fou, on commente jusqu'a la fin de la ligne, vu que le conf est sur une seule ligne
                }
            }
        } else {
            res += vText.substr(lastPos, vText.size() - lastPos);
            lastPos = std::string::npos;
        }
    }

    return res;
}

// todo : faire le test unitaire
bool ShaderStageParsing::FoundTagInCode(const std::string& vCode, const std::string& vTag, size_t* vPos, std::map<size_t, std::string>* vDico) {
    bool res = false;

    if (vPos) {
        if (*vPos != std::string::npos) {
            if (!vTag.empty() && vDico) {
                *vPos = vCode.find(vTag, *vPos);
                if (*vPos != std::string::npos) {
                    *vPos += vTag.size();
                    if (!ShaderStageParsing::IfInCommentZone(vCode, *vPos)) {
                        (*vDico)[*vPos] = vTag;
                        res = true;
                    }
                }
            } else {
                *vPos = std::string::npos;
            }
        }
    }

    return res;
}

// todo : faire le test unitaire
std::string ShaderStageParsing::GetStringBetweenChars(char vOpenChar, char vCloseChar, std::string& vCode, size_t vStartPos, ct::uvec2* vBlockLoc) {
    size_t p = vStartPos;
    size_t open = 0;
    size_t close = 0;

    int depth = 0;
    bool init = true;

    const size_t badPos = std::string::npos;

    if (vCode.find(vOpenChar) != badPos && vCode.find(vCloseChar) != badPos) {
        size_t startPos = badPos;
        size_t endPos = badPos;

        while (depth > 0 || init) {
            init = false;  // sinon avec depth qui vaut 0 au debut, ca va pas demarrer

            if (p != badPos) {
                open = vCode.find(vOpenChar, p);
                close = vCode.find(vCloseChar, p);

                if (open < close &&
                    // open != badPos &&
                    close != badPos) {
                    ++depth;
                    p = open + 1;
                    if (startPos == badPos)
                        startPos = p;
                } else if (close != badPos) {
                    --depth;
                    p = close + 1;
                    endPos = close;
                } else {
                    endPos = vCode.size() - 1;
                    break;
                }
            } else {
                endPos = vCode.size() - 1;
                break;
            }
        }

        if (vBlockLoc) {
            vBlockLoc->x = (uint32_t)startPos;
            vBlockLoc->y = (uint32_t)endPos;
        }

        if (/*!init && */ depth != 0) {
            if (vOpenChar == '(' || vOpenChar == '{') {
                LogVarDebugError("=====================================");
                LogVarDebugError("depth must be at 0 but is not");
                LogVarDebugError("for this code : \"%s\"", vCode.substr(startPos, endPos - startPos).c_str());
                LogVarDebugError("=====================================");
            }
        }

        return vCode.substr(startPos, endPos - startPos);
    }

    return "";
}

// todo : faire le test unitaire
size_t ShaderStageParsing::GetPosAtCoordInString(std::string& vCode, ct::uvec2 vCoord) {
    size_t pos = 0;

    size_t countLine = 0;
    while ((pos = vCode.find('\n', pos)) != std::string::npos) {
        ++pos;
        if (countLine == vCoord.x - 2) {
            break;
        }

        ++countLine;
    }

    if (pos != std::string::npos)
        pos += vCoord.y;

    return pos;
}

// todo : faire le test unitaire
std::string ShaderStageParsing::GetStringFromPosUntilChar(const std::string& vCode, const size_t& vPos, const char& vLimitChar) {
    std::string res;

    if (!vCode.empty() && vPos < vCode.size()) {
        size_t p = vCode.find(vLimitChar, vPos);
        if (p != std::string::npos) {
            res = vCode.substr(vPos, p - vPos + 1);
        }
    }

    return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// PARSE STAGE OR SECTION CONFIG LINE //////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderParsedStruct::ParseNote(const std::string& vNote) {
    note.clear();

    std::map<size_t, std::string> DicoposTag;

    size_t lastFound = 0;

    size_t namePos = 0;
    size_t datePos = 0;
    size_t urlPos = 0;
    size_t parentUrlPos = 0;
    size_t userPos = 0;
    size_t soundUrlPos = 0;

    while (lastFound != std::string::npos) {
        ShaderStageParsing::FoundTagInCode(vNote, "[[NAME]]:", &namePos, &DicoposTag);             // ALL
        ShaderStageParsing::FoundTagInCode(vNote, "[[DATE]]:", &datePos, &DicoposTag);             // ALL
        ShaderStageParsing::FoundTagInCode(vNote, "[[URL]]:", &urlPos, &DicoposTag);               // ALL
        ShaderStageParsing::FoundTagInCode(vNote, "[[PARENT_URL]]:", &parentUrlPos, &DicoposTag);  // SPF_VERTEXSHADERART // SPF_GLSLSANDBOX
        ShaderStageParsing::FoundTagInCode(vNote, "[[USER]]:", &userPos, &DicoposTag);             // ALL
        ShaderStageParsing::FoundTagInCode(vNote, "[[SOUND_URL]]:", &soundUrlPos, &DicoposTag);    // SPF_VERTEXSHADERART

        if (namePos == std::string::npos && datePos == std::string::npos && urlPos == std::string::npos && parentUrlPos == std::string::npos &&
            userPos == std::string::npos && soundUrlPos == std::string::npos) {
            lastFound = std::string::npos;
        }
    }

    size_t maxEndLine = 0;

    for (auto& it : DicoposTag) {
        const size_t& pos = it.first;
        const std::string& tag = it.second;

        if (tag == "[[NAME]]:") {
            size_t endLine = vNote.find('\n', pos);
            if (endLine != std::string::npos) {
                note.dico["name"].push_back(vNote.substr(pos, endLine - pos));
                maxEndLine = ct::maxi(maxEndLine, endLine);
            }
        }
        if (tag == "[[DATE]]:") {
            size_t endLine = vNote.find('\n', pos);
            if (endLine != std::string::npos) {
                note.dico["date"].push_back(vNote.substr(pos, endLine - pos));
                maxEndLine = ct::maxi(maxEndLine, endLine);
            }
        }
        if (tag == "[[URL]]:") {
            size_t endLine = vNote.find('\n', pos);
            if (endLine != std::string::npos) {
                note.dico["url"].push_back(vNote.substr(pos, endLine - pos));
                maxEndLine = ct::maxi(maxEndLine, endLine);
            }
        }
        if (tag == "[[PARENT_URL]]:") {
            size_t endLine = vNote.find('\n', pos);
            if (endLine != std::string::npos) {
                note.dico["parent_url"].push_back(vNote.substr(pos, endLine - pos));
                maxEndLine = ct::maxi(maxEndLine, endLine);
            }
        }
        if (tag == "[[USER]]:") {
            size_t endLine = vNote.find('\n', pos);
            if (endLine != std::string::npos) {
                std::string user = vNote.substr(pos, endLine - pos);
                size_t twoPointPos = user.find(':');
                if (twoPointPos != std::string::npos) {
                    auto _user = user.substr(0, twoPointPos);
                    note.dico["user"].push_back(_user);
                    ++twoPointPos;
                    if (user.size() > twoPointPos)
                        note.urls[_user] = user.substr(twoPointPos);
                } else {
                    note.dico["user"].push_back(user);
                }
                maxEndLine = ct::maxi(maxEndLine, endLine);
            }
        }
        if (tag == "[[SOUND_URL]]:") {
            size_t endLine = vNote.find('\n', pos + 1);
            if (endLine != std::string::npos) {
                note.dico["sound_url"].push_back(vNote.substr(pos, endLine - pos));
                maxEndLine = ct::maxi(maxEndLine, endLine);
            }
        }
    }

    ++maxEndLine;  // pour aller au dela du \n
    if (maxEndLine != std::string::npos) {
        if (vNote.size() > maxEndLine) {
            note.dico["infos"].push_back(vNote.substr(maxEndLine));
        }
    }
}

ShaderSectionConfig ShaderStageParsing::ParseShaderSectionConfigLine(const std::string& vConfigLine, size_t vCurrentFileLine) {
    UNUSED(vCurrentFileLine);

    ShaderSectionConfig res;

    std::string configLine = ShaderStageParsing::supressCommentedCode(vConfigLine);

    ct::replaceString(configLine, "\n", " ");
    ct::replaceString(configLine, "\r", "");

    const auto arr = ct::splitStringToVector(configLine, " ");
    if (!arr.empty()) {
        for (const auto& str : arr) {
            size_t firstParenthesis = str.find('(', 0);
            if (firstParenthesis != std::string::npos) {
                ++firstParenthesis;

                size_t lastParenthesis = str.find(')', firstParenthesis);
                if (lastParenthesis != std::string::npos) {
                    std::string tag = str.substr(0, firstParenthesis - 1);
                    ConfigTagParsedStruct conf = res[tag];
                    conf.tag = tag;
                    size_t currentIdx = conf.params.size();
                    conf.params.push_back({});

                    if (currentIdx) {
                        CTOOL_DEBUG_BREAK;
                    }

                    std::string params = str.substr(firstParenthesis, lastParenthesis - firstParenthesis);
                    std::vector<std::string> arr2 = ct::splitStringToVector(params, ":");
                    for (const auto& param : arr2) {
                        conf.params[currentIdx].emplace_back(ct::splitStringToVector(param, ","));
                    }

                    res[conf.tag] = conf;
                }
            }
        }
    }

    return res;
}

bool ShaderStageParsing::ParseSectionConfig_ReturnSectionName_SayIfWeCanUse(std::shared_ptr<SectionCode> vSectionCode,
                                                                            const std::string& vSectionLine,
                                                                            std::string vSectionType,
                                                                            size_t vCurrentFileLine,
                                                                            const std::string& vDesiredStageName,
                                                                            std::string* vSectionNameToReturn,
                                                                            std::vector<std::string>* vInFileBufferNameToReturn) {
    if (!vSectionCode)
        return false;
    if (vSectionCode->parentKey.expired())
        return false;
    if (!vSectionNameToReturn)
        return false;
    if (!vInFileBufferNameToReturn)
        return false;

    auto pkeyPtr = vSectionCode->parentKey.lock();
    if (pkeyPtr.use_count()) {
        ShaderSectionConfig tags = ParseShaderSectionConfigLine(vSectionLine, vCurrentFileLine);

        if (tags.find("SECTION") != tags.end())  // found
        {
            auto sec = tags["SECTION"];

            if (sec.params.size() == 1)  // 1 seul tag
            {
                auto params = sec.params[0];

                if (!params.empty()) {
                    if (!params[0].empty()) {
                        *vSectionNameToReturn = params[0][0];
                        pkeyPtr->AddSectionName(*vSectionNameToReturn, true);
                    }
                }
            }

            tags.erase("SECTION");
        }

        std::vector<std::string> inFileBufferNames;
        std::string forcedSectionName;

        if (tags.find("BUFFER") != tags.end())  // found
        {
            auto sec = tags["BUFFER"];

            if (sec.params.size() == 1)  // 1 seul tag
            {
                auto params = sec.params[0];

                if (!params.empty()) {
                    inFileBufferNames = params[0];
                    *vInFileBufferNameToReturn = inFileBufferNames;
                    for (const auto& infile : inFileBufferNames) {
                        auto name = infile;
                        ct::replaceString(name, " ", "");
                        pkeyPtr->AddBufferName(name, true);
                    }
                }
            }

            tags.erase("BUFFER");
        }

        if (vSectionType == "NOTE") {
            ParseNoteConfig(vSectionCode, tags, vCurrentFileLine);
        } else if (vSectionType == "PROJECT") {
            auto config = ParseProjectConfig(vSectionCode, tags, vCurrentFileLine);
            if (inFileBufferNames.empty())
                inFileBufferNames.emplace_back("");
            for (const auto& it : inFileBufferNames) {
                if (!forcedSectionName.empty()) {
                    pkeyPtr->puBufferSectionConfig[it][""];
                    config.Complete(&pkeyPtr->puBufferSectionConfig[it][""].projectConfig);
                } else {
                    pkeyPtr->puBufferSectionConfig[it][*vSectionNameToReturn].projectConfig = config;
                }
            }
        } else if (vSectionType == "FRAMEBUFFER") {
            if (!vDesiredStageName.empty())
                return true;

            auto config = ParseFrameBufferConfig(vSectionCode, tags, vCurrentFileLine);
            if (inFileBufferNames.empty())
                inFileBufferNames.emplace_back("");
            for (const auto& it : inFileBufferNames) {
                if (!forcedSectionName.empty()) {
                    pkeyPtr->puBufferSectionConfig[it][""];
                    config.Complete(&pkeyPtr->puBufferSectionConfig[it][""].framebufferConfig);
                } else {
                    pkeyPtr->puBufferSectionConfig[it][*vSectionNameToReturn].framebufferConfig = config;
                }
            }
        } else if (vSectionType == "VERTEX") {
            auto config = ParseVertexConfig(vSectionCode, tags, vCurrentFileLine);
            if (inFileBufferNames.empty())
                inFileBufferNames.emplace_back("");
            for (const auto& it : inFileBufferNames) {
                if (!forcedSectionName.empty()) {
                    pkeyPtr->puBufferSectionConfig[it][""];
                    config.Complete(&pkeyPtr->puBufferSectionConfig[it][""].vertexConfig);
                } else {
                    pkeyPtr->puBufferSectionConfig[it][*vSectionNameToReturn].vertexConfig = config;
                }
            }
        } else if (vSectionType == "GEOMETRY") {
            ParseGeometryConfig(vSectionCode, tags, vCurrentFileLine);
        } else if (vSectionType == "TESSCONTROL") {
            ParseTesselationControlConfig(vSectionCode, tags, vCurrentFileLine);
        } else if (vSectionType == "TESSEVAL") {
            ParseTesselationEvalConfig(vSectionCode, tags, vCurrentFileLine);
        } else if (vSectionType == "COMMON") {
            ParseCommonConfig(vSectionCode, tags, vCurrentFileLine);
        } else if (vSectionType == "FRAGMENT") {
            ParseFragmentConfig(vSectionCode, tags, vCurrentFileLine);
        } else if (vSectionType == "COMPUTE") {
            auto config = ParseComputeConfig(vSectionCode, tags, vCurrentFileLine);
            if (inFileBufferNames.empty())
                inFileBufferNames.emplace_back("");
            for (const auto& it : inFileBufferNames) {
                if (!forcedSectionName.empty()) {
                    pkeyPtr->puBufferSectionConfig[it][""];
                    config.Complete(&pkeyPtr->puBufferSectionConfig[it][""].computeConfig);
                } else {
                    pkeyPtr->puBufferSectionConfig[it][*vSectionNameToReturn].computeConfig = config;
                }
            }
        }
    }

    return true;
}

void ShaderStageParsing::ParseNoteConfig(std::shared_ptr<SectionCode> vSectionCode,
                                         const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags,
                                         size_t vCurrentFileLine) {
    UNUSED(vSectionCode);
    UNUSED(vTags);
    UNUSED(vCurrentFileLine);
}

ProjectConfigStruct ShaderStageParsing::ParseProjectConfig(std::shared_ptr<SectionCode> vSectionCode,
                                                           const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags,
                                                           size_t vCurrentFileLine) {
    UNUSED(vSectionCode);
    UNUSED(vCurrentFileLine);

    ProjectConfigStruct res;
    res.tags = vTags;

    return res;
}

SceneConfigStruct ShaderStageParsing::ParseSceneConfig(std::shared_ptr<SectionCode> vSectionCode,
                                                       const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags,
                                                       size_t vCurrentFileLine) {
    UNUSED(vSectionCode);
    UNUSED(vCurrentFileLine);

    SceneConfigStruct res;

    SceneConfigStruct defaultConfig;  // utilise pour recup les valeurs par default

    for (const auto& it : vTags) {
        ConfigTagParsedStruct conf = it.second;

        if (conf.tag == "LOAD") {
            for (auto& params : conf.params)  // pluesieur tag load
            {
                if (params.size() == 1) {
                    if (params[0].size() == 1)  // LOAD(main)
                    {
                        if (!params[0][0].empty()) {
                            ++res.countChanges;
                            res.shaders.emplace(params[0][0]);
                        }
                    }
                }
            }
        }
    }

    return res;
}

FrameBufferShaderConfigStruct ShaderStageParsing::ParseFrameBufferConfig(std::shared_ptr<SectionCode> vSectionCode,
                                                                         const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags,
                                                                         size_t vCurrentFileLine) {
    FrameBufferShaderConfigStruct config;

    if (!vSectionCode)
        return config;

    FrameBufferShaderConfigStruct defaultConfig;  // utilis� pour recup la esvaleurs default

    for (const auto& it : vTags) {
        ConfigTagParsedStruct conf = it.second;

        if (conf.params.size() == 1)  // 1 seul tag
        {
            auto params = conf.params[0];

            if (conf.tag == "SIZE") {
                /* 12/10/2020
                if (params.size() == 2) // SIZE(picture:toto.jpg)
                {
                    if (!params[0].empty())
                    {
                        if (params[0][0] == "picture")
                        {
                            if (!params[1].empty())
                            {
                                std::string filename = params[1][0];

                                std::string _filepathName = filename;
                                if (!FileHelper::Instance()->IsFileExist(_filepathName))
                                    _filepathName = FileHelper::Instance()->GetExistingFilePathForFile(_filepathName);

                                auto tex = Texture2D::createFromFile(_filepathName.c_str());
                                if (tex)
                                {
                                    ct::ivec2 _size = tex->getSize();
                                    if (_size != defaultConfig.size.xy())
                                    {
                                        ++config.countChanges;
                                        config.size = ct::ivec3(_size, 0);
                                    }
                                    SAFE_DELETE(tex);
                                }
                            }
                        }
                    }
                }
                else */
                if (params.size() == 1) {
                    if (params[0].size() == 1)  // SIZE(600)
                    {
                        int s = ct::ivariant(params[0][0]).GetI();
                        ct::ivec3 _size(s, s, 0);
                        if (s > 0) {
                            ++config.countChanges;
                            config.size = ct::ivec3(s, s, 0);
                        }
                    } else if (params[0].size() == 2)  // SIZE(800,600)
                    {
                        ct::ivec3 _size(ct::ivariant(params[0][0]).GetI(), ct::ivariant(params[0][1]).GetI(), 0);
                        if (!_size.xy().emptyAND()) {
                            ++config.countChanges;
                            config.size = _size;
                        }
                    }
                    /*else if (param[0].size() == 3) // SIZE(800,600,800)
                    {
                        ct::ivec3 _size(
                            ct::ivariant(param[0][0]).GetI(),
                            ct::ivariant(param[0][1]).GetI(),
                            ct::ivariant(param[0][2]).GetI());
                        if (!_size.empty())
                        {
                            ++config.countChanges;
                            config.size = _size;
                        }
                    }*/
                }
            }
            /* 12/10/2020
            else if (conf.tag == "RATIO")
            {
                if (params.size() == 2)
                {
                    if (!params[0].empty())
                    {
                        if (params[0][0] == "picture")
                        {
                            if (!params[1].empty())
                            {
                                std::string filename = params[1][0];

                                std::string _filepathName = filename;
                                if (!FileHelper::Instance()->IsFileExist(_filepathName))
                                    _filepathName = FileHelper::Instance()->GetAbsolutePathForFileLocation(_filepathName, FILE_LOCATION_ASSET);

                                auto tex = Texture2D::createFromFile(_filepathName.c_str());
                                if (tex)
                                {
                                    float ratio = tex->getRatio();
                                    if (IS_FLOAT_DIFFERENT(ratio, defaultConfig.ratio))
                                    {
                                        ++config.countChanges;
                                        config.ratio = ratio;
                                    }
                                    SAFE_DELETE(tex);
                                }
                            }
                        }
                    }
                }
                else if (params.size() == 1) // RATIO(1.25)
                {
                    if (params[0].size() == 1)
                    {
                        float _ratio = ct::fvariant(params[0][0]).GetF();
                        if (IS_FLOAT_DIFFERENT(_ratio, 0.0f))
                        {
                            ++config.countChanges;
                            config.ratio = _ratio;
                        }
                    }
                }
            }*/
            /*else if (conf.tag == "ITERATIONS")
            {
                if (!params.empty())
                {
                    if (params.size() == 1)
                    {
                        if (params[0].size() == 1)
                        {
                            uint32_t def = ct::uvariant(params[0][0]).GetU();
                            config.countIterations = ct::uvec4(0, 0, 0, def);
                            ++config.countChanges;
                        }
                    }
                    else if (params.size() == 3)
                    {
                        uint32_t inf = 0, sup = 0, def = 0;
                        if (params[0].size() == 1U) inf = ct::uvariant(params[0][0]).GetU();
                        if (params[1].size() == 1U) sup = ct::uvariant(params[1][0]).GetU();
                        if (params[2].size() == 1U) def = ct::uvariant(params[2][0]).GetU();
                        config.countIterations = ct::uvec4(inf, sup, def, def);
                        ++config.countChanges;
                    }
                }
            }*/
            else {
                if (params.size() == 1) {
                    if (params[0].size() == 1) {
                        std::string paramStr = params[0][0];
                        ct::fvariant var(paramStr);

                        if (conf.tag == "FORMAT") {
                            if (paramStr != defaultConfig.format) {
                                ++config.countChanges;
                                config.format = paramStr;
                            }
                        }
                        if (conf.tag == "MIPMAP") {
                            bool v = var.GetB();
                            if (v != defaultConfig.mipmap) {
                                ++config.countChanges;
                                config.mipmap = v;
                            }
                        }
                        if (conf.tag == "WRAP") {
                            if (paramStr != defaultConfig.wrap) {
                                ++config.countChanges;
                                config.wrap = paramStr;
                            }
                        }
                        if (conf.tag == "FILTER") {
                            if (paramStr != defaultConfig.filter) {
                                ++config.countChanges;
                                config.filter = paramStr;
                            }
                        }
                        if (conf.tag == "COUNT") {
                            int n = var.GetI();
                            if (n > 0 && n < 9) {
                                if (n != defaultConfig.count) {
                                    ++config.countChanges;
                                    config.count = n;
                                }
                            } else {
                                // auto pKey = vSectionCode->parentKey.lock();
                                // if (pKey)
                                {
                                    if (n < 1) {
                                        CTOOL_DEBUG_BREAK;

                                        vSectionCode->SetSyntaxError(
                                            vSectionCode->relativeFile, "FBO Config error", false, "the FBO count must be comprised between 1 and 8", vCurrentFileLine);

                                        /*pKey->GetSyntaxErrors()->SetSyntaxError(vSectionCode->parentKey, "Parsing Error :", "FBO Config", false,
                                            LineFileErrors(vCurrentFileLine, "", "the FBO count must be comprised between 1 and 8"));*/
                                    } else if (n > 8) {
                                        CTOOL_DEBUG_BREAK;

                                        vSectionCode->SetSyntaxError(vSectionCode->relativeFile,
                                                                     "FBO Config error",
                                                                     false,
                                                                     "A Framebuffer can't have more than 8 attachments\n FBO count must be comprised between 1 and 8",
                                                                     vCurrentFileLine);

                                        /*pKey->GetSyntaxErrors()->SetSyntaxError(vSectionCode->parentKey, "Parsing Error :", "FBO Config", false,
                                            LineFileErrors(vCurrentFileLine, "", "A Framebuffer can't have more than 8 attachments\n FBO count must be comprised between 1
                                           and 8"));*/
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return config;
}

VertexShaderConfigStruct ShaderStageParsing::ParseVertexConfig(std::shared_ptr<SectionCode> vSectionCode,
                                                               const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags,
                                                               size_t vCurrentFileLine) {
    VertexShaderConfigStruct config;

    if (!vSectionCode)
        return config;

    bool haveDisplayTag = false;

    for (const auto& itConf : vTags) {
        ConfigTagParsedStruct conf = itConf.second;

        if (conf.params.size() == 1)  // 1 seul tag
        {
            auto params = conf.params[0];

            if (conf.tag == "DISPLAY") {
                config.line = vCurrentFileLine;

                if (!params.empty()) {
                    haveDisplayTag = true;

                    config.displayMode.clear();

                    // le 1er c'est la liste des modes de visu
                    for (const auto& itMode : params[0]) {
                        ModelRenderModeEnum mode = GetModelRenderModeEnumFromString(itMode);
                        if (mode != ModelRenderModeEnum::MODEL_RENDER_MODE_NONE) {
                            config.displayMode[itMode] = mode;
                            ++config.countChanges;
                        } else {
                            CTOOL_DEBUG_BREAK;

                            vSectionCode->SetSyntaxError(
                                vSectionCode->relativeFile, "Section Display Type error", false, itMode + " is not a valid display mode", vCurrentFileLine);
                        }
                    }

                    // le second la visu par default
                    if (params.size() > 1) {
                        if (params[1].size() == 1) {
                            std::string confDefault = params[1][0];
                            if (config.displayMode.find(confDefault) != config.displayMode.end()) {
                                config.defaultDisplayMode = confDefault;
                                ++config.countChanges;
                            } else {
                                CTOOL_DEBUG_BREAK;

                                vSectionCode->SetSyntaxError(
                                    vSectionCode->relativeFile, "Section Display Type error", false, confDefault + " is not in the display list", vCurrentFileLine);

                                // on prend le 1er de la liste
                                if (!config.displayMode.empty()) {
                                    config.defaultDisplayMode = config.displayMode.begin()->first;
                                }
                                ++config.countChanges;
                            }
                        }
                    } else  // on va en mettre un par default
                    {
                        // on prend le 1er de la liste
                        if (!config.displayMode.empty()) {
                            config.defaultDisplayMode = config.displayMode.begin()->first;
                        }
                        ++config.countChanges;
                    }
                }
            } else if (conf.tag == "MESH") {
                config.type = conf.tag;
                config.line = vCurrentFileLine;

                config.meshType = BaseMeshEnum::PRIMITIVE_TYPE_MESH;
                config.meshFormat = BaseMeshFormatEnum::PRIMITIVE_FORMAT_PNTBTC;

                if (config.displayMode.empty()) {
                    config.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES)] = ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES;
                    config.defaultDisplayMode = GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES);
                    ++config.countChanges;
                }

                if (!params.empty()) {
                    // 3d model file path name
                    if (params.size() == 1) {
                        if (params[0].size() == 1) {
                            config.modelFileToLoad = params[0][0];

                            // extractCode contient le nom du fichier include
                            std::string validPathFile = FileHelper::Instance()->GetAbsolutePathForFileLocation(config.modelFileToLoad, 0);
                            if (validPathFile.empty()) {
                                auto pkeyPtr = vSectionCode->parentKey.lock();
                                if (pkeyPtr.use_count()) {
                                    validPathFile = FileHelper::Instance()->GetRelativePathToParent(config.modelFileToLoad, pkeyPtr->GetPath());
                                }
                                validPathFile = FileHelper::Instance()->SimplifyFilePath(validPathFile);
                            }

                            config.modelFileToLoad = validPathFile;
                            ++config.countChanges;
                        }
                    }
                }
            } else if (conf.tag == "QUAD") {
                config.type = conf.tag;
                config.line = vCurrentFileLine;
                config.meshType = BaseMeshEnum::PRIMITIVE_TYPE_QUAD;

                if (config.displayMode.empty()) {
                    config.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES)] = ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES;
                    config.defaultDisplayMode = GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES);
                    ++config.countChanges;
                }

                if (!params.empty()) {
                }
            } else if (conf.tag == "POINTS") {
                config.type = conf.tag;
                config.line = vCurrentFileLine;

                if (!params.empty()) {
                    if (params.size() == 1) {
                        if (params[0].size() == 1) {
                            uint32_t def = ct::uvariant(params[0][0]).GetU();
                            config.countVertexs = ct::uvec4(0, 0, 0, def);
                            ++config.countChanges;
                        }
                    } else if (params.size() == 3) {
                        uint32_t inf = 0, sup = 0, def = 0;
                        if (params[0].size() == 1U)
                            inf = ct::uvariant(params[0][0]).GetU();
                        if (params[1].size() == 1U)
                            sup = ct::uvariant(params[1][0]).GetU();
                        if (params[2].size() == 1U)
                            def = ct::uvariant(params[2][0]).GetU();
                        config.countVertexs = ct::uvec4(inf, sup, def, def);
                        ++config.countChanges;
                    }
                }

                config.meshType = BaseMeshEnum::PRIMITIVE_TYPE_POINTS;

                if (config.displayMode.empty()) {
                    config.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_POINTS)] = ModelRenderModeEnum::MODEL_RENDER_MODE_POINTS;
                    config.defaultDisplayMode = GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_POINTS);
                    ++config.countChanges;
                }
            } else if (conf.tag == "LINEWIDTH") {
                config.type = conf.tag;
                config.line = vCurrentFileLine;

                if (!params.empty()) {
                    if (params.size() == 1) {
                        if (params[0].size() == 1) {
                            config.lineWidth.w = ct::fvariant(params[0][0]).GetF();
                            ++config.countChanges;
                        }
                    } else if (params.size() == 3) {
                        float inf = 0, sup = 0, def = 0;
                        if (params[0].size() == 1U)
                            inf = ct::fvariant(params[0][0]).GetF();
                        if (params[1].size() == 1U)
                            sup = ct::fvariant(params[1][0]).GetF();
                        if (params[2].size() == 1U)
                            def = ct::fvariant(params[2][0]).GetF();
                        config.lineWidth = ct::fvec4(inf, sup, def, def);
                        ++config.countChanges;
                    }
                }
            } else if (conf.tag == "INSTANCES") {
                config.type = conf.tag;
                config.line = vCurrentFileLine;

                if (!params.empty()) {
                    if (params.size() == 1) {
                        if (params[0].size() == 1) {
                            uint32_t def = ct::uvariant(params[0][0]).GetU();
                            config.countInstances = ct::uvec4(0, 0, 0, def);
                            ++config.countChanges;
                        }
                    } else if (params.size() == 3) {
                        uint32_t inf = 0, sup = 0, def = 0;
                        if (params[0].size() == 1U)
                            inf = ct::uvariant(params[0][0]).GetU();
                        if (params[1].size() == 1U)
                            sup = ct::uvariant(params[1][0]).GetU();
                        if (params[2].size() == 1U)
                            def = ct::uvariant(params[2][0]).GetU();
                        config.countInstances = ct::uvec4(inf, sup, def, def);
                        ++config.countChanges;
                    }
                }
            }
        }
    }

    if (!haveDisplayTag) {
        config.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_POINTS)] = ModelRenderModeEnum::MODEL_RENDER_MODE_POINTS;
        config.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_LINES)] = ModelRenderModeEnum::MODEL_RENDER_MODE_LINES;
        config.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_LINE_STRIP)] = ModelRenderModeEnum::MODEL_RENDER_MODE_LINE_STRIP;
        config.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES)] = ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLES;
        config.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_STRIP)] = ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_STRIP;
        config.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_FAN)] = ModelRenderModeEnum::MODEL_RENDER_MODE_TRIANGLE_FAN;

        // quand on aura le tesselator
        // vSectionCode->parentKey->vertexConfig.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::vk::PrimitiveTopology::ePatchList)] =
        // ModelRenderModeEnum::vk::PrimitiveTopology::ePatchList;
        // vSectionCode->parentKey->vertexConfig.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::vk::PrimitiveTopology::eLineStrip_ADJACENCY)] =
        // ModelRenderModeEnum::vk::PrimitiveTopology::eLineStrip_ADJACENCY;
        // vSectionCode->parentKey->vertexConfig.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::vk::PrimitiveTopology::eLineList_ADJACENCY)] =
        // ModelRenderModeEnum::vk::PrimitiveTopology::eLineList_ADJACENCY;
        // vSectionCode->parentKey->vertexConfig.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::vk::PrimitiveTopology::eTriangleStrip_ADJACENCY)] =
        // ModelRenderModeEnum::vk::PrimitiveTopology::eTriangleStrip_ADJACENCY;
        // vSectionCode->parentKey->vertexConfig.displayMode[GetModelRenderModeEnumString(ModelRenderModeEnum::vk::PrimitiveTopology::eTriangleList_ADJACENCY)] =
        // ModelRenderModeEnum::vk::PrimitiveTopology::eTriangleList_ADJACENCY;
    }

    return config;
}

ComputeShaderConfigStruct ShaderStageParsing::ParseComputeConfig(std::shared_ptr<SectionCode> vSectionCode,
                                                                 const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags,
                                                                 size_t vCurrentFileLine) {
    ComputeShaderConfigStruct config;

    if (!vSectionCode)
        return config;

    for (const auto& itConf : vTags) {
        ConfigTagParsedStruct conf = itConf.second;

        if (conf.params.size() == 1)  // 1 seul tag
        {
            auto params = conf.params[0];

            if (conf.tag == "SIZE") {
                if (params.size() == 1)  // SIZE(10,10,10)
                {
                    if (params[0].size() == 3) {
                        ct::ivec3 _size(ct::ivariant(params[0][0]).GetI(), ct::ivariant(params[0][1]).GetI(), ct::ivariant(params[0][2]).GetI());
                        if (!_size.xy().emptyAND()) {
                            ++config.countChanges;
                            config.size = _size;
                        }
                    } else {
                        CTOOL_DEBUG_BREAK;

                        vSectionCode->SetSyntaxError(
                            vSectionCode->relativeFile, "Section Display Type error", false, "you must give 3 coords for size", vCurrentFileLine);
                    }
                }
            } else if (conf.tag == "WORKGROUPS") {
                if (params.size() == 1)  // WORKGROUPS(10,10,10)
                {
                    if (params[0].size() == 3) {
                        ct::ivec3 _workgroups(ct::ivariant(params[0][0]).GetI(), ct::ivariant(params[0][1]).GetI(), ct::ivariant(params[0][2]).GetI());
                        if (!_workgroups.xy().emptyAND()) {
                            ++config.countChanges;
                            config.workgroups = _workgroups;
                        }
                    } else {
                        CTOOL_DEBUG_BREAK;

                        vSectionCode->SetSyntaxError(
                            vSectionCode->relativeFile, "Section Display Type error", false, "you must give 3 coords for size", vCurrentFileLine);
                    }
                }
            } else if (conf.tag == "ITERATIONS") {
                config.type = conf.tag;
                config.line = vCurrentFileLine;

                if (!params.empty()) {
                    if (params.size() == 1) {
                        if (params[0].size() == 1) {
                            uint32_t def = ct::uvariant(params[0][0]).GetU();
                            config.countIterations = ct::uvec4(0, 0, 0, def);
                            ++config.countChanges;
                        }
                    } else if (params.size() == 3) {
                        uint32_t inf = 0, sup = 0, def = 0;
                        if (params[0].size() == 1U)
                            inf = ct::uvariant(params[0][0]).GetU();
                        if (params[1].size() == 1U)
                            sup = ct::uvariant(params[1][0]).GetU();
                        if (params[2].size() == 1U)
                            def = ct::uvariant(params[2][0]).GetU();
                        config.countIterations = ct::uvec4(inf, sup, def, def);
                        ++config.countChanges;
                    }
                }
            }
        }
    }

    return config;
}

void ShaderStageParsing::ParseGeometryConfig(std::shared_ptr<SectionCode> vSectionCode,
                                             const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags,
                                             size_t vCurrentFileLine) {
    UNUSED(vSectionCode);
    UNUSED(vTags);
    UNUSED(vCurrentFileLine);
}

void ShaderStageParsing::ParseTesselationControlConfig(std::shared_ptr<SectionCode> vSectionCode,
                                                       const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags,
                                                       size_t vCurrentFileLine) {
    UNUSED(vSectionCode);
    UNUSED(vTags);
    UNUSED(vCurrentFileLine);
}

void ShaderStageParsing::ParseTesselationEvalConfig(std::shared_ptr<SectionCode> vSectionCode,
                                                    const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags,
                                                    size_t vCurrentFileLine) {
    UNUSED(vSectionCode);
    UNUSED(vTags);
    UNUSED(vCurrentFileLine);
}

void ShaderStageParsing::ParseCommonConfig(std::shared_ptr<SectionCode> vSectionCode,
                                           const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags,
                                           size_t vCurrentFileLine) {
    UNUSED(vSectionCode);
    UNUSED(vTags);
    UNUSED(vCurrentFileLine);
}

void ShaderStageParsing::ParseFragmentConfig(std::shared_ptr<SectionCode> vSectionCode,
                                             const std::unordered_map<std::string, ConfigTagParsedStruct>& vTags,
                                             size_t vCurrentFileLine) {
    UNUSED(vSectionCode);
    UNUSED(vTags);
    UNUSED(vCurrentFileLine);
}

ReplaceCodeStruct ShaderStageParsing::ParseReplaceCode(std::shared_ptr<SectionCode> vSectionCode, const std::string& vExtractedCode, size_t vCurrentFileLine) {
    UNUSED(vCurrentFileLine);
    UNUSED(vSectionCode);

    /*
     * example de code:
     * le 1er c'est la clef : SLOT:voxelNormal:this is the map normal extraction function:
     * la suite jusqu'a ]] c'est le code par default
     * dans la clef on a type(name:help)
     * ici vCodeLine contient le texte entre les deux [[ et ]]
     * ca peu etre ca :
     * [[SLOT(voxelNormal:"this is the map normal extraction function")
     * vec3 voxelNormal(vec3 p, float prec)
     * {
     *	vec3 e = vec3(prec, 0, 0);
     * 	return normalize(vec3(
     * 		map(p+e)-map(p-e),
     * 		map(p+e.yxz)-map(p-e.yxz),
     * 		map(p+e.zyx)-map(p-e.zyx)));
     * }
     * ]]
     * ou ca :
     * [[SLOT(voxelNormal:"this is the map normal extraction function")]]
     * ou ca :
     * [[SLOT(voxelNormal)]] => le minimum est type et name
     * the syntax is : type:params and params are separated by ':', and params[0] must be the name
     */

    /*
     * autre exemple :
     * [[MAIN_CODE
     * void Init()
     * {
     *
     * }
     * ]]
     */

    /*
     * donc le cas general est :
     * [[PARAMS_LINE
     * CODE_BLOCK
     * ]]
     *
     * et dans PARAMS_LINE on peut avoir :
     * SLOT:vec3 toto(vec3 p, vec2 o) // toto function
     * SLOT est le type
     *
     * MAIN_CODE
     */

    ReplaceCodeStruct replaceCode;

    if (vSectionCode->stageName == "NOTE")
        return replaceCode;

    size_t endLine = vExtractedCode.find_first_of("\n\r");
    if (endLine != std::string::npos) {
        replaceCode.key = vExtractedCode.substr(0, endLine);
        replaceCode.name = replaceCode.key;
        replaceCode.defCode = vExtractedCode.substr(endLine + 1);

        size_t startPar = vExtractedCode.find(':');
        if (startPar != std::string::npos) {
            replaceCode.type = vExtractedCode.substr(0, startPar);

            std::string code = vExtractedCode;
            std::string paramCode = GetStringFromPosUntilChar(code, 0, '\n');
            if (!paramCode.empty()) {
                ct::uvec2 codeBlock;
                std::string params = ShaderStageParsing::GetStringBetweenChars(':', '\n', paramCode, 0, &codeBlock);
                auto vec = ct::splitStringToVector(params, ':');
                int idx = 0;
                for (auto s : vec) {
                    if (idx++ == 0) {
                        replaceCode.name = s;
                    } else {
                        replaceCode.params.push_back(s);
                    }
                }
                // replaceCode.key = code.substr(0, codeBlock.y + 1);
                // replaceCode.defCode = code.substr(codeBlock.y + 1);
            }
        }
    }

    return replaceCode;
}

std::weak_ptr<ShaderKey> ShaderStageParsing::ParseIncludeLine(std::shared_ptr<SectionCode> vSectionCode, const std::string& vCodeLine, size_t vCurrentFileLine) {
    ShaderKeyPtr key;

    auto pkeyPtr = vSectionCode->parentKey.lock();
    if (pkeyPtr.use_count()) {
        std::string _extractCode;
        std::string _forcedSectionName;
        std::string _forcedStageName;

        auto arr = ct::splitStringToVector(vCodeLine, ':');

        if (!arr.empty()) {
            _extractCode = arr[0];

            ct::replaceString(_extractCode, "\"", "");
            ct::replaceString(_extractCode, "\n", "");
            ct::replaceString(_extractCode, "\r", "");
            ct::replaceString(_extractCode, " ", "");

            if (arr.size() > 1) {
                // ca c'est par exemple si le fichier include a été ecrit comme ca :
                // #include "toto.glsl":section_tata
                // et la section c'est ca :
                //@FRAGMENT SECTION(section_tata)

                // si par contre ca a été marqué comme ca
                // #include "toto.glsl":@SDF
                // alors on va extraire un stage particulier
                // par contre on pourra toujours precisier une section en faisant
                // #include "toto.glsl":@SDF:section_tata
                // ca permet de piquer le code d'un stage et de le mettre dans un autre stage.
                // les configs et autre sont copié dnas le nouveua stage.
                // recupere egalement uniforms et common

                _forcedSectionName = arr[1];

                ct::replaceString(_forcedSectionName, "\"", "");
                ct::replaceString(_forcedSectionName, "\n", "");
                ct::replaceString(_forcedSectionName, "\r", "");
                ct::replaceString(_forcedSectionName, " ", "");

                if (arr[1][0] == '@') {
                    _forcedStageName = _forcedSectionName.substr(1U);

                    // si encore un alors on est comme ca :
                    // #include "toto.glsl":@SDF:section_tata

                    if (arr.size() > 2) {
                        _forcedSectionName = arr[2];

                        ct::replaceString(_forcedSectionName, "\"", "");
                        ct::replaceString(_forcedSectionName, "\n", "");
                        ct::replaceString(_forcedSectionName, "\r", "");
                        ct::replaceString(_forcedSectionName, " ", "");
                    } else {
                        _forcedSectionName.clear();
                    }
                }
            }

            // extractCode contient le nom du fichier include
            // si il y a une extention c'est un fichier
            // si non c'est une string et elle doit deja exister dans la codetree
            if (_extractCode.find('.', 0) == std::string::npos) {
                if (pkeyPtr->puParentCodeTree) {
                    key = pkeyPtr->puParentCodeTree->GetIncludeKey(_extractCode);  // .lock();
                    if (key) {
                        // infinite loop, olors on renvois rien
                        if (CheckIfExist_InfiniteIncludeLoop(vSectionCode, key->puKey)) {
                            vSectionCode->SetSyntaxError(
                                key, "Include Error", true, "Infinite Loop detected on Inclusion of " + key->puKey + ", script ignored !", vCurrentFileLine);

                            return std::weak_ptr<ShaderKey>();
                        }

                        key->puUsedByKeys.emplace(pkeyPtr->puKey);
                        key->puMainSection->forcedSectionName = _forcedSectionName;
                        key->puMainSection->forcedStageName = _forcedStageName;
                    }
                }
            } else {
                std::string validPathFile = FileHelper::Instance()->GetExistingFilePathForFile(_extractCode, true);
                if (validPathFile.empty()) {
                    // if (!FileHelper::Instance()->IsAbsolutePath(_extractCode))
                    //	_extractCode = FileHelper::Instance()->SimplifyFilePath(FileHelper::Instance()->GetAppPath() + "/" + pkeyPtr->GetPath() + "/" + _extractCode);

                    validPathFile = FileHelper::Instance()->GetRelativePathToParent(_extractCode, pkeyPtr->GetPath(), true);
                    validPathFile = FileHelper::Instance()->SimplifyFilePath(validPathFile);

                    if (validPathFile.empty()) {
                        validPathFile = FileHelper::Instance()->GetRelativePathToPath(_extractCode, pkeyPtr->GetPath());
                        validPathFile = FileHelper::Instance()->SimplifyFilePath(validPathFile);
                    }
                }

                // tentative de simplifier relativeFilePathName
                /*try
                {
                    auto basePath = FileHelper::Instance()->GetAppPath();
                    auto absPath = std::filesystem::canonical(validPathFile);
                    validPathFile = std::filesystem::relative(absPath, basePath).string();
                }
                catch (const std::exception& ex)
                {
                    LogVarError("%s", ex.what());
                }*/

                key = pkeyPtr->puParentCodeTree->AddOrUpdateFromFileAndGetKey(validPathFile, false, false, true);  // .lock();
                if (key) {
                    // infinite loop, olors on renvois rien
                    if (CheckIfExist_InfiniteIncludeLoop(vSectionCode, key->puKey)) {
                        vSectionCode->SetSyntaxError(
                            key, "Include Error", true, "Infinite Loop detected on Inclusion of " + key->puKey + ", script ignored !", vCurrentFileLine);

                        return std::weak_ptr<ShaderKey>();
                    }

                    key->puUsedByKeys.emplace(pkeyPtr->puKey);
                    key->puMainSection->forcedSectionName = _forcedSectionName;
                    key->puMainSection->forcedStageName = _forcedStageName;
                }

                std::string names = FileHelper::Instance()->GetPathRelativeToApp(validPathFile);
                pkeyPtr->puIncludeFileNames[names] = validPathFile;
            }

            //_extractCode = pkeyPtr->puParentCodeTree->GetCode(_absoluteFile);
        }

        if (key) {
            key->puIncludeFileInCode = vCodeLine;
        }
    }

    return std::weak_ptr<ShaderKey>(key);
}

// on check tout les parents jusqu'au parent 0, pour voir si on tombe sur un parent identique au nom inclu
// le but est de verifier 'lexistance d'une boucle infinie
bool ShaderStageParsing::CheckIfExist_InfiniteIncludeLoop(std::shared_ptr<SectionCode> vSectionCode, const std::string& vIncludeName) {
    auto section = vSectionCode;

    while (section) {
        if (section->isInclude) {
            if (section->relativeFile == vIncludeName) {
                // aie on a found le meme nom. on arrete !
                return true;
            } else {
                // rien a signaler. on remonte au parent !
                section = section->parentSection;
            }
        } else {
            section = nullptr;
        }
    }

    // rien found
    return false;
}

void ShaderStageParsing::ParseGeometryShaderForGetInAndOutPrimitives(std::shared_ptr<SectionCode> vSectionCode, const std::string& vCodeToParse) {
    // layout(triangles) in; // input primitive(s) (must match the current render mode)
    // layout(line_strip, max_vertices = 6) out; // output primitive(s)

    // la primitve input c'est : layout( code ) in;\n
    // la primitve input c'est : layout( code, x ) out;\n

    // std::string tag = "layout(";

    std::string primitive;
    std::string primitiveDirection;

    auto tags = SearchForGeometryLayout(vCodeToParse, "layout", '(', ')', ',', ';');

    auto pkeyPtr = vSectionCode->parentKey.lock();
    if (pkeyPtr.use_count()) {
        for (auto& it : tags) {
            primitiveDirection = it.dir;

            if (!it.params.empty()) {
                primitive = *(it.params.begin());

                if (!primitive.empty() && !primitiveDirection.empty()) {
                    if (primitiveDirection == "in") {
                        pkeyPtr->puGeometryInputRenderMode = (GLenum)GetModelRenderModeEnumFromString(primitive);
                    } else if (primitiveDirection == "out") {
                        pkeyPtr->puGeometryInputRenderMode = (GLenum)GetModelRenderModeEnumFromString(primitive);
                    }
                }
            }
        }
    }
}

std::vector<GeometryLayoutStruct> ShaderStageParsing::SearchForGeometryLayout(const std::string& vCodeToParse,
                                                                              const std::string& vTagToSearch,
                                                                              char vFirstChar,
                                                                              char vEndChar,
                                                                              char vSeparator,
                                                                              char vEndLineChar) {
    std::vector<GeometryLayoutStruct> layouts;

    size_t pos = 0;
    while ((pos = vCodeToParse.find(vTagToSearch, pos)) != std::string::npos) {
        size_t first_char = vCodeToParse.find_first_of(vFirstChar, pos);
        if (first_char != std::string::npos) {
            size_t end_char = vCodeToParse.find_first_of(vEndChar, first_char + 1);
            if (end_char != std::string::npos) {
                std::string str = vCodeToParse.substr(first_char + 1, end_char - first_char - 1);

                GeometryLayoutStruct layout;
                layout.pos = pos;
                layout.params = ct::splitStringToVector(str, vSeparator, false);

                size_t end_line_char = vCodeToParse.find_first_of(vEndLineChar, end_char + 1);
                if (end_line_char != std::string::npos) {
                    layout.line = vCodeToParse.substr(pos, end_line_char + 1 - pos);
                    layout.dir = vCodeToParse.substr(end_char + 1, end_line_char - end_char - 1);
                    ct::replaceString(layout.dir, " ", "");
                    ct::replaceString(layout.dir, "\n", "");
                    ct::replaceString(layout.dir, "\r", "");
                    ct::replaceString(layout.dir, "\t", "");
                    layouts.push_back(layout);
                }
            }
        }
        ++pos;
    }

    return layouts;
}