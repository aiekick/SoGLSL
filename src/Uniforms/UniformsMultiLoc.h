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

#include <Uniforms/UniformVariant.h>
#include <memory>

class RenderPack;
class UniformsMultiLoc
{
public:
	std::string name;
	UniformVariantPtr uniform = nullptr;
	std::unordered_map<UniformVariantPtr, bool> linkedUniforms;

public:
	UniformsMultiLoc(UniformVariantPtr vUniform);
	~UniformsMultiLoc();

	// le vMainRenderPack est fait pour sync les gizmo dont on defini le courant dans le renderpakc justement
	// propagate ,propage les modification sur uniform par les utilisateur vers tout les uniforms enfants
	void Propagate(RenderPackWeak vMainRenderPack = RenderPackWeak());

	// le vMainRenderPack est fait pour sync les gizmo dont on defini le courant dans le renderpakc justement
	// sync, synchronize les modification des uniforms enfant, ( ceux qui dependt du soft et nom de l'interaction avec le user vers le parents 
	void Sync(RenderPackWeak vMainRenderPack = RenderPackWeak());
};
