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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "MeshLoader.h"

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/version.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/Exceptional.h>
#include <assimp/ProgressHandler.hpp>
#include <../code/Common/ScenePrivate.h>

#include <imgui.h>
#include <ctools/Logger.h>
#include <Renderer/RenderPack.h>
#include <Gui/CustomGuiWidgets.h>
#include <Mesh/Model/PNTBTCMesh.h>
#include <Mesh/Model/PNTBTCModel.h>
#include <ImGuiPack.h>

#include <imgui_internal.h>

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

std::atomic<double> MeshLoader::Progress(0.0);
std::atomic<bool> MeshLoader::Working(false);
std::atomic<double> MeshLoader::GenerationTime(0.0);
std::mutex MeshLoader::workerThread_Mutex;

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

class MeshLoaderProgresshandler : public Assimp::ProgressHandler
{
public:
	bool Update(float percentage) override
	{
		if (percentage > 0.0f)
		{
			MeshLoader::Progress = (double)percentage;
		}
		return MeshLoader::Working; // break loading at next possible occasion
	}
};

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

inline static void sLoadMesh(
	std::atomic< double >& vProgress,
	std::atomic< bool >& vWorking,
	std::atomic< double >& vGenerationTime)
{
	vProgress = 0.0;

	vWorking = true;

	vGenerationTime = 0.0f;

	MeshLoader::workerThread_Mutex.lock();
	const std::string filePathName = MeshLoader::Instance()->fileToLoad;
	auto meshPtr = std::dynamic_pointer_cast<PNTBTCModel>(MeshLoader::Instance()->puLastModel.lock());
	MeshLoader::workerThread_Mutex.unlock();

	std::vector<std::string> layouts;
	layouts.emplace_back("Vertex (v3) => Empty");
	layouts.emplace_back("Normal (v3) => Empty");
	layouts.emplace_back("Tangent (v3) => Empty");
	layouts.emplace_back("Bi-Tangent (v3) => Empty");
	layouts.emplace_back("Tex Coord (v2) => Empty");
	layouts.emplace_back("Color (v4) => Empty");

	if (!filePathName.empty() && meshPtr)
	{
		MeshLoader::workerThread_Mutex.lock();
		meshPtr->Clear();
		meshPtr->SetFilePathName(filePathName);
		MeshLoader::workerThread_Mutex.unlock();

		try
		{
			uint32_t assimpFlags = aiProcess_CalcTangentSpace |
				//aiProcess_SortByPType |
				//aiProcess_JoinIdenticalVertices |
				aiProcess_Triangulate;

			const aiScene* scene = nullptr;

			MeshLoaderProgresshandler progresshandler;
			Assimp::Importer* imp = new Assimp::Importer();
			imp->SetProgressHandler(&progresshandler);

			const int64_t firstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
				(std::chrono::system_clock::now().time_since_epoch()).count();

			// and have it read the file
			scene = imp->ReadFile(filePathName.c_str(), assimpFlags);

			const int64_t secondTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
				(std::chrono::system_clock::now().time_since_epoch()).count();

			vGenerationTime = vGenerationTime + (double)(secondTimeMark - firstTimeMark) / 1000.0;

			// if succeeded, store the importer in the scene and keep it alive
			if (scene)
			{
				Assimp::ScenePrivateData* priv = const_cast<Assimp::ScenePrivateData*>(Assimp::ScenePriv(scene));
				priv->mOrigImporter = imp;
				imp->SetProgressHandler(nullptr);

				if (scene->HasMeshes())
				{
					//size_t _last_index_offset = 0U;

					MeshLoader::workerThread_Mutex.lock();
					MeshLoader::Instance()->puLayouts = layouts;
					MeshLoader::Instance()->subMeshCount = scene->mNumMeshes;
					MeshLoader::workerThread_Mutex.unlock();

					const double progressSteps = 1.0 / (double)scene->mNumMeshes;

					for (size_t k = 0; k != scene->mNumMeshes; ++k)
					{
						const aiMesh* mesh = scene->mMeshes[k];

						if (!vWorking)
							break;

						if (mesh)
						{
							auto sceneMeshPtr = PNTBTCMesh::Create();
							if (sceneMeshPtr)
							{
								sceneMeshPtr->GetVertices()->reserve(mesh->mNumVertices);

								if (mesh->mVertices)
								{
									layouts[0] = "Vertex (v3)";
								}

								if (mesh->mNormals)
								{
									layouts[1] = "Normal (v3)";
									sceneMeshPtr->HaveNormals();
								}
								if (mesh->mTangents)
								{
									layouts[2] = "Tangent (v3)";
									sceneMeshPtr->HaveTangeants();
								}
								if (mesh->mBitangents)
								{
									layouts[3] = "Bi-Tangent (v3)";
									sceneMeshPtr->HaveBiTangeants();
								}
								if (mesh->mTextureCoords)
								{
									if (mesh->mNumUVComponents[0] == 2U)
									{
										if (mesh->mTextureCoords[0])
										{
											layouts[4] = "Tex Coord (v2)";
											sceneMeshPtr->HaveTextureCoords();
										}
									}
								}
								if (mesh->mColors)
								{
									if (mesh->mColors[0])
									{
										layouts[5] = "Color (v4)";
										sceneMeshPtr->HaveVertexColors();
									}
								}

								VertexStruct::P3_N3_TA3_BTA3_T2_C4 v;

								const double progressVerts = 1.0 / (double)mesh->mNumVertices;

								for (size_t i = 0; i != mesh->mNumVertices; ++i)
								{
									const int64_t _firstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
										(std::chrono::system_clock::now().time_since_epoch()).count();

									if (!vWorking)
										break;

									v = VertexStruct::P3_N3_TA3_BTA3_T2_C4();

									const auto& vert = mesh->mVertices[i];
									v.p = ct::fvec3(vert.x, vert.y, vert.z);

									if (mesh->mNormals)
									{
										const auto& norm = mesh->mNormals[i];
										v.n = ct::fvec3(norm.x, norm.y, norm.z);
									}

									if (mesh->mTangents)
									{
										const auto& tang = mesh->mTangents[i];
										v.tan = ct::fvec3(tang.x, tang.y, tang.z);
									}

									if (mesh->mBitangents)
									{
										const auto& btan = mesh->mBitangents[i];
										v.btan = ct::fvec3(btan.x, btan.y, btan.z);
									}

									if (mesh->mTextureCoords)
									{
										if (mesh->mNumUVComponents[0] == 2U)
										{
											if (mesh->mTextureCoords[0])
											{
												const auto& coor = mesh->mTextureCoords[0][i];
												v.t = ct::fvec2(coor.x, coor.y);
											}
										}
									}

									if (mesh->mColors)
									{
										if (mesh->mColors[0])
										{
											const auto& colo = mesh->mColors[0][i];
											v.c = ct::fvec4(colo.r, colo.g, colo.b, colo.a);
										}
									}

									sceneMeshPtr->GetVertices()->push_back(v);

									vProgress = vProgress + progressVerts;

									const int64_t _secondTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
										(std::chrono::system_clock::now().time_since_epoch()).count();

									vGenerationTime = vGenerationTime + (double)(_secondTimeMark - _firstTimeMark) / 1000.0;
								}

								sceneMeshPtr->GetIndices()->reserve(mesh->mNumFaces * 3U);
								if (mesh->mNumFaces) sceneMeshPtr->HaveIndices();
								for (size_t i = 0; i != mesh->mNumFaces; ++i)
								{
									if (!vWorking)
										break;

									const int64_t _firstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
										(std::chrono::system_clock::now().time_since_epoch()).count();

									const aiFace& face = mesh->mFaces[i];
									for (size_t j = 0; j != face.mNumIndices; ++j)
									{
										if (!vWorking)
											break;

										sceneMeshPtr->GetIndices()->push_back(face.mIndices[j]);
									}

									const int64_t _secondTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
										(std::chrono::system_clock::now().time_since_epoch()).count();

									vGenerationTime = vGenerationTime + (double)(_secondTimeMark - _firstTimeMark) / 1000.0;
								}

								//if (sceneMeshPtr->Init())
								{
									MeshLoader::workerThread_Mutex.lock();
									meshPtr->AddMesh(sceneMeshPtr);
									MeshLoader::workerThread_Mutex.unlock();
								}
								/*else
								{
									sceneMeshPtr.reset();
									LogVarError("Failed to build the mesh %u VBO/IBO for %s", (uint32_t)k, filePathName.c_str());
								}*/
							}

							vProgress = vProgress + progressSteps;
						}
					}

					MeshLoader::workerThread_Mutex.lock();
					meshPtr->SetLayouts(layouts);
					MeshLoader::workerThread_Mutex.unlock();
				}

				aiReleaseImport(scene);
			}
			else
			{
				// if failed, extract error code and destroy the import
				LogVarError(imp->GetErrorString());
				imp->SetProgressHandler(nullptr);
				delete imp;

				LogVarError("Cant import the file %s", filePathName.c_str());
				meshPtr->Clear();
			}
		}
		catch (const DeadlyImportError& e)
		{
			LogVarError(e.what());
			meshPtr->Clear();
		}
		catch (const std::exception& ex)
		{
			LogVarError(ex.what());
			meshPtr->Clear();
		}
		catch (...)
		{
			LogVarError("Unknown assimp exception");
			meshPtr->Clear();
		}
	}
	
	vWorking = false;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

MeshLoader::MeshLoader(const GuiBackend_Window& vRootWindow)
{
	if (vRootWindow.win)
	{
		puMeshLoaderThread = GuiBackend::Instance()->CreateGuiBackendWindow_Hidden(1, 1, "MeshLoader", vRootWindow);
	}
	else
	{
		puMeshLoaderThread = GuiBackend_Window();
	}

	puFilePath.clear();
	puGenerationTime = 0.0f;
}

MeshLoader::~MeshLoader()
{
	Clear();
}

void MeshLoader::Clear()
{
	puLayouts.clear();
}

void MeshLoader::OpenDialog(RenderPackWeak vRenderpack)
{
	if (vRenderpack.expired()) return;
	auto rpPtr = vRenderpack.lock();
	if (!rpPtr) return;

	if (puFilePath.empty())
	{
		puFilePath = FileHelper::Instance()->GetRegisteredPath(
			(int)FILE_LOCATION_Enum::FILE_LOCATION_ASSET_MESH);
	}
	puRenderPack = vRenderpack;
    puLastModel = rpPtr->GetModel();
    IGFD::FileDialogConfig config;
    config.path = puFilePath;
    config.filePathName = puFilePathName;
    config.countSelectionMax = 1;
    config.flags = ImGuiFileDialogFlags_DisableThumbnailMode | ImGuiFileDialogFlags_Modal;
    ImGuiFileDialog::Instance()->OpenDialog("OpenMeshFileDialog", "Open Mesh File", "3D Files {.obj,.ply,.gltf,.fbx,.stl,.3ds,.dae,.lwo,.blend}",
                                            config);
}

void MeshLoader::ShowDialog(ct::ivec2 vScreenSize)
{
	ImVec2 min = ImVec2(0, 0);
	ImVec2 max = ImVec2(FLT_MAX, FLT_MAX);
	if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
	{
		max = ImVec2((float)vScreenSize.x, (float)vScreenSize.y);
		min = max * 0.5f;
	}

	if (ImGuiFileDialog::Instance()->Display("OpenMeshFileDialog",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			puFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			puFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			LoadFile(puRenderPack, puFilePathName);
		}
		ImGuiFileDialog::Instance()->Close();
	}
}

void MeshLoader::LoadFile(RenderPackWeak vParent, const std::string& vFilePathName)
{
	if (!vFilePathName.empty() && !vParent.expired())
	{
		fileToLoad = vFilePathName;
		puRenderPack = vParent;
		auto rpPtr = puRenderPack.lock();
		if (rpPtr)
		{
			puLastModel = rpPtr->GetModel();
			if (!puLastModel.expired())
			{
				CreateThread();
			}
		}
	}
}

void MeshLoader::DrawImGuiProgress(float vWidth)
{
	if (puWorkerThread.joinable())
	{
		if (MeshLoader::Progress > 0.0)
		{
			char timeBuffer[256];
			const double t = MeshLoader::GenerationTime;
			snprintf(timeBuffer, 256, "%.2lf s", t);
			const float pr = (float)MeshLoader::Progress;
			ImGui::ProgressBar(pr, ImVec2(vWidth, 0), timeBuffer);
			ImGui::SameLine();
		}

		if (ImGui::ContrastedButton("Stop Loading"))
		{
			MeshLoader::Working = false;
		}
	}
}

void MeshLoader::DrawImGuiInfos(float /*vWidth*/)
{
	ImGui::TextWrapped("Model : %s", puFilePathName.c_str());

	int idx = 0;
	for (auto it = puLayouts.begin(); it != puLayouts.end(); ++it)
	{
		ImGui::TextWrapped("Attribute %i => %s", idx, (*it).c_str());
		idx++;
	}

	ImGui::TextWrapped("Sub meshs count : %u", subMeshCount);
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void MeshLoader::CreateThread(std::function<void()> vFinishFunc)
{
	if (!StopWorkerThread())
	{
		auto modelPtr = std::dynamic_pointer_cast<PNTBTCModel>(puLastModel.lock());
		if (modelPtr)
		{
			puFinishFunc = vFinishFunc;
			MeshLoader::Working = true;
			puWorkerThread =
				std::thread(
					sLoadMesh,
					//puMeshLoaderThread,
					//modelPtr,
					std::ref(MeshLoader::Progress),
					std::ref(MeshLoader::Working),
					std::ref(MeshLoader::GenerationTime)
				);
		}
		else
		{
			CTOOL_DEBUG_BREAK;
			LogVarError("Bad Model Format");
		}
	}
}

void MeshLoader::CreateThread()
{
	CreateThread(std::bind(&MeshLoader::UploadDatasToGpu, this));
}

bool MeshLoader::StopWorkerThread()
{
	bool res = false;

	res = puWorkerThread.joinable();
	if (res)
	{
		MeshLoader::Working = false;
		puWorkerThread.join();
		puFinishFunc();
	}

	return res;
}

bool MeshLoader::IsJoinable()
{
	return puWorkerThread.joinable();
}

bool MeshLoader::FinishIfRequired(bool vForce)
{
	if (puWorkerThread.joinable())
	{
		if (vForce)
		{
			
		}

		if (!MeshLoader::Working)
		{
			Join();
			puFinishFunc();
			return true;
		}
	}
	return false;
}

void MeshLoader::Join()
{
	puWorkerThread.join();
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void MeshLoader::UploadDatasToGpu()
{
	if (!puLastModel.expired())
	{
		auto modelPtr = puLastModel.lock();
		if (modelPtr)
		{
			modelPtr->ReLoadModel();
		}

		puLastModel.reset();
	}
}
