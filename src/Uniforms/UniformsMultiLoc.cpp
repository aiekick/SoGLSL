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

#include "UniformsMultiLoc.h"
#include <Renderer/RenderPack.h>
#include <Profiler/TracyProfiler.h>

UniformsMultiLoc::UniformsMultiLoc(UniformVariantPtr vUniform)
{
	ZoneScoped;

	if (vUniform)
	{
		uniform = UniformVariant::create();

		name = vUniform->name;
		uniform->copyValues(vUniform);
		uniform->name = vUniform->name;
        uniform->nameForSearch = vUniform->nameForSearch;
		uniform->constant = vUniform->constant;
		uniform->glslType = vUniform->glslType;
		uniform->widget = vUniform->widget;
		uniform->count = vUniform->count;
		uniform->pipe = vUniform->pipe;
		uniform->canWeSave = vUniform->canWeSave;
		uniform->choices = vUniform->choices;
		uniform->parentMultiLoc = this;
		linkedUniforms[vUniform] = true;
	}
}

UniformsMultiLoc::~UniformsMultiLoc()
{
	ZoneScoped;

	uniform.reset();
}

// le vMainRenderPack est fait pour sync les gizmo dont on defini le courant dans le renderpack justement
// propagate ,propage les modification sur uniform par les utilisateur vers tout les uniforms enfants
void UniformsMultiLoc::Propagate(RenderPackWeak vMainRenderPack)
{
	ZoneScoped;

	for (auto itUni = linkedUniforms.begin(); itUni != linkedUniforms.end(); ++itUni)
	{
		// comme cela si un uniforms est jamais utilis� il sera rouge
		// si il est utilis� ne serait ce que pas un seul uniform il sera vert
		if (itUni->first->loc >= 0)
			uniform->loc = itUni->first->loc;

		itUni->first->copyValues(uniform, vMainRenderPack);
		itUni->first->parentMultiLoc = uniform->parentMultiLoc;
	}
}

// le vMainRenderPack est fait pour sync les gizmo dont on defini le courant dans le renderpakc justement
// sync, synchronize les modification des uniforms enfant, ( ceux qui dependt du soft et nom de l'interaction avec le user vers le parents 
void UniformsMultiLoc::Sync(RenderPackWeak vMainRenderPack)
{
	ZoneScoped;

	for (auto itUni = linkedUniforms.begin(); itUni != linkedUniforms.end(); ++itUni)
	{
		// comme cela si un uniforms est jamais utilis� il sera rouge
		// si il est utilis� ne serait ce que pas un seul uniform il sera vert
		if (itUni->first->loc >= 0)
			uniform->loc = itUni->first->loc;

		uniform->copyValues(itUni->first, vMainRenderPack);
	}
}