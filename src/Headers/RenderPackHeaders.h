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

#define LAYOUT_THREADED_MODE

#include <ctools/cTools.h>
#include <glad/glad.h>
#include <Gui/GuiBackend.h>
#include <functional>
#include <memory>
#include <string>

// #define SHOW_LOGS_FOR_FILE_TRACKER_SYSTEM
// #define SHOW_LOGS_FOR_RENDERPACK_LOADING

class RenderPack;
typedef std::shared_ptr<RenderPack> RenderPackPtr;
typedef std::weak_ptr<RenderPack> RenderPackWeak;

class BaseModel;
typedef std::shared_ptr<BaseModel> BaseModelPtr;
typedef std::weak_ptr<BaseModel> BaseModelWeak;

class UniformVariant;
typedef std::shared_ptr<UniformVariant> UniformVariantPtr;
typedef std::weak_ptr<UniformVariant> UniformVariantWeak;

class Shader;
typedef std::shared_ptr<Shader> ShaderPtr;
typedef std::weak_ptr<Shader> ShaderWeak;

class FrameBuffersPipeLine;
typedef std::shared_ptr<FrameBuffersPipeLine> FrameBuffersPipeLinePtr;
typedef std::weak_ptr<FrameBuffersPipeLine> FrameBuffersPipeLineWeak;

class FrameBuffer;
typedef std::shared_ptr<FrameBuffer> FrameBufferPtr;
typedef std::weak_ptr<FrameBuffer> FrameBufferWeak;

class CodeTree;
typedef std::shared_ptr<CodeTree> CodeTreePtr;
typedef std::weak_ptr<CodeTree> CodeTreeWeak;

class ShaderKey;
typedef std::shared_ptr<ShaderKey> ShaderKeyPtr;
typedef std::weak_ptr<ShaderKey> ShaderKeyWeak;

class Texture2D;
typedef std::shared_ptr<Texture2D> Texture2DPtr;
typedef std::weak_ptr<Texture2D> Texture2DWeak;

class Texture3D;
typedef std::shared_ptr<Texture3D> Texture3DPtr;
typedef std::weak_ptr<Texture3D> Texture3DWeak;

class TextureCube;
typedef std::shared_ptr<TextureCube> TextureCubePtr;
typedef std::weak_ptr<TextureCube> TextureCubeWeak;

namespace ct {
class texture;
}
typedef std::shared_ptr<ct::texture> ctTexturePtr;
typedef std::weak_ptr<ct::texture> ctTextureWeak;

typedef float DisplayQualityType;
typedef ct::frect MeshRectType;

class MouseInterface;
class CameraInterface;
typedef std::function<void(RenderPackWeak, UniformVariantPtr, DisplayQualityType, MouseInterface*, CameraInterface*, MeshRectType*)> UpdateUniformFuncSignature;

#define MAX_PATH 260

// DEFINES
// #define USE_OPENCL
// #define GENERATOR_VERBOSE_MODE
// #define USE_EMBEDDED_SCRIPTS
#define SAVE_SHADERS_TO_FILE_FOR_DEBUG_WHEN_ERRORS
// #define SAVE_SHADERS_TO_FILE_FOR_DEBUG_WHEN_NO_ERRORS

// VARS
#define MAX_CONFIG_COUNT_PER_SHADER_TYPE 1000
#define MAX_UNIFORM_COMMENT_BUFFER_SIZE 512
#define MAX_UNIFORM_BUFFER_SIZE 1024
#define DEFAULT_RENDERING_QUALITY 1.0f
#define MAX_SEARCH_SIZE 255
#define MAX_CHARSET_SIZE 1024
#define MAX_OPENCL_BUFFER_ERROR_SIZE 2048
#define MAX_SHADER_BUFFER_ERROR_SIZE 2048
#define MAX_FILE_DIALOG_NAME_BUFFER 1024
#define SHADER_UNIFORM_FIRST_COLUMN_WIDTH 150.0f
#define TRANSPARENCY_DAMIER_ZOOM 5.0f

#include <string>
#include <unordered_map>
// moyenne du calcul du fps sur COUNT_ITEM_FOR_FPS_MEAN elments
#define COUNT_ITEM_FOR_FPS_MEAN 5

#define SHADER_VERTEX_SHEET_NAME "Vertex"
#define SHADER_FRAGMENT_SHEET_NAME "Fragment"
#define SHADER_GEOMETRY_SHEET_NAME "Geometry"
#define SHADER_NOTES_SHEET_NAME "Notes"

class MainFrame;

enum class FILE_LOCATION_Enum {
    FILE_LOCATION_NONE = 0,
    FILE_LOCATION_ASSET_TEXTURE_2D,
    FILE_LOCATION_ASSET_TEXTURE_3D,
    FILE_LOCATION_ASSET_CUBEMAP,
    FILE_LOCATION_ASSET_SOUND,
    FILE_LOCATION_ASSET_MESH,
    FILE_LOCATION_EXPORT,
    FILE_LOCATION_ASSET_GENERAL,
    FILE_LOCATION_SCRIPT,
    FILE_LOCATION_SCRIPT_INCLUDE,
    FILE_LOCATION_CONF,
    FILE_LOCATION_SHAPES,
    FILE_LOCATION_DEBUG,
    FILE_LOCATION_APP,
    FILE_LOCATION_IMPORT,
    FILE_LCOATION_Count
};

//////////////////////////////////////////
//// MESH DATA TYPE //////////////////////
//////////////////////////////////////////

enum class RenderPack_Type { RENDERPACK_TYPE_BUFFER = 0, RENDERPACK_TYPE_COMPUTE, RENDERPACK_TYPE_Count };

//////////////////////////////////////////
//// TECTURE PARAMS //////////////////////
////// WRAP S ////////////////////////////
////// WRAP T ////////////////////////////
////// MIN FILTER ////////////////////////
////// MAG FILTER ////////////////////////
//////////////////////////////////////////

static int TexParamsArrayIndex[4] = {0, 0, 0, 0};
static const char* TexParamsComboString[3] = {"REPEAT\0CLAMP_TO_EDGE\0MIRRORED_REPEAT\0\0",
                                              "NEAREST\0NEAREST_MIPMAP_NEAREST\0LINEAR_MIPMAP_NEAREST\0NEAREST_MIPMAP_LINEAR\0LINEAR_MIPMAP_LINEAR\0\0",
                                              "NEAREST\0LINEAR\0\0"};
enum class UniformTextureWrapEnum { wrapCLAMP_TO_EDGE = GL_CLAMP_TO_EDGE, wrapREPEAT = GL_REPEAT, wrapMIRRORED_REPEAT = GL_MIRRORED_REPEAT };
enum class UniformTextureMinEnum {
    minNEAREST = GL_NEAREST,
    minLINEAR = GL_LINEAR,
    minNEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
    minLINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
    minNEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
    minLINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
};
enum class UniformTextureMagEnum { magLINEAR = GL_LINEAR, magNEAREST = GL_NEAREST };
enum class UniformTextureFormatEnum { formatFLOAT = GL_FLOAT, formatBYTE = GL_UNSIGNED_BYTE };

//////////////////////////////////////////
//////////////////////////////////////////
//////////////////////////////////////////

enum class ShaderPlaform { SPF_UNKNOW = 0, SPf_SHADERTOY = 1, SPF_GLSLSANDBOX = 2, SPF_VERTEXSHADERART = 3 };

enum class ModifiedTypeBitFlagsEnum {
    NONE_IS_MODIFIED = 0,
    WIDGETS_ARE_MODIFIED = (1 << 1),
    VERTEX_SHADER_IS_MODIFIED = (1 << 2),
    GEOMETRY_SHADER_IS_MODIFIED = (1 << 3),
    FRAGMENT_SHADER_IS_MODIFIED = (1 << 4)
};

enum class ShaderBookTemplateEnum { TEMPLATE_QUAD = 0, TEMPLATE_POINTS = 1, TEMPLATE_MUSIC = 2 };

enum class WEBGLConverterEnum {
    WEBGL_1 = 0,
    WEBGL_2 = 1,
};

static int ShaderTypeArrayIndex = 0;
static const char* ShaderTypeComboString = "VERTEX\0GEOMETRY\0FRAGMENT\0SECTION\0\0";
enum class ShaderTypeEnum {
    SHADER_TYPE_NONE = 0,
    SHADER_TYPE_VERTEX = GL_VERTEX_SHADER,
    SHADER_TYPE_FRAGMENT = GL_FRAGMENT_SHADER,
    SHADER_TYPE_GEOMETRY = GL_GEOMETRY_SHADER,
    SHADER_TYPE_TESSELATION_CONTROL = GL_TESS_CONTROL_SHADER,
    SHADER_TYPE_TESSELATION_EVAL = GL_TESS_EVALUATION_SHADER,
    SHADER_TYPE_COMPUTE = GL_COMPUTE_SHADER,
    LINK_SPECIAL_TYPE = GL_COMPUTE_SHADER + 1,
    NOTE_SPECIAL_TYPE = GL_COMPUTE_SHADER + 2,  // pour faire des annotations
    SECTION_TYPE = GL_COMPUTE_SHADER + 3,
    SECTION_COMMON = GL_COMPUTE_SHADER + 4
};

static inline const char* GetShaderTypeEnumString(ShaderTypeEnum vShaderTypeEnum) {
    switch (vShaderTypeEnum) {
        case ShaderTypeEnum::SHADER_TYPE_VERTEX: return "VERTEX";
        case ShaderTypeEnum::SHADER_TYPE_GEOMETRY: return "GEOMETRY";
        case ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL: return "TESSCONTROL";
        case ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL: return "TESSEVAL";
        case ShaderTypeEnum::SHADER_TYPE_FRAGMENT: return "FRAGMENT";
        case ShaderTypeEnum::SHADER_TYPE_COMPUTE: return "COMPUTE";
        case ShaderTypeEnum::NOTE_SPECIAL_TYPE: return "NOTE";
        case ShaderTypeEnum::LINK_SPECIAL_TYPE: return "LINK";
        case ShaderTypeEnum::SECTION_TYPE: return "SECTION";
        case ShaderTypeEnum::SECTION_COMMON: return "COMMON";
        case ShaderTypeEnum::SHADER_TYPE_NONE:
        default: break;
    }
    return "";
}

static inline ShaderTypeEnum GetShaderTypeEnumFromString(const std::string& vString) {
    if (vString == "VERTEX")
        return ShaderTypeEnum::SHADER_TYPE_VERTEX;
    else if (vString == "GEOMETRY")
        return ShaderTypeEnum::SHADER_TYPE_GEOMETRY;
    else if (vString == "TESSCONTROL")
        return ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL;
    else if (vString == "TESSEVAL")
        return ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL;
    else if (vString == "FRAGMENT")
        return ShaderTypeEnum::SHADER_TYPE_FRAGMENT;
    else if (vString == "COMPUTE")
        return ShaderTypeEnum::SHADER_TYPE_COMPUTE;
    else if (vString == "NOTE")
        return ShaderTypeEnum::NOTE_SPECIAL_TYPE;
    else if (vString == "LINK")
        return ShaderTypeEnum::LINK_SPECIAL_TYPE;
    else if (vString == "SECTION")
        return ShaderTypeEnum::SECTION_TYPE;
    else if (vString == "COMMON")
        return ShaderTypeEnum::SECTION_COMMON;

    return ShaderTypeEnum::SHADER_TYPE_NONE;
}

enum class ShaderVaryingDirection {
    SHADER_NOT_VARYING = 0,
    SHADER_VARYING_IN,
    SHADER_VARYING_OUT,
    SHADER_LAYOUT_IN,
    SHADER_LAYOUT_OUT
};

enum class UniformRequestServiceEnum {
    UNIFORM_REQUEST_SERVICE_NONE = 0,
    UNIFORM_REQUEST_SERVICE_MOUSE_2POSDELTA_SCREEN,               // vec2 differentiel
    UNIFORM_REQUEST_SERVICE_MOUSE_2POSDELTA_SCREEN_RIGHT_BUTTON,  // vec2 differentiel avec appui bouton droit
    UNIFORM_REQUEST_SERVICE_MOUSE_2POS_SCREEN,
    UNIFORM_REQUEST_SERVICE_MOUSE_2POS_NORMALIZED_CENTERED_SCREEN,  // like glslsandbox
    UNIFORM_REQUEST_SERVICE_MOUSE_2POS_2CLICKPOS_SCREEN,            // like shadertoy
    UNIFORM_REQUEST_SERVICE_MOUSE_2POS_RIGHT_BUTTON,                // vec2 avec appui bouton droit
    UNIFORM_REQUEST_SERVICE_SCREEN_SIZE_2,
    UNIFORM_REQUEST_SERVICE_SCREEN_SIZE_3,
    UNIFORM_REQUEST_SERVICE_BUFFER_SIZE_2,
    UNIFORM_REQUEST_SERVICE_FRAME_NUMBER,
    UNIFORM_REQUEST_SERVICE_FRAME_RENDER_TIME
};

static inline std::string GetUniformRequestServiceEnumString(UniformRequestServiceEnum vUniformRequestServiceEnum) {
    switch (vUniformRequestServiceEnum) {
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_MOUSE_2POSDELTA_SCREEN: return "MOUSE_2POSDELTA_SCREEN";
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_MOUSE_2POSDELTA_SCREEN_RIGHT_BUTTON: return "MOUSE_2POSDELTA_SCREEN_RIGHT_BUTTON";
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_MOUSE_2POS_2CLICKPOS_SCREEN: return "MOUSE_2POS_2CLICKPOS_SCREEN";
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_MOUSE_2POS_NORMALIZED_CENTERED_SCREEN: return "MOUSE_2POS_NORMALIZED_CENTERED_SCREEN";
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_MOUSE_2POS_RIGHT_BUTTON: return "MOUSE_2POS_RIGHT_BUTTON";
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_MOUSE_2POS_SCREEN: return "MOUSE_2POS_SCREEN";
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_NONE: return "NONE";
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_SCREEN_SIZE_2: return "SCREEN_SIZE_2";
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_SCREEN_SIZE_3: return "SCREEN_SIZE_3";
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_BUFFER_SIZE_2: return "BUFFER_SIZE_2";
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_FRAME_NUMBER: return "FRAME_NUMBER";
        case UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_FRAME_RENDER_TIME: return "FRAME_RENDER_TIME";
    }
    return "";
}

static inline UniformRequestServiceEnum GetUniformRequestServiceEnumFromString(const std::string& vString) {
    if (vString == "MOUSE_2POSDELTA_SCREEN")
        return UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_MOUSE_2POSDELTA_SCREEN;
    if (vString == "MOUSE_2POS_2CLICKPOS_SCREEN")
        return UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_MOUSE_2POS_2CLICKPOS_SCREEN;
    if (vString == "MOUSE_2POS_NORMALIZED_CENTERED_SCREEN")
        return UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_MOUSE_2POS_NORMALIZED_CENTERED_SCREEN;
    if (vString == "MOUSE_2POS_SCREEN")
        return UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_MOUSE_2POS_SCREEN;
    if (vString == "NONE")
        return UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_NONE;
    if (vString == "SCREEN_SIZE_2")
        return UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_SCREEN_SIZE_2;
    if (vString == "SCREEN_SIZE_3")
        return UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_SCREEN_SIZE_3;
    if (vString == "BUFFER_SIZE_2")
        return UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_BUFFER_SIZE_2;
    return UniformRequestServiceEnum::UNIFORM_REQUEST_SERVICE_NONE;
}

enum class ShaderNodeTypeEnum { SHADER_TYPE_UNKNOW = 0, SHADER_TYPE_FOR_QUAD = 1, SHADER_TYPE_FOR_POINTS = 2, SHADER_TYPE_FOR_CUSTOM_SHAPE = 3 };

enum class DocumentFragmentParseErrorEnum { NOTHING = 0, BAD_XML_FORMAT = 1, DUPLICATE_UNIFORM_NAME = 2, UNIFORM_NAME_MISSED = 3 };

enum class DocumentFileFormatEnum { DOC_FILE_FORMAT_NOTHING = 0, DOC_FILE_FORMAT_ASCII = 1, DOC_FILE_FORMAT_EMBEDDED = 2, DOC_FILE_FORMAT_TEMPLATE = 3 };

// Glslang Command-line options
enum class TOptions {
    EOptionNone = 0,
    EOptionIntermediate = (1 << 0),
    EOptionSuppressInfolog = (1 << 1),
    EOptionMemoryLeakMode = (1 << 2),
    EOptionRelaxedErrors = (1 << 3),
    EOptionGiveWarnings = (1 << 4),
    EOptionLinkProgram = (1 << 5),
    EOptionMultiThreaded = (1 << 6),
    EOptionDumpConfig = (1 << 7),
    EOptionDumpReflection = (1 << 8),
    EOptionSuppressWarnings = (1 << 9),
    EOptionDumpVersions = (1 << 10),
    EOptionSpv = (1 << 11),
    EOptionHumanReadableSpv = (1 << 12),
    EOptionVulkanRules = (1 << 13),
    EOptionDefaultDesktop = (1 << 14),
    EOptionOutputPreprocessed = (1 << 15),
    EOptionOutputHexadecimal = (1 << 16),
    EOptionReadHlsl = (1 << 17),
    EOptionCascadingErrors = (1 << 18),
    EOptionAutoMapBindings = (1 << 19),
    EOptionFlattenUniformArrays = (1 << 20),
    EOptionNoStorageFormat = (1 << 21),
    EOptionKeepUncalled = (1 << 22),
    EOptionHlslOffsets = (1 << 23),
    EOptionHlslIoMapping = (1 << 24),
    EOptionAutoMapLocations = (1 << 25),
    EOptionDebug = (1 << 26),
    EOptionStdin = (1 << 27),
    EOptionOptimizeDisable = (1 << 28),
    EOptionOptimizeSize = (1 << 29),
    EOptionInvertY = (1 << 30),
    EOptionDumpBareVersion = (1 << 31),
};

// Compilation Mode
enum class CompilationModeEnum { COMPILATION_MODE_MANUAL = 0, COMPILATION_MODE_AUTO = 1 };

enum class AttribTypeEnum {
    ATTRIB_TYPE_ATTRIBUTE = 0,
    ATTRIB_TYPE_VARYING,
    ATTRIB_TYPE_LAYOUT,
    ATTRIB_TYPE_CONST_UNIFORM  // un uniform constant comme le nombre mas de vertex
};

enum class ShaderMsg {
    SHADER_MSG_OK = 0,
    SHADER_MSG_ERROR,
    SHADER_MSG_WARNING,
    SHADER_MSG_NOT_SUPPORTED,
};

struct ProgramMsg {
    ShaderMsg syntaxMsg;
    ShaderMsg vertexMsg;
    ShaderMsg fragmentMsg;
    ShaderMsg geometryMsg;
    ShaderMsg tesselationControlMsg;
    ShaderMsg tesselationEvalMsg;
    ShaderMsg computeMsg;
    ShaderMsg linkMsg;

    ProgramMsg() {
        syntaxMsg = ShaderMsg::SHADER_MSG_OK;
        vertexMsg = ShaderMsg::SHADER_MSG_OK;
        fragmentMsg = ShaderMsg::SHADER_MSG_OK;
        geometryMsg = ShaderMsg::SHADER_MSG_OK;
        tesselationControlMsg = ShaderMsg::SHADER_MSG_OK;
        tesselationEvalMsg = ShaderMsg::SHADER_MSG_OK;
        computeMsg = ShaderMsg::SHADER_MSG_OK;
        linkMsg = ShaderMsg::SHADER_MSG_ERROR;  // le seul flag qui est obligaotirement set par le linker
    }

    void CompleteWithShaderResult(ProgramMsg& vOther) {
        vertexMsg = vOther.vertexMsg;
        fragmentMsg = vOther.fragmentMsg;
        geometryMsg = vOther.geometryMsg;
        tesselationControlMsg = vOther.geometryMsg;
        tesselationEvalMsg = vOther.geometryMsg;
        computeMsg = vOther.computeMsg;
        linkMsg = vOther.linkMsg;
    }

    bool isThereSomeErrors() {
        return syntaxMsg == ShaderMsg::SHADER_MSG_ERROR || vertexMsg == ShaderMsg::SHADER_MSG_ERROR || fragmentMsg == ShaderMsg::SHADER_MSG_ERROR ||
            geometryMsg == ShaderMsg::SHADER_MSG_ERROR || tesselationControlMsg == ShaderMsg::SHADER_MSG_ERROR || tesselationEvalMsg == ShaderMsg::SHADER_MSG_ERROR ||
            computeMsg == ShaderMsg::SHADER_MSG_ERROR || linkMsg == ShaderMsg::SHADER_MSG_ERROR;
    }

    bool isThereSomeWarnings() {
        return syntaxMsg == ShaderMsg::SHADER_MSG_WARNING || vertexMsg == ShaderMsg::SHADER_MSG_WARNING || fragmentMsg == ShaderMsg::SHADER_MSG_WARNING ||
            geometryMsg == ShaderMsg::SHADER_MSG_WARNING || tesselationControlMsg == ShaderMsg::SHADER_MSG_WARNING ||
            tesselationEvalMsg == ShaderMsg::SHADER_MSG_WARNING || computeMsg == ShaderMsg::SHADER_MSG_ERROR || linkMsg == ShaderMsg::SHADER_MSG_WARNING;
    }
};

enum class ImportTypeEnum { INPORT_SHADERTOY = 0, INPORT_GLSLSANDBOX };

enum class ExportTypeEnum { EXPORT_SHADERTOY = 0, EXPORT_GLSLSANDBOX, EXPORT_COCOS2DX };

enum class UniformPlaceEnum { UNIFORM_PLACE_NONE = 0, UNIFORM_PLACE_OUTPUT, UNIFORM_PLACE_INPUT, UNIFORM_PLACE_CONTROL };

enum class BaseMeshFormatEnum { PRIMITIVE_FORMAT_PNC = 0, PRIMITIVE_FORMAT_PNTC, PRIMITIVE_FORMAT_PNTBTC };

enum class ModelModeEnum { RENDERING_MODE = 0, EXPORT_MODE = 1 };

struct TextureParamsStruct {
    GLenum internalFormat = GL_RGBA32F;
    GLenum format = GL_RGBA;
    GLenum dataType = GL_FLOAT;
    bool useMipMap = true;
    int maxMipMapLvl = 1000;
    GLenum wrapS = GL_CLAMP_TO_EDGE;
    GLenum wrapT = GL_CLAMP_TO_EDGE;
    GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR;
    GLenum magFilter = GL_LINEAR;
};

struct ShaderInfos {
    std::string id;

    std::string header;
    std::string framebuffer;
    std::string common_uniforms;
    std::string specific_uniforms;
    std::string note;
    std::string common;
    std::string vertex;
    std::string geom;
    std::string tesscontrol;
    std::string tesseval;
    std::string fragment;

    std::string name;
    std::string user;
    std::string description;
    std::string url;
    bool published = false;

    std::string shader_id;  // ex : 3lsSDs pour shadertoy

    std::string error;

    ct::fColor color;
    ShaderPlaform platform = ShaderPlaform::SPF_UNKNOW;
    bool IsBuffer = false;
    bool IsMain = false;
    std::unordered_map<std::string, bool> varying;

    std::list<ShaderInfos> childs;

    std::string GetCode() {
        return header + note + framebuffer + common_uniforms + specific_uniforms + common + vertex + tesscontrol + tesseval + geom + fragment;
    }
};
