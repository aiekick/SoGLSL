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

#include <Uniforms/UniformVariant.h>
#include <Renderer/RenderPack.h>
#include <CodeTree/Parsing/SectionCode.h>

class Texture2D;
class Texture3D;
class TextureCube;
class TextureSound;
class TextureVideo;

class AssetManager
{
public:
	static AssetManager* Instance()
	{
		static AssetManager _instance;
		return &_instance;
	}

protected:
	AssetManager(); // Prevent construction
	AssetManager(const AssetManager&) {}; // Prevent construction by copying
	AssetManager& operator =(const AssetManager&) { return *this; }; // Prevent assignment
	~AssetManager(); // Prevent unwanted destruction

public:
	void Clear();

public:
	/*Texture2DPtr GetTexture2D(UniformParsedStruct *vUniformParsedStruct);
	Texture3DPtr GetTexture3D(UniformParsedStruct *vUniformParsedStruct);
	TextureCubePtr GetTextureCube(UniformParsedStruct *vUniformParsedStruct);
	TextureSoundPtr GetTextureSound(UniformParsedStruct *vUniformParsedStruct);
	TextureVideoPtr GetTextureVideo(UniformParsedStruct *vUniformParsedStruct);*/
};
