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

#include <Systems/GamePadSystem.h>
#include <CodeTree/ShaderKey.h>
#include <Gui/CustomGuiWidgets.h>
#include <CodeTree/CodeTree.h>
#include <Renderer/RenderPack.h>
#include <ctools/Logger.h>

#include <imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>

#include <Res/CustomFont.h>

#ifdef USE_SDL2

#else
#include <GLFW/glfw3.h>
#endif

static void joystick_callback(int /*jid*/, int event)
{
#ifdef USE_SDL2

#else
	if (event == GLFW_CONNECTED)
	{
		GamePadSystem::Instance()->ListGamePads();
	}
	else if (event == GLFW_DISCONNECTED)
	{
		GamePadSystem::Instance()->ListGamePads();
	}
#endif
}

GamePadSystem::GamePadSystem()
{

}

GamePadSystem::~GamePadSystem()
{
	
}

void GamePadSystem::InitMapping()
{
	// ps gamepad

	puAnalogicsUniforms.clear();

	puAnalogicsUniforms["vec3(gamepad:leftthumb).x"] = 0;
	puAnalogicsUniforms["vec3(gamepad:leftthumb).y"] = 1;

	puAnalogicsUniforms["vec3(gamepad:rightthumb).x"] = 2;
	puAnalogicsUniforms["vec3(gamepad:rightthumb).y"] = 5;

	puAnalogicsUniforms["float(gamepad:lefttrigger)"] = 3;
	puAnalogicsUniforms["float(gamepad:righttrigger)"] = 4;

	puButtonsUniforms.clear();

	puButtonsUniforms["vec3(gamepad:leftthumb).z"] = 10;
	puButtonsUniforms["vec3(gamepad:rightthumb).z"] = 11;

	puButtonsUniforms["vec4(gamepad:cross).x"] = 17;
	puButtonsUniforms["vec4(gamepad:cross).y"] = 14;
	puButtonsUniforms["vec4(gamepad:cross).z"] = 15;
	puButtonsUniforms["vec4(gamepad:cross).w"] = 16;

	puButtonsUniforms["vec4(gamepad:mainbuttons).x"] = 1;
	puButtonsUniforms["vec4(gamepad:mainbuttons).y"] = 2;
	puButtonsUniforms["vec4(gamepad:mainbuttons).z"] = 3;
	puButtonsUniforms["vec4(gamepad:mainbuttons).w"] = 0;

	puCurrentJoyStick = 0; // joystick 1
}

bool GamePadSystem::Init(CodeTreePtr vCodeTree)
{
#ifdef USE_SDL2
	CTOOL_DEBUG_BREAK;
#else
	glfwSetJoystickCallback(joystick_callback);
#endif
	puActivated = false;
	memset(puJoySticks_Sticks, 0, sizeof(puJoySticks_Sticks));
	ListGamePads();
	puWasChanged = false;
	puLeftTrigger = 0.0f;
	puRightTrigger = 0.0f;

	InitMapping();

	return true;
}

void GamePadSystem::Unit()
{

}

float GamePadSystem::GetValue(int /*axisCount*/, int /*axisIdx*/)
{
	return 0.0f;
}

float GamePadSystem::ConvertValue(int axisCount, const float *axes, int axisIdx, float inf, float sup)
{
	float v = (axisCount > axisIdx) ? axes[axisIdx] : inf;
	v = (v - inf) / (sup - inf);
	v = ct::clamp(v, 0.0f, 1.0f);
	return v;
}

float GamePadSystem::SetAnalogicValue(int axisCount, const float *axes, int axisIdx, int vSens, int vInv)
{
	if (axisIdx > -1)
	{
		float v = 0.0f;
		if (vSens >= 0)
			v += ConvertValue(axisCount, axes, axisIdx, +0.3f, +0.9f) * vInv;
		if (vSens <= 0)
			v -= ConvertValue(axisCount, axes, axisIdx, -0.3f, -0.9f) * vInv;

		if (IS_FLOAT_DIFFERENT(puJoySticks_Sticks[axisIdx], v))
		{
			puWasChanged = true;
		}

		puJoySticks_Sticks[axisIdx] = v;

		return v;
	}

	return 0.0f;
}

float GamePadSystem::SetButtonValue(int buttonCount, const unsigned char *buttons, int buttonIdx)
{
	if (buttonIdx > -1 && buttonCount > buttonIdx)
	{
		float val = 0.0f;

		if (buttons[buttonIdx] == GuiBackend_MOUSE_PRESS)
		{
			val = 1.0f;			
		}

		if (IS_FLOAT_DIFFERENT(puJoySticks_Buttons[buttonIdx], val))
		{
			puWasChanged = true;
		}

		puJoySticks_Buttons[buttonIdx] = val;

		return val;
	}

	return 0.0f;
}

void GamePadSystem::Update()
{
	if (puActivated)
	{
		UpdateAnalogics();
		UpdateButtons();
	}
}

void GamePadSystem::UpdateAnalogics()
{
#ifdef USE_SDL2
	CTOOL_DEBUG_BREAK;
#else
	// Update gamepad inputs
	int axes_count = 0;
	const float* axes = glfwGetJoystickAxes(puCurrentJoyStick, &axes_count);
	int buttons_count = 0;
	const unsigned char* buttons = glfwGetJoystickButtons(puCurrentJoyStick, &buttons_count);

	puLeftThumb.x = SetAnalogicValue(axes_count, axes, puAnalogicsUniforms["vec3(gamepad:leftthumb).x"], 0, 1);
	puLeftThumb.y = SetAnalogicValue(axes_count, axes, puAnalogicsUniforms["vec3(gamepad:leftthumb).y"], 0, -1);
	puLeftThumb.z = SetButtonValue(buttons_count, buttons, puButtonsUniforms["vec3(gamepad:leftthumb).z"]);

	puRightThumb.x = SetAnalogicValue(axes_count, axes, puAnalogicsUniforms["vec3(gamepad:rightthumb).x"], 0, 1);
	puRightThumb.y = SetAnalogicValue(axes_count, axes, puAnalogicsUniforms["vec3(gamepad:rightthumb).y"], 0, -1);
	puRightThumb.z = SetButtonValue(buttons_count, buttons, puButtonsUniforms["vec3(gamepad:rightthumb).z"]);

	puLeftTrigger = SetAnalogicValue(axes_count, axes, puAnalogicsUniforms["float(gamepad:lefttrigger)"], 1, 1);
	puRightTrigger = SetAnalogicValue(axes_count, axes, puAnalogicsUniforms["float(gamepad:righttrigger)"], 1, 1);

	puCross.x = SetButtonValue(buttons_count, buttons, puButtonsUniforms["vec4(gamepad:cross).x"]);
	puCross.y = SetButtonValue(buttons_count, buttons, puButtonsUniforms["vec4(gamepad:cross).y"]);
	puCross.z = SetButtonValue(buttons_count, buttons, puButtonsUniforms["vec4(gamepad:cross).z"]);
	puCross.w = SetButtonValue(buttons_count, buttons, puButtonsUniforms["vec4(gamepad:cross).w"]);

	puMainButtons.x = SetButtonValue(buttons_count, buttons, puButtonsUniforms["vec4(gamepad:mainbuttons).x"]);
	puMainButtons.y = SetButtonValue(buttons_count, buttons, puButtonsUniforms["vec4(gamepad:mainbuttons).y"]);
	puMainButtons.z = SetButtonValue(buttons_count, buttons, puButtonsUniforms["vec4(gamepad:mainbuttons).z"]);
	puMainButtons.w = SetButtonValue(buttons_count, buttons, puButtonsUniforms["vec4(gamepad:mainbuttons).w"]);
#endif
}

void GamePadSystem::UpdateButtons()
{
#ifdef USE_SDL2
	CTOOL_DEBUG_BREAK;
#else
	int buttons_count = 0;
	const unsigned char* buttons = glfwGetJoystickButtons(puCurrentJoyStick, &buttons_count);

	SetButtonValue(buttons_count, buttons, -1);
#endif
}

bool GamePadSystem::UpdateUniforms(UniformVariantPtr vUniPtr)
{
	const bool change = false;

	if (vUniPtr)
	{
		if (vUniPtr->widget == "gamepad:leftthumb")
		{
			if (vUniPtr->bx)
				vUniPtr->x += puLeftThumb.x;
			else
				vUniPtr->x = puLeftThumb.x;

			if (vUniPtr->by)
				vUniPtr->y += puLeftThumb.y;
			else
				vUniPtr->y = puLeftThumb.y;

			vUniPtr->z = puLeftThumb.z;
			vUniPtr->bz = vUniPtr->z > 0.5f;
		}
		else if (vUniPtr->widget == "gamepad:rightthumb")
		{
			if (vUniPtr->bx)
				vUniPtr->x += puRightThumb.x;
			else
				vUniPtr->x = puRightThumb.x;

			if (vUniPtr->by)
				vUniPtr->y += puRightThumb.y;
			else
				vUniPtr->y = puRightThumb.y;

			vUniPtr->z = puRightThumb.z;
			vUniPtr->bz = vUniPtr->z > 0.5f;
		}
		else  if (vUniPtr->widget == "gamepad:lefttrigger")
		{
			if (vUniPtr->bx)
				vUniPtr->x += puLeftTrigger;
			else
				vUniPtr->x = puLeftTrigger;
		}
		else if (vUniPtr->widget == "gamepad:righttrigger")
		{
			if (vUniPtr->bx)
				vUniPtr->x += puRightTrigger;
			else
				vUniPtr->x = puRightTrigger;
		}
		else if (vUniPtr->widget == "gamepad:cross")
		{
			vUniPtr->x = puCross.x;
			vUniPtr->bx = vUniPtr->x > 0.5f;
			vUniPtr->y = puCross.y;
			vUniPtr->by = vUniPtr->y > 0.5f;
			vUniPtr->z = puCross.z;
			vUniPtr->bz = vUniPtr->z > 0.5f;
			vUniPtr->w = puCross.w;
			vUniPtr->bw = vUniPtr->w > 0.5f;
		}
		else if (vUniPtr->widget == "gamepad:mainbuttons")
		{
			vUniPtr->x = puMainButtons.x;
			vUniPtr->bx = vUniPtr->x > 0.5f;
			vUniPtr->y = puMainButtons.y;
			vUniPtr->by = vUniPtr->y > 0.5f;
			vUniPtr->z = puMainButtons.z;
			vUniPtr->bz = vUniPtr->z > 0.5f;
			vUniPtr->w = puMainButtons.w;
			vUniPtr->bw = vUniPtr->w > 0.5f;
		}
	}

	return change;
}

///////////////////////////////////////////////////////
//// IMGUI ////////////////////////////////////////////
///////////////////////////////////////////////////////

// static, must only use params
bool GamePadSystem::DrawWidget(CodeTreePtr vCodeTree, UniformVariantPtr vUniPtr,
	const float& vMaxWidth, const float& vFirstColumnWidth, RenderPackWeak vRenderPack,
	const bool& vShowUnUsed, const bool& vShowCustom, const bool& vForNodes, bool* vChange)
{
	UNUSED(vShowUnUsed);
	UNUSED(vShowCustom);
	UNUSED(vForNodes);

	bool catched = false;

	if (vUniPtr && vCodeTree && vChange)
	{
		UniformVariantPtr v = vUniPtr;

		ShaderKeyPtr key = nullptr;
		auto rpPtr = vRenderPack.lock();
		if (rpPtr)
			key = rpPtr->GetShaderKey();

		if (vUniPtr->widget == "gamepad:leftthumb")
		{
			vCodeTree->DrawUniformName(key, v);
			ImGui::SameLine(vFirstColumnWidth);

			ImGui::BeginGroup();

			ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "A##accumLX", "Accum LX", &v->bx); 
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
			*vChange |= ImGui::InputFloatDefault(vMaxWidth - ImGui::GetCursorPosX(), "L Thumb X", &v->x, 0.0f);
			ImGui::PopStyleColor();

			ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "A##accumLY", "Accum LY", &v->by);
			ImGui::SameLine(); 
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
			*vChange |= ImGui::InputFloatDefault(vMaxWidth - ImGui::GetCursorPosX(), "L Thumb Y", &v->y, 0.0f);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
			if (ImGui::CheckBoxBoolDefault("L Thumb", &v->bz, false))
			{
				*vChange |= true;
				v->z = v->bz ? 1.0f : 0.0f;
			}
			ImGui::PopStyleColor();

			ImGui::EndGroup();

			catched = true;
		}
		else if (vUniPtr->widget == "gamepad:rightthumb")
		{
			vCodeTree->DrawUniformName(key, v);
			ImGui::SameLine(vFirstColumnWidth);

			ImGui::BeginGroup();

			ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "A##accumRX", "Accum RX", &v->bx); 
			ImGui::SameLine(); 
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
			*vChange |= ImGui::InputFloatDefault(vMaxWidth - ImGui::GetCursorPosX(), "R Thumb X", &v->x, 0.0f);
			ImGui::PopStyleColor();

			ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "A##accumRY", "Accum RY", &v->by);
			ImGui::SameLine(); 
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
			*vChange |= ImGui::InputFloatDefault(vMaxWidth - ImGui::GetCursorPosX(), "R Thumb Y", &v->y, 0.0f);
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
			if (ImGui::CheckBoxBoolDefault("R Thumb", &v->bz, false))
			{
				*vChange |= true;
				v->z = v->bz ? 1.0f : 0.0f;
			}
			ImGui::PopStyleColor();

			ImGui::EndGroup();

			catched = true;
		}
		else if (vUniPtr->widget == "gamepad:lefttrigger")
		{
			vCodeTree->DrawUniformName(key, v);
			ImGui::SameLine(vFirstColumnWidth);

			ImGui::BeginGroup();

			ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "A##accumLX", "Accum Left Trigger", &v->bx);
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
			*vChange |= ImGui::InputFloatDefault(vMaxWidth - ImGui::GetCursorPosX(), "L Trigg X", &v->x, 0.0f);
			ImGui::PopStyleColor();

			ImGui::EndGroup();

			catched = true;
		}
		else if (vUniPtr->widget == "gamepad:righttrigger")
		{
			vCodeTree->DrawUniformName(key, v);
			ImGui::SameLine(vFirstColumnWidth);

			ImGui::BeginGroup();

			ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), "A##accumLX", "Accum Right Trigger", &v->bx); 
			ImGui::SameLine(); 
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
			*vChange |= ImGui::InputFloatDefault(vMaxWidth - ImGui::GetCursorPosX(), "R Trigg Y", &v->x, 0.0f);
			ImGui::PopStyleColor();

			ImGui::EndGroup();

			catched = true;
		}
		else if (vUniPtr->widget == "gamepad:cross")
		{
			vCodeTree->DrawUniformName(key, v);
			ImGui::SameLine(vFirstColumnWidth);

			ImGui::BeginGroup();

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
			if (ImGui::CheckBoxBoolDefault("cross x", &v->bx, false))
			{
				*vChange |= true;
				v->x = v->bx ? 1.0f : 0.0f;
			}
			if (ImGui::CheckBoxBoolDefault("cross y", &v->by, false))
			{
				*vChange |= true;
				v->y = v->by ? 1.0f : 0.0f;
			}
			if (ImGui::CheckBoxBoolDefault("cross z", &v->bz, false))
			{
				*vChange |= true;
				v->z = v->bz ? 1.0f : 0.0f;
			}
			if (ImGui::CheckBoxBoolDefault("cross w", &v->bw, false))
			{
				*vChange |= true;
				v->w = v->bw ? 1.0f : 0.0f;
			}
			ImGui::PopStyleColor();

			ImGui::EndGroup();

			catched = true;
		}
		else if (vUniPtr->widget == "gamepad:mainbuttons")
		{
			vCodeTree->DrawUniformName(key, v);
			ImGui::SameLine(vFirstColumnWidth);

			ImGui::BeginGroup();

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
			if (ImGui::CheckBoxBoolDefault("main buttons x", &v->bx, false))
			{
				*vChange |= true;
				v->x = v->bx ? 1.0f : 0.0f;
			}
			if (ImGui::CheckBoxBoolDefault("main buttons y", &v->by, false))
			{
				*vChange |= true;
				v->y = v->by ? 1.0f : 0.0f;
			}
			if (ImGui::CheckBoxBoolDefault("main buttons z", &v->bz, false))
			{
				*vChange |= true;
				v->z = v->bz ? 1.0f : 0.0f;
			}
			if (ImGui::CheckBoxBoolDefault("main buttons w", &v->bw, false))
			{
				*vChange |= true;
				v->w = v->bw ? 1.0f : 0.0f;
			}
			ImGui::PopStyleColor();

			ImGui::EndGroup();

			catched = true;
		}
	}

	return catched;
}

bool GamePadSystem::UpdateIfNeeded(ShaderKeyPtr  /*vKey*/, ct::ivec2 /*vScreenSize*/)
{
	return true;
}

bool GamePadSystem::WasChanged()
{
	return puWasChanged;
}

void GamePadSystem::ResetChange()
{
	puWasChanged = false;
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string GamePadSystem::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	str += vOffset + "<GamePadSystem>\n";
	const std::string offset = vOffset + vOffset;

	str += offset + "<active>" + ct::toStr(puActivated ? "true" : "false") + "</active>\n";

	str += vOffset + "</GamePadSystem>\n";

	return str;
}

bool GamePadSystem::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	// The puValue of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strName == "GamePadSystem")
	{
		for (tinyxml2::XMLElement* child = vElem->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
		{
			RecursParsingConfig(child->ToElement(), vElem);
		}
	}

	if (strParentName == "GamePadSystem")
	{
		if (strName == "active")
			puActivated = ct::ivariant(strValue).GetB();
	}

	return false;
}

///////////////////////////////////////////////////////
//// SERIALIZATION / STATIC ///////////////////////////
///////////////////////////////////////////////////////

bool GamePadSystem::SerializeUniform(UniformVariantPtr vUniform, std::string *vStr)
{
	bool catched = false;

	if (vStr)
	{
		if (vUniform->widget == "gamepad:leftthumb" || vUniform->widget == "gamepad:rightthumb")
		{
			*vStr = vUniform->name + ":";

			*vStr += ":" + ct::toStr(vUniform->x);
			*vStr += ":" + ct::toStr(vUniform->bx ? "true" : "false");
			*vStr += ":" + ct::toStr(vUniform->y);
			*vStr += ":" + ct::toStr(vUniform->by ? "true" : "false");

			*vStr += '\n';

			catched = true;
		}
		else if (vUniform->widget == "gamepad:lefttrigger" || vUniform->widget == "gamepad:righttrigger")
		{
			*vStr = vUniform->name + ":";

			*vStr += ":" + ct::toStr(vUniform->x);
			*vStr += ":" + ct::toStr(vUniform->bx ? "true" : "false");

			*vStr += '\n';

			catched = true;
		}
	}

	return catched;
}

bool GamePadSystem::DeSerializeUniform(ShaderKeyPtr  /*vShaderKey*/, UniformVariantPtr vUniform, const std::vector<std::string>& vParams)
{
	bool catched = false;

	if (vUniform->widget == "gamepad:leftthumb" || vUniform->widget == "gamepad:rightthumb")
	{
		if (vParams.size() == 5)
		{
			vUniform->x = ct::ivariant(vParams[1]).GetF();
			vUniform->bx = ct::ivariant(vParams[2]).GetB();
			vUniform->y = ct::ivariant(vParams[3]).GetF();
			vUniform->by = ct::ivariant(vParams[4]).GetB();
		}

		catched = true;
	}
	else if (vUniform->widget == "gamepad:lefttrigger" || vUniform->widget == "gamepad:righttrigger")
	{
		if (vParams.size() == 3)
		{
			vUniform->x = ct::ivariant(vParams[1]).GetF();
			vUniform->bx = ct::ivariant(vParams[2]).GetB();
		}

		catched = true;
	}

	return catched;
}

void GamePadSystem::Complete_Uniform(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform)
{
    if (!vRenderPack.expired())
		return;

	vUniform->widget = vUniformParsed.params;

	bool catched = false;

	if (vUniform->widget == "gamepad:leftthumb" || vUniform->widget == "gamepad:rightthumb")
	{
		if (vUniform->glslType == uType::uTypeEnum::U_VEC3)
		{
			vUniform->bx = false;
			vUniform->by = false;
			vUniform->bz = false;
			vUniform->x = 0.0f;
			vUniform->y = 0.0f;
			vUniform->z = 0.0f;
		}
		else
		{
			vParentKey->GetSyntaxErrors()->SetSyntaxError(vParentKey, "Parsing Error :", "Bad Uniform Type", false,
				LineFileErrors(vUniformParsed.sourceCodeLine, "", "uniform widget " + vUniform->widget + " can only be of type vec3"));
		}

		catched = true;
	}
	else if (vUniform->widget == "gamepad:lefttrigger" || vUniform->widget == "gamepad:righttrigger")
	{
		if (vUniform->glslType == uType::uTypeEnum::U_FLOAT)
		{
			vUniform->bx = false;
			vUniform->x = 0.0f;
		}
		else
		{
			vParentKey->GetSyntaxErrors()->SetSyntaxError(vParentKey, "Parsing Error :", "Bad Uniform Type", false,
				LineFileErrors(vUniformParsed.sourceCodeLine, "", "uniform widget " + vUniform->widget + " can only be of type float"));
		}

		catched = true;
	}
	else if (vUniform->widget == "gamepad:cross" || vUniform->widget == "gamepad:mainbuttons")
	{
		if (vUniform->glslType == uType::uTypeEnum::U_VEC4)
		{
			vUniform->bx = false;
			vUniform->by = false;
			vUniform->bz = false;
			vUniform->bw = false;
			vUniform->x = 0.0f;
			vUniform->y = 0.0f;
			vUniform->z = 0.0f;
			vUniform->w = 0.0f;
		}
		else
		{
			vParentKey->GetSyntaxErrors()->SetSyntaxError(vParentKey, "Parsing Error :", "Bad Uniform Type", false,
				LineFileErrors(vUniformParsed.sourceCodeLine, "", "uniform widget " + vUniform->widget + " can only be of type vec4"));
		}

		catched = true;
	}

	if (!catched)
	{
		vParentKey->GetSyntaxErrors()->SetSyntaxError(vParentKey, "Parsing Error :", "Bad Uniform Type", false, 
			LineFileErrors(vUniformParsed.sourceCodeLine, "", "uniform of type gamepad can only be :\n - vec3 for left and right thumb (-1 to 1)\n - float for left and right trigger (0 to 1)\n - vec4 for cross and main buttons (true 1.0 or false 0.0)"));
	}
}

///////////////////////////////////////////////////////
//// IMGUI ////////////////////////////////////////////
///////////////////////////////////////////////////////

bool GamePadSystem::DrawActivationButton()
{
	bool change = false;

	if (ImGui::RadioButtonLabeled(ImVec2(0.0f, 0.0f), ICON_NDP_GAMEPAD " GamePad", "Can Catch GamePad", &puActivated))
	{
        change = true;
    }

	return change;
}

void GamePadSystem::ListGamePads()
{
	puJoysticks.clear();

#ifdef USE_SDL2

#else
	for (int i = 0; i < GLFW_JOYSTICK_LAST; i++)
	{
		if (glfwJoystickPresent(i) == GLFW_TRUE)
		{
			puJoysticks[i] = glfwGetJoystickName(i);
		}
	}
#endif
}

bool GamePadSystem::DrawMenu()
{
	bool change = false;

	if (puActivated)
	{
		if (ImGui::BeginMenu(ICON_NDP_GAMEPAD " GamePad"))
		{
			if (ImGui::BeginMenu("Inputs"))
			{
				for (auto it = puConfigs.begin(); it != puConfigs.end(); ++it)
				{
					bool b = (puCurrentConfig == it->first);
					if (ImGui::Checkbox(it->first.c_str(), &b))
					{
						puCurrentConfig = it->first;
						//puCurrentJoyStick = it->second;
					}
				}

				ImGui::EndMenu();
				change |= true;
			}
					   
			ImGui::EndMenu();
		}
	}

	return change;
}

///////////////////////////////////////////////////////
//// SETTINGS /////////////////////////////////////////
///////////////////////////////////////////////////////

void GamePadSystem::LoadSettings()
{

}

void GamePadSystem::SaveSettings()
{

}

static std::string _currentSelectable = "";
static std::string _currentType = "";
static int _currentChannel = 0;

void GamePadSystem::ShowContent()
{
	//char headerName[150] = "";
	char itemName[150] = "";
	//char listName[150] = "";
	int axes_count = 0;
	int buttons_count = 0;
	//int hats_count = 0;

#ifdef USE_SDL2

#else
	for (int i = 0; i < GLFW_JOYSTICK_LAST; i++)
	{
		if (glfwJoystickPresent(i) == GLFW_TRUE)
		{
			const char* name = glfwGetJoystickName(i);
			if (name)
			{
				bool isChecked = (puCurrentConfig == std::string(name));
				if (ImGui::CollapsingHeader_CheckBox(name, -1.0f, false, true, &isChecked))
				{
					ImGui::BeginChild("Analogics Uniforms", ImVec2(200, 0));

					ImGui::Text("Analogic Uniforms :");

					for (auto it = puAnalogicsUniforms.begin(); it != puAnalogicsUniforms.end(); ++it)
					{
						if (ImGui::Selectable(it->first.c_str(), _currentSelectable == it->first))
						{
							_currentSelectable = it->first;
							_currentType = "axis";
							_currentChannel = it->second;
						}
					}

					ImGui::Text("Buttons Uniforms :");

					for (auto it = puButtonsUniforms.begin(); it != puButtonsUniforms.end(); ++it)
					{
						if (ImGui::Selectable(it->first.c_str(), _currentSelectable == it->first))
						{
							_currentSelectable = it->first;
							_currentType = "button";
							_currentChannel = it->second;
						}
					}

					ImGui::EndChild();

					ImGui::SameLine();

					ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

					ImGui::SameLine();

					if (glfwJoystickIsGamepad(i))
					{
						ImGui::BeginChild("JoyStick", ImVec2(-1, -1));

						if (_currentType == "axis")
						{
							const float* axes = glfwGetJoystickAxes(i, &axes_count);

							ImGui::Text("Select the Axis you want link with the selected uniform channel");

							for (int i2 = 0; i2 < axes_count; i2++)
							{
								snprintf(itemName, 149, "Axis %i => %.5f", i2, axes[i2]);
								if (ImGui::Selectable(itemName, _currentChannel == i2))
								{
									if (!_currentSelectable.empty())
									{
										puAnalogicsUniforms[_currentSelectable] = i2;
										_currentChannel = i2;
									}
								}
							}
						}

						if (_currentType == "button")
						{
							const unsigned char* buttons = glfwGetJoystickButtons(i, &buttons_count);

							ImGui::Text("Select the Button you want link with the selected uniform channel");

							for (int i2 = 0; i2 < buttons_count; i2++)
							{
								snprintf(itemName, 149, "Button %i => %s", i2, buttons[i2] ? "true" : "false");
								if (ImGui::Selectable(itemName, _currentChannel == i2))
								{
									if (!_currentSelectable.empty())
									{
										puButtonsUniforms[_currentSelectable] = i2;
										_currentChannel = i2;
									}
								}
							}
						}

						/*if (_currentType == "hat")
						{
							const unsigned char* hats = glfwGetJoystickHats(i, &hats_count);

							ImGui::Text("Select the Hat you want link with the selected uniform channel");

							for (int i = 0; i < hats_count; i++)
							{
								snprintf(itemName, 149, "Hat %i => %s", i, hats[i] ? "true" : "false");
								if (ImGui::Selectable(itemName, _currentChannel == i))
								{
									if (!currentSelectable.empty())
									{
										puButtonsUniforms[_currentSelectable] = i;
										_currentChannel = i;
									}
								}
							}
						}*/

						ImGui::EndChild();
					}
					else
					{
						ImGui::Text("Not a JoyStick or Gamepad");
					}
				}

				if (isChecked)
				{
					puCurrentConfig = std::string(name);
					puCurrentJoyStick = i;
				}
			}
		}
	}
#endif
}