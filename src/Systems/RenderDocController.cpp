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

#include <Systems/RenderDocController.h>
#ifdef WIN32
    #include <Windows.h>
#elif defined(__linux__)
    #include <dlfcn.h>
#endif
#include <cstdio>

bool RenderDocController::Init()
{
    m_GetApiPtr = nullptr;

#if defined(WIN32)
    HMODULE mod = GetModuleHandleA("renderdoc.dll");
    if (mod)
    {
        m_GetApiPtr = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
    }
#elif defined(__linux__)
    void* mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD);
    if (mod)
    {
        m_GetApiPtr = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
    }
#else
// Apple is not supported by renderdoc
#endif

    if (m_GetApiPtr)
    {
        int ret = m_GetApiPtr(eRENDERDOC_API_Version_1_0_0, (void**)&m_RDdocPtr);
        if (ret != 1)
        {
            m_RDdocPtr = nullptr;
        }
        else
        {
            int major, minor, patch;
            m_RDdocPtr->GetAPIVersion(&major, &minor, &patch);
            printf("-----------\n");
            printf("Renderdoc DLL Loaded\n");
            printf("Renderdoc Api Version : %i.%i.%i\n", major, minor, patch);
            printf("-----------\n");
        }
    }

    return true;
}

void RenderDocController::Unit()
{
    m_RDdocPtr = nullptr;
    m_GetApiPtr = nullptr;
}

void RenderDocController::RequestCapture()
{
    m_Capture_Requested = true;
}

void RenderDocController::StartCaptureIfResquested()
{
    if (!m_Capture_Requested) return;

    // we do that because when we call it via imgui
    // we catch the EndCapture before the StartCapture..
    m_Capture_Requested = false;
    m_Capture_Started = true;

    // To start a frame capture, call StartFrameCapture.
    // You can specify NULL, NULL for the device to capture on if you have only one device and
    // either no windows at all or only one window, and it will capture from that device.
    // See the documentation below for a longer explanation
    if (m_RDdocPtr)
    {
        printf("Renderdoc Capture Started\n");
        m_RDdocPtr->StartFrameCapture(NULL, NULL);
    }
}

void RenderDocController::EndCaptureIfResquested()
{
    if (!m_Capture_Started) return;

    // stop the capture
    if (m_RDdocPtr)
    {
        m_RDdocPtr->EndFrameCapture(NULL, NULL);
        printf("Renderdoc Capture Ended\n");
    }

    m_Capture_Started = false;
}