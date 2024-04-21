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
#include <Headers/RenderPackHeaders.h>

class PingPong
{
public:
	ctTexturePtr puFrontTex = nullptr;
	ctTexturePtr puBackTex = nullptr;

public:
	ctTexturePtr getFront() { if (puFrontTex) return puFrontTex; return ctTexturePtr(); }
	ctTexturePtr getBack() { if (puBackTex) return puBackTex; return ctTexturePtr(); }
	std::string getFilePathName() { if (puBackTex) return puBackTex->relativPath; return ""; }

public:
	PingPong();
	virtual ~PingPong();

	void clean();
	void Swap(); // seulement pour le compute
};
