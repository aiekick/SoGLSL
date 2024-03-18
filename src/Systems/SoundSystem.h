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
#include <Renderer/RenderPack.h>

#include <miniaudio.h>
#include <kiss_fftr.h>

class CameraSystem;
class CodeTree;
class ShaderKey;
class RenderPack;

class UniformVariant;

struct UniformParsedStruct;
class SoundSystem : public WidgetInterface, public conf::ConfigAbstract
{
public:
	static constexpr size_t scFftSize = 512U;
	static constexpr size_t scFullFftSize = scFftSize * 2U;
	static constexpr size_t scMaxAverageSize = 50U;

public:
	bool puActivated = false;
	bool puNeedOneUniformUpdate = false;

private:
	ct::ActionTime m_ActionTime;

private: // audio
	kiss_fftr_cfg m_FFTConfig;
	ma_context m_AudioDriverContext;
	ma_device_id m_AudioCaptureDeviceID;
	ma_device m_AudioCaptureDevice;
	ma_device_config m_AudioDeviceConfig;
	std::vector<ma_device_info> m_AudioDevices;
	size_t m_CurrentAudioDeviceIndex = 0U;
	float m_AudioFloatAmplification = 100.0f;

	bool m_FirstAudioDriverCheck = true;
	bool m_IsAudioCaptureReady = false;
	bool m_IsNewCaptureDataAvailable = false;
	bool m_NeedNewDeviceCheck = false;

	ctTexturePtr m_FFTTexture1DPtr = nullptr;

	// * 2 because left and right channel
	kiss_fft_scalar m_AudioCaptureSampleBuffer[scFullFftSize] = {};

	size_t m_AverageLastValueIndex = 0U;
	float m_AverageFFTDatas[scFftSize * scMaxAverageSize] = {};
	float m_AverageFFTValues[scFftSize] = {};
	size_t m_CurrentAverageSize = 10U;
	size_t m_CountDisplayPoints = 150U;
	float m_MaxFFTValue = 0.0f;

	RenderPackPtr m_SoundHisto_RenderPack_Ptr = nullptr;
	ShaderKeyPtr m_SoundHisto_Key_Ptr = nullptr;
	UniformVariantPtr m_SoundFFTTexturePtr = nullptr;

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

	int UploadUniformForGlslType(const GuiBackend_Window& vWin, UniformVariantPtr vUniPtr, int vTextureSlotId, bool vIsCompute);

	bool Use();

	bool IsActivated();
	void SetActivation(bool vActivation);
	bool DrawMenu();
	bool DrawTooltips(RenderPackWeak vRenderPack, float vDisplayQuality, CameraSystem* vCamera, ct::ivec2 vScreenSize);
	bool DrawPane(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack, float vDisplayQuality, CameraSystem* vCamera);
	bool NeedRefresh();
	void ResetToDefault(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack);

	bool IsThereAnyNewCaptureDataAvailable() { return m_IsNewCaptureDataAvailable; }
	void Update();
	void Render(float vDeltaTime);

	void ResetFrame();
	void ResetTime();

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

public:
	std::string InitRenderPack(const GuiBackend_Window& vWin, CodeTreePtr vCodeTree);
	bool LoadRenderPack();
	void FinishRenderPack();
	void DestroyRenderPack();
	RenderPackWeak GetRenderPack() { return m_SoundHisto_RenderPack_Ptr; }
	ShaderKeyPtr GetShaderKey() { return m_SoundHisto_Key_Ptr; }

private: // renderpack
	void Complete_Uniform_Sound_With_Sound(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform);
	std::string GetSoundHistoShaderString();

public: // Audio
	void AddFrames(const uint32_t& vFrameCount, const void* vInputPtr); 
	bool ComputeFFT();

public: // notifications
	void NewNotification(const ma_device_notification* pNotification);

private: // audio
	bool InitAudioDriver();
	void UnitAudioDriver();
	void EnumerateAudioDevices();
	void SelectAudioDevice(const size_t& vDeviceIndex);
	
private: // effects
	double BlackmanWindow(const int& n);

private: // opengl
	void UpdateHistoRenderPackUniforms(RenderPackWeak vRenderPack, UniformVariantPtr vUniPtr, DisplayQualityType vDisplayQuality, MouseInterface* vMouse, CameraInterface* vCamera);
	ctTexturePtr CreateR32FTexture1D(const size_t& vWidth);
	ctTexturePtr CreateR32FTexture2D(const size_t& vWidth, const size_t& vHeight);
	void UpdateR32FTexture1D(ctTexturePtr vTexturePtr, const float* vSamplesPtr);
	void UpdateR32FTexture2D(ctTexturePtr vTexturePtr, const float* vSamplesPtr);

public:
	static SoundSystem* Instance()
	{
		static SoundSystem _instance;
		return &_instance;
	}

protected:
	SoundSystem() = default; // Prevent construction
	SoundSystem(const SoundSystem&) = default; // Prevent construction by copying
	SoundSystem& operator =(const SoundSystem&) { return *this; }; // Prevent assignment
	~SoundSystem() = default; // Prevent unwanted destruction
};
