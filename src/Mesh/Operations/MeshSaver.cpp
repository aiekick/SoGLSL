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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "MeshSaver.h"
#include <imgui.h>
#include <Gui/CustomGuiWidgets.h>
#include <Renderer/RenderPack.h>
#include <ImGuiPack.h>

using namespace std::placeholders;

#define fileSeek fseek
#define fileTell ftell

#ifndef IMGUI_DEFINE_MATH_OPERATORS

#endif
#include <imgui_internal.h>

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

std::atomic<double> MeshSaver::Progress(0.0);
std::atomic<bool> MeshSaver::Working(false);
std::atomic<double> MeshSaver::GenerationTime(0.0);
std::mutex MeshSaver::workerThread_Mutex;

std::atomic<bool> MeshSaver::exportNormals(true);
std::atomic<bool> MeshSaver::exportTexCoords(true);
std::atomic<bool> MeshSaver::exportVertexColor(true);
std::atomic<bool> MeshSaver::exportFaces(true);

std::atomic<int> MeshSaver::countVectorX(0);
std::atomic<int> MeshSaver::countVectorY(0);
std::atomic<int> MeshSaver::countVectorZ(0);
std::atomic<float> MeshSaver::minBoundX(-100.0f);
std::atomic<float> MeshSaver::minBoundY(-100.0f);
std::atomic<float> MeshSaver::minBoundZ(-100.0f);
std::atomic<float> MeshSaver::maxBoundX(100.0f);
std::atomic<float> MeshSaver::maxBoundY(100.0f);
std::atomic<float> MeshSaver::maxBoundZ(100.0f);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/*
// re compute normals
void AddTriangleAndComputeNormals(
	size_t vI0, size_t vI1, size_t vI2, 
	const size_t *v)
{
	MeshSaver::workerThread_Mutex.lock();

	VertexStruct::P3_N3_T2_C4 *v0 = 0, *v1 = 0, *v2 = 0, *v3 = 0;

	size_t n = MeshSaver::Instance()->vertices.size();

	if (v[0] < n) v0 = &MeshSaver::Instance()->vertices[v[vI0] - 1];
	if (v[1] < n) v1 = &MeshSaver::Instance()->vertices[v[vI1] - 1];
	if (v[2] < n) v2 = &MeshSaver::Instance()->vertices[v[vI2] - 1];

	if (v0 && v1 && v2)
	{
		ct::fvec3 vec0, vec1;

		vec0 = v0->p - v1->p;
		vec0.normalize();

		vec1 = v0->p - v2->p;
		vec1.normalize();

		ct::fvec3 nor = cCross(vec0, vec1).GetNormalized();

		v0->n += nor;
		v1->n += nor;
		v2->n += nor;
	}

	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI0] - 1)));
	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI1] - 1)));
	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI2] - 1)));

	MeshSaver::workerThread_Mutex.unlock();
}

void AddTriangleWithTexCoordsAndComputeNormals(
	size_t vI0, size_t vI1, size_t vI2, 
	const std::vector<ct::fvec2>& vTexCoords, 
	const size_t *v, const size_t *vt)
{
	MeshSaver::workerThread_Mutex.lock();

	VertexStruct::P3_N3_T2_C4 *v0 = 0, *v1 = 0, *v2 = 0, *v3 = 0;

	size_t n = MeshSaver::Instance()->vertices.size();

	if (v[0] < n) v0 = &MeshSaver::Instance()->vertices[v[vI0] - 1];
	if (v[1] < n) v1 = &MeshSaver::Instance()->vertices[v[vI1] - 1];
	if (v[2] < n) v2 = &MeshSaver::Instance()->vertices[v[vI2] - 1];

	if (v0 && v1 && v2)
	{
		ct::fvec3 vec0, vec1;

		vec0 = v0->p - v1->p;
		vec0.normalize();

		vec1 = v0->p - v2->p;
		vec1.normalize();

		ct::fvec3 nor = cCross(vec0, vec1).GetNormalized();

		v0->n += nor;
		v1->n += nor;
		v2->n += nor;

		// texcoords
		if (vt[0] < vTexCoords.size())
		{
			v0->t = vTexCoords[vt[vI0] - 1];
		}
		if (vt[1] < vTexCoords.size())
		{
			v1->t = vTexCoords[vt[vI1] - 1];
		}
		if (vt[2] < vTexCoords.size())
		{
			v2->t = vTexCoords[vt[vI2] - 1];
		}
	}

	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI0] - 1)));
	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI1] - 1)));
	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI2] - 1)));

	MeshSaver::workerThread_Mutex.unlock();
}

void AddTriangleWithNormals(
	size_t vI0, size_t vI1, size_t vI2, 
	const std::vector<ct::fvec3>& vNormals, 
	const size_t *v, const size_t *vn)
{
	MeshSaver::workerThread_Mutex.lock();

	VertexStruct::P3_N3_T2_C4 *v0 = 0, *v1 = 0, *v2 = 0, *v3 = 0;

	size_t n = MeshSaver::Instance()->vertices.size();

	if (v[0] < n) v0 = &MeshSaver::Instance()->vertices[v[vI0] - 1];
	if (v[1] < n) v1 = &MeshSaver::Instance()->vertices[v[vI1] - 1];
	if (v[2] < n) v2 = &MeshSaver::Instance()->vertices[v[vI2] - 1];

	if (v0 && v1 && v2)
	{
		if (vn[0] < vNormals.size())
		{
			v0->n = vNormals[vn[vI0] - 1];
		}
		if (vn[1] < vNormals.size())
		{
			v1->n = vNormals[vn[vI1] - 1];
		}
		if (vn[2] < vNormals.size())
		{
			v2->n = vNormals[vn[vI2] - 1];
		}
	}
	
	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI0] - 1)));
	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI1] - 1)));
	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI2] - 1)));

	MeshSaver::workerThread_Mutex.unlock();
}

void AddTriangleWithNormalsAndTexCoords(
	size_t vI0, size_t vI1, size_t vI2, 
	const std::vector<ct::fvec3>& vNormals, 
	const std::vector<ct::fvec2>& vTexCoords, 
	const size_t *v, const size_t *vn, const size_t *vt)
{
	MeshSaver::workerThread_Mutex.lock();

	VertexStruct::P3_N3_T2_C4 *v0 = 0, *v1 = 0, *v2 = 0, *v3 = 0;

	size_t n = MeshSaver::Instance()->vertices.size();

	if (v[0] < n) v0 = &MeshSaver::Instance()->vertices[v[vI0] - 1];
	if (v[1] < n) v1 = &MeshSaver::Instance()->vertices[v[vI1] - 1];
	if (v[2] < n) v2 = &MeshSaver::Instance()->vertices[v[vI2] - 1];

	if (v0 && v1 && v2)
	{
		// normals
		if (vn[0] < vNormals.size())
		{
			v0->n = vNormals[vn[vI0] - 1];
		}
		if (vn[1] < vNormals.size())
		{
			v1->n = vNormals[vn[vI1] - 1];
		}
		if (vn[2] < vNormals.size())
		{
			v2->n = vNormals[vn[vI2] - 1];
		}

		// texcoords
		if (vt[0] < vTexCoords.size())
		{
			v0->t = vTexCoords[vt[vI0] - 1];
		}
		if (vt[1] < vTexCoords.size())
		{
			v1->t = vTexCoords[vt[vI1] - 1];
		}
		if (vt[2] < vTexCoords.size())
		{
			v2->t = vTexCoords[vt[vI2] - 1];
		}
	}

	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI0] - 1)));
	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI1] - 1)));
	MeshSaver::Instance()->indices.emplace_back(VertexStruct::I1((uint32_t)(v[vI2] - 1)));

	MeshSaver::workerThread_Mutex.unlock();
}
*/
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

int MeshPlySaver(
	std::string vFilePathName,
	std::atomic<double>& vProgress,
	std::atomic<bool>& vWorking,
	std::atomic<double>& vGenerationTime,
	std::atomic<bool>& vExportNormals,
	std::atomic<bool>& vExportTexCoords,
	std::atomic<bool>& vExportVertexColor,
	std::atomic<bool>& vExportFaces)
{
	vWorking = true;

	vGenerationTime = 0.0;

	if (!vFilePathName.empty())
	{
		std::vector<VertexStruct::P3_N3_T2_C4> vertexs;
		size_t countVertexs = 0;
		std::vector<VertexStruct::I1> indices;
		size_t countIndices = 0;

		MeshSaver::workerThread_Mutex.lock();

		vertexs = MeshSaver::Instance()->vertices;
		countVertexs = vertexs.size();
		indices = MeshSaver::Instance()->indices;
		countIndices = indices.size();

		MeshSaver::workerThread_Mutex.unlock();

		std::string headerStr;
		std::string vertexsStr;
		//std::string normalsStr;
		std::string facesStr;

		char vertBuffer[1024];
		//char norBuffer[1024];
		char FaceBuffer[1024];

		//int faceId = 0;
		//int vertexId = 0;
		int n = 0;

		headerStr += "ply\n";
		headerStr += "format ascii 1.0\n";
		headerStr += "comment Created NoodlesPlate 1.0\n";
		headerStr += "element vertex " + ct::toStr(countVertexs) + "\n";
		headerStr += "property float x\n";
		headerStr += "property float y\n";
		headerStr += "property float z\n";

		if (vExportNormals)
		{
			headerStr += "property float nx\n";
			headerStr += "property float ny\n";
			headerStr += "property float nz\n";
		}

		if (vExportTexCoords)
		{
			headerStr += "property float s\n";
			headerStr += "property float t\n";
		}

		if (vExportVertexColor)
		{
			headerStr += "property uchar red\n";
			headerStr += "property uchar green\n";
			headerStr += "property uchar blue\n";
			headerStr += "property uchar alpha\n";
		}

		if (vExportFaces)
		{
			if (countIndices > 0)
			{
				headerStr += "element face " + ct::toStr(countIndices / 3) + "\n";
			}
			else
			{
				headerStr += "element face " + ct::toStr(countVertexs / 3) + "\n";
			}

			headerStr += "property list uchar uint vertex_indices\n";
		}

		headerStr += "end_header\n";

		if (countIndices > 0)
		{
			for (size_t i = 0; i < countVertexs; i++)
			{
				vProgress = (double)i / (double)(countVertexs - 1) * 0.5;

				if (!(vWorking != NULL)) break;

				int64_t firstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
					(std::chrono::system_clock::now().time_since_epoch()).count();

				VertexStruct::P3_N3_T2_C4 *vn = &vertexs.data()[i];

				// clamp color :
				vn->c = ct::clamp<ct::fvec4>(vn->c, 0.0f, 1.0f);
				
				if (vExportTexCoords)
				{
					if (vExportNormals)
					{
						if (vExportVertexColor)
						{
							n = snprintf(vertBuffer, 512, "%.5f %.5f %.5f %.5f %.5f %.5f %.5f %.5f %i %i %i %i\n",
								vn->p.x, vn->p.y, vn->p.z,
								vn->n.x, vn->n.y, vn->n.z,
								vn->t.x, vn->t.y,
								(int)(vn->c.x * 255), (int)(vn->c.y * 255), (int)(vn->c.z * 255), (int)(vn->c.w * 255));
						}
						else
						{
							n = snprintf(vertBuffer, 256, "%.5f %.5f %.5f %.5f %.5f %.5f %.5f %.5f\n",
								vn->p.x, vn->p.y, vn->p.z,
								vn->n.x, vn->n.y, vn->n.z,
								vn->t.x, vn->t.y);
						}
					}
					else
					{
						if (vExportVertexColor)
						{
							n = snprintf(vertBuffer, 512, "%.5f %.5f %.5f %.5f %.5f %i %i %i %i\n",
								vn->p.x, vn->p.y, vn->p.z,
								vn->t.x, vn->t.y,
								(int)(vn->c.x * 255), (int)(vn->c.y * 255), (int)(vn->c.z * 255), (int)(vn->c.w * 255));
						}
						else
						{
							n = snprintf(vertBuffer, 256, "%.5f %.5f %.5f %.5f %.5f\n",
								vn->p.x, vn->p.y, vn->p.z,
								vn->t.x, vn->t.y);
						}
					}
				}
				else
				{
					if (vExportNormals)
					{
						if (vExportVertexColor)
						{
							n = snprintf(vertBuffer, 512, "%.5f %.5f %.5f %.5f %.5f %.5f %i %i %i %i\n",
								vn->p.x, vn->p.y, vn->p.z,
								vn->n.x, vn->n.y, vn->n.z,
								(int)(vn->c.x * 255), (int)(vn->c.y * 255), (int)(vn->c.z * 255), (int)(vn->c.w * 255));
						}
						else
						{
							n = snprintf(vertBuffer, 256, "%.5f %.5f %.5f %.5f %.5f %.5f\n",
								vn->p.x, vn->p.y, vn->p.z,
								vn->n.x, vn->n.y, vn->n.z);
						}
					}
					else
					{
						if (vExportVertexColor)
						{
							n = snprintf(vertBuffer, 512, "%.5f %.5f %.5f %i %i %i %i\n",
								vn->p.x, vn->p.y, vn->p.z,
								(int)(vn->c.x * 255), (int)(vn->c.y * 255), (int)(vn->c.z * 255), (int)(vn->c.w * 255));
						}
						else
						{
							n = snprintf(vertBuffer, 256, "%.5f %.5f %.5f\n",
								vn->p.x, vn->p.y, vn->p.z);
						}
					}
				}

				vertexsStr += vertBuffer;

				int64_t secondTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
					(std::chrono::system_clock::now().time_since_epoch()).count();

				vGenerationTime = vGenerationTime + (double)(secondTimeMark - firstTimeMark) / 1000.0;
			}

			if (vWorking)
			{
				if (vExportFaces && countIndices > 0)
				{
					for (size_t i = 0; i < countIndices; i += 3)
					{
						vProgress = (double)i / (double)(countIndices - 1) * 0.5 + 0.5;

						if (!(vWorking != NULL)) break;

						int64_t firstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
							(std::chrono::system_clock::now().time_since_epoch()).count();

						n = snprintf(FaceBuffer, 256, "3 %i %i %i\n",
							indices[i + 0 + 0],
							indices[i + 0 + 1],
							indices[i + 0 + 2]
						);

						facesStr += FaceBuffer;

						int64_t secondTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
							(std::chrono::system_clock::now().time_since_epoch()).count();

						vGenerationTime = vGenerationTime + (secondTimeMark - firstTimeMark) / 1000.0f;
					}
				}
			}
		}
		else
		{
			for (size_t i = 0; i < countVertexs; i += 3)
			{
				vProgress = (double)i / (double)(countVertexs - 1);

				if (!(vWorking != NULL)) break;

				int64_t firstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
					(std::chrono::system_clock::now().time_since_epoch()).count();

				for (int k = 0; k < 3; k++)
				{
					VertexStruct::P3_N3_T2_C4 *vn = &vertexs.data()[i + k];

					// clamp color :
					vn->c = ct::clamp<ct::fvec4>(vn->c, 0.0f, 1.0f);

					if (vExportTexCoords)
					{
						if (vExportNormals)
						{
							if (vExportVertexColor)
							{
								n = snprintf(vertBuffer, 512, "%.5f %.5f %.5f %.5f %.5f %.5f %.5f %.5f %i %i %i %i\n",
									vn->p.x, vn->p.y, vn->p.z,
									vn->n.x, vn->n.y, vn->n.z,
									vn->t.x, vn->t.y,
									(int)(vn->c.x * 255), (int)(vn->c.y * 255), (int)(vn->c.z * 255), (int)(vn->c.w * 255));
							}
							else
							{
								n = snprintf(vertBuffer, 256, "%.5f %.5f %.5f %.5f %.5f %.5f %.5f %.5f\n",
									vn->p.x, vn->p.y, vn->p.z,
									vn->n.x, vn->n.y, vn->n.z,
									vn->t.x, vn->t.y);
							}
						}
						else
						{
							if (vExportVertexColor)
							{
								n = snprintf(vertBuffer, 512, "%.5f %.5f %.5f %.5f %.5f %i %i %i %i\n",
									vn->p.x, vn->p.y, vn->p.z,
									vn->t.x, vn->t.y,
									(int)(vn->c.x * 255), (int)(vn->c.y * 255), (int)(vn->c.z * 255), (int)(vn->c.w * 255));
							}
							else
							{
								n = snprintf(vertBuffer, 256, "%.5f %.5f %.5f %.5f %.5f\n",
									vn->p.x, vn->p.y, vn->p.z,
									vn->t.x, vn->t.y);
							}
						}
					}
					else
					{
						if (vExportNormals)
						{
							if (vExportVertexColor)
							{
								n = snprintf(vertBuffer, 512, "%.5f %.5f %.5f %.5f %.5f %.5f %i %i %i %i\n",
									vn->p.x, vn->p.y, vn->p.z,
									vn->n.x, vn->n.y, vn->n.z,
									(int)(vn->c.x * 255), (int)(vn->c.y * 255), (int)(vn->c.z * 255), (int)(vn->c.w * 255));
							}
							else
							{
								n = snprintf(vertBuffer, 256, "%.5f %.5f %.5f %.5f %.5f %.5f\n",
									vn->p.x, vn->p.y, vn->p.z,
									vn->n.x, vn->n.y, vn->n.z);
							}
						}
						else
						{
							if (vExportVertexColor)
							{
								n = snprintf(vertBuffer, 512, "%.5f %.5f %.5f %i %i %i %i\n",
									vn->p.x, vn->p.y, vn->p.z,
									(int)(vn->c.x * 255), (int)(vn->c.y * 255), (int)(vn->c.z * 255), (int)(vn->c.w * 255));
							}
							else
							{
								n = snprintf(vertBuffer, 256, "%.5f %.5f %.5f\n",
									vn->p.x, vn->p.y, vn->p.z);
							}
						}
					}

					vertexsStr += vertBuffer;
				}

				if (vExportFaces)
				{
					n = snprintf(FaceBuffer, 256, "3 %zi %zi %zi\n",
						i + 0 + 0,
						i + 0 + 1,
						i + 0 + 2
					);

					facesStr += FaceBuffer;
				}

				int64_t secondTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
					(std::chrono::system_clock::now().time_since_epoch()).count();

				vGenerationTime = vGenerationTime + (double)(secondTimeMark - firstTimeMark) / 1000.0;
			}
		}

		if (vWorking)
		{
			std::ofstream fileWriter(vFilePathName, std::ios::out);

			if (fileWriter.bad() == false)
			{
				fileWriter << headerStr;
				fileWriter << vertexsStr;

				if (vExportFaces)
				{
					fileWriter << facesStr;
				}
			}

			fileWriter.close();
		}
	}

	vWorking = false;

	return 0;
}

///////////////////////////////////////////////////////
//// SAVE TO FGA (VectorField) ////////////////////////
///////////////////////////////////////////////////////

int MeshFgaSaver(
	std::string vFilePathName,
	std::atomic<double>& vProgress,
	std::atomic<bool>& vWorking,
	std::atomic<double>& vGenerationTime,
	std::atomic<int>& vCountVectorX,
	std::atomic<int>& vCountVectorY,
	std::atomic<int>& vCountVectorZ,
	std::atomic<float> &minBoundX,
	std::atomic<float> &minBoundY,
	std::atomic<float> &minBoundZ,
	std::atomic<float> &maxBoundX,
	std::atomic<float> &maxBoundY,
	std::atomic<float> &maxBoundZ)
{
	vWorking = true;

	vGenerationTime = 0.0;

	if (!vFilePathName.empty())
	{
		ct::ivec3 count = { vCountVectorX, vCountVectorY, vCountVectorZ };
		ct::fvec3 minBoundingBox = { minBoundX, minBoundY, minBoundZ };
		ct::fvec3 maxBoundingBox = { maxBoundX, maxBoundY, maxBoundZ };
		
		if (!count.emptyAND())
		{
			size_t countVectorToExport = count.x * count.y * count.z;

			std::vector<VertexStruct::P3_N3_T2_C4> vertexs;
			size_t countVertexs = 0;

			MeshSaver::workerThread_Mutex.lock();

			vertexs = MeshSaver::Instance()->vertices;
			countVertexs = vertexs.size();

			MeshSaver::workerThread_Mutex.unlock();

			if (countVectorToExport <= countVertexs)
			{
				std::string headerStr;
				std::string vectorStr;

				char lineBuffer[1024];
				int n = 0;

				n = snprintf(lineBuffer, 1023, "%i,%i,%i\n", count.x, count.y, count.z); vectorStr += lineBuffer;
				n = snprintf(lineBuffer, 1023, ",%.5f,%.5f,%.5f\n", minBoundingBox.x, minBoundingBox.y, minBoundingBox.z); vectorStr += lineBuffer;
				n = snprintf(lineBuffer, 1023, ",%.5f,%.5f,%.5f\n", maxBoundingBox.x, maxBoundingBox.y, maxBoundingBox.z); vectorStr += lineBuffer;

				for (size_t i = 0; i < countVectorToExport; i++)
				{
					vProgress = (double)i / (double)(countVectorToExport - 1) * 0.5;

					if (!(vWorking != NULL)) break;

					int64_t firstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
						(std::chrono::system_clock::now().time_since_epoch()).count();

					VertexStruct::P3_N3_T2_C4 *vn = &vertexs.data()[i];

					n = snprintf(lineBuffer, 1023, ",%.5f,%.5f,%.5f\n", vn->n.x, vn->n.y, vn->n.z);

					vectorStr += lineBuffer;

					int64_t secondTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
						(std::chrono::system_clock::now().time_since_epoch()).count();

					vGenerationTime = vGenerationTime + (double)(secondTimeMark - firstTimeMark) / 1000.0;
				}

				if (vWorking)
				{
					std::ofstream fileWriter(vFilePathName, std::ios::out);

					if (fileWriter.bad() == false)
					{
						fileWriter << headerStr;
						fileWriter << vectorStr;
					}

					fileWriter.close();
				}
			}
		}
	}

	vWorking = false;

	return 0;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

MeshSaver::MeshSaver(const GuiBackend_Window& vRootWindow)
{
	if (vRootWindow.win)
	{
		puMeshSaverThread = GuiBackend::Instance()->CreateGuiBackendWindow_Hidden(1, 1, "MeshSaver", vRootWindow);
	}
	else
	{
		puMeshSaverThread = GuiBackend_Window();
	}

	puFilePath.clear();
	puGenerationTime = 0.0f;
}

MeshSaver::~MeshSaver()
{
	
}

// if vCantContinue is false, the user cant validate the dialog
inline void InfosPane(std::string vFilter, IGFDUserDatas /*vUserDatas*/, bool *vCantContinue) 
{
	if (vFilter == ".ply") // ply
	{
		bool val = MeshSaver::exportNormals;
		if (ImGui::Checkbox("Export Normals", &val)) { MeshSaver::exportNormals = val; }
		val = MeshSaver::exportTexCoords;
		if (ImGui::Checkbox("Export Tex Coords", &val)) { MeshSaver::exportTexCoords = val; }
		val = MeshSaver::exportVertexColor;
		if (ImGui::Checkbox("Export Vertex Color", &val)) { MeshSaver::exportVertexColor = val; }
		val = MeshSaver::exportFaces;
		if (ImGui::Checkbox("Export Faces", &val)) { MeshSaver::exportFaces = val; }
		if (vCantContinue)
			*vCantContinue = true;
	}
	else if (vFilter == ".fga") // fga
	{
		ImGui::Text("a FGA file is a vector field format\nUseable with unreal engine by ex\nOnly normals witl be exported");

		bool canGo = true;

		{
			ImGui::Text("Vector Field Size (Count Vectors) X Y Z :");
			ImGui::Indent();
			int val = MeshSaver::countVectorX;
			if (ImGui::InputInt("Count X", &val)) { MeshSaver::countVectorX = val; }
			val = MeshSaver::countVectorY;
			if (ImGui::InputInt("Count Y", &val)) { MeshSaver::countVectorY = val; }
			val = MeshSaver::countVectorZ;
			if (ImGui::InputInt("Count Z", &val)) { MeshSaver::countVectorZ = val; }
			ImGui::Unindent();

			if (MeshSaver::countVectorX <= 0 ||
				MeshSaver::countVectorY <= 0 ||
				MeshSaver::countVectorZ <= 0)
			{
				ImGui::TextColored(ImVec4(0.9f,0.1f,0.1f,1.0f), "The size of the vector field must be over 0 !");
				canGo = false;
			}
		}

		{
			ImGui::Text("Min Bounding Box X Y Z :");
			ImGui::Indent();
			float val = MeshSaver::minBoundX;
			if (ImGui::InputFloat("min X", &val)) { MeshSaver::minBoundX = val; }
			val = MeshSaver::minBoundY;
			if (ImGui::InputFloat("min Y", &val)) { MeshSaver::minBoundY = val; }
			val = MeshSaver::minBoundZ;
			if (ImGui::InputFloat("min Z", &val)) { MeshSaver::minBoundZ = val; }
			ImGui::Unindent();

			ImGui::Text("Max Bounding Box X Y Z :");
			ImGui::Indent();
			val = MeshSaver::maxBoundX;
			if (ImGui::InputFloat("max X", &val)) { MeshSaver::maxBoundX = val; }
			val = MeshSaver::maxBoundY;
			if (ImGui::InputFloat("max Y", &val)) { MeshSaver::maxBoundY = val; }
			val = MeshSaver::maxBoundZ;
			if (ImGui::InputFloat("max Z", &val)) { MeshSaver::maxBoundZ = val; }
			ImGui::Unindent();
		
			if (MeshSaver::maxBoundX - MeshSaver::minBoundX <= 0.0f ||
				MeshSaver::maxBoundY - MeshSaver::minBoundY <= 0.0f ||
				MeshSaver::maxBoundZ - MeshSaver::minBoundZ <= 0.0f)
			{
				ImGui::TextColored(ImVec4(0.9f, 0.1f, 0.1f, 1.0f), "the Bounding box volume (Max-Min) must be over 0 !");
				canGo = false;
			}
		}

		if (vCantContinue)
			*vCantContinue = canGo;
	}
}

void MeshSaver::OpenDialog()
{
	if (puFilePath.empty())
	{
		puFilePath = FileHelper::Instance()->GetRegisteredPath(
			(int)FILE_LOCATION_Enum::FILE_LOCATION_EXPORT);
    }
    IGFD::FileDialogConfig config;
    config.path = puFilePath;
    config.filePathName = puFilePathName;
    config.countSelectionMax = 1;
    config.sidePane = std::bind(&InfosPane, _1, _2, _3);
    config.sidePaneWidth = 350.0f;
    config.flags = ImGuiFileDialogFlags_DisableThumbnailMode | ImGuiFileDialogFlags_Modal;
	ImGuiFileDialog::Instance()->OpenDialog("SaveMeshFileDialog", "Save Mesh File", "Stanford Ply (*.ply){.ply},UE4 VectorField (*.fga){.fga}", config);
}

void MeshSaver::ShowDialog(ct::ivec2 vScreenSize)
{
	ImVec2 min = ImVec2(0, 0);
	ImVec2 max = ImVec2(FLT_MAX, FLT_MAX);
	if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
	{
		max = ImVec2((float)vScreenSize.x, (float)vScreenSize.y);
		min = max * 0.5f;
	}

	if (ImGuiFileDialog::Instance()->Display("SaveMeshFileDialog",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			puFilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			puFilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

			if (ImGuiFileDialog::Instance()->GetCurrentFilter() == "Stanford Ply (*.ply)") // ply mesh file format
			{
				SavePlyFile(puFilePathName);
			}
			else if (ImGuiFileDialog::Instance()->GetCurrentFilter() == "UE4 VectorField (*.fga)") // VectorField
			{
				SaveFgaFile(puFilePathName);
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}
}

void MeshSaver::SavePlyFile(const std::string& vFilePathName) // Stanford Ply File
{
	if (!vFilePathName.empty())
	{
		fileToLoad = vFilePathName;
		CreateThread(MeshFormatEnum::MESH_FORMAT_PLY);
	}
}

void MeshSaver::SaveFgaFile(const std::string& vFilePathName) // VectorField File
{
	if (!vFilePathName.empty())
	{
		fileToLoad = vFilePathName;
		CreateThread(MeshFormatEnum::MESH_FORMAT_FGA);
	}
}

void MeshSaver::DrawImGuiProgress(float vWidth)
{
	if (puWorkerThread.joinable())
	{
		if (MeshSaver::Progress > 0.0)
		{
			char timeBuffer[256];
			const double t = MeshSaver::GenerationTime;
			snprintf(timeBuffer, 256, "%.2lf s", t);
			const float pr = (float)MeshSaver::Progress;
			ImGui::ProgressBar(pr, ImVec2(vWidth, 0), timeBuffer);
			ImGui::SameLine();
		}

		if (ImGui::ContrastedButton("Stop Export"))
		{
			StopWorkerThread();
		}
	}

	FinishIfRequired();
}

void MeshSaver::DrawImGuiInfos(float /*vWidth*/)
{
	ImGui::TextWrapped("Model : %s", puFilePathName.c_str());

	int idx = 0;
	for (auto it = puLayouts.begin(); it != puLayouts.end(); ++it)
	{
		ImGui::TextWrapped("Attribute %i => %s", idx, (*it).c_str());
		idx++;
	}
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void MeshSaver::CreateThread(/*std::function<void()> vFinishFunc, */MeshFormatEnum vMeshFormat)
{
	if (!StopWorkerThread())
	{
		if (vMeshFormat == MeshFormatEnum::MESH_FORMAT_PLY)
		{
			//puFinishFunc = vFinishFunc;
			MeshSaver::Working = true;
			puWorkerThread =
				std::thread(
					MeshPlySaver,
					fileToLoad,
					std::ref(MeshSaver::Progress),
					std::ref(MeshSaver::Working),
					std::ref(MeshSaver::GenerationTime),
					std::ref(MeshSaver::exportNormals),
					std::ref(MeshSaver::exportTexCoords),
					std::ref(MeshSaver::exportVertexColor),
					std::ref(MeshSaver::exportFaces));
		}
		else if (vMeshFormat == MeshFormatEnum::MESH_FORMAT_FGA)
		{
			//puFinishFunc = vFinishFunc;
			MeshSaver::Working = true;
			puWorkerThread =
				std::thread(
					MeshFgaSaver,
					fileToLoad,
					std::ref(MeshSaver::Progress),
					std::ref(MeshSaver::Working),
					std::ref(MeshSaver::GenerationTime),
					std::ref(MeshSaver::countVectorX),
					std::ref(MeshSaver::countVectorY),
					std::ref(MeshSaver::countVectorZ),
					std::ref(MeshSaver::minBoundX),
					std::ref(MeshSaver::minBoundY),
					std::ref(MeshSaver::minBoundZ),
					std::ref(MeshSaver::maxBoundX),
					std::ref(MeshSaver::maxBoundY),
					std::ref(MeshSaver::maxBoundZ));
		}
	}
}

bool MeshSaver::StopWorkerThread()
{
	bool res = false;

	res = puWorkerThread.joinable();
	if (res)
	{
		MeshSaver::Working = false;
		puWorkerThread.join();
		puFinishFunc();
	}

	return res;
}

bool MeshSaver::IsJoinable()
{
	return puWorkerThread.joinable();
}

bool MeshSaver::FinishIfRequired()
{
	if (puWorkerThread.joinable() && !MeshSaver::Working)
	{
		puWorkerThread.join();
		if (puFinishFunc)
		{
			puFinishFunc();
			return true;
		}
	}

	return false;
}

void MeshSaver::Join()
{
	puWorkerThread.join();
}

