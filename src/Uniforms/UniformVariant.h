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

#ifdef _DEBUG
// #define DEBUG_UNIFORMS
#endif

#include <glad/glad.h>
#include <Headers/RenderPackHeaders.h>
#include <ctools/cTools.h>
#include <uTypes/uTypes.h>

#pragma warning(push)
#pragma warning(disable : 4201)  // suppress even more warnings about nameless structs
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)

/////////////////////////////////////////////////////////////
///////// cUniformVariant ///////////////////////////////////
/////////////////////////////////////////////////////////////

// struct mem4 { float x, y, z, w; };
class Texture2D;
class TextureCube;
class Texture3D;
class TextureSound;
class UniformsMultiLoc;
class FrameBuffersPipeLine;
class RecordBuffer;
class ShaderKey;
class UniformVariant {
public:
#ifdef DEBUG_UNIFORMS
    static std::vector<std::string> save;
#endif
    static size_t counter;
    static UniformVariantPtr create();
#ifdef DEBUG_UNIFORMS
    static UniformVariantPtr create(void *vParentPtr, const std::string &vName, std::string vDebug);
#else
    static UniformVariantPtr create(ShaderKeyPtr vParentPtr, const std::string &vName);
#endif

#ifdef DEBUG_UNIFORMS
    static bool destroy(UniformVariantPtr vPtr, void *vParentPtr, std::string vDebug);
#else
    static bool destroy(UniformVariantPtr vPtr, ShaderKeyPtr vParentPtr);
#endif

public:
    UniformVariantPtr m_This = nullptr;
    ShaderKeyPtr owner = nullptr;
    UniformsMultiLoc *parentMultiLoc;  // il sera defini uniquement dans UniformsMultiLoc::Popagate

#ifdef DEBUG_UNIFORMS
    std::string ownerString;
#endif

    uType::uTypeEnum glslType = uType::uTypeEnum::U_VOID;
    bool constant = false;
    bool canWeSave = false;
    bool noExport = false;
    FrameBuffersPipeLinePtr pipe = nullptr;
    std::shared_ptr<RecordBuffer> record = nullptr;
    bool uSampler2DFromThread = false;
    int *uSampler2DArr = nullptr;
    bool ownSampler2DArr = false;
    int uSamplerCube = -1;
    int id[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int loc = -1;
    int slot = -1;
    std::string nameForSearch;
    std::string name;
    std::string widget;
    std::string widgetType;
    bool timeLineSupported = false;
    std::vector<std::string> filePathNames;
    std::vector<std::string> choices;
    std::string bufferShaderName;
    std::string inFileBufferName;
    std::string computeShaderName;
    int attachment = -1;
    std::string target;

    std::string bufferFileInCode;  // pour al generation de template

    // si true, alors on ne maj pas cet uniform quand on charge une config
    bool lockedAgainstConfigLoading = false;

    std::string text;
    bool uiOnly = false;

    bool mipmap = false;
    bool flip = false;
    std::string filter;
    std::string wrap;
    int maxmipmaplvl = -1;

    bool textureChoiceActivated = false;
    bool textureFileChoosebox = false;
    bool textureFlipChoosebox = false;
    bool textureMipmapChoosebox = false;
    bool textureFilterChoosebox = false;
    bool textureWrapChoosebox = false;

    bool soundChoiceActivated = false;
    bool soundFileChoosebox = false;
    bool soundLoopChoosebox = false;

    bool bufferChoiceActivated = false;
    bool bufferFileChoosebox = false;
    bool bufferFlipChoosebox = false;
    bool bufferMipmapChoosebox = false;
    bool bufferFilterChoosebox = false;
    bool bufferWrapChoosebox = false;

    int uSampler2D = -1;
    int uImage2D = -1;
    Texture2DPtr texture_ptr = nullptr;
    bool ownTexture = false;

    float ratioXY = 0.0f;
    int uSampler3D = -1;
    int uImage3D = -1;
    Texture3DPtr volume_ptr = nullptr;
    bool ownVolume = false;
    std::string volumeFormat;
    GLenum computeTextureFormat;

    int uSampler1D = -1;
    int uImage1D = -1;
    ctTexturePtr sound_ptr = nullptr;        // 1D
    ctTexturePtr sound_histo_ptr = nullptr;  // 2D
    float soundVolume = 0.0f;
    bool soundLoop = false;
    int soundHisto = -1;
    bool ownSound = false;

    TextureCubePtr cubemap_ptr = nullptr;
    bool ownCubeMap = false;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
    bool bx = false;
    bool by = false;
    bool bz = false;
    bool bw = false;
    int ix = 0;
    int iy = 0;
    int iz = 0;
    int iw = 0;
    uint32_t ux = 0;
    uint32_t uy = 0;
    uint32_t uz = 0;
    uint32_t uw = 0;
    int count = 0;  // indique le nombre de var utilisée donc valeur de 1 à 4 pour slider ou nombre de composant pour array
    bool IsIntegrated = false;
    ct::fvec4 inf;
    ct::fvec4 sup;
    ct::fvec4 def;
    ct::fvec4 step;
    ct::bvec4 bdef;

    std::string buttonName0;
    std::string buttonName1;
    std::string buttonName2;
    std::string buttonName3;

    float *uFloatArr = nullptr;
    bool ownFloatArr = false;
    ct::fvec2 *uVec2Arr = nullptr;
    bool ownVec2Arr = false;
    ct::fvec3 *uVec3Arr = nullptr;
    bool ownVec3Arr = false;
    ct::fvec4 *uVec4Arr = nullptr;
    bool ownVec4Arr = false;
    int frame = 0;

    glm::mat2 mat2;     // matrix
    glm::mat2 mat2Def;  // default matrix

    glm::mat3 mat3;     // matrix
    glm::mat3 mat3Def;  // default matrix

    glm::mat4 mat4;     // matrix
    glm::mat4 mat4Def;  // default matrix

    // cam
    ct::fvec2 camRotXY;
    float camZoom = 0.0f;
    ct::fvec3 camTranslateXYZ;
    ct::fvec2 camDefRotXY;
    float camDefZoom = 0.0f;
    ct::fvec3 camDefTranslateXYZ;
    bool camNeedRecalc = false;

    std::string sectionName;
    int sectionOrder = 0;
    std::string sectionCond;  // section a finaliser dans la parse andcompil du renderpack ( une fois que tout les uniforms sont listés)

    // section vivible sur checkbox
    bool useVisCheckCond = false;      // pour savoir si il y a une checkbox d'activée sur cette section
    std::string uniCheckCondName;      // le nom le l'uniform de la checkbox
    bool uniCheckCond = false;         // la conditon de checkbox, cad true ou false
    float *uniCheckCondPtr = nullptr;  // pointeur sur la valeur x de l'uniform dont depend la condition

    // float cond
    int useVisOpCond = 0;             // 0 => no op // 1 > // 2 >= // 3 < // 4 <=
    float uniOpCondThreshold = 0.0f;  // valeur de seuil
    float *uniCondPtr = nullptr;      // pointeur sur la valeur x de l'uniform dont depend la condition

    // section visible sur combobox
    bool useVisComboCond = false;    // pour savoir si il y a une combobox d'activée sur cette section
    std::string uniComboCondName;    // le nom le l'uniform de la combobox
    int uniComboCond = 0;            // la conditon de combobox, cad quel item 0,1,2,3, etc..
    int *uniComboCondPtr = nullptr;  // pointeur sur la valeur x de l'uniform dont depend la condition
    bool uniComboCondDir = false;    // sens de comparaison eq or not eq (neq)

    std::string comment;
    // size_t commentBufferLen;
    // char commentBuffer[MAX_UNIFORM_COMMENT_BUFFER_SIZE];

    size_t SourceLinePos;
    size_t TargetLinePos;

    // midi
    std::string midiDeviceName;
    std::vector<int> midiId;  // identification bytes
    uint32_t midiCountBytes = 0;
    std::vector<uint8_t> midiBytes;  // midi bytes used for value

public:
    void operator=(const UniformVariant &) = delete;
    UniformVariant();
    virtual ~UniformVariant();
    void ResetToDefault();
    void copy(UniformVariantPtr vUniPtr, RenderPackWeak vMainRenderPack = RenderPackWeak());
    void copyValues(UniformVariantPtr vUniPtr, RenderPackWeak vMainRenderPack = RenderPackWeak());
    void copyValuesForTimeLine(UniformVariantPtr vUniPtr, RenderPackWeak vMainRenderPack = RenderPackWeak());
};
