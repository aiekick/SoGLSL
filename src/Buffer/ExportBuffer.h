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

#include <Headers/RenderPackHeaders.h>

#include <string>
#include <vector>

#include <Mesh/Model/BaseModel.h>
#include <Mesh/Utils/VertexStruct.h>

// Transform Feedback export
// http://www.java-gaming.org/topics/opengl-transform-feedback/27786/view.html
// https://community.khronos.org/t/transform-feedback-tutorial-not-getting-the-desired-output/76581
// https://github.com/progschj/OpenGL-Examples/blob/master/09transforpufeedback.cpp => a compiler pour voir

class ExportBuffer
{
public:
	std::vector<VertexStruct::P3_N3_T2_C4> vertices;
	std::vector<VertexStruct::I1> indices;
	std::vector<std::string> puLayouts;
	std::string fileToLoad;

public:
	bool puNeedCapture = false;
	GLuint queryTBO = 0;
	GLuint queryIBO = 0;
	GLuint ibo = 0;
	GLuint tbo = 0;
	GLuint fdbo = 0;
	size_t puCapturedCountPoints = 0;
	size_t puCapturedCountIndices = 0;
	size_t puMaxCountPoints = 50000;

public:
	std::string puFilePathName;
	std::string puFilePath;

public:
	ExportBuffer(); // Prevent construction
	~ExportBuffer(); // Prevent unwanted destruction

public:
	void DrawImGui(const GuiBackend_Window& vWin, BaseModelWeak vModel);

public:
	void Clear();
	void InitBuffer(BaseModelWeak vModel);
	size_t GetCountPoints();
	void StartCapture();
	void BeforeCapture();
	void Capture(BaseModelWeak vModel, const GLuint& vRenderMode);
	void AfterCapture(const GLuint& vRenderMode);
	void ExportToFile();

};
