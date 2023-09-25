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

#include <Systems/RenderDocApp.h>

class RenderDocController
{
private:
	RENDERDOC_API_1_0_0* m_RDdocPtr = nullptr;
	pRENDERDOC_GetAPI m_GetApiPtr = nullptr;

	bool m_Capture_Requested = false;
	bool m_Capture_Started = false;

public:
	bool Init();
	void Unit();

	void RequestCapture();

	void StartCaptureIfResquested();
	void EndCaptureIfResquested();

public:
	static RenderDocController* Instance(RenderDocController* vCopy = nullptr, bool vForce = false)
	{
		static RenderDocController _instance;
		static RenderDocController* _instance_copy = nullptr;
		if (vCopy || vForce)
			_instance_copy = vCopy;
		if (_instance_copy)
			return _instance_copy;
		return &_instance;
	}
protected:
	RenderDocController() = default; // Prevent construction
	RenderDocController(const RenderDocController&) = default; // Prevent construction by copying
	RenderDocController& operator =(const RenderDocController&) { return *this; }; // Prevent assignment
	~RenderDocController() = default; // Prevent unwanted destruction
};