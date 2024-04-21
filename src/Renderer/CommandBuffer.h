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
#include <glad/glad.h>
#include <Headers/RenderPackHeaders.h>


class CommandBuffer
{
private:
	bool prMultiSampling_Started = false;
	bool prDepth_Started = false;
	bool prCulling_Started = false;
	bool prBlending_Started = false;
	bool prLineSmoothing_Started = false;
	bool prPointSize_Started = false; 
	bool prTransparency_Started = false;

public:
	void Begin(const GuiBackend_Window& vContext);
	void End();

	void BeginMultiSampling();
	void EndMultiSampling();

	void BeginDepth(float vStartDepth, float vEndDepth, GLenum vDepthFunc);
	void EndDepth();

	void BeginCulling(GLenum vCullFace, GLenum vFrontFace);
	void EndCulling();

	void BeginBlending(GLenum vStartFactor, GLenum vEndFactorn, GLenum vEquation);
	void EndBlending();

	void BeginLineSmoothing(float vLineThickness);
	void EndLineSmoothing();

	void BeginPointSize();
	void EndPointSize();

	void BeginTransparency();
	void EndTransparency();

	void ClearColor(ct::fvec4 vColor);
	void SetViewPort(ct::fvec4 vViewPort);
};