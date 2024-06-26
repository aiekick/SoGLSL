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

#include <imgui.h>
#include <Headers/RenderPackHeaders.h>

class GuiModel {
private:
    ImGuiListClipper m_SceneClipper;
    int m_Selection = 0;
    ImGuiTableFlags m_Flags = ImGuiTableFlags_SizingFixedFit |
        // ImGuiTableFlags_RowBg |
        // ImGuiTableFlags_Hideable |
        ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoHostExtendY;

public:
    GuiModel();
    ~GuiModel();

    bool DrawGui(RenderPackWeak vParent, BaseModelWeak vBaseModelWeak, bool vCanOpenModel);
};
