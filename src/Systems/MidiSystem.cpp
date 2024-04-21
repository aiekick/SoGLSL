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

#include <Systems/MidiSystem.h>
#include <CodeTree/ShaderKey.h>
#include <Gui/CustomGuiWidgets.h>
#include <CodeTree/CodeTree.h>
#include <Renderer/RenderPack.h>
#include <ctools/Logger.h>
#include <ImGuiPack.h>

#include <Res/CustomFont.h>
#include <Gui/GuiBackend.h>

#ifdef USE_SDL2

#else
#include <GLFW/glfw3.h>
#endif

///////////////////////////////////////////////////////
//// CALLBACK /////////////////////////////////////////
///////////////////////////////////////////////////////

void mycallback(double deltatime, std::vector<uint8_t>* message, void* userData)
{
	MidiSystem::Instance()->AddMessage(deltatime, message, userData);
}

static void Markdown(const std::string& markdown_)
{
	ImFont* font = ImGui::GetFont();

	// You can make your own Markdown function with your prefered string container and markdown config.
	// > C++14 can use ImGui::MarkdownConfig mdConfig{ LinkCallback, NULL, ImageCallback, ICON_FA_LINK, { { H1, true }, { H2, true }, { H3, false } }, NULL };
	ImGui::MarkdownConfig mdConfig;
	mdConfig.linkCallback = NULL;
	mdConfig.tooltipCallback = NULL;
	mdConfig.imageCallback = NULL;
	mdConfig.linkIcon = "[link]";
	mdConfig.headingFormats[0] = { font, true };
	mdConfig.headingFormats[1] = { font, true };
	mdConfig.headingFormats[2] = { font, false };
	mdConfig.userData = NULL;
	ImGui::Markdown(markdown_.c_str(), markdown_.length(), mdConfig);
}

///////////////////////////////////////////////////////
//// CTOR/DTOR ////////////////////////////////////////
///////////////////////////////////////////////////////

MidiSystem::MidiSystem()
{
	
}

MidiSystem::~MidiSystem()
{
	
}

///////////////////////////////////////////////////////
//// INIT/UNIT ////////////////////////////////////////
///////////////////////////////////////////////////////

bool MidiSystem::Init(CodeTreePtr vCodeTree)
{
	m_RtMidis.push_back(std::make_shared<RtMidiIn>());
	m_MidiDevices.push_back(MidiStruct());
	
	return true;
}

void MidiSystem::Unit()
{
	m_RtMidis.clear();
}

// update va gerer les cas ou on connect/deconnect un device midi
// va initialiser ou deinitialiser les instance
// ouvrir / fermer les port
// definir / supprimer le binding sur le callback
void MidiSystem::Update()
{
	static uint32_t maxOpenedPort = 0;

	uint32_t countPorts = m_RtMidis[0]->getPortCount();
	uint32_t countInstance = (uint32_t)m_RtMidis.size();

	try
	{
		if (countPorts > 0)
		{
			if (maxOpenedPort == 0)
			{
				maxOpenedPort = countPorts;
				m_RtMidis[0]->openPort(0);
				m_RtMidis[0]->setCallback(&mycallback, (void*)0);
				m_RtMidis[0]->ignoreTypes(false, false, false);
				std::string deviceName = m_RtMidis[0]->getPortName(0);
				ct::replaceString(deviceName, " ", "");
				m_MidiDevices[0].deviceName = deviceName;

			}
		}
		else if (maxOpenedPort)
		{
			m_RtMidis[0]->closePort();
			m_RtMidis[0]->cancelCallback();
			maxOpenedPort = 0;
		}

		if (countPorts != countInstance)
		{
			// il doit toujours rester une instance au mini
			if (countPorts < countInstance && countInstance > 1) // supression d'instance
			{
				auto ptr = m_RtMidis.back();
				if (ptr)
				{
					ptr->closePort();
					ptr->cancelCallback();
				}

				m_RtMidis.erase(m_RtMidis.end() - 1);
				m_MidiDevices.erase(m_MidiDevices.end() - 1);
			}
			else if (countPorts > countInstance) // ajout d'instance
			{
				m_RtMidis.push_back(std::make_shared<RtMidiIn>(RtMidi::Api::UNSPECIFIED, "RtMidiIn", 2));
				m_MidiDevices.push_back(MidiStruct());
				std::string deviceName = (*(m_RtMidis.end() - 1))->getPortName(countInstance);
				ct::replaceString(deviceName, " ", "");
				m_MidiDevices.back().deviceName = deviceName;
				auto ptr = m_RtMidis.back();
				if (ptr)
				{
					ptr->openPort(countInstance);
					ptr->setCallback(&mycallback, (void*)(size_t)countInstance);
					ptr->ignoreTypes(false, false, false);
				}
			}
		}
	}
	catch (std::exception ex)
	{
		LogVarError("Maybe Another app is using the Midi : %s", ex.what());
	}
}

///////////////////////////////////////////////////////
//// WIDGET CONFIGURATION /////////////////////////////
///////////////////////////////////////////////////////

void MidiSystem::AddMessage(double vDeltatime, std::vector<uint8_t> *vMessagePtr, void *vUserDataPtr)
{
	UNUSED(vDeltatime);

	if (vMessagePtr && vUserDataPtr)
	{
		const size_t& port = (size_t)vUserDataPtr;
		if (port < m_MidiDevices.size())
		{
			if (!vMessagePtr->empty())
			{
				// vUserData va contenir ici le numero du port

				m_MidiDevices[port].lastMessage = m_MidiDevices[port].currentMessage;
				m_MidiDevices[port].currentMessage.name = m_MidiDevices[port].deviceName,
				m_MidiDevices[port].currentMessage.bytes = *vMessagePtr;
				updateMidiNeeded = true; // upload in gpu
			}
		}
	}
}

bool MidiSystem::UpdateUniforms(UniformVariantPtr vUniPtr)
{
	const bool change = false;

	if (vUniPtr && vUniPtr->widget == "midi" && updateMidiNeeded)
	{
		bool found = false;
		ct::fvec4 val = 0.0f;

		const uint32_t& countDevices = GetCountDevices();
		for (uint32_t i = 0; i < countDevices; i++)
		{
			auto msg = GetMidiMessage(i);
			if (msg.name == vUniPtr->midiDeviceName) // les deux devices doivent avoir le meme nom
			{
				uint32_t count = (uint32_t)vUniPtr->midiId.size();
				if (msg.bytes.size() == count) // les deux messages doivent avoir le meme nombre de bytes
				{
					val = 0.0f;
					uint32_t vIdx = 0;
					for (uint32_t j = 0; j < count; j++)
					{
						if ((int)msg.bytes[j] != vUniPtr->midiId[j]) // si les bytes ne correspondet pas, ca peut etre une valeur
						{
							if (vUniPtr->midiId[j] < 0) // si c'est un valeur
							{
								if (vIdx < (uint32_t)vUniPtr->midiBytes.size() && vIdx < 1)
								{
									if (vUniPtr->midiBytes[vIdx] == j) // on prendre le byter 0 de la valeur
									{
										float* ptr = &val.x + vIdx;
										*ptr = msg.bytes[j];
										vIdx++;
										found = true;
									}
								}
							}
							else break;
						}
					}
				}
			}
		}

		if (found)
		{
			vUniPtr->x = ct::mix(vUniPtr->inf.x, vUniPtr->sup.x, ct::clamp(val.x / 127.0f, 0.0f, 1.0f));
			if (vUniPtr->step.x)
			{
				vUniPtr->x = (float)(ct::floor(vUniPtr->x / vUniPtr->step.x) * vUniPtr->step.x);
			}
		}
	}

	return change;
}

///////////////////////////////////////////////////////
//// IMGUI ////////////////////////////////////////////
///////////////////////////////////////////////////////

// static, must only use params
bool MidiSystem::DrawWidget(
	CodeTreePtr vCodeTree, 
	UniformVariantPtr vUniPtr,
	const float& vMaxWidth, 
	const float& vFirstColumnWidth, 
	RenderPackWeak vRenderPack,
    const bool& /*vShowUnUsed*/, 
	const bool& /*vShowCustom*/, 
	const bool& /*vForNodes*/, 
	bool* vChange)
{
	bool catched = false;

	if (vUniPtr && vCodeTree && vChange)
	{
		if (vUniPtr->widget == "midi")
		{
			UniformVariantPtr v = vUniPtr;

			ShaderKeyPtr key = nullptr;
			auto rpPtr = vRenderPack.lock();
			if (rpPtr)
				key = rpPtr->GetShaderKey();

			vCodeTree->DrawUniformName(key, v, 0);
			ImGui::SameLine(vFirstColumnWidth);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetUniformLocColor(v->loc));
			if (ImGui::SliderFloatDefault(vMaxWidth - ImGui::GetCursorPosX(), ("##x" + v->name).c_str(), &v->x, v->inf.x, v->sup.x, v->def.x, 0.0f, "%.5f"))
			{
				*vChange = true;
				vCodeTree->RecordToTimeLine(key, v, 0);
			}
			ImGui::PopStyleColor();

			catched = true;
		}
	}

	return catched;
}

bool MidiSystem::UpdateIfNeeded(ShaderKeyPtr /*vKey*/, ct::ivec2 /*vScreenSize*/)
{
	return true;
}

bool MidiSystem::WasChanged()
{
	return m_WasChanged;
}

void MidiSystem::ResetChange()
{
	m_WasChanged = false;
}

///////////////////////////////////////////////////////
//// SERIALIZATION / STATIC ///////////////////////////
///////////////////////////////////////////////////////

bool MidiSystem::SerializeUniform(UniformVariantPtr /*vUniform*/, std::string* vStr) {
	bool catched = false;

	if (vStr)
	{
		/*
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
		*/
	}

	return catched;
}

bool MidiSystem::DeSerializeUniform(ShaderKeyPtr /*vShaderKey*/, UniformVariantPtr /*vUniform*/, const std::vector<std::string>& /*vParams*/) {
	bool catched = false;

	/*
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
	*/

	return catched;
}

void MidiSystem::Complete_Uniform(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform)
{
    if (!vRenderPack.expired())
		return;

	bool catched = false;

	vUniform->widget = "midi";
	vUniform->canWeSave = true;
	vUniform->timeLineSupported = true;

	//size_t idx_numbers = 0U;

	vUniform->inf = 0.0f;
	vUniform->sup = 0.0f;
	vUniform->def = 0.0f;
	vUniform->step = 0.0f;

	if (!vUniformParsed.paramsArray.empty())
	{
		if (vUniformParsed.paramsArray.size() > 1U)
		{
			if (vUniformParsed.paramsArray[1].find("id") != std::string::npos)
			{
				auto arr = vUniformParsed.paramsDico.at("id");
				int idx = 0;
				for (auto s : arr)
				{
					if (idx++ == 0)
						vUniform->midiDeviceName = s;
					else
						vUniform->midiId.push_back(ct::ivariant(s).GetI());
				}
			}
		}
		if (vUniformParsed.paramsArray.size() > 2U)
		{
			if (vUniformParsed.paramsArray[2].find("value") != std::string::npos)
			{
				auto arr = vUniformParsed.paramsDico.at("value");
				for (auto s : arr)
				{
					vUniform->midiBytes.push_back((uint8_t)ct::uvariant(s).GetU());
				}
			}
		}
		if (vUniformParsed.paramsArray.size() > 3U)
		{
			vUniform->inf = ct::fvariant(vUniformParsed.paramsArray[3]).GetV4();
		}
		if (vUniformParsed.paramsArray.size() > 4U)
		{
			vUniform->sup = ct::fvariant(vUniformParsed.paramsArray[4]).GetV4();
		}
		if (vUniformParsed.paramsArray.size() > 5U)
		{
			vUniform->def = ct::fvariant(vUniformParsed.paramsArray[5]).GetV4();
		}
		if (vUniformParsed.paramsArray.size() > 6U)
		{
			vUniform->step = ct::fvariant(vUniformParsed.paramsArray[6]).GetV4();
		}
	}

	vUniform->x = vUniform->def.x;
	vUniform->y = vUniform->def.y;
	vUniform->z = vUniform->def.z;
	vUniform->w = vUniform->def.w;

	if (vUniform->glslType == uType::uTypeEnum::U_FLOAT)
	{
		catched = true;
	}

	if (!catched)
	{
		vParentKey->GetSyntaxErrors()->SetSyntaxError(vParentKey, "Parsing Error :", "Bad Uniform Type", false,
			LineFileErrors(vUniformParsed.sourceCodeLine, "", "uniform of type midi can only be a float for the moment"));
	}
}

///////////////////////////////////////////////////////
//// SETTINGS /////////////////////////////////////////
///////////////////////////////////////////////////////

void MidiSystem::LoadSettings()
{

}

void MidiSystem::SaveSettings()
{

}

void MidiSystem::ShowContent(GuiBackend_Window vWin)
{
	DisplayConfigPane(vWin);
}

///////////////////////////////////////////////////////
//// PRIVATR //////////////////////////////////////////
///////////////////////////////////////////////////////

void MidiSystem::DisplayConfigPane(GuiBackend_Window vWin)
{
	uint32_t countPorts = m_RtMidis[0]->getPortCount();
	ImGui::Text("Detected midi ports count : %u", countPorts);

	//uint32_t countInstance = (uint32_t)m_RtMidis.size();

	std::string portName;
	for (uint32_t i = 0; i < countPorts; i++)
	{
		if (i >= m_RtMidis.size()) break;
		
		portName = m_RtMidis[0]->getPortName(i);
		ImGui::Text("port : %u - controller : %s", i, portName.c_str());

		if (i > 0)
			ImGui::Separator();
		
		if (m_RtMidis[i]->isPortOpen())
		{
			std::string id = m_MidiDevices[i].deviceName;
			std::string value;

			if (!m_MidiDevices[i].currentMessage.bytes.empty())
			{
				ImGui::Text("Message : ");

				ImGui::SameLine();

				for (size_t idx = 0; idx < m_MidiDevices[i].currentMessage.bytes.size(); idx++)
				{
					bool sameWord = false;
					if (!m_MidiDevices[i].lastMessage.bytes.empty() && idx < m_MidiDevices[i].lastMessage.bytes.size())
						sameWord = (m_MidiDevices[i].currentMessage.bytes[idx] == m_MidiDevices[i].lastMessage.bytes[idx]);

					if (idx)
					{
						ImGui::SameLine();
					}

					if (!id.empty())
					{
						id += ',';
					}

					if (sameWord)
					{
						id += ct::toStr((int)m_MidiDevices[i].currentMessage.bytes[idx]);
						ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%hhu", m_MidiDevices[i].currentMessage.bytes[idx]);
					}
					else
					{
						id += "-1";
						if (!value.empty())
						{
							value += ",";
						}

						value += ct::toStr(idx);
						int v = (int)m_MidiDevices[i].currentMessage.bytes[idx];
						range.x = ct::mini(range.x, v);
						range.y = ct::maxi(range.y, v);
						ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%hhu", m_MidiDevices[i].currentMessage.bytes[idx]);
					}
				}

				if (id != lastId)
				{
					range = ct::ivec2((int)1e7, (int)-1e7);
				}

				lastId = id;

				ImGui::Separator();

				const std::string markdownText = u8R"(# Auto Detection of Uniform Syntax :
Will use 'not changed bytes' as id, and 'changed bytes' as value
So you need to trigger many times the widgets for have good syntax

You can add (after value param) numbers for inf, sup, def and step like slider float widget

[uniform](#00FF0B) [float](#00C9FF)([midi](#E2FF00):id=)" + id + u8R"(:value=)" + value + u8R"(:0.0:1.0:0.5:0.0) inputName)";
				Markdown(markdownText);

				if (ImGui::ContrastedButton("Copy midi id+value to clipboard"))
				{
					GuiBackend::Instance()->SetClipboardString(vWin,
						ct::toStr("midi:id=%s:value=%s", id.c_str(), value.c_str()));
				}

				ImGui::SameLine();

				if (ImGui::ContrastedButton("Copy uniform to clipboard"))
				{
					GuiBackend::Instance()->SetClipboardString(vWin,
						ct::toStr("uniform float(midi:id=%s:value=%s:0.0:1.0:0.5:0.0) midiInput;", id.c_str(), value.c_str()));
				}
			}
		}
	}
}

///////////////////////////////////////////////////////
//// OVERRIDE /////////////////////////////////////////
///////////////////////////////////////////////////////

std::vector<MidiMessage> MidiSystem::GetMidiMessages()
{
	std::vector<MidiMessage> msgs;

	for (const auto & m : m_MidiDevices)
	{
		msgs.push_back(m.currentMessage);
	}

	return msgs;
}

MidiMessage MidiSystem::GetMidiMessage(const uint32_t& vPort)
{
	if ((size_t)vPort < m_MidiDevices.size())
	{
		return m_MidiDevices[vPort].currentMessage;
	}

	return MidiMessage();
}

MidiMessage MidiSystem::GetAndClearMidiMessage(const uint32_t& vPort)
{
	if ((size_t)vPort < m_MidiDevices.size())
	{
		auto msg = m_MidiDevices[vPort].currentMessage;
		m_MidiDevices[vPort].currentMessage = MidiMessage();
		return msg;
	}

	return MidiMessage();
}

MidiMessage MidiSystem::ClearMidiMessage(const uint32_t& vPort)
{
	if ((size_t)vPort < m_MidiDevices.size())
	{
		m_MidiDevices[vPort].currentMessage = MidiMessage();
	}

	return MidiMessage();
}

MidiStruct MidiSystem::GetMidiMessageDB(const uint32_t& vPort)
{
	if ((size_t)vPort < m_MidiDevices.size())
	{
		return m_MidiDevices[vPort];
	}

	return MidiStruct();
}

std::string MidiSystem::GetMidiDeviceName(const uint32_t& vPort)
{
	if ((size_t)vPort < m_MidiDevices.size())
	{
		return m_MidiDevices[vPort].deviceName;
	}

	return "";
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string MidiSystem::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/) {
	std::string str;

	str += vOffset + "<MidiSystem>\n";

	std::string offset = vOffset + "\t";

	//str += offset + "<active>" + ct::toStr(m_Activated ? "true" : "false") + "</active>\n";

	str += vOffset + "</MidiSystem>\n";

	return str;
}

bool MidiSystem::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	// The m_Value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != 0)
		strParentName = vParent->Value();

	if (strName == "MidiSystem")
	{
		for (tinyxml2::XMLElement* child = vElem->FirstChildElement(); child != 0; child = child->NextSiblingElement())
		{
			RecursParsingConfig(child->ToElement(), vElem, vUserDatas);
		}
	}

	if (strParentName == "MidiSystem")
	{
		//if (strName == "active")
		//	m_Activated = ct::ivariant(strValue).GetB();
	}

	return true;
}
