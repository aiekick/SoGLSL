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

#include "FilesTrackerSystem.h"
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <Headers/RenderPackHeaders.h>

FilesTrackerSystem::FilesTrackerSystem() {
    // Create the file system watcher instance
    // efsw::FileWatcher allow a first boolean parameter that indicates if it should start with the generic file watcher instead of the platform
    // specific backend
    m_FilesTracker = std::make_unique<efsw::FileWatcher>();
}

FilesTrackerSystem::~FilesTrackerSystem() {
    for (auto wid : m_WatchIDs)
        m_FilesTracker->removeWatch(wid);
    m_FilesTracker.reset();
}

void FilesTrackerSystem::addWatch(const std::string& vPath) {
    // Add a folder to watch, and get the efsw::WatchID
    // It will watch the /tmp folder recursively ( the third parameter indicates that is recursive )
    // Reporting the files and directories changes to the instance of the listener
    m_WatchIDs.emplace(m_FilesTracker->addWatch(vPath, this, false));
    LogVarDebugInfo("Watch for change in directory : %s", vPath.c_str());
}

void FilesTrackerSystem::update() {
    m_FilesTracker->watch();
}

void FilesTrackerSystem::handleFileAction(efsw::WatchID vWatchid, const std::string& vDir, const std::string& vFileName, efsw::Action vAction, std::string vOldFilename) {
    UNUSED(vWatchid);
    switch (vAction) {
        case efsw::Actions::Modified: {
            LogVarDebugInfo("DIR (%s) FILE (%s) has been Modified", vDir.c_str(), vFileName.c_str());
            auto ps = FileHelper::Instance()->ParsePathFileName(vDir + vFileName);
            if (ps.isOk) {
                files.emplace(ps.GetFPNE());
                Changes = true;
            }
            break;
        }
        case efsw::Actions::Add:
        case efsw::Actions::Delete:
        case efsw::Actions::Moved:
        default: break;
    }
}