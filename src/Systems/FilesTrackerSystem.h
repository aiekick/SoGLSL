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

#include <efsw/efsw.hpp>
#include <set>
#include <list>
#include <string>
#include <memory>

class FilesTrackerSystem : public efsw::FileWatchListener
{
public:
	bool Changes = false;
	std::set<std::string> files;

private:
	std::unique_ptr<efsw::FileWatcher> m_FilesTracker = nullptr;
	std::set<efsw::WatchID> m_WatchIDs;

public:
	static FilesTrackerSystem* Instance()
	{
		static FilesTrackerSystem _instance;
		return &_instance;
	}

protected:
	void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename, efsw::Action action, std::string oldFilename = "") override;

	FilesTrackerSystem(); // Prevent construction
	FilesTrackerSystem(const FilesTrackerSystem&) = default; // Prevent construction by copying
	FilesTrackerSystem& operator =(const FilesTrackerSystem&) { return *this; }; // Prevent assignment
	~FilesTrackerSystem(); // Prevent unwanted destruction

public:
	void addWatch(const std::string& vPath);
	void update();
};
