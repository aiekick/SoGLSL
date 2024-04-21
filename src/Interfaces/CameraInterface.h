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

#pragma warning(push)
#pragma warning(disable:4201)   // suppress even more warnings about nameless structs
#include <glm/mat4x4.hpp>
#pragma warning(pop)

class CameraInterface
{
public:
	glm::mat4 uView = glm::mat4(1.0f);
	glm::mat4 uProj = glm::mat4(1.0f);
	glm::mat4 uModel = glm::mat4(1.0f);
	glm::mat4 uCam = glm::mat4(1.0f);
	glm::mat4 uInvCam = glm::mat4(1.0f);
	glm::mat4 uNormalMatrix = glm::mat4(1.0f); // matrice pour afficher les normales
	glm::mat4 uSliceModel = glm::mat4(1.0f);
};