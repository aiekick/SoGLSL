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

#include "GuiBackend.h"

#include <ctools/Logger.h>

#ifdef USE_SDL2
	#include <SDL.h>
	#include <backends/imgui_impl_sdl.h>
#else
	#include <GLFW/glfw3.h>
    #ifdef _MSC_VER
        #define GLFW_EXPOSE_NATIVE_WIN32
        #include <GLFW/glfw3native.h>
    #endif
#endif

#ifdef USE_SDL2
void GuiBackend::CheckForError()
{
	const char* err = SDL_GetError();
	if (err && strlen(err))
		if (m_ErrorFun)
			m_ErrorFun(0, err);
}
#endif

bool GuiBackend::Init()
{
#ifdef USE_SDL2
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		LogVarLightError("Error: %s\n", SDL_GetError());
		return false;
	}
#else
	if (!glfwInit())
		return false;

#ifdef APPLE
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

#endif
	return true;
}

bool GuiBackend::IsGlversionSupported(int vMajor, int vMinor)
{
	SetGlobalOpenglVersion(vMajor, vMinor);
	const auto hnd = CreateGuiBackendWindow_Hidden(1, 1, "");
	if (hnd.win)
	{
		DestroyWindow(hnd);
		return true;
	}

	return false;
}

void GuiBackend::MakeContextCurrent(const GuiBackend_Window& window)
{
#ifdef USE_SDL2
	SDL_GL_MakeCurrent(window.win, window.context);
	CheckForError();
#else
	glfwMakeContextCurrent(window.win);
#endif
}

void GuiBackend::SetClipboardString(const GuiBackend_Window& window, const std::string& string)
{
#ifdef USE_SDL2
	CTOOL_DEBUG_BREAK;
	CheckForError();
#else
	glfwSetClipboardString(window.win, string.c_str());
#endif
}

void GuiBackend::GetCursorPos(const GuiBackend_Window& window, double* xpos, double* ypos)
{
#ifdef USE_SDL2
	if (xpos && ypos)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		CheckForError();
		*xpos = (double)x;
		*ypos = (double)y;
	}

#else
	glfwGetCursorPos(window.win, xpos, ypos);
#endif
}

GuiBackend_Window GuiBackend::GetCurrentContext()
{
	GuiBackend_Window cnt;
#ifdef USE_SDL2
	cnt.win = SDL_GL_GetCurrentWindow();
	CheckForError();
	cnt.context = SDL_GL_GetCurrentContext();
	CheckForError();
#else
	cnt.win = glfwGetCurrentContext();
#endif
	return cnt;
}

GuiBackend_Window GuiBackend::CreateGuiBackendWindow_Hidden(int width, int height, const char* title, const GuiBackend_Window& share)
{
	GuiBackend_Window res;
#ifdef USE_SDL2
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI);
	res.win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
	CheckForError();
	res.context = SDL_GL_CreateContext(res.win);
	CheckForError();
#else
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	res.win = glfwCreateWindow(width, height, title, nullptr, share.win);
#endif
	return res;
}

GuiBackend_Window GuiBackend::CreateGuiBackendWindow_Visible(int width, int height, const char* title, const GuiBackend_Window& share)
{
	GuiBackend_Window res;
#ifdef USE_SDL2
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	res.win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
	CheckForError();
	res.context = SDL_GL_CreateContext(res.win);
	CheckForError();
#else
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	res.win = glfwCreateWindow(width, height, title, nullptr, share.win);
#endif
	return res;
}

void GuiBackend::SwapInterval(int vInterval)
{
#ifdef USE_SDL2
	SDL_GL_SetSwapInterval(vInterval);
	CheckForError();
#else
	glfwSwapInterval(vInterval);
#endif
}

void GuiBackend::SetWindowFocusCallback(const GuiBackend_Window& window, GuiBackend_WindowFocusFun vWindowFocusCallback)
{
#ifdef USE_SDL2
	m_WindowFocusFun = vWindowFocusCallback;
#else
	glfwSetWindowFocusCallback(window.win, (GLFWwindowfocusfun)vWindowFocusCallback);
#endif
}

void GuiBackend::SetWindowPosCallback(const GuiBackend_Window& window, GuiBackend_WindowPosFun vWindowPosCallback)
{
#ifdef USE_SDL2
	m_WindowPosFun = vWindowPosCallback;
#else
	glfwSetWindowPosCallback(window.win, (GLFWwindowposfun)vWindowPosCallback);
#endif
}

void GuiBackend::SetDropCallback(const GuiBackend_Window& window, GuiBackend_WindowDropFun vWindowDropCallback)
{
#ifdef USE_SDL2
	m_WindowDropFun = vWindowDropCallback;
#else
	glfwSetDropCallback(window.win, (GLFWdropfun)vWindowDropCallback);
#endif
}

float GuiBackend::GetHDPIPixelRatio(const GuiBackend_Window& window)
{
#ifdef USE_SDL2
	CTOOL_DEBUG_BREAK;
	assert(0);
	return 0.0f;
#else
	ct::ivec2 bs, ws;
	glfwGetFramebufferSize(window.win, &bs.x, &bs.y);
	glfwGetWindowSize(window.win, &ws.x, &ws.y);

	if (bs.emptyOR() || ws.emptyOR())
		return 1.0f;

	return ((float)bs.maxi() / (float)ws.maxi());
#endif
}

float GuiBackend::GetHDPIScale(const GuiBackend_Window& window)
{
#ifdef USE_SDL2
	CTOOL_DEBUG_BREAK;
	assert(0);
	return 0.0f;
#else
	ct::fvec2 wcs;
	glfwGetWindowContentScale(window.win, &wcs.x, &wcs.y);
	if (wcs.emptyOR())
		return 1.0f;
	return wcs.maxi();
#endif
}


void GuiBackend::SetDecorated(const GuiBackend_Window& window, bool vCond)
{
#ifdef USE_SDL2
	SDL_SetWindowBordered(window.win, (SDL_bool)vCond);
	CheckForError();
#else
	glfwSetWindowAttrib(window.win, GLFW_DECORATED, vCond);
#endif
}

void  GuiBackend::SetWindowTitle(const GuiBackend_Window& window, const char* vTitle)
{
#ifdef USE_SDL2
	SDL_SetWindowTitle(window.win, vTitle);
	CheckForError();
#else
	glfwSetWindowTitle(window.win, vTitle);
#endif
}

void GuiBackend::SwapBuffers(const GuiBackend_Window& window)
{
#ifdef USE_SDL2
	SDL_GL_SwapWindow(window.win);
	//un check des erreurs apres un swap ne sert a rien
	//car si le swap merde, il n'y a aucune moyen d'empecher le driver opengl de crasher l'app
	//CheckForError(); 
#else
	glfwSwapBuffers(window.win);
#endif
}

bool GuiBackend::IsMaximized(const GuiBackend_Window& vWindow)
{
#ifdef USE_SDL2
	auto flags = SDL_GetWindowFlags(vWindow.win);
	CheckForError();
	if (flags & SDL_WINDOW_MAXIMIZED)
		return true;
	return false;
#else
	return (bool)glfwGetWindowAttrib(vWindow.win, GLFW_MAXIMIZED);
#endif
}

void GuiBackend::GetWindowPos(const GuiBackend_Window& window, int* w, int* h)
{
#ifdef USE_SDL2
	SDL_GetWindowPosition(window.win, w, h);
	CheckForError();
#else
	glfwGetWindowPos(window.win, w, h);
#endif
}

void GuiBackend::SetWindowPos(const GuiBackend_Window& window, int w, int h)
{
#ifdef USE_SDL2
	SDL_SetWindowPosition(window.win, w, h);
	CheckForError();
#else
	glfwSetWindowPos(window.win, w, h);
#endif
}

void GuiBackend::SetWindowSize(const GuiBackend_Window& window, int w, int h)
{
#ifdef USE_SDL2
	SDL_SetWindowSize(window.win, w, h);
	CheckForError();
#else
	glfwSetWindowSize(window.win, w, h);
#endif
}

void GuiBackend::GetWindowSize(const GuiBackend_Window& window, int* w, int* h)
{
#ifdef USE_SDL2
	SDL_GetWindowSize(window.win, w, h);
	CheckForError();
#else
	glfwGetWindowSize(window.win, w, h);
#endif
}

void GuiBackend::MaximizeWindow(const GuiBackend_Window& window)
{
#ifdef USE_SDL2
	SDL_MaximizeWindow(window.win);
	CheckForError();
#else
	glfwMaximizeWindow(window.win);
#endif
}

void GuiBackend::RestoreWindow(const GuiBackend_Window& window)
{
#ifdef USE_SDL2
	SDL_RestoreWindow(window.win);
	CheckForError();
#else
	glfwRestoreWindow(window.win);
#endif
}

int GuiBackend::GetMouseButton(const GuiBackend_Window& window, int vButton)
{
#ifdef USE_SDL2
	if (vButton < 3)
		return m_ButtonsDown[vButton];
	return GuiBackend_RELEASE;
#else
	// return GLFW_PRESS or GLFW_RELEASE
	return glfwGetMouseButton(window.win, vButton);
#endif
}

std::map<std::string, GuiBackend_Monitor> GuiBackend::GetMonitors()
{
	std::map<std::string, GuiBackend_Monitor> res;

#ifdef USE_SDL2
	CTOOL_DEBUG_BREAK;
	CheckForError();
#else
	int count;
	const auto monitors = glfwGetMonitors(&count);
	for (auto i = 0; i < count; i++)
	{
		GuiBackend_Monitor mon;
		
		mon.id = monitors[i];
		mon.name = glfwGetMonitorName(mon.id);
		if (res.find(mon.name) != res.end()) // found
		{
			mon.name += " " + ct::toStr(i);
		}
		res[mon.name] = mon;
	}
#endif

	return res;
}

GuiBackend_Monitor GuiBackend::GetPrimaryMonitor()
{
	GuiBackend_Monitor mon;

#ifdef USE_SDL2
	CTOOL_DEBUG_BREAK;
	CheckForError();
#else
	mon.id = glfwGetPrimaryMonitor();
	mon.name = glfwGetMonitorName(mon.id);
#endif

	return mon;
}

GuiBackend_VideoMode GuiBackend::GetVideoMode(const GuiBackend_Monitor& vMonitor)
{
	GuiBackend_VideoMode res;

#ifdef USE_SDL2
	CTOOL_DEBUG_BREAK;
	CheckForError();
#else
	auto mode = glfwGetVideoMode(vMonitor.id);
	if (mode)
	{
		res.width = mode->width;
		res.height = mode->height;
		res.redBits = mode->redBits;
		res.greenBits = mode->greenBits;
		res.blueBits = mode->blueBits;
	}
#endif

	return res;
}

void GuiBackend::GetMonitorPos(const GuiBackend_Monitor& vMonitor, int* xpos, int* ypos)
{
#ifdef USE_SDL2
	CTOOL_DEBUG_BREAK;
	CheckForError();
#else
	glfwGetMonitorPos(vMonitor.id, xpos, ypos);
#endif
}

void GuiBackend::SetErrorCallback(GuiBackend_ErrorFun cbfun)
{
#ifdef USE_SDL2
	m_ErrorFun = cbfun;
#else
	glfwSetErrorCallback((GLFWerrorfun)cbfun);
#endif
}

void GuiBackend::SetGlobalTransparent(bool vCond)
{
#ifdef USE_SDL2
	// pas besoin avec SDL2, toujours actif et a gerer via SDL_SetWindowOpacity(SDL_Window * window, float opacity);
#else
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, vCond);
#endif
}

void GuiBackend::SetGlobalDecorated(bool vCond)
{
#ifdef USE_SDL2
	CTOOL_DEBUG_BREAK;
	CheckForError();
#else
	glfwWindowHint(GLFW_DECORATED, vCond);
#endif
}

void GuiBackend::SetGlobalOpenglVersion(int vMajorVersion, int vMinorVersion)
{
#ifdef USE_SDL2
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, vMajorVersion);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, vMinorVersion);
	CheckForError();
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, vMajorVersion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, vMinorVersion);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
}

bool GuiBackend::WindowShouldClose(const GuiBackend_Window& vWindow)
{
	bool res = false;

#ifdef USE_SDL2
	SDL_Event event;
	if (!m_DropedFiles.empty())
	{
		m_DropedFiles.clear();
		m_DropedWindow = GuiBackend_Window();
	}
	memset(m_ButtonsDown, 0, sizeof(bool) * 3);
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);

		if (event.type == SDL_QUIT)
		{
			res = true;
		}
		else if (event.type == SDL_DROPFILE)
		{
			m_DropedFiles.push_back(event.drop.file);
			m_DropedWindow.winId = event.window.windowID;
			m_DropedWindow.win = SDL_GetWindowFromID(m_DropedWindow.winId);
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (event.button.button == SDL_BUTTON_LEFT) { m_ButtonsDown[0] = true; }
			else if (event.button.button == SDL_BUTTON_RIGHT) { m_ButtonsDown[1] = true; }
			else if (event.button.button == SDL_BUTTON_MIDDLE) { m_ButtonsDown[2] = true; }
		}
		else if (event.type == SDL_MOUSEBUTTONUP)
		{
			
		}
		else if (event.type == SDL_WINDOWEVENT)
		{
			if (event.window.event == SDL_WINDOWEVENT_CLOSE && 
				event.window.windowID == SDL_GetWindowID(vWindow.win))
			{
				res = true;
			}
			else
			{
				switch (event.window.event) 
				{
				case SDL_WINDOWEVENT_SHOWN:
					LogVarDebug("Window %d shown", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_HIDDEN:
					LogVarDebug("Window %d hidden", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_EXPOSED:
					LogVarDebug("Window %d exposed", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_MOVED:
					LogVarDebug("Window %d moved to %d,%d",
						event.window.windowID, event.window.data1,
						event.window.data2);
					break;
				case SDL_WINDOWEVENT_RESIZED:
					LogVarDebug("Window %d resized to %dx%d",
						event.window.windowID, event.window.data1,
						event.window.data2);
					break;
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					LogVarDebug("Window %d size changed to %dx%d",
						event.window.windowID, event.window.data1,
						event.window.data2);
					break;
				case SDL_WINDOWEVENT_MINIMIZED:
					LogVarDebug("Window %d minimized", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_MAXIMIZED:
					LogVarDebug("Window %d maximized", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_RESTORED:
					LogVarDebug("Window %d restored", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_ENTER:
					LogVarDebug("Mouse entered window %d",
						event.window.windowID);
					break;
				case SDL_WINDOWEVENT_LEAVE:
					LogVarDebug("Mouse left window %d", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					if (m_WindowFocusFun)
					{
						GuiBackend_Window win;
						win.win = SDL_GetWindowFromID(event.window.windowID);
						m_WindowFocusFun(win, 1); // 1 mean Focused
					}	
					LogVarDebug("Window %d gained keyboard focus",
						event.window.windowID);
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					if (m_WindowFocusFun)
					{
						GuiBackend_Window win;
						win.win = SDL_GetWindowFromID(event.window.windowID);
						m_WindowFocusFun(win, 0); // 0 mean not Focused
					}
					LogVarDebug("Window %d lost keyboard focus",
						event.window.windowID);
					break;
				case SDL_WINDOWEVENT_CLOSE:
					LogVarDebug("Window %d closed", event.window.windowID);
					break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
				case SDL_WINDOWEVENT_TAKE_FOCUS:
					LogVarDebug("Window %d is offered a focus", event.window.windowID);
					break;
				case SDL_WINDOWEVENT_HIT_TEST:
					LogVarDebug("Window %d has a special hit test", event.window.windowID);
					break;
#endif
				default:
					LogVarDebug("Window %d got unknown event %d",
						event.window.windowID, event.window.event);
					break;
				}
			}
		}
		CheckForError();
	}
	if (!m_DropedFiles.empty())
	{
		if (m_WindowDropFun)
			m_WindowDropFun(m_DropedWindow, m_DropedFiles);
	}
	MakeContextCurrent(vWindow);
	return res;
#else
	res = glfwWindowShouldClose(vWindow.win);

	if (!res)
	{
		glfwPollEvents();
	}
#endif

	return res;
}

void GuiBackend::DestroyWindow(const GuiBackend_Window& vWindow)
{
#ifdef USE_SDL2
	SDL_GL_DeleteContext(vWindow.context);
	CheckForError();
	SDL_DestroyWindow(vWindow.win);
	CheckForError();
#else
	glfwDestroyWindow(vWindow.win);
#endif
}

void GuiBackend::Terminate()
{
#ifdef USE_SDL2
	SDL_Quit();
	CheckForError();
#else
	glfwTerminate();
#endif
}

void GuiBackend::SetEmbeddedIconApp(const GuiBackend_Window& vWindow, const char* vEmbeddedIconID)
{
#if WIN32
	auto icon_h_inst = LoadIconA(GetModuleHandle(NULL), vEmbeddedIconID);
#ifdef USE_SDL2
	
#else
	SetClassLongPtrA(glfwGetWin32Window(vWindow.win), GCLP_HICON, (LONG_PTR)icon_h_inst);
#endif
#endif
}

GLuint GuiBackend::ExtractEmbeddedIconToGLTexture(const char* vEmbeddedIconID)
{
#if WIN32
	// embedded icon to opengl texture

	auto icon_h_inst = LoadIconA(GetModuleHandle(NULL), vEmbeddedIconID);

	ICONINFO app_icon_info;
	if (GetIconInfo(icon_h_inst, &app_icon_info))
	{
		HDC hdc = GetDC(0);

		BITMAPINFO MyBMInfo = { 0 };
		MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);
		if (GetDIBits(hdc, app_icon_info.hbmColor, 0, 0, NULL, &MyBMInfo, DIB_RGB_COLORS))
		{
			uint8_t* bytes = new uint8_t[MyBMInfo.bmiHeader.biSizeImage];

			MyBMInfo.bmiHeader.biCompression = BI_RGB;
			if (GetDIBits(hdc, app_icon_info.hbmColor, 0, MyBMInfo.bmiHeader.biHeight, (LPVOID)bytes, &MyBMInfo, DIB_RGB_COLORS))
			{
				uint8_t R, G, B;

				int index, i;

				// swap BGR to RGB
				for (i = 0; i < MyBMInfo.bmiHeader.biWidth * MyBMInfo.bmiHeader.biHeight; i++)
				{
					index = i * 4;

					B = bytes[index];
					G = bytes[index + 1];
					R = bytes[index + 2];

					bytes[index] = R;
					bytes[index + 1] = G;
					bytes[index + 2] = B;
				}

				//create texture from loaded bmp image 

				glEnable(GL_TEXTURE_2D);

				GLuint texID = 0;
				glGenTextures(1, &texID);

				glBindTexture(GL_TEXTURE_2D, texID);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTexImage2D(GL_TEXTURE_2D, 0, 4, MyBMInfo.bmiHeader.biWidth, MyBMInfo.bmiHeader.biHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
				glFinish();

				glBindTexture(GL_TEXTURE_2D, 0);

				glDisable(GL_TEXTURE_2D);

				return texID;
			}
		}
	}
#endif

	return 0;
}

GLuint GuiBackend::ExtractEmbeddedImageToGLTexture(const char* vEmbeddedImageID)
{
#if WIN32
	// embedded icon to opengl texture

	auto hBitmap = LoadImageA(GetModuleHandle(NULL), vEmbeddedImageID, IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE | LR_CREATEDIBSECTION);

	BITMAP bm;
	if (GetObjectA(hBitmap, sizeof(BITMAP), &bm))
	{
		uint8_t* bytes = (uint8_t*)bm.bmBits;
		uint8_t tmp_blue_to_red = 0U;

		int index = 0;

		// swap BGR to RGB
		for (int i = 0; i < bm.bmWidth * bm.bmHeight; ++i)
		{
			index = i * 3;

			tmp_blue_to_red = bytes[index];
			bytes[index] = bytes[index + 2];
			bytes[index + 2] = tmp_blue_to_red;
		}

		//create texture from loaded bmp image 

		glEnable(GL_TEXTURE_2D);

		GLuint texID = 0;
		glGenTextures(1, &texID);

		glBindTexture(GL_TEXTURE_2D, texID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bm.bmWidth, bm.bmHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bytes);
		glFinish();

		glBindTexture(GL_TEXTURE_2D, 0);

		glDisable(GL_TEXTURE_2D);

		return texID;
	}
#endif

	return 0;
}
