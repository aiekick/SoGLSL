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

class FloatBuffer;

class ScreenGrabber {
public:
    static ScreenGrabber* Instance() {
        static ScreenGrabber _instance;
        return &_instance;
    }

protected:
    ScreenGrabber();                        // Prevent construction
    ScreenGrabber(const ScreenGrabber&){};  // Prevent construction by copying
    ScreenGrabber& operator=(const ScreenGrabber&) {
        return *this;
    };                 // Prevent assignment
    ~ScreenGrabber();  // Prevent unwanted destruction

public:  // save screenshot to file format
    bool SaveToPng(const GuiBackend_Window& vWin, const std::string& vFilePathName, int vSubSamplesCount, ct::fvec2 vNewSize);
    bool SaveToBmp(const GuiBackend_Window& vWin, const std::string& vFilePathName, int vSubSamplesCount, ct::fvec2 vNewSize);
    bool SaveToJpg(const GuiBackend_Window& vWin, const std::string& vFilePathName, int vSubSamplesCount, int vQualityFrom0To100, ct::fvec2 vNewSize);
    bool SaveToTga(const GuiBackend_Window& vWin, const std::string& vFilePathName, int vSubSamplesCount, ct::fvec2 vNewSize);

private:
    uint8_t* GetRGBABytesFromWindow(const GuiBackend_Window& vWin, int* vWidth, int* vHeight, int* vBufSize);
    std::shared_ptr<FloatBuffer> GetFloatBufferFromWindow_4_Chan(const GuiBackend_Window& vWin);
};
