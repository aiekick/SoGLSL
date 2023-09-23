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

#ifdef USE_VR

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <thread>
#include <mutex>
#include <set>
#include <string>
#include <vector>
#include <list>
using namespace std;

class CodeTree;
class VRGui
{
private:
	std::weak_ptr<CodeTree> m_CodeTreeWeak;

public:
	void Draw();
	void SetCodeTree(std::weak_ptr<CodeTree> vCodeTreeWeak);

public:
	static VRGui* Instance()
	{
		static VRGui _instance;
		return &_instance;
	}

protected:
	VRGui(); // Prevent construction
	VRGui(const VRGui&) {}; // Prevent construction by copying
	VRGui& operator =(const VRGui&) { return *this; }; // Prevent assignment
	~VRGui(); // Prevent unwanted destruction
};

#endif