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

#include <glad/glad.h>

#include <string>
#include <vector>
#include <map>

// this helper is an abstract class
// for use GLFW or SDL in the same way

#ifdef USE_SDL2
	struct SDL_Window;
	typedef void* SDL_GLContext;
	struct SDL_Monitor;
#else
	struct GLFWwindow;
	struct GLFWmonitor;
	struct GLFWvidmode;
#endif

#define GuiBackend_KEY_DELETE ImGuiKey_Delete
#define GuiBackend_KEY_F10 ImGuiKey_F10
#define GuiBackend_KEY_F11 ImGuiKey_F11
#define GuiBackend_KEY_F12 ImGuiKey_F12
#define GuiBackend_KEY_0 ImGuiKey_0
#define GuiBackend_KEY_KP_0 ImGuiKey_Keypad0
#define GuiBackend_MOUSE_PRESS 1
#define GuiBackend_MOUSE_RELEASE 0
#define GuiBackend_KEY_UP ImGuiKey_UpArrow
#define GuiBackend_KEY_DOWN ImGuiKey_DownArrow

struct GuiBackend_Window
{
#ifdef USE_SDL2
	SDL_Window* win = nullptr;
	SDL_GLContext context;
#else
	GLFWwindow* win = nullptr;
#endif

	void clear()
	{
		win = nullptr;
	}
};

typedef void (*GuiBackend_WindowFocusFun)(const GuiBackend_Window&, int);
typedef void (*GuiBackend_WindowPosFun)(const GuiBackend_Window&, int, int);
typedef void (*GuiBackend_WindowDropFun)(const GuiBackend_Window&, std::vector<std::string>);
typedef void (*GuiBackend_ErrorFun)(int, const char*);

struct GuiBackend_Monitor
{
#ifdef USE_SDL2
	SDL_Monitor* id = nullptr;
#else
	GLFWmonitor* id = nullptr;
#endif
	std::string name;
};

struct GuiBackend_VideoMode
{
#ifdef USE_SDL2
	int width = 0;
	int height = 0;
	int redBits = 0;
	int greenBits = 0;
	int blueBits = 0;
	int refreshRate = 0;
#else
	int width = 0;
	int height = 0;
	int redBits = 0;
	int greenBits = 0;
	int blueBits = 0;
	int refreshRate = 0;
#endif
};

class GuiBackend
{
public:
	static void MakeContextCurrent(const GuiBackend_Window& window = GuiBackend_Window());

private:
	std::vector<std::string> m_DropedFiles;
	GuiBackend_Window m_DropedWindow;
	GuiBackend_ErrorFun m_ErrorFun = nullptr;
	GuiBackend_WindowFocusFun m_WindowFocusFun = nullptr;
	GuiBackend_WindowPosFun m_WindowPosFun = nullptr;
	GuiBackend_WindowDropFun m_WindowDropFun = nullptr;
	int m_ButtonsDown[3] = { GuiBackend_MOUSE_RELEASE, GuiBackend_MOUSE_RELEASE, GuiBackend_MOUSE_RELEASE };

private:
	void CheckForError();

public:
	bool Init();
	bool IsGlversionSupported(int vMajor, int vMinor);
	void SetGlobalOpenglVersion(int vMajorVersion, int vMinorVersion);

	GuiBackend_Window GetCurrentContext();

	GuiBackend_Window CreateGuiBackendWindow_Hidden(int width, int height, const char* title, const GuiBackend_Window& share = GuiBackend_Window());
	GuiBackend_Window CreateGuiBackendWindow_Visible(int width, int height, const char* title, const GuiBackend_Window& share = GuiBackend_Window());
	void SwapInterval(int vInterval);
	
	void SwapBuffers(const GuiBackend_Window& window);

	void SetErrorCallback(GuiBackend_ErrorFun cbfun);
	void SetWindowFocusCallback(const GuiBackend_Window& window, GuiBackend_WindowFocusFun vWindowFocusCallback);
	void SetWindowPosCallback(const GuiBackend_Window& window, GuiBackend_WindowPosFun vWindowPosCallback);
	void SetDropCallback(const GuiBackend_Window& window, GuiBackend_WindowDropFun vWindowDropCallback);

	float GetHDPIPixelRatio(const GuiBackend_Window& window);
	float GetHDPIScale(const GuiBackend_Window& window);

	void SetGlobalTransparent(bool vCond);
	void SetGlobalDecorated(bool vCond);
	void SetDecorated(const GuiBackend_Window& window, bool vCond);
	void SetWindowTitle(const GuiBackend_Window& window, const char* vTitle);
	void SetWindowPos(const GuiBackend_Window& window, int w, int h);
	void SetWindowSize(const GuiBackend_Window& window, int w, int h);
	void SetClipboardString(const GuiBackend_Window& window, const std::string& string);

	void GetWindowPos(const GuiBackend_Window& window, int* w, int* h);
	void GetWindowSize(const GuiBackend_Window& window, int* w, int* h);
	void GetCursorPos(const GuiBackend_Window& window, double* xpos, double* ypos);
	int GetMouseButton(const GuiBackend_Window& window, int vButton);

	bool IsMaximized(const GuiBackend_Window& vWindow);
	void MaximizeWindow(const GuiBackend_Window& window);
	void RestoreWindow(const GuiBackend_Window& window);

	std::map<std::string, GuiBackend_Monitor> GetMonitors();
	GuiBackend_Monitor GetPrimaryMonitor();
	GuiBackend_VideoMode GetVideoMode(const GuiBackend_Monitor& vMonitor);
	void GetMonitorPos(const GuiBackend_Monitor& vMonitor, int* xpos, int* ypos);

	bool WindowShouldClose(const GuiBackend_Window& vWindow);
	void DestroyWindow(const GuiBackend_Window& vWindow);
	void Terminate();

	void SetEmbeddedIconApp(const GuiBackend_Window& vWindow, const char* vEmbeddedIconID);
	GLuint ExtractEmbeddedIconToGLTexture(const char* vEmbeddedIconID);
	GLuint ExtractEmbeddedImageToGLTexture(const char* vEmbeddedImageID);

public:
	static GuiBackend* Instance()
	{
		static GuiBackend _instance;
		return &_instance;
	}

protected:
	GuiBackend() = default; // Prevent construction
	GuiBackend(const GuiBackend&) = default; // Prevent construction by copying
	GuiBackend& operator =(const GuiBackend&) { return *this; }; // Prevent assignment
	~GuiBackend() = default; // Prevent unwanted destruction
};