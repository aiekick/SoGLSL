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

#include <Renderer/Shader.h>
#include <Renderer/RenderPack.h>
#include <ctools/Logger.h>
#include <ctools/GLVersionChecker.h>
#include <Profiler/TracyProfiler.h>

ShaderPtr Shader::Create(const GuiBackend_Window& vWin, std::string vShaderName) {
    auto res = std::make_shared<Shader>(vWin, vShaderName);
    res->m_This = res;
    return res;
}

Shader::Shader(const GuiBackend_Window& vWin, std::string vShaderName) : puWindow(vWin), _isValid(false), puShaderName(vShaderName), puuProgram(0), linked(-1), err(0) {
    // LogToOutput(vShaderName + " : ", true, ShaderMsg::SHADER_MSG_OK); // on efface et on ecrit que dalle
}

Shader::~Shader() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("Shader::Destroy");

    if (puuProgram) {
        glDeleteProgram(puuProgram);
        puuProgram = 0;
    }
}

ProgramMsg Shader::InitAndLinkShaderProgram(bool vIsCompute, ShaderParsedStruct shaderCode, std::string /*geomLayoutParams*/, std::string version) {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("Shader::InitAndLinkShaderProgram");

    _isValid = false;

    // clear arrays

    if (!vIsCompute) {
        // Load the vertex/fragment shaders
        const GLuint vertexShader = LoadFromString(ShaderTypeEnum::SHADER_TYPE_VERTEX, shaderCode.GetSection("VERTEX"), version);

        const GLuint geometryShader = LoadFromString(ShaderTypeEnum::SHADER_TYPE_GEOMETRY, shaderCode.GetSection("GEOMETRY"), version);

        const GLuint tessControlShader = LoadFromString(ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL, shaderCode.GetSection("TESSCONTROL"), version);

        const GLuint tessEvalShader = LoadFromString(ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL, shaderCode.GetSection("TESSEVAL"), version);

        const GLuint fragmentShader = LoadFromString(ShaderTypeEnum::SHADER_TYPE_FRAGMENT, shaderCode.GetSection("FRAGMENT"), version);

        if (vertexShader > 0) {
            puuProgram = glCreateProgram();
            LogGlError();

            glAttachShader(puuProgram, vertexShader);
            LogGlError();
            glDeleteShader(vertexShader);
            LogGlError();

            if (fragmentShader > 0)  // pas necessaire pour du transform feedback
            {
                glAttachShader(puuProgram, fragmentShader);
                LogGlError();
                glDeleteShader(fragmentShader);
                LogGlError();
            }

            if (geometryShader > 0) {
                glAttachShader(puuProgram, geometryShader);
                LogGlError();
                glDeleteShader(geometryShader);
                LogGlError();
            }

            if (tessControlShader > 0) {
                glAttachShader(puuProgram, tessControlShader);
                LogGlError();
                glDeleteShader(tessControlShader);
                LogGlError();
            }

            if (tessEvalShader > 0) {
                glAttachShader(puuProgram, tessEvalShader);
                LogGlError();
                glDeleteShader(tessEvalShader);
                LogGlError();
            }

            if (puState.vertexMsg == ShaderMsg::SHADER_MSG_OK) {
                // link
                puState.linkMsg = LinkShaderProgram();
            }
        }
    } else  // compute
    {
        // Load the vertex/fragment shaders
        const GLuint computeShader = LoadFromString(ShaderTypeEnum::SHADER_TYPE_COMPUTE, shaderCode.GetSection("COMPUTE"), version);

        if (computeShader > 0) {
            puuProgram = glCreateProgram();
            LogGlError();

            glAttachShader(puuProgram, computeShader);
            LogGlError();
            glDeleteShader(computeShader);
            LogGlError();

            // link
            puState.linkMsg = LinkShaderProgram();
        }
    }

    // check states error
    if (!puError[ShaderTypeEnum::SHADER_TYPE_VERTEX].empty())
        puState.vertexMsg = ShaderMsg::SHADER_MSG_ERROR;
    if (!puError[ShaderTypeEnum::SHADER_TYPE_GEOMETRY].empty() && GLVersionChecker::Instance()->m_GeometryShaderSupported)
        puState.geometryMsg = ShaderMsg::SHADER_MSG_ERROR;
    if (!puError[ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL].empty() && GLVersionChecker::Instance()->m_TesselationShaderSupported)
        puState.tesselationControlMsg = ShaderMsg::SHADER_MSG_ERROR;
    if (!puError[ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL].empty() && GLVersionChecker::Instance()->m_TesselationShaderSupported)
        puState.tesselationEvalMsg = ShaderMsg::SHADER_MSG_ERROR;
    if (!puError[ShaderTypeEnum::SHADER_TYPE_FRAGMENT].empty())
        puState.fragmentMsg = ShaderMsg::SHADER_MSG_ERROR;
    if (!puError[ShaderTypeEnum::SHADER_TYPE_COMPUTE].empty() && GLVersionChecker::Instance()->m_ComputeShaderSupported)
        puState.computeMsg = ShaderMsg::SHADER_MSG_ERROR;

    // check states warnings
    if (!puWarnings[ShaderTypeEnum::SHADER_TYPE_VERTEX].empty())
        puState.vertexMsg = ShaderMsg::SHADER_MSG_WARNING;
    if (!puWarnings[ShaderTypeEnum::SHADER_TYPE_GEOMETRY].empty() && GLVersionChecker::Instance()->m_GeometryShaderSupported)
        puState.geometryMsg = ShaderMsg::SHADER_MSG_WARNING;
    if (!puWarnings[ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL].empty() && GLVersionChecker::Instance()->m_TesselationShaderSupported)
        puState.tesselationControlMsg = ShaderMsg::SHADER_MSG_WARNING;
    if (!puWarnings[ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL].empty() && GLVersionChecker::Instance()->m_TesselationShaderSupported)
        puState.tesselationEvalMsg = ShaderMsg::SHADER_MSG_WARNING;
    if (!puWarnings[ShaderTypeEnum::SHADER_TYPE_FRAGMENT].empty())
        puState.fragmentMsg = ShaderMsg::SHADER_MSG_WARNING;
    if (!puWarnings[ShaderTypeEnum::SHADER_TYPE_COMPUTE].empty() && GLVersionChecker::Instance()->m_ComputeShaderSupported)
        puState.computeMsg = ShaderMsg::SHADER_MSG_WARNING;

    return puState;
}

// return 0 if NOK
GLuint Shader::LoadFromString(ShaderTypeEnum type, ShaderParsedCodeStruct shaderCode, std::string /*version*/) {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("Shader::LoadShaderStage");

    GLuint shader = 0;

    // le geometry n'est pas support�
    if (type == ShaderTypeEnum::SHADER_TYPE_GEOMETRY && !GLVersionChecker::Instance()->m_GeometryShaderSupported)
        return 0;
    if (type == ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL && !GLVersionChecker::Instance()->m_TesselationShaderSupported)
        return 0;
    if (type == ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL && !GLVersionChecker::Instance()->m_TesselationShaderSupported)
        return 0;
    if (type == ShaderTypeEnum::SHADER_TYPE_COMPUTE && !GLVersionChecker::Instance()->m_ComputeShaderSupported)
        return 0;

    std::string shaderSrc = shaderCode.GetCode();
    if (!shaderSrc.empty()) {
        GLint compiled;

        // version
        /*OpenGlVersionStruct versionStruct = GLVersionChecker::Instance()->GetOpenglVersionStruct(version);
        puVersion = GLVersionChecker::Instance()->GetGlslVersions(version);
        if (puVersion != "")
            puVersion += "\n";*/

        std::string fullShaderSrc = shaderSrc;

        const int countItems = 1;
        const GLchar* sources = fullShaderSrc.c_str();

        // Create the shader object
        shader = glCreateShader((GLenum)type);
        LogGlError();

        if (shader != 0) {
            // const GLint len = strlen((char*)sources);

            // Load the shader source
            glShaderSource(shader, countItems, &sources, nullptr);
            LogGlError();

            // Compile the shader
            glCompileShader(shader);
            LogGlError();

            // Check the compile status
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            LogGlError();

            std::string typeStr;
            if (type == ShaderTypeEnum::SHADER_TYPE_FRAGMENT)
                typeStr = "FRAGMENT";
            if (type == ShaderTypeEnum::SHADER_TYPE_VERTEX)
                typeStr = "VERTEX";
            if (type == ShaderTypeEnum::SHADER_TYPE_GEOMETRY)
                typeStr = "GEOMETRY";
            if (type == ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL)
                typeStr = "TESSCONTROL";
            if (type == ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL)
                typeStr = "TESSEVAL";
            if (type == ShaderTypeEnum::SHADER_TYPE_COMPUTE)
                typeStr = "COMPUTE";

            if (!compiled) {
                GLint infoLen = 0;

                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                LogGlError();

                if (infoLen > 1) {
                    char* infoLog = new char[infoLen];

                    glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
                    LogGlError();

                    puError[type] = std::string(infoLog, infoLen);

                    puWarnings[type].clear();

                    typeStr += " SHADER COMPILING => FAIL !!!";
                    LogToOutput(type, typeStr, false, ShaderMsg::SHADER_MSG_ERROR);

                    LogToOutput(type, infoLog, false, ShaderMsg::SHADER_MSG_ERROR);

                    delete[] infoLog;
                }

                glDeleteShader(shader);
                LogGlError();

#ifdef SAVE_SHADERS_TO_FILE_FOR_DEBUG_WHEN_ERRORS
                if (type == ShaderTypeEnum::SHADER_TYPE_VERTEX)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.vert");
                else if (type == ShaderTypeEnum::SHADER_TYPE_FRAGMENT)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.frag");
                else if (type == ShaderTypeEnum::SHADER_TYPE_GEOMETRY)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.geom");
                else if (type == ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.tesscontrol");
                else if (type == ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.tesseval");
                else if (type == ShaderTypeEnum::SHADER_TYPE_COMPUTE)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.comp");
#endif
                return 0;
            } else {
                typeStr += " SHADER COMPILING +> OK";
                LogToOutput(type, typeStr, false, ShaderMsg::SHADER_MSG_OK);

                puError[type].clear();

                // warnings

                GLint infoLen = 0;

                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                LogGlError();

                if (infoLen > 1) {
                    char* infoLog = new char[infoLen];

                    glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
                    LogGlError();

                    puWarnings[type] = std::string(infoLog, infoLen);

                    LogToOutput(type, infoLog, false, ShaderMsg::SHADER_MSG_WARNING);

                    delete[] infoLog;
                }

#ifdef SAVE_SHADERS_TO_FILE_FOR_DEBUG_WHEN_NO_ERRORS
                if (type == ShaderTypeEnum::SHADER_TYPE_VERTEX)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.vert");
                else if (type == ShaderTypeEnum::SHADER_TYPE_FRAGMENT)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.frag");
                else if (type == ShaderTypeEnum::SHADER_TYPE_GEOMETRY)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.geom");
                else if (type == ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.tesscontrol");
                else if (type == ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.tesseval");
                else if (type == ShaderTypeEnum::SHADER_TYPE_COMPUTE)
                    SaveToFile(fullShaderSrc, puShaderName + "_final_for_debug.comp");
#endif
            }
        }
    } else {
        if (type != ShaderTypeEnum::SHADER_TYPE_GEOMETRY && type != ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL &&
            type != ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL) {
            // le geometry est un shader optionnel
            // le tesselation control est un shader optionnel
            // le tesselaton eval est un shader optionnel
            // donc s'il est vide comme il ne peut pas empecher le link
            // on ne definit pas une erreur ni un warngings
            if (type == ShaderTypeEnum::SHADER_TYPE_VERTEX)
                puError[type] = "the Vertex shader is empty. it can be due to the file encodage. change to UTF8 maybe";
            else if (type == ShaderTypeEnum::SHADER_TYPE_FRAGMENT)
                puError[type] = "the Fragment shader is empty. it can be due to the file encodage. change to UTF8 maybe";
            else if (type == ShaderTypeEnum::SHADER_TYPE_COMPUTE)
                puError[type] = "the Compute shader is empty. it can be due to the file encodage. change to UTF8 maybe";
        }
    }

    return shader;
}

ShaderMsg Shader::LinkShaderProgram() {
    GuiBackend::MakeContextCurrent(puWindow);

    TracyGpuZone("Shader::LinkShaderProgram");

    ShaderMsg state = ShaderMsg::SHADER_MSG_OK;

    _isValid = false;

    if (!puuProgram)
        return ShaderMsg::SHADER_MSG_ERROR;

    // Link the program
    glLinkProgram(puuProgram);
    LogGlError();

    // Check the link status
    glGetProgramiv(puuProgram, GL_LINK_STATUS, &linked);
    LogGlError();

    if (!linked) {
        GLint infoLen = 0;

        glGetProgramiv(puuProgram, GL_INFO_LOG_LENGTH, &infoLen);
        LogGlError();

        if (infoLen > 1) {
            char* infoLog = new char[infoLen];

            glGetProgramInfoLog(puuProgram, infoLen, nullptr, infoLog);
            LogGlError();

            puError[ShaderTypeEnum::LINK_SPECIAL_TYPE] = std::string(infoLog, infoLen);
            puWarnings[ShaderTypeEnum::LINK_SPECIAL_TYPE].clear();

            LogToOutput(ShaderTypeEnum::LINK_SPECIAL_TYPE, "PROGRAM LINKING => FAIL !!!", false, ShaderMsg::SHADER_MSG_ERROR);
            LogToOutput(ShaderTypeEnum::LINK_SPECIAL_TYPE, infoLog, false, ShaderMsg::SHADER_MSG_ERROR);

            // parse car il peut y avoir un bug avec un des shaders ok au compil
            // genre de messages :
            // t:10799.7s PROGRAM LINKING => FAIL !!!
            // t:166.334s Vertex info
            // t : 166.334s---------- -
            // t : 166.334s 0(137) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.334s 0(146) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s 0(156) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s 0(166) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s Geometry info
            // t : 166.335s------------ -
            // t : 166.335s 0(141) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s 0(150) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s 0(160) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s 0(170) : error C7623 : implicit narrowing of type from "vec3" to "float"

            const std::string infoLinkingLog = ct::toStr(infoLog);
            std::vector<std::string> vec = ct::splitStringToVector(infoLinkingLog, "\n");
            ShaderTypeEnum problemType = ShaderTypeEnum::SHADER_TYPE_VERTEX;
            for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); ++it) {
                std::string line = *it;
                if (line.size() > 2) {
                    if (line.find("Vertex") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_VERTEX;
                    } else if (line.find("Geometry") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_GEOMETRY;
                    } else if (line.find("TessControl") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL;
                    } else if (line.find("TessEval") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL;
                    } else if (line.find("Fragment") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_FRAGMENT;
                    } else if (line.find("Compute") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_COMPUTE;
                    }

                    if (line.find("error") != std::string::npos) {
                        puError[problemType] += line + "\n";
                    }
                    if (line.find("warning") != std::string::npos) {
                        puWarnings[problemType] += line + "\n";
                    }
                }
            }

            state = ShaderMsg::SHADER_MSG_ERROR;

            delete[] infoLog;
        }

        glDeleteProgram(puuProgram);
        LogGlError();
        puuProgram = 0;
    } else {
        const std::string typeStr = "PROGRAM LINKING => OK";
        LogToOutput(ShaderTypeEnum::LINK_SPECIAL_TYPE, typeStr, false, ShaderMsg::SHADER_MSG_OK);

        puError[ShaderTypeEnum::LINK_SPECIAL_TYPE].clear();

        // warnings

        GLint infoLen = 0;

        glGetProgramiv(puuProgram, GL_INFO_LOG_LENGTH, &infoLen);
        LogGlError();

        if (infoLen > 1) {
            char* infoLog = new char[infoLen];

            glGetProgramInfoLog(puuProgram, infoLen, nullptr, infoLog);
            LogGlError();

            puWarnings[ShaderTypeEnum::LINK_SPECIAL_TYPE] = std::string(infoLog, infoLen);

            LogToOutput(ShaderTypeEnum::LINK_SPECIAL_TYPE, infoLog, false, ShaderMsg::SHADER_MSG_WARNING);

            // parse car il peut y avoir un bug avec un des shaders ok au compil
            // genre de messages :
            // t:10799.7s PROGRAM LINKING => FAIL !!!
            // t:166.334s Vertex info
            // t : 166.334s---------- -
            // t : 166.334s 0(137) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.334s 0(146) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s 0(156) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s 0(166) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s Geometry info
            // t : 166.335s------------ -
            // t : 166.335s 0(141) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s 0(150) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s 0(160) : error C7623 : implicit narrowing of type from "vec3" to "float"
            // t : 166.335s 0(170) : error C7623 : implicit narrowing of type from "vec3" to "float"

            const std::string infoLinkingLog = ct::toStr(infoLog);
            std::vector<std::string> vec = ct::splitStringToVector(infoLinkingLog, "\n");
            ShaderTypeEnum problemType = ShaderTypeEnum::SHADER_TYPE_VERTEX;
            for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); ++it) {
                std::string line = *it;
                if (line.size() > 2) {
                    if (line.find("Vertex") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_VERTEX;
                    } else if (line.find("Geometry") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_GEOMETRY;
                    } else if (line.find("TessControl") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL;
                    } else if (line.find("TessEval") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL;
                    } else if (line.find("Fragment") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_FRAGMENT;
                    } else if (line.find("Compute") != std::string::npos) {
                        problemType = ShaderTypeEnum::SHADER_TYPE_COMPUTE;
                    }

                    if (line.find("error") != std::string::npos) {
                        puError[problemType] += line + "\n";
                    }
                    if (line.find("warning") != std::string::npos) {
                        puWarnings[problemType] += line + "\n";
                    }
                }
            }

            state = ShaderMsg::SHADER_MSG_WARNING;

            delete[] infoLog;
        }
    }

    _isValid = (state != ShaderMsg::SHADER_MSG_ERROR);

    return state;
}

void Shader::LogToOutput(ShaderTypeEnum /*type*/, std::string vBuild, bool /*vErase*/, ShaderMsg vMsg) {
    if (vMsg != ShaderMsg::SHADER_MSG_OK) {
        if (vMsg == ShaderMsg::SHADER_MSG_ERROR)
            LogVarLightError("%s", vBuild.c_str());
        if (vMsg == ShaderMsg::SHADER_MSG_WARNING)
            LogVarLightWarning("%s", vBuild.c_str());
        if (vMsg == ShaderMsg::SHADER_MSG_NOT_SUPPORTED)
            LogVarLightInfo("%s", vBuild.c_str());
    }
}

void Shader::Use() {
    GuiBackend::MakeContextCurrent(puWindow);

    glUseProgram(puuProgram);
    LogGlError();
}

void Shader::UnUse() {
    GuiBackend::MakeContextCurrent(puWindow);

    glUseProgram(0);
    LogGlError();
}

bool Shader::IsValid() {
    return _isValid;
}

void Shader::Render() {
}

std::string Shader::GetWebGlTemplate_CreateShader(std::string vOffset) {
    std::string code;

    code += vOffset + "function createShader(src, type)\n";
    code += vOffset + "{\n";
    code += vOffset + "\tvar shader = gl.createShader(type);\n";
    code += vOffset + "\tvar line, lineNum, lineError, index = 0, indexEnd;\n";
    code += vOffset + "\tgl.shaderSource(shader, src);\n";
    code += vOffset + "\tgl.compileShader(shader);\n";
    code += vOffset + "\tif (!gl.getShaderParameter(shader, gl.COMPILE_STATUS))\n";
    code += vOffset + "\t{\n";
    code += vOffset + "\t\tvar error = gl.getShaderInfoLog(shader);\n";
    code += vOffset + "\t\tconsole.error(error);\n";
    code += vOffset + "\t\treturn null;\n";
    code += vOffset + "\t}\n";
    code += vOffset + "\treturn shader;\n";
    code += vOffset + "}\n";

    return code;
}

std::string Shader::GetWebGlTemplate_LoadFromString(std::string vOffset) {
    std::string code;

    code += vOffset + "";

    return code;
}

std::string Shader::GetWebGlTemplate_LinkShader(std::string vOffset) {
    std::string code;

    code += vOffset + "";

    return code;
}

std::string Shader::GetWebGlTemplate_TestCreateShaderWithPrecision(std::string vOffset) {
    std::string code;

    code += vOffset + "";

    return code;
}

std::string Shader::GetWebGlTemplate_DetermineShaderPrecision(std::string vOffset) {
    std::string code;

    code += vOffset + "";

    return code;
}

void Shader::SaveToFile(std::string vCode, const std::string& vFilePathName) {
    const std::string absolutePathFileNameForDebug = FileHelper::Instance()->GetAbsolutePathForFileLocation(vFilePathName, (int)FILE_LOCATION_Enum::FILE_LOCATION_DEBUG);
    std::ofstream fileWriter(absolutePathFileNameForDebug, std::ios::out);
    if (fileWriter.bad() == false) {
        fileWriter << vCode;
        fileWriter.close();
    }
}

GLint Shader::getUniformLocationForName(std::string name) {
    GuiBackend::MakeContextCurrent(puWindow);

    if (name.empty()) {
        LogVarError("Invalid uniform name %s", name.c_str());
        return -1;
    }

    if (!puuProgram) {
        LogVarError("Invalid operation. Cannot get uniform location when program is not initialized");
        return -1;
    }

    if (glIsProgram(puuProgram) != GL_TRUE) {
        LogVarError("Program invalid");
        return -1;
    }

    return glGetUniformLocation(puuProgram, name.c_str());
}