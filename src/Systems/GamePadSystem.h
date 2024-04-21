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

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>
#include <Uniforms/UniformVariant.h>
#include <CodeTree/Parsing/SectionCode.h>
#include <Headers/RenderPackHeaders.h>
#include <CodeTree/Parsing/UniformParsing.h>
#include <Systems/Interfaces/WidgetInterface.h>

#include <string>

struct GamePadStruct
{
	
};

class CodeTree;
class ShaderKey;
class RenderPack;
class UniformVariant;
class GamePadSystem : public WidgetInterface, public conf::ConfigAbstract
{
private:
	bool puAccumModeX = false;
	bool puAccumModeY = false;
	bool puAccumModeZ = false;
	bool puAccumModeW = false;
	UniformVariant puValue;
	std::unordered_map<int, std::string> puJoysticks;
	float puJoySticks_Sticks[100] = {};
	float puJoySticks_Buttons[100] = {};
	bool puWasChanged = false;

public:
	bool puActivated = false;

public:
	ct::fvec3 puLeftThumb; // pos x,y, bouton z
	ct::fvec3 puRightThumb; // pos x,y, bouton z
	float puLeftTrigger = 0.0f;
	float puRightTrigger = 0.0f;
	ct::fvec4 puCross;
	ct::fvec4 puMainButtons;

	std::unordered_map<std::string, GamePadStruct> puConfigs;
	std::string puCurrentConfig;
	int puCurrentJoyStick = 0;

	std::unordered_map<std::string, int> puAnalogicsUniforms;
	std::unordered_map<std::string, int> puButtonsUniforms;

public:
	bool Init(CodeTreePtr vCodeTree) override;
	void Unit() override;
	bool DrawWidget(CodeTreePtr vCodeTree, UniformVariantPtr vUniPtr,
		const float& vMaxWidth, const float& vFirstColumnWidth, RenderPackWeak vRenderPack,
		const bool& vShowUnUsed, const bool& vShowCustom, const bool& vForNodes, bool* vChange) override;
	bool SerializeUniform(UniformVariantPtr vUniform, std::string* vStr) override;
	bool DeSerializeUniform(ShaderKeyPtr vShaderKey, UniformVariantPtr vUniform, const std::vector<std::string>& vParams) override;
	void Complete_Uniform(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) override;
	bool UpdateUniforms(UniformVariantPtr vUniPtr) override;

	void InitMapping();
	void ListGamePads();
	float GetValue(int axisCount, int axisIdx);
	float ConvertValue(int axisCount, const float *axes, int axisIdx, float inf, float sup);
	float SetAnalogicValue(int axisCount, const float *axes, int axisIdx, int vSens, int vInv);
	float SetButtonValue(int buttonCount, const unsigned char *buttons, int buttonIdx);
	void Update();
	void UpdateAnalogics();
	void UpdateButtons();
	bool UpdateIfNeeded(ShaderKeyPtr vKey, ct::ivec2 vScreenSize);
	bool WasChanged();
	void ResetChange(); // will reset puWasChanged
	bool DrawActivationButton();
	bool DrawMenu();

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
	void ShowContent();

public:
	static GamePadSystem* Instance()
	{
		static GamePadSystem _instance;
		return &_instance;
	}

protected:
	GamePadSystem(); // Prevent construction
	GamePadSystem(const GamePadSystem&) {}; // Prevent construction by copying
	GamePadSystem& operator =(const GamePadSystem&) { return *this; }; // Prevent assignment
	~GamePadSystem(); // Prevent unwanted destruction
};
