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

#include "pingpong.h"

PingPong::PingPong()
{
	puFrontTex = nullptr;
	puBackTex = nullptr;
}
PingPong::~PingPong()
{
	clean();
}

void PingPong::clean()
{
	if (puFrontTex && puFrontTex->glTex > 0)
	{
		glBindTexture(puFrontTex->glTextureType, 0);
		glDeleteTextures(1, &puFrontTex->glTex);
		puFrontTex->glTex = 0;
		puFrontTex.reset();
	}

	if (puBackTex && puBackTex->glTex > 0)
	{
		glBindTexture(puBackTex->glTextureType, 0);
		glDeleteTextures(1, &puBackTex->glTex);
		puBackTex->glTex = 0;
		puBackTex.reset();
	}
}

void PingPong::Swap() // seulement pour le compute
{
	ctTexturePtr tex = puBackTex;
	puBackTex = puFrontTex;
	puFrontTex = tex;
}
