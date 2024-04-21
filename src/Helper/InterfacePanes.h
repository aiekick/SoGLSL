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

#include <ImGuiPack.h>
#include <Headers/RenderPackHeaders.h>
#include <Gui/GuiBackend.h>

class RenderPack;
class SectionCode;
class CodeTree;

class InterfacePanes
{
public:
	TextEditor puTextEditor;
	std::string puCodeContent;

public:
	bool puForceRefreshCode = false;
	int puShowCodeSection = 0;
	bool puShowCodeTreeStruct = false;

public:
	static InterfacePanes* Instance()
	{
		static InterfacePanes _instance;
		return &_instance;
	}

protected:
	InterfacePanes(); // Prevent construction
	InterfacePanes(const InterfacePanes&) {}; // Prevent construction by copying
	InterfacePanes& operator =(const InterfacePanes&) { return *this; }; // Prevent assignment
	~InterfacePanes(); // Prevent unwanted destruction

public:
	void DisplayMessageOfRenderPack(const GuiBackend_Window& vWin, const bool& vHideWarnings, CodeTreePtr vCodeTree, 
		RenderPackWeak vRenderPack, const bool& vShowCode, const bool& vUseTextEditor, const bool& vShowEditBtn);
	void DisplayScriptHelp(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack);
	void DisplayOptiPane(const GuiBackend_Window& vWin, CodeTreePtr vCodeTree, RenderPackWeak vRenderPack);
	void DisplayScriptCode(const GuiBackend_Window& vWin, CodeTreePtr vCodeTree, RenderPackWeak vRenderPack, const bool& vUseTextEditor);
	void DisplayShaderNote(CodeTreePtr vCodeTree, RenderPackWeak vRenderPack);

	void DisplayCodeTree(std::string vSectionNameForErrorsLines, RenderPackWeak vRenderPack);
	void DisplayCodeTreeSectionRecurs(std::shared_ptr<SectionCode> vSectionCode, std::string vSectionNameForErrorsLines);

	void DrawDebugPane(CodeTreePtr vCodeTree);
};