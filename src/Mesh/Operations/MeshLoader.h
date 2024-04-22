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
#include <ctools/cTools.h>

#include <future>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>

#include <Mesh/Model/BaseModel.h>
#include <Mesh/Utils/VertexStruct.h>

#include <functional>

class RenderPack;

class MeshLoader {
public:
    GuiBackend_Window puMeshLoaderThread;

public:
    static std::mutex workerThread_Mutex;
    static std::atomic<double> Progress;
    static std::atomic<bool> Working;
    static std::atomic<double> GenerationTime;

private:
    std::thread puWorkerThread;
    float puGenerationTime = 0.0f;
    std::function<void()> puFinishFunc;
    RenderPackWeak puRenderPack;

public:
    std::vector<std::string> puLayouts;
    std::string fileToLoad;
    BaseModelWeak puLastModel;
    uint32_t subMeshCount = 0U;

public:
    std::string puFilePathName;
    std::string puFilePath;

public:
    void Clear();

    void OpenDialog(RenderPackWeak vParent);
    void ShowDialog(ct::ivec2 vScreenSize);

    void LoadFile(RenderPackWeak vParent, const std::string& vFilePathName);

    void DrawImGuiProgress(float vWidth = 150.0f);
    void DrawImGuiInfos(float vWidth = 150.0f);

    bool FinishIfRequired(bool vForce = false);

private:
    void CreateThread(std::function<void()> vFinishFunc);
    void CreateThread();
    bool StopWorkerThread();
    bool IsJoinable();
    void Join();
    void UploadDatasToGpu();

public:
    static MeshLoader* Instance(const GuiBackend_Window& vRootWindow = GuiBackend_Window()) {
        static MeshLoader _instance(vRootWindow);
        return &_instance;
    }

protected:
    MeshLoader(const GuiBackend_Window& vRootWindow);  // Prevent construction
    MeshLoader(const MeshLoader&){};                   // Prevent construction by copying
    MeshLoader& operator=(const MeshLoader&) {
        return *this;
    };              // Prevent assignment
    ~MeshLoader();  // Prevent unwanted destruction
};
