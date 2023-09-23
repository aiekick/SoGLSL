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

class MeshSaver
{
public:
	GuiBackend_Window puMeshSaverThread;

	enum MeshFormatEnum
	{
		MESH_FORMAT_PLY = 0,
		MESH_FORMAT_OBJ,
		MESH_FORMAT_GLTF,
		MESH_FORMAT_FGA,
		MESH_FORMAT_VOX,
		MESh_FORMAT_Count
	};

public:
	static std::mutex workerThread_Mutex;
	static std::atomic<double> Progress;
	static std::atomic<bool> Working;
	static std::atomic<double> GenerationTime;

public: // PLY
	static std::atomic<bool> exportNormals;
	static std::atomic<bool> exportTexCoords;
	static std::atomic<bool> exportVertexColor;
	static std::atomic<bool> exportFaces;

public: // FGA
	static std::atomic<int> countVectorX;
	static std::atomic<int> countVectorY;
	static std::atomic<int> countVectorZ;
	static std::atomic<float> minBoundX;
	static std::atomic<float> minBoundY;
	static std::atomic<float> minBoundZ;
	static std::atomic<float> maxBoundX;
	static std::atomic<float> maxBoundY;
	static std::atomic<float> maxBoundZ;

private:
	std::thread puWorkerThread;
	float puGenerationTime = 0.0f;
	std::function<void()> puFinishFunc;
	
public:
	std::vector<VertexStruct::P3_N3_T2_C4> vertices;
	std::vector<VertexStruct::I1> indices;
	std::vector<std::string> puLayouts;
	std::string fileToLoad;

public:
	std::string puFilePathName;
	std::string puFilePath;
	
public:
	static MeshSaver* Instance(const GuiBackend_Window& vRootWindow = GuiBackend_Window())
	{
		static MeshSaver _instance(vRootWindow);
		return &_instance;
	}

protected:
	MeshSaver(const GuiBackend_Window& vRootWindow); // Prevent construction
	MeshSaver(const MeshSaver&) {}; // Prevent construction by copying
	MeshSaver& operator =(const MeshSaver&) { return *this; }; // Prevent assignment
	~MeshSaver(); // Prevent unwanted destruction

public:
	void OpenDialog();
	void ShowDialog(ct::ivec2 vScreenSize);

	//void SaveObjFile(const std::string& vFilePathName, std::shared_ptr<ModelRendering> vModel);
	//void SaveGltfFile(const std::string& vFilePathName, std::shared_ptr<ModelRendering> vModel);
	//void SaveVoxFile(const std::string& vFilePathName, std::shared_ptr<ModelRendering> vModel);
	void SavePlyFile(const std::string& vFilePathName); // Stanford Ply File
	void SaveFgaFile(const std::string& vFilePathName); // VectorField File

	void DrawImGuiProgress(float vWidth = 150.0f);
	void DrawImGuiInfos(float vWidth = 150.0f);

public:
	void CreateThread(/*std::function<void()> vFinishFunc,*/ MeshFormatEnum vMeshFormat);
	//void CreateThread(MeshFormatEnum vMeshFormat);
	bool StopWorkerThread();
	bool IsJoinable();
	void Join();
	bool FinishIfRequired();

private:
	void UploadDatasToGpu();
};