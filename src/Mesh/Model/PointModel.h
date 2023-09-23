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

#include <Mesh/Model/BaseModel.h>

class PointModel;
typedef std::shared_ptr<PointModel> PointModelPtr;
typedef std::weak_ptr<PointModel> PointModelWeak;

class PointModel : public BaseModel
{
private:
	BaseMeshDatas<float> m_MeshDatas;

public:
	static PointModelPtr Create(const GuiBackend_Window& vWin, const uint64_t& vVerticesCount);

protected:
	PointModelWeak m_This;

public:
	PointModel(const GuiBackend_Window& vWin, const uint64_t& vVerticesCount);
	~PointModel() override;

	void Clear() override;
	bool IsValid() override;

	void DrawModel(const std::string& vName, const GLenum& vRenderMode = GL_TRIANGLES, const bool& vUseTesselation = false) override;
	void SetVerticesCount(uint64_t vVerticesCount) override;

	uint32_t GetVaoID(uint32_t vMeshID = 0U) override;
	uint32_t GetVboID(uint32_t vMeshID = 0U) override;
	uint32_t GetIboID(uint32_t vMeshID = 0U) override;

private:
	bool PreparePoint(uint64_t vVerticesCount, bool vUpdate);
};