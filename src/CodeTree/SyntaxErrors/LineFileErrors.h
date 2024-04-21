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

#include <string>
#include <vector>
#include <map>

class ErrorLineFragment
{
public:
	std::string file;
	size_t line = 0;
	std::string error;

public:
	ErrorLineFragment();
	ErrorLineFragment(size_t vLine, std::string vFile, std::string vErr);
};

class ErrorLine
{
public:
	std::vector<ErrorLineFragment> fragments;

public:
	ErrorLine();
	ErrorLine(std::vector<ErrorLineFragment> vFragment);
	ErrorLine(std::string vErr);
	ErrorLine(size_t vLine, std::string vFile, std::string vErr);
};

class LineFileErrors
{
public:
	std::map<size_t, std::map<std::string, ErrorLine>> errLine;

public:
	LineFileErrors();
	LineFileErrors(const size_t& vLine, const std::string& vFile, const std::string& error);
	void Set(const size_t& vLine, const std::string& vFile, const std::string& error);
	void Set(const size_t& vLine, const std::string& vFile, ErrorLine vErrorLineStruct);
};