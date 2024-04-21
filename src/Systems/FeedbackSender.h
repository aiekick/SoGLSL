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

#ifdef USE_NETWORK
#include <ctools/ConfigAbstract.h>

#include <unordered_map>
#include <list>
#include <string>

class FeedbackSender : public conf::ConfigAbstract
{
public:
	// category, widget type, widget name, doc
	std::unordered_map<std::string, std::list<std::string>> puRecentFiles;

public:
	FeedbackSender();
	~FeedbackSender();

	void SendFeedbackMail();
	void SendIssueMail();
	void SendCongratMail();

	///////////////////////////////////////////////////////
	//// CONFIGURATION ////////////////////////////////////
	///////////////////////////////////////////////////////

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
};
#endif // #ifdef USE_NETWORK
