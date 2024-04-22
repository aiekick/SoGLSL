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

// #define USE_MINIAUDIO_LIB
// #define USE_KISSFFT_LIB

// #undef USE_BASS_LIB
//  vlc block le chargment de bass

#include <Headers/RenderPackHeaders.h>
#include <ctools/cTools.h>

#include <string>

#pragma warning(push)
#pragma warning(disable : 4201)  // suppress even more warnings about nameless structs
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)

#include <miniaudio.h>
#include <kiss_fft.h>
#include <kiss_fftr.h>

class TextureSound {
private:
    std::string puType;
    std::string puFilePathName;

    ct::texture puFFTTexture;
    float* puDatas;
    bool puLoopPlayBack;
    bool puPlay;
    int puNumHistorySamples;

public:
    static TextureSound* Create(/*const GuiBackend_Window& vWin, */ const std::string& vFilePathName, int vNumHistorySamples = 0);
    static void Init();
    static void Release();
    static bool IsOk;
    static int CountSoundRecorder;

public:
    std::string GetSoundFilePathName() {
        return puFilePathName;
    }

public:
    TextureSound();
    virtual ~TextureSound();

    bool Load(const std::string& vFilePathName, int vNumHistorySamples);

    void ReplaceSoundFilePathName(const std::string& vFilePathName);

    void SetLoopPlayBack(bool vLoopPlayBack);
    void Reset();
    void Play();
    void Pause();
    void SetVolume(float vVolume = 0.1f);
    void Step(float vStepCoef);
    void StepInPercentOfTotalLength(float vOffsetInPercentOfTotalLength);
    void StepInSeconds(float vOffsetInSeconds);
    void SetPosInPercentOfTotalLength(float vPosInPercentOfTotalLength);
    void SetPosInSeconds(float vPosInSeconds);
    void SetPos(float vStepCoef, float vOffsetInPercentOfTotalLength, float vProgress);
    float GetCurrentPosInSeconds();
    float GetCurrentPosInPercents();
    float GetLengthInSeconds();

    GLuint GetTexId();

    bool CreateTexture(int vWidth);
    bool UpdateTexture(int vTextureIndex);

private:
    bool GetFFT(float* vSamples);
};
