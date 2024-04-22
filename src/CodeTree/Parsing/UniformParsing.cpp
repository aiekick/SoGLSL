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

#include "UniformParsing.h"
#include <CodeTree/ShaderKey.h>
#include <CodeTree/CodeTree.h>
#include <ctools/Logger.h>

UniformParsedStruct UniformParsing::ParseUniformString(const std::string& vUniformString, const size_t& vCurrentFileLine, std::shared_ptr<SectionCode> vSectionCodePtr) {
    UniformParsedStruct uniParsed;

    if (!vSectionCodePtr) {
        LogVarError("vSectionCodePtr is null");
        return uniParsed;
    }

    if (!vUniformString.empty()) {
        size_t uniformPos = vUniformString.find("uniform");
        if (uniformPos != std::string::npos) {
            uniformPos += 7;

            if (vCurrentFileLine != std::string::npos)
                uniParsed.sourceCodeLine = vCurrentFileLine;

            // section
            size_t sectionPos = vUniformString.find_first_of("\t (", uniformPos);
            if (sectionPos != std::string::npos) {
                if (vUniformString[sectionPos] == '(') {
                    sectionPos += 1;

                    size_t lastParenthesisPos = vUniformString.find_first_of("()", sectionPos);
                    if (lastParenthesisPos != std::string::npos) {
                        if (vUniformString[lastParenthesisPos] == ')') {
                            uniParsed.sectionParams = vUniformString.substr(sectionPos, lastParenthesisPos - sectionPos);
                            uniformPos = lastParenthesisPos + 1;
                        } else {
                            uniParsed.badSyntaxDetected = true;
                            vSectionCodePtr->SetSyntaxError(vSectionCodePtr->relativeFile, "Uniform error", true, "bad syntax, missing )", vCurrentFileLine);
                        }
                    } else {
                        uniParsed.badSyntaxDetected = true;
                        vSectionCodePtr->SetSyntaxError(vSectionCodePtr->relativeFile, "Uniform error", true, "bad syntax, missing )", vCurrentFileLine);
                    }
                }
            }

            // type and params
            size_t typePos = vUniformString.find_first_of("abcdefghijklmnopqrstuvwxyz", uniformPos);
            if (typePos != std::string::npos) {
                size_t firstParenthesisPos = vUniformString.find('(', typePos + 1);
                if (firstParenthesisPos != std::string::npos) {
                    firstParenthesisPos += 1;

                    uniParsed.type = vUniformString.substr(typePos, firstParenthesisPos - (typePos + 1));

                    size_t lastParenthesisPos = vUniformString.find_first_of("()", firstParenthesisPos);
                    if (lastParenthesisPos != std::string::npos) {
                        if (vUniformString[lastParenthesisPos] == ')') {
                            uniParsed.params = vUniformString.substr(firstParenthesisPos, lastParenthesisPos - firstParenthesisPos);
                            uniformPos = lastParenthesisPos + 1;
                        } else {
                            uniParsed.badSyntaxDetected = true;
                            vSectionCodePtr->SetSyntaxError(vSectionCodePtr->relativeFile, "Uniform error", true, "bad syntax, missing )", vCurrentFileLine);
                        }
                    } else {
                        uniParsed.badSyntaxDetected = true;
                        vSectionCodePtr->SetSyntaxError(vSectionCodePtr->relativeFile, "Uniform error", true, "bad syntax, missing )", vCurrentFileLine);
                    }
                } else {
                    size_t nextSpace = vUniformString.find_first_of(' ', typePos + 1);
                    if (nextSpace != std::string::npos) {
                        uniParsed.type = vUniformString.substr(typePos, nextSpace - typePos);
                        uniformPos = nextSpace + 1;
                    }
                }
            }

            // name
            // un nom d'uniform de peut pas commencer par un nombre
            size_t namePos = vUniformString.find_first_of("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", uniformPos);
            if (namePos != std::string::npos) {
                // mais il peut en contenir, donc on rajoute les nombre dans la condition
                size_t firstEndName = vUniformString.find_first_not_of("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", namePos);
                if (firstEndName != std::string::npos) {
                    uniParsed.name = vUniformString.substr(namePos, firstEndName - namePos);
                    uniformPos = firstEndName + 1;
                } else {
                    vSectionCodePtr->SetSyntaxError(vSectionCodePtr->relativeFile,
                                                    "Uniform error",
                                                    true,
                                                    "accepted chars are _abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                                                    vCurrentFileLine);
                }
            } else {
                uniParsed.badSyntaxDetected = true;
                vSectionCodePtr->SetSyntaxError(vSectionCodePtr->relativeFile, "Uniform error", true, "bad uniform name", vCurrentFileLine);
            }

            // array
            size_t arrayPos = vUniformString.find_first_of('[', uniformPos);
            if (arrayPos != std::string::npos) {
                arrayPos += 1;
                size_t endArray = vUniformString.find_first_of("[]", arrayPos);
                if (endArray != std::string::npos) {
                    if (vUniformString[endArray] == ']') {
                        uniParsed.array = vUniformString.substr(arrayPos, endArray - arrayPos);
                        uniformPos = endArray + 1;
                    } else {
                        uniParsed.badSyntaxDetected = true;
                        vSectionCodePtr->SetSyntaxError(vSectionCodePtr->relativeFile, "Uniform error", true, "bad syntax, missing ]", vCurrentFileLine);
                    }
                } else {
                    uniParsed.badSyntaxDetected = true;
                    vSectionCodePtr->SetSyntaxError(vSectionCodePtr->relativeFile, "Uniform error", true, "bad syntax, missing ]", vCurrentFileLine);
                }
            }

            // comment
            size_t coma_pos = vUniformString.find_first_of(";\n", 0);
            if (coma_pos != std::string::npos) {
                if (vUniformString[coma_pos] == ';') {
                    coma_pos += 1;
                    size_t end_comment_pos = vUniformString.find('\n', coma_pos);
                    if (end_comment_pos != std::string::npos) {
                    } else {
                        end_comment_pos = vUniformString.size();
                    }

                    if (end_comment_pos != std::string::npos) {
                        std::string afterComaStr = vUniformString.substr(coma_pos, end_comment_pos - coma_pos);
                        if (!afterComaStr.empty()) {
                            size_t uniform_comment_pos = afterComaStr.find("//");
                            if (uniform_comment_pos != std::string::npos) {
                                uniform_comment_pos += 2;
                                uniParsed.commentOriginal = afterComaStr.substr(uniform_comment_pos);
                                uniParsed.comment = uniParsed.commentOriginal;
                                ct::replaceString(uniParsed.comment, "\\n", "\n");
                            }
                        }
                    }
                } else {
                    uniParsed.badSyntaxDetected = true;
                    vSectionCodePtr->SetSyntaxError(vSectionCodePtr->relativeFile, "Uniform error", true, "uniform line must finish by ;", vCurrentFileLine);
                }
            }
        }

        uniParsed.originalParams = uniParsed.params;

        // ct::replaceString(uniParsed.sectionParams, " ", ""); // il peut y avoir du texte espacé
        ct::replaceString(uniParsed.type, " ", "");
        ct::replaceString(uniParsed.params, " ", "");
        ct::replaceString(uniParsed.name, " ", "");
        ct::replaceString(uniParsed.array, " ", "");
        // ct::replaceString(uniParsed.comment, " ", ""); // il peut y avoir du texte espacé

        ct::replaceString(uniParsed.sectionParams, "\t", "");
        ct::replaceString(uniParsed.type, "\t", "");
        ct::replaceString(uniParsed.params, "\t", "");
        ct::replaceString(uniParsed.name, "\t", "");
        ct::replaceString(uniParsed.array, "\t", "");
        ct::replaceString(uniParsed.comment, "\t", "");

        ct::replaceString(uniParsed.sectionParams, "\r", "");
        ct::replaceString(uniParsed.type, "\r", "");
        ct::replaceString(uniParsed.params, "\r", "");
        ct::replaceString(uniParsed.name, "\r", "");
        ct::replaceString(uniParsed.array, "\r", "");
        ct::replaceString(uniParsed.comment, "\r", "");

        // compute
        if (uniParsed.params == "in")
            uniParsed.direction = "in";
        if (uniParsed.params == "out")
            uniParsed.direction = "out";

        if (!uniParsed.isOk()) {
#ifdef DEBUG_UNIFORM_PARSING
            LogVarDebug("----------------------------------------\n");
            LogVarDebug("Parsing of : %s\n", vUniformString.c_str());
            LogVarDebug("\tsection : %s\n", uniParsed.sectionParams.c_str());
            LogVarDebug("\ttype : %s\n", uniParsed.type.c_str());
            LogVarDebug("\tparams : %s\n", uniParsed.params.c_str());
            LogVarDebug("\tname : %s\n", uniParsed.name.c_str());
            LogVarDebug("\tarray : %s\n", uniParsed.array.c_str());
            LogVarDebug("\tcomment : %s\n", uniParsed.comment.c_str());
            LogVarDebug("\tfile line : %zu\n", uniParsed.sourceCodeLine);
#endif
            vSectionCodePtr->SetSyntaxError(vSectionCodePtr->relativeFile, "Uniform error", true, "bad uniform syntax", vCurrentFileLine);

            // on le reset
            uniParsed = UniformParsedStruct();
        } else {
            // todo: ultime verif qui serait pas mal, ce serait de verifier si le params est possible ou non
            // genre une bd qui listerais tout les features qu'on autoriserait ou non dans cette version

            uniParsed.notUploadableToGPU = IsUniformNotUploadableToGPU(uniParsed.type);
            ParseUniformSectionParamsString(&uniParsed, vCurrentFileLine, vSectionCodePtr);
            ParseUniformTypeParamsString(&uniParsed, vCurrentFileLine, vSectionCodePtr);
            ParseUniformSection(&uniParsed);
            CompleteUniformParsedStructWithCustomHeaderCode(&uniParsed);

            // if (!UniformParsing::IsUniformTypeSupported(uniParsed.type, !uniParsed.array.empty(), vSectionCodePtr->parentKey.lock(),
            // vCurrentFileLine))
            /*
            if (uniParsed.params.empty())
            {
                // on le reset
                uniParsed = UniformParsedStruct();
            }
            */
        }
    }

    return uniParsed;
}

// parse section params and set errors
void UniformParsing::ParseUniformSectionParamsString(UniformParsedStruct* vUniform, const size_t& vCurrentFileLine, std::shared_ptr<SectionCode> vSectionCode) {
    std::vector<std::string> words = ct::splitStringToVector(vUniform->sectionParams, ':');
    vUniform->sectionParamsArray = words;

    for (const auto& word : words) {
        std::vector<std::string> fields = ct::splitStringToVector(word, '=');
        if (fields.size() == 2) {
            vUniform->sectionParamsDico[fields[0]] = ct::splitStringToVector(fields[1], ',');
        } else if (fields.size() > 2) {
            vUniform->badSyntaxDetected = true;
            if (vSectionCode)
                vSectionCode->SetSyntaxError(vSectionCode->relativeFile, "Uniform error", true, "too mush = in " + word, vCurrentFileLine);
        } else {
            vUniform->sectionParamsDico[word];
        }
    }
}

// parse uniform params and set errors
void UniformParsing::ParseUniformTypeParamsString(UniformParsedStruct* vUniform, const size_t& vCurrentFileLine, std::shared_ptr<SectionCode> vSectionCode) {
    // la syntaxe generale c'est :
    // (widgetName:params
    // et param contient des liste de mot separee apr ;, les mot peuvent etre mot(liste de mot separ� par ,)

    {
        vUniform->paramsArray = ct::splitStringToVector(vUniform->params, ':');

        for (auto& word : vUniform->paramsArray) {
            std::vector<std::string> fields = ct::splitStringToVector(word, '=');
            if (fields.size() == 2) {
                vUniform->paramsDico[fields[0]] = ct::splitStringToVector(fields[1], ',');
            } else if (fields.size() > 2) {
                vUniform->badSyntaxDetected = true;
                if (vSectionCode)
                    vSectionCode->SetSyntaxError(vSectionCode->relativeFile, "Uniform section error", true, "too mush = in " + word, vCurrentFileLine);
            } else {
                vUniform->paramsDico[word];
            }
        }
    }

    {
        vUniform->originalParamsArray = ct::splitStringToVector(vUniform->originalParams, ':');

        for (auto& word : vUniform->originalParamsArray) {
            std::vector<std::string> fields = ct::splitStringToVector(word, '=');
            if (fields.size() == 2) {
                vUniform->originalParamsDico[fields[0]] = ct::splitStringToVector(fields[1], ',');
            } else if (fields.size() > 2) {
                vUniform->badSyntaxDetected = true;
                if (vSectionCode)
                    vSectionCode->SetSyntaxError(vSectionCode->relativeFile, "Uniform section error", true, "too mush = in " + word, vCurrentFileLine);
            } else {
                vUniform->originalParamsDico[word];
            }
        }
    }
}

inline bool sort_section_order(UniformVariantPtr a, UniformVariantPtr b) {
    if (a && b) {
        return a->sectionOrder < b->sectionOrder;
    }
    return false;
}

void UniformParsing::ParseUniformSection(UniformParsedStruct* vUniformParsed) {
    if (vUniformParsed) {
        std::string sectionString = vUniformParsed->sectionParams;

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
                for (const auto& it : vec) {
                    std::string param = it;

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
                                if (param == "noexport") {
                                    vUniformParsed->noExport = true;
                                } else {
                                    sectionName = param;
                                }
                            }
                        }
                    }
                }

                // name
                if (!sectionName.empty()) {
                    vUniformParsed->sectionName = sectionName;
                }

                // order
                if (!sectionOrder.empty()) {
                    // c'est un order
                    vUniformParsed->sectionOrder = ct::fvariant(sectionOrder).GetI();
                }

                // cond
                if (!sectionCond.empty()) {
                    ct::replaceString(sectionCond, " ", "");
                    ct::replaceString(sectionCond, "\t", "");
                    ct::replaceString(sectionCond, "\r", "");
                    ct::replaceString(sectionCond, "\n", "");

                    vUniformParsed->sectionCond = sectionCond;
                }
            }
        }
    }
}

void UniformParsing::FinaliseUniformSectionParsing(UniformParsedStruct* vUniformParsed) {
    UNUSED(vUniformParsed);

    // cond
    /*if (!vUniformParsed.sectionCond.empty())
    {
        LineFileErrors err;

        std::string var0;
        std::string var1;

        std::string op = "eq";
        size_t condPos = vUniformParsed.sectionCond.find("==");
        if (condPos == std::string::npos)
        {
            condPos = vUniformParsed.sectionCond.find("!=");
            op = "neq";
        }
        if (condPos == std::string::npos)
        {
            condPos = vUniformParsed.sectionCond.find('>');
            op = "sup";
        }
        if (condPos == std::string::npos)
        {
            condPos = vUniformParsed.sectionCond.find('<');
            op = "inf";
        }
        if (condPos == std::string::npos)
        {
            condPos = vUniformParsed.sectionCond.find("<=");
            op = "infeq";
        }
        if (condPos == std::string::npos)
        {
            condPos = vUniformParsed.sectionCond.find(">=");
            op = "supeq";
        }
        if (condPos != std::string::npos)
        {
            var0 = vUniformParsed.sectionCond.substr(0, condPos);
            var1 = vUniformParsed.sectionCond.substr(condPos + 2);
        }

        if (!var0.empty() && !var1.empty())
        {
            auto uni0 = GetUniformByName(var0);
            auto uni1 = GetUniformByName(var1);

            //if (uni0 && uni1)
            //{
            //	m_SyntaxErrors.SetSyntaxError(this, "Parsing Error :", "Uniform Section Error",
            //		false, "Condition is bad, cant be two uniform names", vUniformParsed.SourceLinePos);
            //}
            //else
            if (!uni0 && !uni1)
            {
                std::string fromFile = this->puKey;
                if (!this->m_InFileBufferFromKey.empty())
                    fromFile = this->m_InFileBufferFromKey;
                m_SyntaxErrors.SetSyntaxError(this, "Parsing Error :", "Uniform Section Error", false,
                    LineFileErrors(vUniformParsed.SourceLinePos, fromFile, "The Condition '" + vUniformParsed.sectionCond + "' is bad, no uniform to
    compare to a value"));
            }
            else
            {
                std::weak_ptr<UniformWidgetBase> uni = uni0;
                std::string var = var1;

                if (uni0 && !uni1)
                {
                    uni = uni0;
                    var = var1;
                }
                else if (uni1 && !uni0)
                {
                    uni = uni1;
                    var = var0;
                }

                if (uni && !var.empty())
                {
                    if (uni->widget == "checkbox")
                    {
                        // le bool c'est valable que pour la checkbox
                        if (var == "true")
                        {
                            vUniformParsed.useVisCheckCond = true;
                            vUniformParsed.uniCheckCondName = uni->name;

                            if (op == "eq")
                                vUniformParsed.uniCheckCond = true;
                            else if (op == "neq")
                                vUniformParsed.uniCheckCond = false;

                            vUniformParsed.uniCheckCondPtr = &(uni->x);
                        }
                        else if (var == "false")
                        {
                            vUniformParsed.useVisCheckCond = true;
                            vUniformParsed.uniCheckCondName = uni->name;

                            if (op == "eq")
                                vUniformParsed.uniCheckCond = false;
                            else if (op == "neq")
                                vUniformParsed.uniCheckCond = true;

                            vUniformParsed.uniCheckCondPtr = &(uni->x);
                        }
                        else
                        {
                            std::string fromFile = this->puKey;
                            if (!this->m_InFileBufferFromKey.empty())
                                fromFile = this->m_InFileBufferFromKey;
                            m_SyntaxErrors.SetSyntaxError(this, "Parsing Error :", "Uniform Section Error", false,
                                LineFileErrors(vUniformParsed.SourceLinePos, fromFile, "Condition value true or false (boolean) must be used with
    checbox widget only"));
                        }
                    }
                    else if (uni->widget == "combobox")
                    {
                        // sinon on a la combobox
                        // on va voir si on trouve le choix par mi les chocies
                        bool found = false;
                        int idx = 0;
                        for (const auto& it : uni->choices)
                        {
                            if (*it == var)
                            {
                                found = true;
                                break;
                            }

                            ++idx;
                        }
                        if (found)
                        {
                            vUniformParsed.useVisComboCond = true;
                            vUniformParsed.uniComboCondName = var;
                            vUniformParsed.uniComboCondPtr = &(uni->ix);
                            vUniformParsed.uniComboCond = idx;

                            if (op == "eq")
                                vUniformParsed.uniComboCondDir = true;
                            else if (op == "neq")
                                vUniformParsed.uniComboCondDir = false;
                        }
                        else
                        {
                            std::string fromFile = this->puKey;
                            if (!this->m_InFileBufferFromKey.empty())
                                fromFile = this->m_InFileBufferFromKey;
                            m_SyntaxErrors.SetSyntaxError(this, "Parsing Error :", "Uniform Section Error", false,
                                LineFileErrors(vUniformParsed.SourceLinePos, fromFile, "The Condition '" + vUniformParsed.sectionCond +
                                    "' is bad, the combobox choice " + var + " can't be found is the comobobox " + uni->name));
                        }
                    }
                    else if (uni->glslType == UniformTypeEnum::U_FLOAT)
                    {
                        if (op == "sup")
                            vUniformParsed.useVisOpCond = 1;
                        else if (op == "supeq")
                            vUniformParsed.useVisOpCond = 2;
                        else if (op == "inf")
                            vUniformParsed.useVisOpCond = 3;
                        else if (op == "infeq")
                            vUniformParsed.useVisOpCond = 4;

                        vUniformParsed.uniOpCondThreshold = ct::fvariant(var).GetF();
                        vUniformParsed.uniCondPtr = &(uni->x);
                    }
                    else
                    {
                        std::string fromFile = this->puKey;
                        if (!this->m_InFileBufferFromKey.empty())
                            fromFile = this->m_InFileBufferFromKey;
                        m_SyntaxErrors.SetSyntaxError(this, "Parsing Error :", "Uniform Section Error", false,
                            LineFileErrors(vUniformParsed.SourceLinePos, fromFile, "Condition " + var + "for the uniform widget " +
                                uni->name + ", is only supported with checkbox or combobox"));
                    }
                }
            }
        }
        else
        {
            std::string fromFile = this->puKey;
            if (!this->m_InFileBufferFromKey.empty())
                fromFile = this->m_InFileBufferFromKey;
            m_SyntaxErrors.SetSyntaxError(this, "Parsing Error :", "Uniform Section Error", false,
                LineFileErrors(vUniformParsed.SourceLinePos, fromFile, "The Condition '" + vUniformParsed.sectionCond + "' is incomplete"));
        }
    }*/
}

#ifdef USE_UNIFORMWIDGETS
#include <UniformWidgets/UniformWidgets.h>
#endif

void UniformParsing::CompleteUniformParsedStructWithCustomHeaderCode(UniformParsedStruct* vUniformParsed) {
#ifdef USE_UNIFORMWIDGETS
    UniformWidgets::CompleteUniformParsedStructWithWidgetCustomHeaderCode(vUniformParsed);
#else
    UNUSED(vUniformParsed);
#endif
}

bool UniformParsing::IsUniformTypeSupported(const std::string& vType, const bool& vIsArray, ShaderKeyPtr vKey, const size_t& vCurrentFileLine) {
    bool res = false;

    if (vType == "text")
        res = true;

    if (vIsArray)  // float arr[15] by example
    {
        if (vType == "float")
            res = true;
        else if (vType == "int")
            res = true;
        else if (vType == "vec2")
            res = true;
        else if (vType == "vec3")
            res = true;
        else if (vType == "vec4")
            res = true;
        else if (vType == "sampler1D")
            res = true;
        else if (vType == "sampler2D")
            res = true;
        else if (vType == "sampler3D")
            res = true;
        else if (vType == "image1D")
            res = false;
        else if (vType == "image2D")
            res = false;
        else if (vType == "image3D")
            res = false;
    } else {
        if (vType == "float")
            res = true;
        else if (vType == "vec2")
            res = true;
        else if (vType == "vec3")
            res = true;
        else if (vType == "vec4")
            res = true;
        else if (vType == "int")
            res = true;
        else if (vType == "ivec2")
            res = true;
        else if (vType == "ivec3")
            res = true;
        else if (vType == "ivec4")
            res = true;
        else if (vType == "uint")
            res = true;
        else if (vType == "uvec2")
            res = true;
        else if (vType == "uvec3")
            res = true;
        else if (vType == "uvec4")
            res = true;
        else if (vType == "bool")
            res = true;
        else if (vType == "bvec2")
            res = true;
        else if (vType == "bvec3")
            res = true;
        else if (vType == "bvec4")
            res = true;
        else if (vType == "sampler1D")
            res = true;
        else if (vType == "sampler2D")
            res = true;
        else if (vType == "sampler3D")
            res = true;
        else if (vType == "image1D")
            res = false;
        else if (vType == "image2D")
            res = false;
        else if (vType == "image3D")
            res = false;
    }

    // if (vType == "sampler2DArray") res = true;
    if (vType == "samplerCube")
        res = true;
    else if (vType == "mat2")
        res = true;
    else if (vType == "mat3")
        res = true;
    else if (vType == "mat4")
        res = true;

    if (!res) {
        CTOOL_DEBUG_BREAK;

        if (vKey) {
            std::string fromFile = vKey->puKey;
            if (!vKey->puInFileBufferFromKey.empty())
                fromFile = vKey->puInFileBufferFromKey;
            vKey->GetSyntaxErrors()->SetSyntaxError(vKey,
                                                    "Parsing Error :",
                                                    "uniform type warning",
                                                    false,
                                                    LineFileErrors(vCurrentFileLine, fromFile, "an uniform of type " + vType + " is not supported for the moment.\n"));
        }
    }

    return res;
}

bool UniformParsing::IsUniformWidgetSupported(const std::string& vWidget, ShaderKeyPtr vKey, const size_t& vCurrentFileLine) {
    bool res = true;

    // on va faire du = false car plutot que mettre res a false et faire du = true,
    // car il y a des widgets custom qui dependent de l'appli
    // donc ne peuvent pas etre rejet�s par la lib
    if (vWidget == "compute")
        res = true;

    if (!res) {
        CTOOL_DEBUG_BREAK;

        if (vKey) {
            vKey->GetSyntaxErrors()->SetSyntaxError(vKey,
                                                    "Parsing Error :",
                                                    "uniform widget warning",
                                                    false,
                                                    LineFileErrors(vCurrentFileLine, vKey->puKey, "widget " + vWidget + " is not supported for the moment.\n"));
        }
    }

    return res;
}

bool UniformParsing::IsUniformNotUploadableToGPU(const std::string& vType) {
    bool res = false;

    if (vType == "text")
        res = true;

    return res;
}
