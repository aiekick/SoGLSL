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

#include "ExportBuffer.h"

#include <Gui/CustomGuiWidgets.h>
#include <Renderer/RenderPack.h>
#include <Mesh/Operations/MeshSaver.h>
#include <ctools/Logger.h>

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
static char _bufferCount[50 + 1] = "\0";
ExportBuffer::ExportBuffer() {
    snprintf(_bufferCount, 50, "%zu", puMaxCountPoints);
}

ExportBuffer::~ExportBuffer() {
    Clear();
}

void ExportBuffer::DrawImGui(const GuiBackend_Window& vWin, BaseModelWeak vModel) {
    if (!vModel.expired())
        return;

    if (ImGui::InputText("Count", _bufferCount, 19)) {
        puMaxCountPoints = ct::ivariant(std::string(_bufferCount)).GetU();
        InitBuffer(vModel);
    }

    if (ImGui::ContrastedButton("Capture")) {
        StartCapture();
    }

    if (ImGui::ContrastedButton("Get Shader Code",
                                "Give Attributes export declaration code in Clipborad\n\
you must put it in vertex / geometry or tesselation eval section\n\
you must fill each channel with corresponding values\n")) {
        const std::string code =
            u8R"(
layout(xfb_buffer = 0, xfb_stride = 48) out Data
{
	layout(xfb_offset = 0) vec3 pos;
	layout(xfb_offset = 12) vec3 nor;
	layout(xfb_offset = 24) vec2 uv;
	layout(xfb_offset = 32) vec4 col;
} datas;)";
#ifdef USE_SDL2
        CTOOL_DEBUG_BREAK;
#else
        FileHelper::Instance()->SaveInClipBoard(vWin.win, code);
#endif
    }

    if (puCapturedCountPoints > 0) {
        MeshSaver::Instance()->DrawImGuiProgress(150.0f);

        if (!MeshSaver::Instance()->IsJoinable()) {
            ImGui::Text("Success => %zu", puCapturedCountPoints);

            if (ImGui::ContrastedButton("Export")) {
                ExportToFile();
            }
        }
    }
}

void ExportBuffer::Clear() {
    if (tbo) {
        glDeleteBuffers(1, &tbo);
        LogGlError();
    }

    if (fdbo) {
        glDeleteTransformFeedbacks(1, &fdbo);
        LogGlError();
    }

    if (queryTBO) {
        glDeleteQueries(1, &queryTBO);
        LogGlError();
    }
}

void ExportBuffer::InitBuffer(BaseModelWeak vModel) {
    auto modelPtr = vModel.lock();
    if (modelPtr) {
        Clear();

        glGenBuffers(1, &tbo);
        LogGlError();
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tbo);
        LogGlError();

        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, puMaxCountPoints * sizeof(VertexStruct::P3_N3_T2_C4), nullptr, GL_STATIC_READ);
        LogGlError();

        // We create our transform feedback object. We then bind it and
        // tell it to store its output into outputVBO.
        glGenTransformFeedbacks(1, &fdbo);
        LogGlError();
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fdbo);
        LogGlError();
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);
        LogGlError();
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        LogGlError();

        // We also create a query object. This object will be used to
        // query how many points that were stored in the output VBO.
        glGenQueries(1, &queryTBO);
        LogGlError();
    }
}

void ExportBuffer::StartCapture() {
    puNeedCapture = true;
}

void ExportBuffer::BeforeCapture() {
}

void ExportBuffer::AfterCapture(const GLuint& vRenderMode) {
    glFlush();
    LogGlError();

    puNeedCapture = false;

    puCapturedCountPoints = GetCountPoints();

    if (vRenderMode == GL_TRIANGLES || vRenderMode == GL_PATCHES) {
        puCapturedCountPoints *= 3;
    }
}

size_t ExportBuffer::GetCountPoints() {
#if defined(VERSION_X32)
    GLuint _count;
    glGetQueryObjectuiv(queryTBO, GL_QUERY_RESULT, &_count);
#elif defined(VERSION_X64)
    GLuint64 _count;
    glGetQueryObjectui64v(queryTBO, GL_QUERY_RESULT, &_count);
#endif
    LogGlError();

    return (size_t)_count;
}

void ExportBuffer::Capture(BaseModelWeak vModel, const GLuint& vRenderMode) {
    if (!vModel.expired()) {
        return;
    }

    auto modelPtr = vModel.lock();
    if (modelPtr != nullptr) {
        // bind
        glBindVertexArray(modelPtr->GetVaoID());  // select first VAO
        LogGlError();
        glEnableVertexAttribArray(0);  // pos
        LogGlError();
        glEnableVertexAttribArray(1);  // nor
        LogGlError();
        glEnableVertexAttribArray(2);  // uv
        LogGlError();
        glEnableVertexAttribArray(3);  // col
        LogGlError();

        if (modelPtr->GetPatchVerticesCount() && glPatchParameteri) {
            glPatchParameteri(GL_PATCH_VERTICES, (GLint)modelPtr->GetPatchVerticesCount());
            LogGlError();
        }
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fdbo);
        LogGlError();
        glEnable(GL_RASTERIZER_DISCARD);
        LogGlError();
        glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, queryTBO);
        LogGlError();
        if (vRenderMode == GL_PATCHES) {
            glBeginTransformFeedback(GL_TRIANGLES);
            LogGlError();
        } else {
            glBeginTransformFeedback(vRenderMode);
            LogGlError();
        }
        {
            if (modelPtr->GetInstancesCount()) {
                glDrawElementsInstanced(vRenderMode, (GLsizei)modelPtr->GetIndicesCountToShow(), GL_UNSIGNED_INT, nullptr, (GLsizei)modelPtr->GetInstancesCount());
                LogGlError();
            } else {
                glDrawElements(vRenderMode, (GLsizei)modelPtr->GetIndicesCountToShow(), GL_UNSIGNED_INT, nullptr);
                LogGlError();
            }
        }

        glEndTransformFeedback();
        LogGlError();
        glDisable(GL_RASTERIZER_DISCARD);
        LogGlError();
        glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
        LogGlError();
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
        LogGlError();
        glDisableVertexAttribArray(3);  // col
        LogGlError();
        glDisableVertexAttribArray(2);  // uv
        LogGlError();
        glDisableVertexAttribArray(1);  // nor
        LogGlError();
        glDisableVertexAttribArray(0);  // pos
        LogGlError();
        glBindVertexArray(0);
        LogGlError();
    }
}

void ExportBuffer::ExportToFile() {
    if (puCapturedCountPoints > 0) {
        VertexStruct::P3_N3_T2_C4* arr = new VertexStruct::P3_N3_T2_C4[puCapturedCountPoints];
        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, puCapturedCountPoints * sizeof(VertexStruct::P3_N3_T2_C4), arr);
        LogGlError();

        MeshSaver::Instance()->vertices.clear();
        MeshSaver::Instance()->vertices.assign(arr, arr + puCapturedCountPoints);

        SAFE_DELETE_ARRAY(arr);

        MeshSaver::Instance()->indices.clear();

        MeshSaver::Instance()->OpenDialog();
    }
}