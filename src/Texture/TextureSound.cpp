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

//#define DR_FLAC_IMPLEMENTATION
//#include <miniaudio/extras/dr_flac.h>  /* Enables FLAC decoding. */
//#define DR_MP3_IMPLEMENTATION
//#include <miniaudio/extras/dr_mp3.h>   /* Enables MP3 decoding. */
//#define DR_WAV_IMPLEMENTATION
//#include <miniaudio/extras/dr_wav.h>   /* Enables WAV decoding. */
//#define MINIAUDIO_IMPLEMENTATION
//#include <miniaudio/miniaudio.h>

#include "TextureSound.h"
#include <ctools/Logger.h>
#include <Renderer/RenderPack.h>
#include <Buffer/FrameBuffer.h>
#include <Mesh/Model/BaseModel.h>
#include <Uniforms/UniformVariant.h>
#include <Buffer/FrameBuffersPipeLine.h>
#include <Profiler/TracyProfiler.h>

#include <stdio.h>
#include <functional>

bool TextureSound::IsOk = false;
int TextureSound::CountSoundRecorder = 0;

TextureSound* TextureSound::Create(const std::string& vFilePathName, int vNumHistorySamples)
{
	auto res = new TextureSound();

	if (res->Load(vFilePathName, vNumHistorySamples))
	{

	}
	else
	{
		SAFE_DELETE(res);
	}

	return res;
}

void TextureSound::Init()
{

}

void TextureSound::Release()
{

}

TextureSound::TextureSound()
{
	puPlay = false;
	puDatas = nullptr;
	puLoopPlayBack = true;
	puNumHistorySamples = 0;
}

TextureSound::~TextureSound()
{
	SAFE_DELETE_ARRAY(puDatas);

	if (glIsTexture(puFFTTexture.glTex) == GL_TRUE)
	{
		glDeleteTextures(1, &puFFTTexture.glTex);
		LogGlError();
	}
}

bool TextureSound::Load(const std::string& vFilePathName, int vNumHistorySamples)
{
	const auto res = false;

	puNumHistorySamples = vNumHistorySamples;
	
	if (vFilePathName == "mic")
	{
		puType = "mic";
	}
	else
	{

	}

	return res;
}

void TextureSound::ReplaceSoundFilePathName(const std::string& vFilePathName)
{
	Load(vFilePathName, puNumHistorySamples);
}

void TextureSound::SetLoopPlayBack(bool vLoopPlayBack)
{
	UNUSED(vLoopPlayBack);

#ifdef USE_BASS_LIB
	if (puChannel)
	{
		if (puLoopPlayBack != vLoopPlayBack)
		{
			puLoopPlayBack = vLoopPlayBack;

			if (puLoopPlayBack)
			{
				puSampleInfos.flags = BASS_SAMPLE_LOOP | BASS_SAMPLE_FLOAT;
			}
			else
			{
				puSampleInfos.flags = BASS_SAMPLE_FLOAT;
			}

			BASS_SampleSetInfo(puChannel, &puSampleInfos);
		}
	}
#endif
}

void TextureSound::Reset()
{
#ifdef USE_BASS_LIB
	if (puChannel)
	{
		if (BASS_ChannelPlay(puChannel, true))
		{
			if (puPlay == false)
			{
				puPlay = true;
				Pause();
			}
		}
	}
#endif
}

void TextureSound::Play()
{
#ifdef USE_BASS_LIB
	if (puChannel)
	{
		if (BASS_ChannelPlay(puChannel, false))
		{
			puPlay = true;
		}
	}
#endif
}

void TextureSound::Pause()
{
#ifdef USE_BASS_LIB
	if (puChannel)
	{
		if (puPlay == true)
		{
			if (BASS_ChannelPause(puChannel))
			{
				puPlay = false;
			}
		}
	}
#endif
}

void TextureSound::SetVolume(float vVolume)
{
	UNUSED(vVolume);

#ifdef USE_BASS_LIB
	vVolume = ct::clamp(vVolume, 0.0f, 1.0f);
	if (!BASS_SetVolume(vVolume))
	{
		LogVarLightError("Bass lib error : Unable to set Volume");
	}
#endif
}

void TextureSound::Step(float vStepCoef)
{
	UNUSED(vStepCoef);

#ifdef USE_BASS_LIB
	if (puChannel)
	{
		float stepSize = vStepCoef / puSampleInfos.freq;
		StepInSeconds(stepSize);
	}
#endif
}

void TextureSound::SetPos(float vStepCoef, float vOffsetInPercentOfTotalLength, float vProgress)
{
	UNUSED(vStepCoef);
	UNUSED(vOffsetInPercentOfTotalLength);
	UNUSED(vProgress);

#ifdef USE_BASS_LIB
	if (puChannel)
	{
		float currentTimeInSeconds = GetCurrentPosInSeconds();
		float stepSizeInSeconds = vStepCoef / puSampleInfos.freq;
		currentTimeInSeconds += stepSizeInSeconds;

		float startOffsetInSeconds = vOffsetInPercentOfTotalLength * puTotalLengthInSeconds;

		//QWORD pos = BASS_ChannelSeconds2Bytes(puChannel, (double)currentTime);
		//BASS_ChannelSetPosition(puChannel, pos, BASS_POS_BYTE);
		//StepInSeconds(stepSize);
	}
#endif
}

void TextureSound::StepInPercentOfTotalLength(float vOffsetInPercentOfTotalLength)
{
	UNUSED(vOffsetInPercentOfTotalLength);

#ifdef USE_BASS_LIB
	if (puChannel && puTotalLengthInSeconds > 0.0f)
	{
		float offset = vOffsetInPercentOfTotalLength * puTotalLengthInSeconds;
		StepInSeconds(offset);
	}
#endif
}

void TextureSound::SetPosInPercentOfTotalLength(float vPosInPercentOfTotalLength)
{
	UNUSED(vPosInPercentOfTotalLength);

#ifdef USE_BASS_LIB
	if (puChannel && puTotalLengthInSeconds > 0.0f)
	{
		float pos = vPosInPercentOfTotalLength * puTotalLengthInSeconds;
		SetPosInSeconds(pos);
	}
#endif
}

void TextureSound::SetPosInSeconds(float vPosInSeconds)
{
	UNUSED(vPosInSeconds);

#ifdef USE_BASS_LIB
	if (puChannel)
	{
		QWORD pos = BASS_ChannelSeconds2Bytes(puChannel, (double)vPosInSeconds);
		BASS_ChannelSetPosition(puChannel, pos, BASS_POS_BYTE);
	}
#endif
}

void TextureSound::StepInSeconds(float vOffsetInSeconds)
{
	UNUSED(vOffsetInSeconds);

#ifdef USE_BASS_LIB
	if (puChannel)
	{
		float currentTime = GetCurrentPosInSeconds();
		currentTime += vOffsetInSeconds;
		QWORD pos = BASS_ChannelSeconds2Bytes(puChannel, (double)currentTime);
		BASS_ChannelSetPosition(puChannel, pos, BASS_POS_BYTE);
	}
#endif
}

float TextureSound::GetCurrentPosInSeconds()
{
#ifdef USE_BASS_LIB
	if (puChannel)
	{
		QWORD currentPos = BASS_ChannelGetPosition(puChannel, BASS_POS_BYTE);
		return (float)BASS_ChannelBytes2Seconds(puChannel, currentPos);
	}
#endif

	return 0.0f;
}

float TextureSound::GetCurrentPosInPercents()
{
#ifdef USE_BASS_LIB
	if (puChannel)
	{
		QWORD currentPos = BASS_ChannelGetPosition(puChannel, BASS_POS_BYTE);
		return ((float)BASS_ChannelBytes2Seconds(puChannel, currentPos)) / puTotalLengthInSeconds;
	}
#endif

	return 0.0f;
}

float TextureSound::GetLengthInSeconds()
{
#ifdef USE_BASS_LIB
	if (puChannel)
	{
		QWORD len = BASS_ChannelGetLength(puChannel, BASS_POS_BYTE);
		return (float)BASS_ChannelBytes2Seconds(puChannel, len);
	}
#endif

	return 0.0f;
}

bool TextureSound::GetFFT(float *vSamples)
{
	UNUSED(vSamples);

#ifdef USE_BASS_LIB
	if (!puChannel)
		return false;

	unsigned int len = 0;

	switch (FFT_SIZE * 2) // for 256 fft, only 128 values will contain DC in our case
	{
	case 256:
		len = BASS_DATA_FFT256;
		break;
	case 512:
		len = BASS_DATA_FFT512;
		break;
	case 1024:
		len = BASS_DATA_FFT1024;
		break;
	case 2048:
		len = BASS_DATA_FFT2048;
		break;
	case 4096:
		len = BASS_DATA_FFT4096;
		break;
	case 8192:
		len = BASS_DATA_FFT8192;
		break;
	case 16384:
		len = BASS_DATA_FFT16384;
		break;
	default:
		//fprintf( stderr, "BASS invalid fft window size\n" );
		break;
	}

	const int numBytes = BASS_ChannelGetData(puChannel, vSamples, len | BASS_DATA_FFT_REMOVEDC | BASS_DATA_FLOAT);

	if (numBytes <= 0)
		return false;
#endif

	return true;
}

GLuint TextureSound::GetTexId()
{
	if (puNumHistorySamples > 0)
	{
		/*if (puPipe)
		{
			return puPipe->getFrontTextureID(0);
		}*/
	}
	
	return puFFTTexture.glTex;
}

bool TextureSound::CreateTexture(int vWidth)
{
	auto res = false;

	TracyGpuZone("TextureSound::CreateTexture");

	// on va creer une texture 1d car pas besoin de la copier une autre texture2d

	// FFT Texture
	glGenTextures(1, &puFFTTexture.glTex);
	LogGlError();
	glBindTexture(GL_TEXTURE_1D, puFFTTexture.glTex);
	LogGlError();

	puFFTTexture.glWrapS = GL_REPEAT;
	puFFTTexture.glMinFilter = GL_LINEAR;
	puFFTTexture.glMagFilter = GL_LINEAR;

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, puFFTTexture.glWrapS);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, puFFTTexture.glMinFilter);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, puFFTTexture.glMagFilter);

	puFFTTexture.glinternalformat = GL_R32F;
	puFFTTexture.glformat = GL_RED;
	puFFTTexture.gldatatype = GL_FLOAT;

	puFFTTexture.w = (size_t)vWidth;
	puFFTTexture.h = 1;

	puDatas = new float[vWidth];

	for (auto i = 0; i < vWidth; ++i)
		puDatas[i] = 0.0f;

	glTexImage1D(GL_TEXTURE_1D,
		0, puFFTTexture.glinternalformat,
		(int)puFFTTexture.w, // w 
		0,
		puFFTTexture.glformat, 
		puFFTTexture.gldatatype,
		puDatas);
	LogGlError();

	glBindTexture(GL_TEXTURE_1D, 0);
	LogGlError();

	res = true;

	return res;
}

bool TextureSound::UpdateTexture(int vTextureIndex)
{
	const auto res = false;

	TracyGpuZone("TextureSound::UpdateTexture");

	//GuiBackend::MakeContextCurrent(puWindow);

	if (GetFFT(puDatas))
	{
		glActiveTexture(GL_TEXTURE0 + vTextureIndex);
		LogGlError();
		glBindTexture(GL_TEXTURE_1D, puFFTTexture.glTex);
		LogGlError();
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, (int)puFFTTexture.w, GL_RED, GL_FLOAT, puDatas);
		LogGlError();
		glBindTexture(GL_TEXTURE_1D, 0);
		LogGlError();

		return true;
	}
		
	return res;
}

/*FrameBuffersPipeLinePtr TextureSound::GetPipe()
{
	return puPipe;
}*/
