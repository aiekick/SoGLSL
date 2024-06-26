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

#include "AssetManager.h"

AssetManager::AssetManager() {
}

AssetManager::~AssetManager() {
    Clear();
}

void AssetManager::Clear() {
}

/*
Texture2DPtr AssetManager::GetTexture2D(UniformParsedStruct *vUniformParsedStruct)
{
    return nullptr;
}

Texture3DPtr AssetManager::GetTexture3D(UniformParsedStruct* vUniformParsedStruct)
{
    return nullptr;
}

TextureCubePtr AssetManager::GetTextureCube(UniformParsedStruct* vUniformParsedStruct)
{
    return nullptr;
}

TextureSound* AssetManager::GetTextureSound(UniformParsedStruct* vUniformParsedStruct)
{
    return nullptr;
}

TextureVideo* AssetManager::GetTextureVideo(UniformParsedStruct* vUniformParsedStruct)
{
    return nullptr;
}
*/