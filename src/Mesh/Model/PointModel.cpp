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

#include "PointModel.h"

#include <cassert>
#include <ctools/Logger.h>
#include <ImGuiPack.h>
#include <InAppGpuProfiler/iagp.h>
#include <Profiler/TracyProfiler.h>
#include <Mesh/Utils/VertexStruct.h>

PointModelPtr PointModel::Create(const GuiBackend_Window& vWin, const uint64_t& vVerticesCount) {
    auto res = std::make_shared<PointModel>(vWin, vVerticesCount);
    res->m_This = res;
    return res;
}

PointModel::PointModel(const GuiBackend_Window& vWin, const uint64_t& vVerticesCount) : BaseModel(vWin) {
    m_MeshType = BaseMeshEnum::PRIMITIVE_TYPE_POINTS;
    m_IsLoaded = PreparePoint(ct::maxi<uint64_t>(vVerticesCount, 1U), false);
}

PointModel::~PointModel() {
    Clear();
}

void PointModel::Clear() {
    m_MeshDatas.Clear(m_Window);
}

bool PointModel::IsValid() {
    return m_IsLoaded;
}

void PointModel::DrawModel(const std::string& vName, const GLenum& vRenderMode, const bool& vUseTesselation) {
    GuiBackend::MakeContextCurrent(m_Window);

    AIGPScoped(vName, "Draw Points");

    TracyGpuZone("PointModel::DrawModel");

    if (glIsVertexArray(m_MeshDatas.m_Vao) == GL_TRUE) {
        // bind
        glBindVertexArray(m_MeshDatas.m_Vao);  // select first VAO
        LogGlError();
        glEnableVertexAttribArray(0);
        LogGlError();

        if (vUseTesselation && m_PatchVerticesCount && glPatchParameteri)
            glPatchParameteri(GL_PATCH_VERTICES, (GLint)m_PatchVerticesCount);

        // draw
        if (m_InstanceCount > 1U) {
            glDrawArraysInstanced(vRenderMode, 0, (GLsizei)m_VerticesCount, (GLsizei)m_InstanceCount);  // draw first object 6 => decalage 3 coord * 2 (float)
            LogGlError();
        } else {
            glDrawArrays(vRenderMode, 0, (GLsizei)m_VerticesCount);
            LogGlError();
        }

        // unbind
        glDisableVertexAttribArray(0);
        LogGlError();
        glBindVertexArray(0);
        LogGlError();
    }
}

void PointModel::SetVerticesCount(uint64_t vVerticesCount) {
    PreparePoint(vVerticesCount, true);
}

uint32_t PointModel::GetVaoID(uint32_t /*vMeshID*/) {
    return m_MeshDatas.GetVaoID();
}

uint32_t PointModel::GetVboID(uint32_t /*vMeshID*/) {
    return m_MeshDatas.GetVboID();
}

uint32_t PointModel::GetIboID(uint32_t /*vMeshID*/) {
    return m_MeshDatas.GetIboID();
}

bool PointModel::PreparePoint(uint64_t vVerticesCount, bool vUpdate) {
    GuiBackend::MakeContextCurrent(m_Window);

    TracyGpuZone("PointModel::PreparePoint");

    BaseModel::SetVerticesCount(vVerticesCount);

    m_MeshDatas.m_Vertices.clear();
    m_MeshDatas.m_Vertices.resize(GetVerticesCount());
    memset(m_MeshDatas.m_Vertices.data(), 0, m_MeshDatas.m_Vertices.size() * sizeof(float));

    for (size_t i = 0; i < GetVerticesCount(); i++) {
        m_MeshDatas.m_Vertices[i] = (float)i;
    }

    if (!vUpdate) {
        if (m_MeshDatas.m_Vbo > 0) {
            glDeleteBuffers(1, &m_MeshDatas.m_Vao);
        }
        if (m_MeshDatas.m_Vao > 0) {
            glDeleteVertexArrays(1, &m_MeshDatas.m_Vbo);
        }
        glGenVertexArrays(1, &m_MeshDatas.m_Vao);
        glGenBuffers(1, &m_MeshDatas.m_Vbo);
    }

    glBindVertexArray(m_MeshDatas.m_Vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_MeshDatas.m_Vbo);
    glBufferData(GL_ARRAY_BUFFER, m_MeshDatas.verticeSize * m_MeshDatas.m_Vertices.size(), m_MeshDatas.m_Vertices.data(), GL_STATIC_DRAW);

    if (!vUpdate) {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (void*)nullptr);
        glDisableVertexAttribArray(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}