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

#include <string>
#include <Headers/RenderPackHeaders.h>

class SystemInterface {
public:
    virtual bool Use(const std::string& vKey) = 0;
    virtual void Render(const float& vDeltaTime, UpdateUniformFuncSignature vSpecificUniformsFunc, const DisplayQualityType& vDisplayQuality,
                        MouseInterface* vMouse, const ct::ivec2& vScreenSize, MeshRectType* vMouseRect, GLenum* vRenderMode,
                        FrameBuffersPipeLinePtr vPipe) = 0;
    virtual void DisplayMessageOfRenderPack(const GuiBackend_Window& vWin, const bool& vHideWarnings, CodeTreePtr vCodeTree,
                                            const bool& vShowCode, const bool& vUseTextEditor,
                                            const bool& vShowEditBtn) = 0;
    virtual bool IsTheseSomeErrors() = 0;
    virtual bool IsTheseSomeWarnings() = 0;
    virtual bool DrawUniformPane() = 0;
    virtual bool DrawConfigPane() = 0;

};