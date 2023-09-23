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

#include <Headers/RenderPackHeaders.h>
#include <string>

enum class RenderingModeEnum {    //
    RENDERING_MODE_PICTURES = 0,  //
    RENDERING_MODE_VIDEO,         //
    RENDERING_MODE_GIF,           //
    RENDERING_MODE_HEIC,          //
    RENDERING_MODE_Count          //
};                                //

class RenderingInterface {
protected:
    std::string puRenderingPath;                                                     // donne le chemin de stockage
    std::string puRenderingPrefix;                                                   // donne le prefix des fichier rendus
    std::string puRenderingFileName;                                                 // donne le nom de fichier
    bool puRendering                  = false;                                       // dit si on est en mode rendu
    RenderingModeEnum puRenderingMode = RenderingModeEnum::RENDERING_MODE_PICTURES;  // donne le type de rendu

public:
    virtual RenderingModeEnum GetRenderingMode() {
        return puRenderingMode;
    }
    virtual std::string GetRenderingPrefix() {
        return puRenderingPrefix;
    }
    virtual void SetRenderingPath(const std::string& vPath) {
        puRenderingPath = vPath;
    }
    virtual std::string GetRenderingPath() {
        return puRenderingPath;
    }
    virtual bool IsRendering() {
        return puRendering;
    }
    virtual void UseRendering() {
        puRendering = true;
    }

public:
    virtual std::string GetRenderingFilePathNameForCurrentFrame() = 0;  //
};
