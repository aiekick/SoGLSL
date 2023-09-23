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

#include "CommandBuffer.h"

#include <ctools/Logger.h>

void CommandBuffer::Begin(const GuiBackend_Window& vContext)
{
	GuiBackend::MakeContextCurrent(vContext);

	prMultiSampling_Started = false;
	prDepth_Started = false;
	prCulling_Started = false;
	prBlending_Started = false;
	prLineSmoothing_Started = false;
	prPointSize_Started = false;
	prTransparency_Started = false;
}

void CommandBuffer::End()
{

}

void CommandBuffer::BeginMultiSampling()
{
	prMultiSampling_Started = true;
	glEnable(GL_MULTISAMPLE);
}

void CommandBuffer::EndMultiSampling()
{
	if (prMultiSampling_Started)
		glDisable(GL_MULTISAMPLE);
	prMultiSampling_Started = false;
}

void CommandBuffer::BeginDepth(float vStartDepth, float vEndDepth, GLenum vDepthFunc)
{
	prDepth_Started = true;
	glEnable(GL_DEPTH_TEST);
	glDepthRange(vStartDepth, vEndDepth);
	glDepthFunc(vDepthFunc);
}

void CommandBuffer::EndDepth()
{
	if (prDepth_Started)
		glDisable(GL_DEPTH_TEST);
	prDepth_Started = false;
}

void CommandBuffer::BeginCulling(GLenum vCullFace, GLenum vFrontFace)
{
	prCulling_Started = true;
	glEnable(GL_CULL_FACE);
	glCullFace(vCullFace);
	glFrontFace(vFrontFace);
}

void CommandBuffer::EndCulling()
{
	if (prCulling_Started)
		glDisable(GL_CULL_FACE);
	prCulling_Started = false;
}

void CommandBuffer::BeginBlending(GLenum vStartFactor, GLenum vEndFactorn, GLenum vEquation)
{
	prBlending_Started = true;
	glEnable(GL_BLEND);
	glBlendFunc(vStartFactor, vEndFactorn);
	glBlendEquation(vEquation);
}

void CommandBuffer::EndBlending()
{
	if (prBlending_Started)
		glDisable(GL_BLEND);
	prBlending_Started = false;
}

void CommandBuffer::BeginLineSmoothing(float vLineThickness)
{
	prLineSmoothing_Started = true;
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glHint.xhtml
	glLineWidth(vLineThickness);
}

void CommandBuffer::EndLineSmoothing()
{
	if (prLineSmoothing_Started)
		glDisable(GL_LINE_SMOOTH);
	prLineSmoothing_Started = false;
}

void CommandBuffer::BeginPointSize()
{
	prPointSize_Started = true;
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

void CommandBuffer::EndPointSize()
{
	if (prPointSize_Started)
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	prPointSize_Started = false;
}

void CommandBuffer::BeginTransparency()
{
	prTransparency_Started = true;
	glDepthMask(GL_FALSE);
}

void CommandBuffer::EndTransparency()
{
	if (prTransparency_Started)
		glDepthMask(GL_TRUE);
	prTransparency_Started = false;
}

void CommandBuffer::ClearColor(ct::fvec4 vColor)
{
	glClearColor(vColor.x, vColor.y, vColor.z, vColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CommandBuffer::SetViewPort(ct::fvec4 vViewPort)
{
	glViewport((int)vViewPort.x, (int)vViewPort.y, (int)vViewPort.z, (int)vViewPort.w);
}
