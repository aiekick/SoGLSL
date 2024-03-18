// NoodlesPlate Copyright (C) 2017-2023 Stephane Cuillerdier aka Aiekick
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

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>
#include <Headers/RenderPackHeaders.h>
#include <Systems/Interfaces/WidgetInterface.h>

#include <ImGuiPack/ImGuiPack.h>

enum GizmoCullingPimitiveEnum {
    GIZMO_CULLING_PRIMITIVE_CUBE = 0,
    GIZMO_CULLING_PRIMITIVE_SPHERE,
    GIZMO_CULLING_PRIMITIVE_NONE,
    GIZMO_CULLING_PRIMITIVE_Count
};

class CameraSystem;
class CodeTree;
class ShaderKey;
class RenderPack;

class UniformVariant;

struct UniformParsedStruct;
class GizmoSystem : public WidgetInterface, public conf::ConfigAbstract {
public:
    bool puActivated = false;
    bool puUseGizmoCulling = false;
    ImGuizmo::OPERATION puCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
    ImGuizmo::MODE puCurrentGizmoMode = ImGuizmo::MODE::WORLD;
    GizmoCullingPimitiveEnum puGizmoCullingPrimitiveEnum = GizmoCullingPimitiveEnum::GIZMO_CULLING_PRIMITIVE_CUBE;
    bool puNeedOneUniformUpdate = false;

public:
    RenderPackPtr puGizmoCulling_RenderPack = nullptr;
    ShaderKeyPtr puGizmoCulling_Key = nullptr;
    UniformVariantPtr puGizmoCullingUniform = nullptr;

private:
    ct::ActionTime puActionTime;

public:
    static GizmoSystem* Instance() {
        static GizmoSystem _instance;
        return &_instance;
    }

protected:
    GizmoSystem();                      // Prevent construction
    GizmoSystem(const GizmoSystem&){};  // Prevent construction by copying
    GizmoSystem& operator=(const GizmoSystem&) {
        return *this;
    };               // Prevent assignment
    ~GizmoSystem();  // Prevent unwanted destruction

public:
    bool Init(CodeTreePtr vCodeTree) override;
    void Unit() override;
    bool DrawWidget(CodeTreePtr vCodeTree, UniformVariantPtr vUniPtr, const float& vMaxWidth, const float& vFirstColumnWidth,
                    RenderPackWeak vRenderPack, const bool& vShowUnUsed, const bool& vShowCustom, const bool& vForNodes, bool* vChange) override;
    bool SerializeUniform(UniformVariantPtr vUniform, std::string* vStr) override;
    bool DeSerializeUniform(ShaderKeyPtr vShaderKey, UniformVariantPtr vUniform, const std::vector<std::string>& vParams) override;
    void Complete_Uniform(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed,
                          UniformVariantPtr vUniform) override;
    bool UpdateUniforms(UniformVariantPtr vUniPtr) override;

    bool Use();
    bool UseCulling();

    bool IsActivated();
    void SetActivation(bool vActivation);
    bool DrawMenu();
    bool DrawTooltips(RenderPackWeak vRenderPack, float vDisplayQuality, CameraSystem* vCamera, ct::ivec2 vScreenSize);
    bool DrawPane(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack, float vDisplayQuality, CameraSystem* vCamera);
    void Capture(RenderPackWeak vRenderPack, float vDisplayQuality, CameraSystem* vCamera);
    void Resize(RenderPackWeak vRenderPack, float vDisplayQuality, CameraSystem* vCamera);
    bool NeedRefresh();
    void ResetToDefault(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack);

public:
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

public:
    bool DrawGizmoTransformDialog(UniformVariantPtr vUniPtr, CameraSystem* vCamera);
    bool UpdateGizmos(RenderPackWeak vRenderPack, UniformVariantPtr vUniPtr, CameraSystem* vCamera, ct::ivec2 vScreenSize);

private:
    void Complete_Uniform_Gizmo(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed,
                                UniformVariantPtr vUniform);
    void Complete_Uniform_Culling(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed,
                                  UniformVariantPtr vUniform);

public:
    std::string InitRenderPack(const GuiBackend_Window& vWin, CodeTreePtr vCodeTree);
    bool LoadRenderPack();
    void SetRenderPackFXAA(bool vUseFXAA, int vCountSamples);
    void SaveRenderPack();
    void FinishRenderPack();
    void DestroyRenderPack();
    void CreateGizmoCullingScript();
    RenderPackWeak GetRenderPack() {
        return puGizmoCulling_RenderPack;
    }
    ShaderKeyPtr GetShaderKey() {
        return puGizmoCulling_Key;
    }

private:
    bool CreateFilePathName(const std::string& vFilePathName, std::string vType, bool useStandardPaths = true);
};
