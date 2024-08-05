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
#include <ctools/ConfigAbstract.h>
#include <Interfaces/MidiInterface.h>
#include <Interfaces/WidgetInterface.h>
#include <Uniforms/UniformVariant.h>
#include <CodeTree/Parsing/SectionCode.h>
#include <Headers/RenderPackHeaders.h>
#include <CodeTree/Parsing/UniformParsing.h>
#include <Uniforms/UniformWidgets.h>
#include <RtMidi.h>
#include <imgui.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <set>

class CodeTree;
class ShaderKey;
class RenderPack;
struct GuiBackend_Window;
class UniformVariant;
class MidiSystem : public WidgetInterface, public UniformWidgets, public MidiInterface, public conf::ConfigAbstract {
private:
    std::vector<std::shared_ptr<RtMidiIn>> m_RtMidis;
    bool m_ShowConfigDialog = false;
    ImGuiListClipper m_Clipper;
    ct::ivec2 range = ct::ivec2((int)1e5, (int)-1e5);
    std::string lastId;
    bool m_WasChanged = false;
    bool m_Activated = false;

public:
    bool Init(CodeTreePtr vCodeTree) override;
    void Unit() override;
    bool DrawWidget(CodeTreePtr vCodeTree,
                    UniformVariantPtr vUniPtr,
                    const float& vMaxWidth,
                    const float& vFirstColumnWidth,
                    RenderPackWeak vRenderPack,
                    const bool& vShowUnUsed,
                    const bool& vShowCustom,
                    const bool& vForNodes,
                    bool* vChange) override;
    bool SerializeUniform(UniformVariantPtr vUniform, std::string* vStr) override;
    bool DeSerializeUniform(ShaderKeyPtr vShaderKey, UniformVariantPtr vUniform, const std::vector<std::string>& vParams) override;
    void Complete_Uniform(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) override;
    bool UpdateUniforms(UniformVariantPtr vUniPtr) override;

    void Update();
    void AddMessage(double vDeltatime, std::vector<uint8_t>* vMessage, void* vUserData);

    bool UpdateIfNeeded(ShaderKeyPtr vKey, ct::ivec2 vScreenSize);
    bool WasChanged();
    void ResetChange();  // will reset puWasChanged

    ///////////////////////////////////////////////////////
    //// CONFIGURATION ////////////////////////////////////
    ///////////////////////////////////////////////////////

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

    ///////////////////////////////////////////////////////
    //// SETTINGS /////////////////////////////////////////
    ///////////////////////////////////////////////////////

    void LoadSettings();
    void SaveSettings();
    void ShowContent(GuiBackend_Window vWin);

private:
    void DisplayConfigPane(GuiBackend_Window vWin);

public:  // overide
    std::vector<MidiMessage> GetMidiMessages() override;
    MidiMessage GetMidiMessage(const uint32_t& vPort) override;
    MidiMessage GetAndClearMidiMessage(const uint32_t& vPort) override;
    MidiMessage ClearMidiMessage(const uint32_t& vPort) override;
    MidiStruct GetMidiMessageDB(const uint32_t& vPort) override;
    std::string GetMidiDeviceName(const uint32_t& vPort) override;

public:
    static MidiSystem* Instance() {
        static MidiSystem* _instance = new MidiSystem();
        return _instance;
    }

protected:
    MidiSystem();                     // Prevent construction
    MidiSystem(const MidiSystem&){};  // Prevent construction by copying
    MidiSystem& operator=(const MidiSystem&) {
        return *this;
    };              // Prevent assignment
    ~MidiSystem();  // Prevent unwanted destruction
};
