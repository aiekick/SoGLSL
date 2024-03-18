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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "SoundSystem.h"

#include <CodeTree/CodeTree.h>
#include <imgui.h>
#include <Gui/CustomGuiWidgets.h>
#include <Systems/CameraSystem.h>
#include <CodeTree/Parsing/SectionCode.h>
#include <CodeTree/ShaderKey.h>
#include <ctools/Logger.h>
#include <ctools/cTools.h>
#include <Res/CustomFont2.h>
#include <Texture/Texture2D.h>
#include <Profiler/TracyProfiler.h>
#include <ImGuiPack.h>
#include "RenderDocController.h"

bool SoundSystem::Init(CodeTreePtr vCodeTree)
{
	puActivated = false;
	puNeedOneUniformUpdate = false;

	InitAudioDriver();

	return true;
}

void SoundSystem::Unit()
{
	UnitAudioDriver();
}

bool SoundSystem::Use()
{
	return puActivated || puNeedOneUniformUpdate;
}

bool SoundSystem::IsActivated()
{
	return puActivated;
}

void SoundSystem::SetActivation(bool vActivation)
{
	puActivated = vActivation;

	if (!puActivated)
	{
		// on doit propoager une derniere fois avant de desactiver les gizmo, pour desactiver le useCulling
		puNeedOneUniformUpdate = true;
	}
}

bool SoundSystem::DrawMenu()
{
	bool change = false;

	if (puActivated)
	{

	}

	return change;
}

bool SoundSystem::DrawTooltips(RenderPackWeak /*vRenderPack*/, float /*vDisplayQuality*/, CameraSystem* /*vCamera*/, ct::ivec2 /*vScreenSize*/) {
	bool change = false;

	if ((puActivated || puNeedOneUniformUpdate))
	{
		puNeedOneUniformUpdate = false;
	}

	return change;
}

static inline ImPlotPoint fft_points(int idx, void* /*user_data*/) {
	if (ImPlot::IsPlotHovered())
	{
		auto pi = (int)ImPlot::PixelsToPlot(ImGui::GetMousePos()).x;
		if (pi == idx)
		{
			static float af = 1.0f / (float)SoundSystem::scFftSize;
			auto uvx = ct::clamp(idx * af, 0.0f, 1.0f);
			auto hz = 44100.0f * uvx;
			ImGui::SetTooltip("Hz : %.4f\nuv.x : %.4f", hz, uvx);
		}
	}
	return ImPlotPoint((double)idx, 0.0);
}

bool SoundSystem::DrawPane(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack, float vDisplayQuality, CameraSystem* /*vCamera*/) {
	UNUSED(vDisplayQuality);

	bool change = false;

	if (!vRenderPack.expired())
	{
		if (Use())
		{
			if (ImGui::CollapsingHeader("Sound System"))
			{
				ImGui::Indent();

				if (ImGui::ContrastedButton("Refresh"))
				{
					EnumerateAudioDevices();
				}

				if (!m_AudioDevices.empty())
				{
					if (m_CurrentAudioDeviceIndex >= m_AudioDevices.size())
					{
						SelectAudioDevice(m_CurrentAudioDeviceIndex);
					}

					ImGui::Text("Audio devices :");
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::BeginContrastedCombo("##Audio devices", m_AudioDevices[m_CurrentAudioDeviceIndex].name, ImGuiComboFlags_None))
					{
						size_t idx = 0U;
						for (const auto& device : m_AudioDevices)
						{
							if (ImGui::Selectable(device.name, m_CurrentAudioDeviceIndex == idx))
							{
								SelectAudioDevice(idx);
							}

							++idx;
						}

						ImGui::EndCombo();
					}
					ImGui::PopItemWidth();

					ImGui::Separator();

					const auto& current_device = m_AudioDevices[m_CurrentAudioDeviceIndex];

					//ImGui::Text("%s", current_device.name);
					//ImGui::Text("%u", current_device.id);

					ImGui::Text("Formats :");
					ImGui::Indent();
					for (size_t i = 0U; i < current_device.nativeDataFormatCount; ++i)
					{
						if (i)
						{
							ImGui::Text("");
						}

						const auto& data_format = current_device.nativeDataFormats[i];
						ImGui::Text("Channels : %u", data_format.channels);
						ImGui::Text("Sample rate : %u", data_format.sampleRate);

						switch (data_format.format)
						{
						case ma_format_unknown:
							ImGui::Text("Type : Unknow");
							break;
						case ma_format_u8:
							ImGui::Text("Type : U8");
							break;
						case ma_format_s16:
							ImGui::Text("Type : S16");
							break;
						case ma_format_s24:
							ImGui::Text("Type : S24");
							break;
						case ma_format_s32:
							ImGui::Text("Type : S32");
							break;
						case ma_format_f32:
							ImGui::Text("Type : F32");
							break;
						case ma_format_count:
						default:
							break;
						}

					}
					ImGui::Unindent();

					ImGui::Separator();

					if (current_device.name[1] == 'i') // [in] => mic
					{

					}
					else if (current_device.name[1] == 'o') // [out] => playback
					{

					}

					ImGui::Header("FFT Sound"); 
					
					const auto aw = ImGui::GetContentRegionAvail().x;
					
					if (ImGui::SliderSizeTDefaultCompact(aw, "Count Points", &m_CountDisplayPoints, 1U, scFftSize, 150U))
					{
						m_CountDisplayPoints = ct::clamp<size_t>(m_CountDisplayPoints, 1U, scFftSize);
					}

					if (ImPlot::BeginPlot("FFT", ImVec2(aw, 150.0f), ImPlotFlags_NoChild | ImPlotFlags_CanvasOnly | ImPlotFlags_NoFrame))
					{
						ImPlot::SetupAxes(nullptr, nullptr, 
							ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_NoMenus, 
							ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_NoMenus);
						ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 1.0f);
						
						ImPlot::PlotBars("##FFTPoints", m_AverageFFTValues, (int)m_CountDisplayPoints, 0.67f);
						ImPlot::PlotBarsG("##FFTPoints2", fft_points, m_AverageFFTValues, (int)m_CountDisplayPoints, 0.67f);

						ImPlot::EndPlot();
					}

					if (ImGui::SliderFloatDefaultCompact(aw, "Audio Amplification", &m_AudioFloatAmplification, 0.0f, 500.0f, 100.0f))
					{
						m_AudioFloatAmplification = ct::maxi(m_AudioFloatAmplification, 1.0f);
					}

					if (ImGui::SliderSizeTDefaultCompact(aw, "Mean Average size", &m_CurrentAverageSize, 1U, scMaxAverageSize, 3U))
					{
						m_CurrentAverageSize = ct::clamp<size_t>(m_CurrentAverageSize, 1U, scMaxAverageSize);

						//reset
						m_AverageLastValueIndex = 0U;
						memset(m_AverageFFTDatas, 0, sizeof(float) * SoundSystem::scFftSize * SoundSystem::scMaxAverageSize);
						memset(m_AverageFFTValues, 0, sizeof(float) * SoundSystem::scFftSize);
					}

					if (m_SoundHisto_RenderPack_Ptr)
					{
						ImGui::Header("Texture Sound Histo");
						ImGui::Texture(m_SoundHisto_RenderPack_Ptr->GetTexture(), ImGui::GetContentRegionAvail().x, ImVec4(1, 1, 1, 1), 10);
					}
				}

				ImGui::Unindent();
			}
		}
	}

	return change;
}

bool SoundSystem::NeedRefresh()
{
	CTOOL_DEBUG_BREAK;

	return false;
}

void SoundSystem::ResetToDefault(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack)
{
	CTOOL_DEBUG_BREAK;
}

std::string SoundSystem::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	str += vOffset + "<soundsystem>\n";

	str += vOffset + "\t<active>" + ct::toStr(puActivated ? "true" : "false") + "</active>\n";
	str += vOffset + "\t<audio_amplification>" + ct::toStr(m_AudioFloatAmplification) + "</audio_amplification>\n";
	str += vOffset + "\t<average_size>" + ct::toStr(m_CurrentAverageSize) + "</average_size>\n";
	str += vOffset + "\t<count_display_points>" + ct::toStr(m_CountDisplayPoints) + "</count_display_points>\n";

	str += vOffset + "</soundsystem>\n";

	return str;
}

bool SoundSystem::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "soundsystem")
	{
		if (strName == "active")
			puActivated = ct::ivariant(strValue).GetB();
		else if (strName == "audio_amplification")
			m_AudioFloatAmplification = ct::fvariant(strValue).GetF();
		else if (strName == "average_size")
			m_CurrentAverageSize = (size_t)ct::uvariant(strValue).GetU();
		else if (strName == "count_display_points")
			m_CountDisplayPoints = (size_t)ct::uvariant(strValue).GetU();
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////

bool SoundSystem::UpdateUniforms(UniformVariantPtr vUniPtr)
{
	const bool change = false;

	if (vUniPtr)
	{
		
	}

	return change;
}

int SoundSystem::UploadUniformForGlslType(const GuiBackend_Window& /*vWin*/, UniformVariantPtr vUniPtr, int vTextureSlotId, bool /*vIsCompute*/) {
	if (vUniPtr)
	{
		if (vUniPtr->widgetType == "sound")
		{
			vUniPtr->sound_ptr = m_FFTTexture1DPtr;

			if (vUniPtr->sound_ptr)
			{
				vUniPtr->uSampler1D = vUniPtr->sound_ptr->glTex;

				if (vUniPtr->uSampler1D > -1 && 
					vUniPtr->loc > -1)
				{
					glActiveTexture(GL_TEXTURE0 + vTextureSlotId);
					glBindTexture(GL_TEXTURE_1D, vUniPtr->uSampler1D);
					glUniform1i(vUniPtr->loc, vTextureSlotId);
					++vTextureSlotId;
				}
			}
		}
		else if (vUniPtr->widgetType == "sound_histo")
		{
			if (m_SoundHisto_RenderPack_Ptr)
			{
				vUniPtr->sound_histo_ptr = m_SoundHisto_RenderPack_Ptr->GetTexture();

				if (vUniPtr->sound_histo_ptr)
				{
					vUniPtr->uSampler2D = vUniPtr->sound_histo_ptr->glTex;

					if (vUniPtr->uSampler2D > -1 && 
						vUniPtr->loc > -1)
					{
						glActiveTexture(GL_TEXTURE0 + vTextureSlotId);
						glBindTexture(GL_TEXTURE_2D, vUniPtr->uSampler2D);
						glUniform1i(vUniPtr->loc, vTextureSlotId);
						++vTextureSlotId;
					}
				}
			}
		}
	}

	return vTextureSlotId;
}

bool SoundSystem::DrawWidget(CodeTreePtr vCodeTree, UniformVariantPtr vUniPtr,
	const float& /*vMaxWidth*/, const float& vFirstColumnWidth, RenderPackWeak vRenderPack,
	const bool& /*vShowUnUsed*/, const bool& /*vShowCustom*/, const bool& /**/, bool* vChange)
{
	bool catched = false;

	if (vUniPtr && vCodeTree && vChange)
	{
		UniformVariantPtr v = vUniPtr;

		ShaderKeyPtr key = nullptr;
		auto rpPtr = vRenderPack.lock();
		if (rpPtr)
			key = rpPtr->GetShaderKey();

		/*if (v->widgetType == "sound")
		{
			catched = true;
			ImGui::Separator();

			vCodeTree->DrawUniformName(key, v);
			ImGui::SameLine(vFirstColumnWidth);

			//this is a texture1D, cant shown with imgui
			ImGui::Texture(m_FFTTexture1DPtr, 100.0f, ImVec4(1, 1, 1, 1), 1);
		}
		else */if (v->widgetType == "sound_histo" && m_SoundHisto_RenderPack_Ptr)
		{
			catched = true;
			ImGui::Separator();

			vCodeTree->DrawUniformName(key, v);
			ImGui::SameLine(vFirstColumnWidth);
			ImGui::Texture(m_SoundHisto_RenderPack_Ptr->GetTexture(), 100.0f, ImVec4(1, 1, 1, 1), 10);
		}
	}

	return catched;
}

bool SoundSystem::SerializeUniform(UniformVariantPtr vUniform, std::string* vStr)
{
	bool catched = false;

	if (vStr)
	{
		if (vUniform->widget == "sound")
		{
			/*if (vUniform->sound_histo_ptr && vUniform->filePathNames.size() == 1)
			{
				*vStr = vUniform->name + ":choose:" + FileHelper::Instance()->GetPathRelativeToApp(vUniform->filePathNames[0]);
				*vStr += ":play:" + ct::toStr(vUniform->bx ? "true" : "false");
				*vStr += ":time:" + ct::toStr(vUniform->y);
				*vStr += ":volume:" + ct::toStr(vUniform->soundVolume);
				*vStr += ":loop:" + ct::toStr(vUniform->soundLoop ? "true" : "false");
				*vStr += ":histo:" + ct::toStr(vUniform->soundHisto);
				*vStr += "\n";
			}*/
		}
		else if (vUniform->widget == "sound_histo")
		{

		}
	}

	return catched;
}

bool SoundSystem::DeSerializeUniform(ShaderKeyPtr /*vShaderKey*/, UniformVariantPtr vUniform, const std::vector<std::string>& /*vParams*/) {
	bool catched = false;

	if (vUniform->widget == "sound")
	{
		/*if (vParams.size() > 3)
		{
			size_t idx = 0;
			for (auto it = vParams.begin(); it != vParams.end(); ++it)
			{
				std::string word = *it;
				if (word == "choose" && (vUniform->soundFileChoosebox || vUniform->soundChoiceActivated))
				{
					if (vParams.size() > idx + 1)
					{
						vUniform->filePathNames.clear();
						vUniform->filePathNames.emplace_back(vParams[idx + 1]);
						vUniform->soundFileChoosebox = true;
						vUniform->soundChoiceActivated = true;
					}
				}
				else if (word == "play")
				{
					if (vParams.size() > idx + 1)
						vUniform->bx = ct::ivariant(vParams[idx + 1]).GetB();
				}
				else if (word == "time")
				{
					if (vParams.size() > idx + 1)
						vUniform->y = ct::fvariant(vParams[idx + 1]).GetF();
				}
				else if (word == "volume")
				{
					if (vParams.size() > idx + 1)
						vUniform->soundVolume = ct::fvariant(vParams[idx + 1]).GetF();
				}
				else if (word == "loop")
				{
					if (vParams.size() > idx + 1)
						vUniform->soundLoop = ct::ivariant(vParams[idx + 1]).GetB();
				}
				else if (word == "histo")
				{
					if (vParams.size() > idx + 1)
						vUniform->soundHisto = ct::ivariant(vParams[idx + 1]).GetI();
				}
				idx++;
			}
		}

		vShaderKey->Complete_Uniform_Sound_With_Sound(GuiBackend_Window(), vUniform, true);

		if (vUniform->sound_histo_ptr)
		{
			//vUniform->x = vUniform->sound_histo_ptr->GetCurrentPosInSeconds();
		}*/
	}
	else if (vUniform->widget == "sound_histo")
	{

	}

	return catched;
}

void SoundSystem::Complete_Uniform(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform)
{
	vUniform->widget = vUniform->widgetType;

	if (vUniform->widgetType == "sound")
	{
		if (vUniform->glslType == uType::uTypeEnum::U_SAMPLER1D)
		{
			auto rpPtr = vRenderPack.lock();
			if (rpPtr)
			{
				vUniform->widget = "sound";
				vUniform->timeLineSupported = true;

				for (auto it = vUniformParsed.paramsDico.begin(); it != vUniformParsed.paramsDico.end(); ++it)
				{
					std::string key = it->first;

					if (key == "file")
					{
						if (it->second.size() == 1)
						{
							vUniform->filePathNames.clear();
							vUniform->filePathNames.emplace_back(*it->second.begin());
							vUniform->soundFileChoosebox = false;
						}
						else
						{
							// une choose box a afficher
							vUniform->soundFileChoosebox = true;
						}

						vUniform->soundChoiceActivated = false;
					}
				}
			}

			Complete_Uniform_Sound_With_Sound(vParentKey, vRenderPack, vUniformParsed, vUniform);
		}
	}
	else if (vUniform->widget == "sound_histo")
	{
		if (vUniform->glslType == uType::uTypeEnum::U_SAMPLER2D)
		{
			Complete_Uniform_Sound_With_Sound(vParentKey, vRenderPack, vUniformParsed, vUniform);
		}
	}
}

void SoundSystem::Complete_Uniform_Sound_With_Sound(ShaderKeyPtr vParentKeyPtr, RenderPackWeak vRenderPack, const UniformParsedStruct& /*vUniformParsed*/, UniformVariantPtr vUniform) {
    if (!vRenderPack.expired() || !vParentKeyPtr)
		return;

	if (vUniform->filePathNames.empty())
	{
		//vUniform->sound_ptr.reset();
		//vUniform->sound_histo_ptr = m_FFTHistoTexture2DPtr;
	}

	/*if (vUniform->filePathNames.size() == 1U)
	{
		std::string _filepathName = vUniform->filePathNames[0];
		if (!FileHelper::Instance()->IsFileExist(_filepathName, true))
			_filepathName = FileHelper::Instance()->GetAbsolutePathForFileLocation(_filepathName, (int)FILE_LOCATION_Enum::FILE_LOCATION_ASSET_SOUND);

		if (FileHelper::Instance()->IsFileExist(_filepathName, true))
		{
			vUniform->sound_histo_ptr.reset();
			vUniform->sound_histo_ptr = m_FFTHistoTexture2DPtr;
			if (vUniform->sound_histo_ptr)
			{
				if (!vSoundConfigExist)
				{
					vUniform->bx = false;
					vUniform->x = 0.0f;
				}
				else
				{
					//vUniform->bx ? vUniform->sound_histo_ptr->Play() : vUniform->sound_histo_ptr->Pause();
				}

				vUniform->attachment = 0;
				vUniform->ownSound = true;
				vUniform->pipe = nullptr;// vUniform->sound->GetPipe();
				vUniform->sound_histo_ptr->SetLoopPlayBack(vUniform->soundLoop);
				vUniform->sound_histo_ptr->SetPosInPercentOfTotalLength(vUniform->y);
				vUniform->sound_histo_ptr->SetVolume(vUniform->soundVolume);
			}
		}
		else
		{
			std::string fromFile = vParentKeyPtr->puKey;
			if (!vParentKeyPtr->puInFileBufferFromKey.empty())
				fromFile = vParentKeyPtr->puInFileBufferFromKey;
			vParentKeyPtr->GetSyntaxErrors()->SetSyntaxError(
				vParentKeyPtr, 
				"Asset not found :", 
				"Sound file not found", 
				false,
				LineFileErrors(
					vUniform->SourceLinePos, 
					fromFile, 
					"The file Condition '" + vUniform->filePathNames[0] + "' not found (" + _filepathName + ")"));
		}
	}*/
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

std::string SoundSystem::InitRenderPack(const GuiBackend_Window& vWin, CodeTreePtr vCodeTree)
{
	if (vCodeTree)
	{
		auto shader_string = GetSoundHistoShaderString();
		m_SoundHisto_Key_Ptr = vCodeTree->LoadFromString("SoundHisto", shader_string, "SoundHisto.glsl", "", KEY_TYPE_Enum::KEY_TYPE_SHADER);
		m_SoundHisto_RenderPack_Ptr = RenderPack::createBufferWithFileWithoutLoading(
			vWin,
			"SoundHisto",
			ct::ivec3(SoundSystem::scFftSize, 240U, 0),
			m_SoundHisto_Key_Ptr,
			false, true
		);
	}

	return "";
}

bool SoundSystem::LoadRenderPack()
{
	if (m_SoundHisto_RenderPack_Ptr)
	{
		return m_SoundHisto_RenderPack_Ptr->Load();
	}
	return false;
}

void SoundSystem::FinishRenderPack()
{
	if (m_SoundHisto_RenderPack_Ptr)
	{
		m_SoundHisto_RenderPack_Ptr->Finish(false);
	}
}

void SoundSystem::DestroyRenderPack()
{
	m_SoundHisto_RenderPack_Ptr.reset();
}

std::string SoundSystem::GetSoundHistoShaderString()
{
	return u8R"(
@FRAMEBUFFER

WRAP(clamp)
MIPMAP(true)
FILTER(nearest)
COUNT(1)
SIZE(%u)

@UNIFORMS

uniform int(frame) frame;
uniform float(time) time;
uniform sampler1D(sound) input_fft; // input
uniform sampler2D(buffer) last_buffer; // input
uniform vec2(buffer) last_buffer_size;

@VERTEX

layout(location = 0) in vec2 a_position;

void main()
{
	gl_Position = vec4(a_position, 0.0, 1.0);
}

@FRAGMENT

layout(location = 0) out vec4 fragColor;

void main()
{
	if (frame < 1) 
	{
		fragColor = vec4(0);
	} 
	else 
	{
		fragColor = (gl_FragCoord.y < 1.0) ? 
			texelFetch(input_fft, int(gl_FragCoord.x), 0) :
			texelFetch(last_buffer, ivec2(gl_FragCoord.xy) - ivec2(0,1), 0);
	}
}
)";
}

////////////////////////////////////////////////////
//// PUBLIC / AUDIO ////////////////////////////////
////////////////////////////////////////////////////

void SoundSystem::NewNotification(const ma_device_notification* /*pNotification*/) {
	if (Use() && 
		m_IsAudioCaptureReady)
	{
		m_NeedNewDeviceCheck = true;
	}
}

// https://webaudio.github.io/web-audio-api/#current-time-domain-data
double SoundSystem::BlackmanWindow(const int& n)
{
	static const double& N_coef = 1.0 / (scFftSize - 1U);  // Longueur du filtre
	static const double& pi = 3.14159265358979323846;
	static const double& a = 0.16;
	static const double& a0 = (1.0 - a) * 0.5;
	static const double& a1 = 0.5;
	static const double& a2 = a * 0.5;
	return a0 - a1 * cos(2.0 * pi * n * N_coef) + a2 * cos(4.0 * pi * n * N_coef);
}

void SoundSystem::AddFrames(const uint32_t& vFrameCount, const void* vInputPtr)
{
	m_IsNewCaptureDataAvailable = true;

	auto frameCount = ct::mini(vFrameCount, (uint32_t)SoundSystem::scFullFftSize);

	// Just rotate the buffer; copy existing, append new
	const float* samples = (const float*)vInputPtr;
	memset(m_AudioCaptureSampleBuffer, 0, sizeof(float) * SoundSystem::scFullFftSize);

	float* p = m_AudioCaptureSampleBuffer;

	for (uint32_t i = 0; i < frameCount; ++i)
	{
		// moyenne des deux canneaux comme defini par m_AudioDeviceConfig.capture.channels
		*(p++) = (samples[i * 2] + samples[i * 2 + 1]) * 
			0.5f * m_AudioFloatAmplification * (float)BlackmanWindow(i);
	}

}

/*
SHADERTOY
let waveLen = Math.min( inp.mWaveData.length, 512 );
this.mRenderer.UpdateTexture(inp.globject, 0, 0, 512,     1, inp.mFreqData); // FFT
this.mRenderer.UpdateTexture(inp.globject, 0, 1, waveLen, 1, inp.mWaveData); // PCM
*/

bool SoundSystem::ComputeFFT()
{
	if (Use() && 
		m_IsAudioCaptureReady &&
		m_IsNewCaptureDataAvailable)
	{
		static kiss_fft_cpx out_spectrum[SoundSystem::scFftSize + 1U]; // N / 2 + 1
		kiss_fftr(m_FFTConfig, m_AudioCaptureSampleBuffer, out_spectrum);

		static const float scaling = 1.0f / (float)SoundSystem::scFftSize / (float)m_CurrentAverageSize;

		float* p = m_AverageFFTDatas + m_AverageLastValueIndex * SoundSystem::scFftSize;

		for (size_t i = 0U; i < SoundSystem::scFftSize; ++i)
		{
			const auto& v = out_spectrum[i];
			float magnitude = sqrtf((float)(v.r * v.r + v.i * v.i)) * scaling;
			//magnitude = 10.0f * std::log10f(magnitude); // conversion in db

			// Retirer la valeur la plus ancienne de la somme
			auto* old_value = p + i;
			m_AverageFFTValues[i] -= *old_value;

			// Ajouter la nouvelle valeur � la somme
			*old_value = magnitude;
			m_AverageFFTValues[i] += magnitude;
		}

		// Incr�menter l'index
		m_AverageLastValueIndex = ct::maxi<size_t>((m_AverageLastValueIndex + 1U) % m_CurrentAverageSize, 0U);

		return true;
	}

	return false;
}

void SoundSystem::Update()
{
	// renderdoc config
	static size_t frame_to_capture_start = 1000U;
	static size_t frame_to_capture_count = 10U;
	// renderdoc capture trigger
	static size_t frame = 0U;
	if (frame_to_capture_count && 
		frame++ > frame_to_capture_start)
	{
		RenderDocController::Instance()->RequestCapture();
		--frame_to_capture_count;
	}
	
	if (Use() && 
		m_IsAudioCaptureReady)
	{
		static ma_device_state last_device_state = ma_device_state_uninitialized;

		// device seem to have issue
		// maybe deconnected
		auto device_state = ma_device_get_state(&m_AudioCaptureDevice);
		if (last_device_state != device_state || m_NeedNewDeviceCheck)
		{
			switch (device_state)
			{
			case ma_device_state_uninitialized:
				LogVarInfo("Currente Device is not initialized");
				break;
			case ma_device_state_stopped:
				LogVarInfo("Currente Device is stopped");
				break;
			case ma_device_state_started:
				LogVarInfo("Currente Device is started");
				break;
			case ma_device_state_starting:
				LogVarInfo("Currente Device is starting");
				break;
			case ma_device_state_stopping:
				LogVarInfo("Currente Device is stopping");
				break;
			}

			if (device_state == ma_device_state_stopping ||
				device_state == ma_device_state_stopped ||
				device_state == ma_device_state_uninitialized)
			{
				//m_FirstAudioDriverCheck = true;
				EnumerateAudioDevices();
				SelectAudioDevice(m_CurrentAudioDeviceIndex);
			}

			last_device_state = device_state;
		}
	}
}

void SoundSystem::Render(float vDeltaTime)
{
	if (Use() && 
		m_IsNewCaptureDataAvailable)
	{
		TracyGpuZone("MainBackend::RenderSoundHisto");

		if (m_SoundHisto_RenderPack_Ptr)
		{
			if (ComputeFFT())
			{
				UpdateR32FTexture1D(m_FFTTexture1DPtr, m_AverageFFTValues);
			}

			m_SoundHisto_RenderPack_Ptr->UpdateTimeWidgets(vDeltaTime);
			m_SoundHisto_RenderPack_Ptr->UpdateUniforms(std::bind(&SoundSystem::UpdateHistoRenderPackUniforms, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
			m_SoundHisto_RenderPack_Ptr->RenderNode();
		}

		m_IsNewCaptureDataAvailable = false;
	}
}

void SoundSystem::ResetFrame()
{
	if (m_SoundHisto_RenderPack_Ptr)
	{
		m_SoundHisto_RenderPack_Ptr->ResetFrame();
	}

	m_AverageLastValueIndex = 0U;
	memset(m_AverageFFTDatas, 0, sizeof(float)* SoundSystem::scFftSize* SoundSystem::scMaxAverageSize);
	memset(m_AverageFFTValues, 0, sizeof(float)* SoundSystem::scFftSize);
}

void SoundSystem::ResetTime()
{
	if (m_SoundHisto_RenderPack_Ptr)
	{
		m_SoundHisto_RenderPack_Ptr->ResetTime();
	}

	m_AverageLastValueIndex = 0U;
	memset(m_AverageFFTDatas, 0, sizeof(float) * SoundSystem::scFftSize * SoundSystem::scMaxAverageSize);
	memset(m_AverageFFTValues, 0, sizeof(float) * SoundSystem::scFftSize);
}

void SoundSystem::UpdateHistoRenderPackUniforms(RenderPackWeak /*vRenderPack*/,
                                                UniformVariantPtr vUniPtr,
                                                DisplayQualityType /*vDisplayQuality*/,
                                                MouseInterface* /*vMouse*/,
                                                CameraInterface* /*vCamera*/) {
	if (vUniPtr)
	{
		if (vUniPtr->widgetType == "sound")
		{
			vUniPtr->sound_ptr = m_FFTTexture1DPtr;
		}
	}
}

////////////////////////////////////////////////////
//// PRIVATE / AUDIO ///////////////////////////////
////////////////////////////////////////////////////

bool SoundSystem::InitAudioDriver()
{
	if (ma_context_init(NULL, 0, NULL, &m_AudioDriverContext) != MA_SUCCESS) {
		LogVarError("Failed to initialize context.\n");
		return false;
	}

	memset(m_AudioCaptureSampleBuffer, 0, sizeof(float) * SoundSystem::scFullFftSize);
	memset(m_AverageFFTDatas, 0, sizeof(float) * SoundSystem::scFftSize * SoundSystem::scMaxAverageSize);
	memset(m_AverageFFTValues, 0, sizeof(float) * SoundSystem::scFftSize);
	m_FFTConfig = kiss_fftr_alloc(SoundSystem::scFullFftSize, 0, NULL, NULL);

	EnumerateAudioDevices();
	SelectAudioDevice(m_CurrentAudioDeviceIndex);

	m_FFTTexture1DPtr = CreateR32FTexture1D(SoundSystem::scFftSize);

	return true;
}

void SoundSystem::UnitAudioDriver()
{
	m_FFTTexture1DPtr.reset();

	kiss_fft_free(m_FFTConfig);

	if (ma_device_is_started(&m_AudioCaptureDevice)) {
		ma_device_stop(&m_AudioCaptureDevice);
	}
	ma_context_uninit(&m_AudioDriverContext);
}

void SoundSystem::EnumerateAudioDevices()
{
	m_AudioDevices.clear();

	ma_device_info* pPlaybackDeviceInfos;
	ma_uint32 playbackDeviceCount;
	ma_device_info* pCaptureDeviceInfos;
	ma_uint32 captureDeviceCount;
	ma_result result = ma_context_get_devices(&m_AudioDriverContext, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
	if (result != MA_SUCCESS) {
		LogVarError("Failed to retrieve device information.\n");
		return;
	}

	for (ma_uint32 iDevice = 0; iDevice < playbackDeviceCount; ++iDevice)
	{
		auto& device = pPlaybackDeviceInfos[iDevice];

		if (m_FirstAudioDriverCheck && device.isDefault)
		{
			m_CurrentAudioDeviceIndex = m_AudioDevices.size();
			m_FirstAudioDriverCheck = false;
		}

		// even if failed, we continue
		ma_context_get_device_info(&m_AudioDriverContext, ma_device_type_playback, &device.id, &device);

		auto new_name = ct::toStr("[out] : %s", device.name);
		ct::ResetBuffer(device.name);
		ct::AppendToBuffer(device.name, 256, new_name);

		m_AudioDevices.push_back(device);
	}

	for (ma_uint32 iDevice = 0; iDevice < captureDeviceCount; ++iDevice)
	{
		auto& device = pCaptureDeviceInfos[iDevice];

		if (m_FirstAudioDriverCheck && device.isDefault)
		{
			m_CurrentAudioDeviceIndex = m_AudioDevices.size();
			m_FirstAudioDriverCheck = false;
		}

		// even if failed, we continue
		ma_context_get_device_info(&m_AudioDriverContext, ma_device_type_capture, &device.id, &device);

		auto new_name = ct::toStr("[in]  : %s", device.name);
		ct::ResetBuffer(device.name);
		ct::AppendToBuffer(device.name, 256, new_name);

		m_AudioDevices.push_back(device);
	}
}

static void sCaptureFrames(ma_device* pDevice, void* /*pOutput*/, const void* pInput, ma_uint32 frameCount) {
	SoundSystem* sound_ptr = reinterpret_cast<SoundSystem*>(pDevice->pUserData);
	if (sound_ptr)
	{
		sound_ptr->AddFrames(frameCount, pInput);
	}
}

static void sNewNotification(const ma_device_notification* pNotification)
{
	SoundSystem* sound_ptr = reinterpret_cast<SoundSystem*>(pNotification->pDevice->pUserData);
	if (sound_ptr)
	{
		sound_ptr->NewNotification(pNotification);
	}
}

void SoundSystem::SelectAudioDevice(const size_t& vDeviceIndex)
{
	m_IsAudioCaptureReady = false;

	if (!m_AudioDevices.empty())
	{
		if (vDeviceIndex < m_AudioDevices.size())
		{
			m_CurrentAudioDeviceIndex = vDeviceIndex;
		}
		else
		{
			m_CurrentAudioDeviceIndex = 0U;
		}

		m_AudioCaptureDeviceID = m_AudioDevices[m_CurrentAudioDeviceIndex].id;

		bool useLoopback = ma_is_loopback_supported(m_AudioDriverContext.backend);
		m_AudioDeviceConfig = ma_device_config_init(useLoopback ? ma_device_type_loopback : ma_device_type_capture);
		m_AudioDeviceConfig.capture.pDeviceID = &m_AudioCaptureDeviceID;
		m_AudioDeviceConfig.capture.format = ma_format_f32;
		m_AudioDeviceConfig.capture.channels = 2;
		m_AudioDeviceConfig.sampleRate = 44100;
		m_AudioDeviceConfig.dataCallback = sCaptureFrames;
		m_AudioDeviceConfig.notificationCallback = sNewNotification;
		m_AudioDeviceConfig.pUserData = this;

		//auto frame_size_ = ma_get_bytes_per_frame(ma_format_f32, 2);

		if (ma_device_is_started(&m_AudioCaptureDevice)) {
			ma_device_stop(&m_AudioCaptureDevice);
		}

		if (ma_device_get_state(&m_AudioCaptureDevice) != ma_device_state_uninitialized) {
			ma_device_uninit(&m_AudioCaptureDevice);
		}

		ma_result result = ma_device_init(&m_AudioDriverContext, &m_AudioDeviceConfig, &m_AudioCaptureDevice);
		if (result != MA_SUCCESS)
		{
			LogVarError("[FFT] Failed to initialize capture device: %d\n", result);
			return;
		}
		
		result = ma_device_start(&m_AudioCaptureDevice);
		if (result != MA_SUCCESS)
		{
			ma_device_uninit(&m_AudioCaptureDevice);
			LogVarError("[FFT] Failed to start capture device: %d\n", result);
			return ;
		}

		m_IsAudioCaptureReady = true;
	}
}

ctTexturePtr SoundSystem::CreateR32FTexture1D(const size_t& vWidth)
{
	auto res = std::make_shared<ct::texture>();

	int countChannels = 1;
	res->gldatatype = GL_FLOAT;
	res->glformat = GL_RED;
	res->glinternalformat = GL_R32F;

	const size_t verifSize = vWidth * countChannels;
	if (verifSize > 0)
	{
		res->useMipMap = true;

		res->glTextureType = GL_TEXTURE_1D;

		res->w = vWidth;
		res->h = 1;

		glGenTextures(1, &res->glTex);
		LogGlError();

		glBindTexture(GL_TEXTURE_1D, res->glTex);
		LogGlError();

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		res->glWrapS = GL_CLAMP_TO_EDGE;
		res->glWrapT = GL_CLAMP_TO_EDGE;

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, res->glWrapS);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, res->glWrapT);

		LogGlError();

		res->glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
		res->glMagFilter = GL_LINEAR;

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);

		LogGlError();

		glTexImage1D(GL_TEXTURE_1D, 0, res->glinternalformat,
			(GLsizei)res->w, 0, res->glformat, res->gldatatype, nullptr);

		LogGlError();

		glGenerateMipmap(GL_TEXTURE_1D);
		LogGlError();
	}

	return res;
}

ctTexturePtr SoundSystem::CreateR32FTexture2D(const size_t& vWidth, const size_t& vHeight)
{
	auto res = std::make_shared<ct::texture>();

	int countChannels = 1;
	res->gldatatype = GL_FLOAT;
	res->glformat = GL_RED;
	res->glinternalformat = GL_R32F;

	const size_t verifSize = vWidth * vHeight * countChannels;
	if (verifSize > 0)
	{
		res->useMipMap = true;

		res->glTextureType = GL_TEXTURE_2D;

		res->w = vWidth;
		res->h = vHeight;

		glGenTextures(1, &res->glTex);
		LogGlError();

		glBindTexture(GL_TEXTURE_2D, res->glTex);
		LogGlError();

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		res->glWrapS = GL_CLAMP_TO_EDGE;
		res->glWrapT = GL_CLAMP_TO_EDGE;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, res->glWrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, res->glWrapT);

		LogGlError();

		res->glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
		res->glMagFilter = GL_LINEAR;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, res->glMinFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, res->glMagFilter);

		LogGlError();

		glTexImage2D(GL_TEXTURE_2D, 0, res->glinternalformat,
			(GLsizei)res->w, (GLsizei)res->h,
			0, res->glformat, res->gldatatype, nullptr);

		LogGlError();

		glGenerateMipmap(GL_TEXTURE_2D);
		LogGlError();
	}

	return res;
}

void SoundSystem::UpdateR32FTexture1D(ctTexturePtr vTexturePtr, const float* vSamplesPtr)
{
	if (vTexturePtr && 
		vSamplesPtr && 
		vTexturePtr->glTextureType == GL_TEXTURE_1D &&
		vTexturePtr->glTex)
	{
		glBindTexture(GL_TEXTURE_1D, vTexturePtr->glTex);
		LogGlError();
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, (int)vTexturePtr->w, GL_RED, GL_FLOAT, vSamplesPtr);
		LogGlError();
		glBindTexture(GL_TEXTURE_1D, 0);
		LogGlError();
	}
}

void SoundSystem::UpdateR32FTexture2D(ctTexturePtr vTexturePtr, const float* vSamplesPtr)
{
	if (vTexturePtr && 
		vSamplesPtr && 
		vTexturePtr->glTextureType == GL_TEXTURE_2D &&
		vTexturePtr->glTex)
	{
		glBindTexture(GL_TEXTURE_2D, vTexturePtr->glTex);
		LogGlError();
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (int)vTexturePtr->w, (int)vTexturePtr->h, GL_RED, GL_FLOAT, vSamplesPtr);
		LogGlError();
		glBindTexture(GL_TEXTURE_2D, 0);
		LogGlError();
	}
}
